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

#ifndef _CUDA_UPDATERS_H_
#define _CUDA_UPDATERS_H_

#include <stdio.h>
#include <cuda_runtime_api.h>

#include "gpu_pdata.h"

/*! \file gpu_updaters.h
	\brief Declares structures and classes for all of the Updaters on the GPU
	\ingroup cuda_code
*/

extern "C" {

//! Does the first part of the NVE update
void nve_pre_step(gpu_pdata_arrays *pdata, gpu_boxsize *box, float deltaT);

//! Does the second part of the NVE update
void nve_step(gpu_pdata_arrays *pdata, float4 **force_data_ptrs, int num_forces, float deltaT);

/////////////////////////////////////// NVT stuff
//! Data structure storing needed intermediate values for NVT integration
struct gpu_nvt_data
	{
	float *Xi;	// Friction parameter in Nose-Hoover
	float *Xi_dbl;	// Double buffered Xi
	float *partial_Ksum; // NBlocks elements, each is a partial sum of m*v^2
	float *Ksum;	// fully reduced Ksum
	int NBlocks;	// Number of blocks in the computation (must be a power of 2)
	int block_size;
	};

//! Allocates memory on the device (block size must be a power of 2)
void nvt_alloc_data(gpu_nvt_data *d_nvt_data, int N, int block_size);

//! Frees memory on the device
void nvt_free_data(gpu_nvt_data *d_nvt_data);

//! Does the first step of the computation
void nvt_pre_step(gpu_pdata_arrays *pdata, gpu_boxsize *box, gpu_nvt_data *d_nvt_data, float deltaT);

void nvt_reduce_ksum(gpu_nvt_data *d_nvt_data);

//! Does the second step of the computaiton
void nvt_step(gpu_pdata_arrays *pdata, gpu_nvt_data *d_nvt_data, float4 **force_data_ptrs, int num_forces, float deltaT, float Q, float T);

}

#endif