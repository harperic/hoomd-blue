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

#include "ForceCompute.h"
#include <iostream>
using namespace std;

#ifdef USE_PYTHON
#include <boost/python.hpp>
using namespace boost::python;
#endif
#include <boost/shared_ptr.hpp>
using namespace boost;

#ifdef USE_CUDA
#include "gpu_utils.h"
#endif

/*! \post \c fx, \c fy, \c fz are all set to NULL
*/
ForceDataArrays::ForceDataArrays() : fx(NULL), fy(NULL), fz(NULL)
	{
	}
	
/*! \param pdata ParticleData to compute forces on
	\post The Compute is initialized and all memory needed for the forces is allocated
	\post \c fx, \c fy, \c fz pointers in m_arrays are set
	\post All forces are initialized to 0
*/
ForceCompute::ForceCompute(boost::shared_ptr<ParticleData> pdata) : Compute(pdata)
	{
	assert(pdata);
	assert(pdata->getN());
				
	// allocate the memory here in the same way as with the ParticleData: put all 3
	// arrays back to back. 256-byte align the data so that uninterleaved <-> interleaved
	// translation can be done easily
	#ifdef USE_CUDA
	// start by adding up the number of bytes needed for the Scalar arrays, rounding up by 256
	unsigned int single_xarray_bytes = sizeof(Scalar) * pdata->getN();
	if ((single_xarray_bytes & 255) != 0)
		single_xarray_bytes += 256 - (single_xarray_bytes & 255);
	
	// total all bytes from scalar arrays
	m_nbytes = single_xarray_bytes * 3;
	
	CUDA_SAFE_CALL( cudaMallocHost( (void **)((void *)&m_data), m_nbytes) );	
	#else
	// start by adding up the number of bytes needed for the Scalar arrays
	unsigned int single_xarray_bytes = sizeof(Scalar) * pdata->getN();
	
	// total all bytes from scalar arrays
	m_nbytes = single_xarray_bytes * 3;
	m_data = (Scalar *)malloc(m_nbytes);
	#endif

	assert(m_data);

	// Now that m_data is allocated, we need to play some pointer games to assign
	// the x,y,z, etc... pointers
	char *cur_byte = (char *)m_data;
	m_arrays.fx = m_fx = (Scalar *)cur_byte;  cur_byte += single_xarray_bytes;
	m_arrays.fy = m_fy = (Scalar *)cur_byte;  cur_byte += single_xarray_bytes;
	m_arrays.fz = m_fz = (Scalar *)cur_byte;  cur_byte += single_xarray_bytes;
	
	// should be good to go now
	assert(m_fx);
	assert(m_fy);
	assert(m_fz);
	assert(m_arrays.fx);
	assert(m_arrays.fy);
	assert(m_arrays.fz);
	
	// zero the data
	memset((void*)m_data, 0, m_nbytes); 
	
	#ifdef USE_CUDA
	// allocate device memory for the forces and staging memory
	m_uninterleave_pitch = single_xarray_bytes/4;
	m_single_xarray_bytes = single_xarray_bytes;

	CUDA_SAFE_CALL( cudaMalloc( (void **)((void *)&m_d_forces), single_xarray_bytes*4) );
	CUDA_SAFE_CALL( cudaMalloc( (void **)((void *)&m_d_staging), single_xarray_bytes*4) );

	deviceToHostCopy();
	m_data_location = cpugpu;
	#endif
	}	
	
/*! Frees allocated memory
*/
ForceCompute::~ForceCompute()
	{
	assert(m_data);
		
	// free the data, which needs to be condtionally compiled for CUDA and non CUDA builds
	#ifdef USE_CUDA
	CUDA_SAFE_CALL( cudaFreeHost(m_data) );
	#else
	free(m_data);
	#endif
	
	m_data = NULL;
	m_arrays.fx = m_fx = NULL;
	m_arrays.fy = m_fy = NULL;
	m_arrays.fz = m_fz = NULL;
	
	#ifdef USE_CUDA
	CUDA_SAFE_CALL( cudaFree(m_d_forces) );
	CUDA_SAFE_CALL( cudaFree(m_d_staging) );
	#endif
	}
	
/*! Access the computed forces on the CPU, this may require copying data from the GPU
 	\returns Structure of arrays of the x,y,and z components of the forces on each particle
 			calculated by the last call to compute()
 	\note These are const pointers so the caller cannot muss with the data
 */
