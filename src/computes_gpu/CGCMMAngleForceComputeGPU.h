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
#pragma warning( disable : 4103 )
#endif

#include "CGCMMAngleForceCompute.h"
#include "CGCMMAngleForceGPU.cuh"

#include <boost/shared_ptr.hpp>
#include <boost/signals.hpp>

/*! \file HarmonicAngleForceComputeGPU.h
	\brief Declares the HarmonicAngleForceGPU class
*/

#ifndef __CGCMMANGLEFORCECOMPUTEGPU_H__
#define __CGCMMANGLEFORCECOMPUTEGPU_H__

//! Implements the CGCMM harmonic angle force calculation on the GPU
/*!	CGCMMAngleForceComputeGPU implements the same calculations as CGCMMAngleForceCompute,
	but executing on the GPU.
	
	Per-type parameters are stored in a simple global memory area pointed to by
	\a m_gpu_params. They are stored as float2's with the \a x component being K and the
	\a y component being t_0.
	
	The GPU kernel can be found in angleforce_kernel.cu.

	\ingroup computes
*/
class CGCMMAngleForceComputeGPU : public CGCMMAngleForceCompute
	{
	public:
		//! Constructs the compute
		CGCMMAngleForceComputeGPU(boost::shared_ptr<ParticleData> pdata);
		//! Destructor
		~CGCMMAngleForceComputeGPU();
		
		//! Sets the block size to run on the device
		/*! \param block_size Block size to set
		*/
		void setBlockSize(int block_size) { m_block_size = block_size; }
		
		//! Set the parameters
		virtual void setParams(unsigned int type, Scalar K, Scalar t_0, unsigned int cg_type, Scalar eps, Scalar sigma);
		
	protected:
		int m_block_size;		//!< Block size to run calculation on
		vector<float2 *> m_gpu_params;	//!< k, t0 Parameters stored on the GPU
		float2 *m_host_params;	//!<  k, t0 parameters stored on the host

		float prefact[4];//!< prefact precomputed prefactors for CG-CMM angles
		float cgPow1[4];//!< cgPow1 1st powers for CG-CMM angles
		float cgPow2[4];//!< cgPow2 2nd powers for CG-CMM angles

		// below are just for the CG-CMM angle potential
		vector<float2 *> m_gpu_CGCMMsr;//!< GPU copy of the angle's epsilon/sigma/rcut (esr)	
		float2 *m_host_CGCMMsr;				//!< Host copy of the angle's epsilon/sigma/rcut (esr)
		vector<float4 *> m_gpu_CGCMMepow;//!< GPU copy of the angle's powers (pow1,pow2) and prefactor
		float4 *m_host_CGCMMepow;				//!< Host copy of the angle's powers (pow1,pow2) and prefactor

		//! Actually compute the forces
		virtual void computeForces(unsigned int timestep);
	};
	
//! Export the CGCMMAngleForceComputeGPU class to python
void export_CGCMMAngleForceComputeGPU();

#endif

#ifdef WIN32
#pragma warning( pop )
#endif

