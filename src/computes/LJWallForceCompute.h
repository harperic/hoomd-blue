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

#include <boost/shared_ptr.hpp>

#include "ForceCompute.h"

#ifndef __LJWallForceCompute__
#define __LJWallForceCompute__

class LJWallForceCompute :	public ForceCompute
	{
	public:
		LJWallForceCompute(boost::shared_ptr<ParticleData> pdata, Scalar r_cut);

		~LJWallForceCompute(void);

		virtual void setParams(unsigned int typ1, Scalar lj1, Scalar lj2);

		virtual void computeForces(unsigned int timestep);

	protected:
		Scalar m_r_cut;	//!< Cuttoff distance beyond which the force is set to 0
		unsigned int m_ntypes;	//!< Store the width and height of lj1 and lj2 here

		// this memory layout is shamelessly stolen from lammps
		// except that lammps also stores all of the parameters epsilon, sigma, and alpha
		// it needs to in order to do mixing of these values. My code architecture is different.
		// This is a low level force summing class, it ONLY sums forces, and doesn't do high
		// level concepts like mixing. That is for the caller to handle. So, I only store 
		// lj1 and lj2 here
		Scalar * __restrict__ m_lj1;	//!< Parameter for computing forces (m_ntypes by m_ntypes array)
		Scalar * __restrict__ m_lj2;	//!< Parameter for computing forces	(m_ntypes by m_ntypes array)
	};

#endif