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

#ifdef USE_PYTHON
#include <boost/python.hpp>
using namespace boost::python;
#endif

#include <iostream>
using namespace std;

#include "Compute.h"

/*! \file Compute.cc
	\brief Contains code for the Compute class
*/

/*! \param pdata Particle data this compute will act on. Must not be NULL.
	\post The Compute is constructed with the given particle data and a NULL profiler.
*/
Compute::Compute(boost::shared_ptr<ParticleData> pdata) : m_pdata(pdata), m_last_computed(0), m_first_compute(true)
	{
	// sanity check
	assert(pdata);
	assert(pdata->getN() > 0);
	}
		
/*! It is useful for the user to know where computation time is spent. All Computes
	should profile themselves. This method sets the profiler for them to use.
 	This method does not need to be called, as Computes will not profile themselves
 	on a NULL profiler. 
 	\param prof Pointer to a profiler for the compute to use. Set to boost::shared_ptr<Profiler>()
		(a null shared pointer) to stop the compute from profiling itself.
 	\note Derived classes MUST check for m_prof != NULL before calling any profiler methods.
*/
void Compute::setProfiler(boost::shared_ptr<Profiler> prof)
	{
	m_prof = prof;
	}

/*! \param timestep Current time step
 	\returns true if computations should be performed, false if they have already been done
 		at this \a timestep.
 	\note This method is designed to only be called once per time step.
*/
bool Compute::shouldCompute(unsigned int timestep)
	{
	// handle case where no computation has been performed yet
	if (m_first_compute)
		{
		m_first_compute = false;
		m_last_computed = timestep;
		return true;
		}

	// otherwise, we update if the last computed timestep is less than the current
	if (m_last_computed != timestep)
		{	
		m_last_computed = timestep;
		return true;
		}
	
	// failing the above, we perform no computation
	return false;
	}
	
#ifdef USE_PYTHON
//! Wrapper class for handling virtual methods of Compute in python
class ComputeWrap : public Compute, public wrapper<Compute>
	{
	public:
		//! Constructor
		/*! \param pdata Particle data to pass on to the base class */
		ComputeWrap(boost::shared_ptr<ParticleData> pdata) : Compute(pdata) 
			{ 
			}
	
		//! Calls overidden Compute::compute()
		/*! \param timestep Parameter to pass on to the base class method */
		void compute(unsigned int timestep)
			{
			this->get_override("compute")(timestep);
			}
		
		//! Calls overridden Compute::printStats()
		void printStats()
			{
			if (override f = this->get_override("printStats"))
				f();
			else
				Compute::printStats();
			}
			
		//! Default implementation of Compute::printStats()
		void default_printStats() 
			{
			this->Compute::printStats();
			}
	
	// A decision has been made to not currently support deriving new compute classes in python
	// thus, the internal methods of Compute that are only needed for that purpose do not need to be
	// exported, only the public interface
	//protected:
		// Calls overridden Compute::shouldCompute()
		/* \param timestep Parameter to pass on to the base class method */
		/*bool shouldCompute(unsigned int timestep)
			{
			if (override f = this->get_override("shouldCompute"))
				return f(timestep);
			else
				return Compute::shouldCompute(timestep);
			}*/
			
		// Default implementation of Compute::shouldCompute()
		/* \param timestep Parameter to pass on to the base class method */
		/*bool default_shouldCompute(unsigned int timestep) 
			{
			return this->Compute::shouldCompute(timestep);
			}
	
	// The python export needs to be a friend to export protected members
	friend void export_Compute();*/
};

void export_Compute()
	{
	class_<ComputeWrap, boost::shared_ptr<ComputeWrap>, boost::noncopyable>("Compute", init< boost::shared_ptr<ParticleData> >())
		.def("compute", pure_virtual(&Compute::compute))
		.def("printStats", &Compute::printStats, &ComputeWrap::default_printStats)
		.def("setProfiler", &Compute::setProfiler)
		//.def("shouldCompute", &Compute::shouldCompute, &ComputeWrap::default_shouldCompute)
		//.def_readonly("m_pdata", &Compute::m_pdata)
		//.def_readonly("m_prof", &Compute::m_prof)
		;
	}

#endif