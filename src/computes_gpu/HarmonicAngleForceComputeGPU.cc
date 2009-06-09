/*
Highly Optimized Object-Oriented Molecular Dynamics (HOOMD) Open
Source Software License
Copyright (c) 2008 Ames Laboratory Iowa State University
All rights reserved.

Redistribution and use of HOOMD, in source and binary forms, with or
without modification, are permitted, provided that the following
conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names HOOMD's
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND
CONTRIBUTORS ``AS IS''  AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS  BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

// $Id$
// $URL$
// Maintainer: dnlebard

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif

#include "HarmonicAngleForceComputeGPU.h"

#include <boost/python.hpp>
using namespace boost::python;

#include <boost/bind.hpp>
using namespace boost;

using namespace std;

/*! \param pdata ParticleData to compute angle forces on
*/
HarmonicAngleForceComputeGPU::HarmonicAngleForceComputeGPU(boost::shared_ptr<ParticleData> pdata)
	: HarmonicAngleForceCompute(pdata)
	{
	// can't run on the GPU if there aren't any GPUs in the execution configuration
	if (exec_conf.gpu.size() == 0)
		{
		cerr << endl << "***Error! Creating a AngleForceComputeGPU with no GPU in the execution configuration" << endl << endl;
		throw std::runtime_error("Error initializing AngleForceComputeGPU");
		}
		
	// default block size is the highest performance in testing on different hardware
	// choose based on compute capability of the device
	cudaDeviceProp deviceProp;
	int dev;
	exec_conf.gpu[0]->call(bind(cudaGetDevice, &dev));
	exec_conf.gpu[0]->call(bind(cudaGetDeviceProperties, &deviceProp, dev));
	if (deviceProp.major == 1 && deviceProp.minor == 0)
		m_block_size = 64;
	else if (deviceProp.major == 1 && deviceProp.minor == 1)
		m_block_size = 64;
	else if (deviceProp.major == 1 && deviceProp.minor < 4)
		m_block_size = 288;
	else
		{
		cout << "***Warning! Unknown compute " << deviceProp.major << "." << deviceProp.minor << " when tuning block size for HarmonicAngleForceComputeGPU" << endl;
		m_block_size = 64;
		}
	
	// allocate and zero device memory
	m_gpu_params.resize(exec_conf.gpu.size());
	exec_conf.tagAll(__FILE__, __LINE__);
	for (unsigned int cur_gpu = 0; cur_gpu < exec_conf.gpu.size(); cur_gpu++)
		{
		exec_conf.gpu[cur_gpu]->call(bind(cudaMalloc, (void**)((void*)&m_gpu_params[cur_gpu]), m_angle_data->getNAngleTypes()*sizeof(float2)));
		exec_conf.gpu[cur_gpu]->call(bind(cudaMemset, (void*)m_gpu_params[cur_gpu], 0, m_angle_data->getNAngleTypes()*sizeof(float2)));
		}
	
	m_host_params = new float2[m_angle_data->getNAngleTypes()];
	memset(m_host_params, 0, m_angle_data->getNAngleTypes()*sizeof(float2));
	}
	
HarmonicAngleForceComputeGPU::~HarmonicAngleForceComputeGPU()
	{
	// free memory on the GPU
	exec_conf.tagAll(__FILE__, __LINE__);
	for (unsigned int cur_gpu = 0; cur_gpu < exec_conf.gpu.size(); cur_gpu++)
		{	
		exec_conf.gpu[cur_gpu]->call(bind(cudaFree, (void*)m_gpu_params[cur_gpu]));
		m_gpu_params[cur_gpu] = NULL;
		}
	
	// free memory on the CPU
	delete[] m_host_params;
	m_host_params = NULL;
	}

/*! \param type Type of the angle to set parameters for
	\param K Stiffness parameter for the force computation
	\param t_0 Equilibrium angle (in radians) for the force computation
	
	Sets parameters for the potential of a particular angle type and updates the 
	parameters on the GPU.
*/
void HarmonicAngleForceComputeGPU::setParams(unsigned int type, Scalar K, Scalar t_0)
	{
	HarmonicAngleForceCompute::setParams(type, K, t_0);
	
	// update the local copy of the memory
	m_host_params[type] = make_float2(K, t_0);
	
	// copy the parameters to the GPU
	exec_conf.tagAll(__FILE__, __LINE__);
	for (unsigned int cur_gpu = 0; cur_gpu < exec_conf.gpu.size(); cur_gpu++)
		exec_conf.gpu[cur_gpu]->call(bind(cudaMemcpy, m_gpu_params[cur_gpu], m_host_params, m_angle_data->getNAngleTypes()*sizeof(float2), cudaMemcpyHostToDevice));
	}

/*! Internal method for computing the forces on the GPU. 
	\post The force data on the GPU is written with the calculated forces
	
	\param timestep Current time step of the simulation
	
	Calls gpu_compute_harmonic_angle_forces to do the dirty work.
*/
void HarmonicAngleForceComputeGPU::computeForces(unsigned int timestep)
	{
	// start the profile
	//if (m_prof) m_prof->push(exec_conf, "Angle");
		
	vector<gpu_angletable_array>& gpu_angletable = m_angle_data->acquireGPU();
	
	// the angle table is up to date: we are good to go. Call the kernel
	vector<gpu_pdata_arrays>& pdata = m_pdata->acquireReadOnlyGPU();
	gpu_boxsize box = m_pdata->getBoxGPU();
	
	// run the kernel in parallel on all GPUs
	exec_conf.tagAll(__FILE__, __LINE__);
	for (unsigned int cur_gpu = 0; cur_gpu < exec_conf.gpu.size(); cur_gpu++)
		exec_conf.gpu[cur_gpu]->callAsync(bind(gpu_compute_harmonic_angle_forces, m_gpu_forces[cur_gpu].d_data, pdata[cur_gpu], box, gpu_angletable[cur_gpu], m_gpu_params[cur_gpu], m_angle_data->getNAngleTypes(), m_block_size));
	exec_conf.syncAll();
		
	// the force data is now only up to date on the gpu
	m_data_location = gpu;
	
	m_pdata->release();
	
	// UNCOMMENT BELOW FOR SOME KIND OF PERFORMANCE CHECK... but first, count all the flops + memory transfers
	//int64_t mem_transfer = m_pdata->getN() * 4+16+20 + m_angle_data->getNumAngles() * 2 * (8+16+8);
	//int64_t flops = m_angle_data->getNumAngles() * 2 * (3+12+16+3+7);
	//if (m_prof)	m_prof->pop(exec_conf, flops, mem_transfer);
	}

void export_HarmonicAngleForceComputeGPU()
	{
	class_<HarmonicAngleForceComputeGPU, boost::shared_ptr<HarmonicAngleForceComputeGPU>, bases<HarmonicAngleForceCompute>, boost::noncopyable >
		("HarmonicAngleForceComputeGPU", init< boost::shared_ptr<ParticleData> >())
		.def("setBlockSize", &HarmonicAngleForceComputeGPU::setBlockSize)
		;
	}
