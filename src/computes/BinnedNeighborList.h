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
#include "NeighborList.h"

/*! \file BinnedNeighborList.h
	\brief Defines an optimized O(N) neighbor list generator
*/

#ifndef __BINNED_NEIGHBORLIST_H__
#define __BINNED_NEIGHBORLIST_H__

//! A more efficient neighborlist algorithm for very large N
/*! This implementation puts particles into cubic "bins" with side length r_max. Then, each
	particle can only have neighbors from it's bin and neighboring bins. Thus, as long as the 
	density is mostly uniform: the N^2 algorithm is transformed into an O(N) algorithm.
	
	Neighbor lists are generated much faster with BinnedNeighborList than
	NeighborList, even for small systems of only 1000 particles.
	\ingroup computes
*/
class BinnedNeighborList : public NeighborList
	{
	public:
		//! Constructor
		BinnedNeighborList(boost::shared_ptr<ParticleData> pdata, Scalar r_cut, Scalar r_buff);
				
		//! Computes the NeighborList if it needs updating
		virtual void compute(unsigned int timestep);
		 
		//! Print statistics on the neighborlist
		virtual void printStats();

	protected:
		std::vector< std::vector<unsigned int> > m_bins;	//!< Bins of particle indices
		std::vector < std::vector<Scalar> > m_binned_x;		//!< coordinates of the particles in the bins
		std::vector < std::vector<Scalar> > m_binned_y;		//!< coordinates of the particles in the bins
		std::vector < std::vector<Scalar> > m_binned_z;		//!< coordinates of the particles in the bins
		std::vector < std::vector<unsigned int> > m_binned_tag;	//!< tags of the particles in the bins
		
		unsigned int m_Mx;	//!< Number of bins in x direction
		unsigned int m_My;	//!< Number of bins in y direction
		unsigned int m_Mz;	//!< Number of bins in z direction

		//! Puts the particles into their bins
		void updateBins();
	
		//! Updates the neighborlist using the binned data
		void updateListFromBins();
	};

#ifdef USE_PYTHON
//! Exports the BinnedNeighborList class to python
void export_BinnedNeighborList();
#endif

#endif
