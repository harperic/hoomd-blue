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

/*! \file Initializers.h
	\brief Defines a few initializers for setting up ParticleData instances
*/

#include "ParticleData.h"

#ifndef __INITIALIZERS_H__
#define __INITIALIZERS_H__

//! Inits a ParticleData with a simple cubic array of particles
/*! A number of particles along each axis are specified along with a spacing
	between particles. This initializer only generates a single particle type.
	\ingroup data_structs
*/
class SimpleCubicInitializer : public ParticleDataInitializer
	{
	public:
		//! Set the parameters
		SimpleCubicInitializer(unsigned int M, Scalar spacing);
		//! Empty Destructor
		virtual ~SimpleCubicInitializer() { }
		
		//! Returns the number of particles to be initialized
		virtual unsigned int getNumParticles() const;
		
		//! Returns the number of particle types to be initialized
		virtual unsigned int getNumParticleTypes() const;

		//! Returns the box the particles will sit in
		virtual BoxDim getBox() const;		
		
		//! Initializes the particle data arrays
		virtual void initArrays(const ParticleDataArrays &pdata) const;
	private:
		unsigned int m_M;	//!< Number of particles wide to make the box
		Scalar m_spacing;	//!< Spacing between particles
		BoxDim box;			//!< Precalculated box
	};

//! Inits a ParticleData with randomly placed particles in a cube
/*! A minimum distance parameter is provided so that particles are never
	placed too close together. This initializer only generates a single particle
	type.
*/
class RandomInitializer : public ParticleDataInitializer
	{
	public:
		//! Set the parameters
		RandomInitializer(unsigned int N, Scalar phi_p, Scalar min_dist);
		//! Empty Destructor
		virtual ~RandomInitializer() { }
		
		//! Returns the number of particles to be initialized
		virtual unsigned int getNumParticles() const;
		
		//! Returns the number of particle types to be initialized
		virtual unsigned int getNumParticleTypes() const;

		//! Returns the box the particles will sit in
		virtual BoxDim getBox() const;
		
		//! Initializes the particle data arrays
		virtual void initArrays(const ParticleDataArrays &pdata) const;

		//! Sets the random seed to use in the generation
		void setSeed(unsigned int seed);
	protected:
		unsigned int m_N;	//!< Number of particles to generate
		Scalar m_phi_p;		//!< Packing fraction to generate the particles at
		Scalar m_min_dist;	//!< Minimum distance to separate particles by
		BoxDim m_box;		//!< Box to put the particles in
	};



class RandomInitializerWithWalls : public RandomInitializer
	{
	public:
		//! Set the parameters
		RandomInitializerWithWalls(unsigned int N, Scalar phi_p, Scalar min_dist);
		//! Empty Destructor
		virtual ~RandomInitializerWithWalls() ;
		
		virtual WallData getWalls() const;

	protected:
		WallData m_walls;
	};
	
#ifdef USE_PYTHON
//! Exports the SimpleCubicInitializer class to python
void export_SimpleCubicInitializer();
//! Exports the RandomInitializer class to python
void export_RandomInitializer();
//! Exports the RandomInitializerWithWalls class to python
void export_RandomInitializerWithWalls();
#endif

#endif