const ForceDataArrays& ForceCompute::acquire()
	{
	#ifdef USE_CUDA

	// this is the complicated graphics card version, need to do some work
	// switch based on the current location of the data
	switch (m_data_location)
		{
		case cpu:
			// if the data is solely on the cpu, life is easy, return the data arrays
			// and stay in the same state
			return m_arrays;
			break;
		case cpugpu:
			// if the data is up to date on both the cpu and gpu, life is easy, return
			// the data arrays and stay in the same state
			return m_arrays;
			break;
		case gpu:
			// if the data resides on the gpu, it needs to be copied back to the cpu
			// this changes to the cpugpu state since the data is now fully up to date on 
			// both
			deviceToHostCopy();
			m_data_location = cpugpu;
			return m_arrays;
			break;
		default:
			// anything other than the above is an undefined state!
			assert(false);
			return m_arrays;	
			break;
        }

    #else

	return m_arrays;
	#endif
	}

#ifdef USE_CUDA
/*! Access computed forces on the GPU. This may require copying data from the CPU if the forces
	were computed there.
	\returns Data pointer to the forces on the GPU

	\note For performance reasons, the returned pointer will \b not change
	from call to call. The call still must be made, however, to ensure that
	the data has been copied to the GPU.
*/
float4 *ForceCompute::acquireGPU()
    {
	// this is the complicated graphics card version, need to do some work
	// switch based on the current location of the data
	switch (m_data_location)
		{
		case cpu:
			// if the data is on the cpu, we need to copy it over to the gpu
			hostToDeviceCopy();
			// now we are in the cpugpu state
			m_data_location = cpugpu;
			return m_d_forces;
			break;
		case cpugpu:
			// if the data is up to date on both the cpu and gpu, life is easy
			// state remains the same, and return it
			return m_d_forces;
			break;
		case gpu:
			// if the data resides on the gpu, life is easy
			// state remains the same, and return it     
			return m_d_forces;
			break;
		default:
			// anything other than the above is an undefined state!
			assert(false);
			return m_d_forces;	
			break;		
		}
	}

/*! The data copy is performed efficiently by transferring data as it is on the CPU
	(256 byte aligned arrays) and then interleaving it on the GPU
*/
void ForceCompute::hostToDeviceCopy()
	{
	if (m_prof) m_prof->push("ForceCompute - CPU->GPU");
	
	// copy force data to the staging area
	CUDA_SAFE_CALL( cudaMemcpy(m_d_staging, m_data, m_single_xarray_bytes*3, cudaMemcpyHostToDevice) );
	// interleave the data
	CUDA_SAFE_CALL( gpu_interleave_float4(m_d_forces, m_d_staging, m_pdata->getN(), m_uninterleave_pitch) );	
	
	if (m_prof)
		{
		cudaThreadSynchronize(); 
		m_prof->pop(0, m_single_xarray_bytes*3);
		}
	}

/*! \sa hostToDeviceCopy()
*/
void ForceCompute::deviceToHostCopy()
	{
	if (m_prof) m_prof->push("ForceCompute - GPU->GPU");

	// uninterleave the data
	CUDA_SAFE_CALL( gpu_uninterleave_float4(m_d_staging, m_d_forces, m_pdata->getN(), m_uninterleave_pitch) );
	// copy force data from the staging area
	CUDA_SAFE_CALL( cudaMemcpy(m_data, m_d_staging, m_single_xarray_bytes*3, cudaMemcpyDeviceToHost) );
	
	if (m_prof)
		{
		cudaThreadSynchronize(); 
		m_prof->pop(0, m_single_xarray_bytes*3);
		}
	}

#endif
	
/*! Performs the force computation.
	\param timestep Current Timestep
	\note If compute() has previously been called with a value of timestep equal to
		the current value, the forces are assumed to already have been computed and nothing will 
		be done
*/
void ForceCompute::compute(unsigned int timestep)
	{
	// skip if we shouldn't compute this step
	if (!shouldCompute(timestep))
		return;
		
	computeForces(timestep);
	}
	
#ifdef USE_PYTHON

//! Wrapper class for wrapping pure virtual methodos of ForceCompute in python
class ForceComputeWrap : public ForceCompute, public wrapper<ForceCompute>
	{
	public:
		//! Constructor
		/*! \param pdata Particle data passed to the base class */
		ForceComputeWrap(shared_ptr<ParticleData> pdata) : ForceCompute(pdata) { }
	protected:
		//! Calls the overidden ForceCompute::computeForces()
		/*! \param timestep parameter to pass on to the overidden method */
		void computeForces(unsigned int timestep)
			{
			this->get_override("computeForces")(timestep);
			}
	};

// a decision has been made to not support python classes derived from force computes at this time:
// only export the public interface

void export_ForceCompute()
	{
	class_< ForceComputeWrap, boost::shared_ptr<ForceComputeWrap>, bases<Compute>, boost::noncopyable >
		("ForceCompute", init< boost::shared_ptr<ParticleData> >())
		.def("acquire", &ForceCompute::acquire, return_value_policy<copy_const_reference>())
		//.def("computeForces", pure_virtual(&ForceCompute::computeForces))
		;
	}
#endif