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

#ifndef __ANALYZER_H__
#define __ANALYZER_H__

#include <boost/shared_ptr.hpp>

#include "ParticleData.h"
#include "Profiler.h"

/*! \ingroup hoomd_lib
	@{
*/

/*! \defgroup analyzers Analyzers
	\brief All classes that implement the Analyzer concept.
	\details See \ref page_dev_info for more information
*/

/*! @}
*/

//! Base class for analysis of particle data
/*! An Analyzer is a concept that encapsulates some process that is performed during
	the simulation with the sole purpose of outputting data to the user in some fashion.
	The results of an Analyzer can not modify the simulation in any way: that is what
	Updaters are for. In general, analyzers are likely to only be called every 1,000 
	time steps or much less often (this value entirely up to the user's discrestion).
	The System class will handle this. An Analyzer just needs to perform its calculations
	and make its output every time analyze() is called.

	By design Analyzers can reference any number of Computes while performing their
	analysis. The base class provides no methods for doing this, derived classes must
	make their own implementation (it is recomenned to pass a pointer to the Compute
	into the constructor of the derived class).

	See \ref page_dev_info for more information
	
	\ingroup analyzers
*/
class Analyzer
	{
	public:
		//! Constructs the analyzer and associates it with the ParticleData
		Analyzer(boost::shared_ptr<ParticleData> pdata);
		virtual ~Analyzer() {};
		
		//! Abstract method that performs the analysis
		/*! Derived classes will implement this method to calculate their results
			\param timestep Current time step of the simulation
			*/
		virtual void analyze(unsigned int timestep) = 0;
		
		//! Sets the profiler for the analyzer to use
		void setProfiler(boost::shared_ptr<Profiler> prof);
		
	protected:
		const boost::shared_ptr<ParticleData> m_pdata;	//!< The particle data this analyzer is associated with
		boost::shared_ptr<Profiler> m_prof;				//!< The profiler this analyzer is to use
	};
	
#ifdef USE_PYTHON
//! Export the Analyzer class to python
void export_Analyzer();
#endif

#endif
