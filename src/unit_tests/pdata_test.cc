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

/*! \file pdata_test.cc
	\brief Unit tests for BoxDim, ParticleData, SimpleCubicInitializer, and RandomInitializer classes.
	\ingroup unit_tests
*/

#include <iostream>

//! Name the boost unit test module
#define BOOST_TEST_MODULE ParticleDataTests
#include "boost_utf_configure.h"

#include <boost/test/floating_point_comparison.hpp>

#include "ParticleData.h"
#include "Initializers.h"

#ifdef USE_CUDA
#include "gpu_utils.h"
#endif

using namespace std;

//! Need a simple define for checking two close values whether they are double or single
#define MY_BOOST_CHECK_CLOSE(a,b,c) BOOST_CHECK_CLOSE(a,Scalar(b),Scalar(c))

//! Perform some basic tests on the boxdim structure
BOOST_AUTO_TEST_CASE( BoxDim_test )
	{
	Scalar tol = Scalar(1e-6);

	// test default constructor
	BoxDim a;
	MY_BOOST_CHECK_CLOSE(a.xlo,0.0, tol);
	MY_BOOST_CHECK_CLOSE(a.ylo,0.0, tol);
	MY_BOOST_CHECK_CLOSE(a.zlo,0.0, tol);
	MY_BOOST_CHECK_CLOSE(a.xhi,0.0, tol);
	MY_BOOST_CHECK_CLOSE(a.yhi,0.0, tol);
	MY_BOOST_CHECK_CLOSE(a.zhi,0.0, tol);
	
	BoxDim b(10.0);
	MY_BOOST_CHECK_CLOSE(b.xlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.ylo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.zlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.xhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.yhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.zhi,5.0, tol);
	
	BoxDim c(10.0, 30.0, 50.0);
	MY_BOOST_CHECK_CLOSE(c.xlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(c.ylo,-15.0, tol);
	MY_BOOST_CHECK_CLOSE(c.zlo,-25.0, tol);
	MY_BOOST_CHECK_CLOSE(c.xhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(c.yhi,15.0, tol);
	MY_BOOST_CHECK_CLOSE(c.zhi,25.0, tol);
	
	// test for assignment and copy constructor
	BoxDim d(c);
	MY_BOOST_CHECK_CLOSE(d.xlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(d.ylo,-15.0, tol);
	MY_BOOST_CHECK_CLOSE(d.zlo,-25.0, tol);
	MY_BOOST_CHECK_CLOSE(d.xhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(d.yhi,15.0, tol);
	MY_BOOST_CHECK_CLOSE(d.zhi,25.0, tol);
	
	BoxDim e;
	e = c;
	MY_BOOST_CHECK_CLOSE(e.xlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(e.ylo,-15.0, tol);
	MY_BOOST_CHECK_CLOSE(e.zlo,-25.0, tol);
	MY_BOOST_CHECK_CLOSE(e.xhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(e.yhi,15.0, tol);
	MY_BOOST_CHECK_CLOSE(e.zhi,25.0, tol);
	
	b = b;
	MY_BOOST_CHECK_CLOSE(b.xlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.ylo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.zlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.xhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.yhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(b.zhi,5.0, tol);
	}
	
//! Test operation of the particle data class
BOOST_AUTO_TEST_CASE( ParticleData_test )
	{
	BoxDim box(10.0, 30.0, 50.0);
	ParticleData a(1, box);
	
	Scalar tol = Scalar(1e-6);
	
	// make sure the box is working
	const BoxDim& c = a.getBox();
	MY_BOOST_CHECK_CLOSE(c.xlo,-5.0, tol);
	MY_BOOST_CHECK_CLOSE(c.ylo,-15.0, tol);
	MY_BOOST_CHECK_CLOSE(c.zlo,-25.0, tol);
	MY_BOOST_CHECK_CLOSE(c.xhi,5.0, tol);
	MY_BOOST_CHECK_CLOSE(c.yhi,15.0, tol);
	MY_BOOST_CHECK_CLOSE(c.zhi,25.0, tol);
	
	BoxDim box2(5.0, 5.0, 5.0);
	a.setBox(box2);
	const BoxDim& d = a.getBox();
	MY_BOOST_CHECK_CLOSE(d.xlo,-2.5, tol);
	MY_BOOST_CHECK_CLOSE(d.ylo,-2.5, tol);
	MY_BOOST_CHECK_CLOSE(d.zlo,-2.5, tol);
	MY_BOOST_CHECK_CLOSE(d.xhi,2.5, tol);
	MY_BOOST_CHECK_CLOSE(d.yhi,2.5, tol);
	MY_BOOST_CHECK_CLOSE(d.zhi,2.5, tol);	
	
	// make sure that getN is working
	BOOST_CHECK(a.getN() == 1);
	
	// Test the ability to acquire data
	ParticleDataArrays arrays = a.acquireReadWrite();
	// begin by verifying that the defaults the class adversizes are set
	BOOST_CHECK(arrays.nparticles == 1);
	MY_BOOST_CHECK_CLOSE(arrays.x[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.y[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.z[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.vx[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.vy[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.vz[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.ax[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.ay[0], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays.az[0], 0.0, tol);
	BOOST_CHECK(arrays.type[0] == 0);
	BOOST_CHECK(arrays.rtag[0] == 0);
	BOOST_CHECK(arrays.tag[0] == 0);
	
	// set some new values for testing
	arrays.x[0] = 1.0;
	arrays.y[0] = 2.0;
	arrays.z[0] = -2.0;
	arrays.vx[0] = 11.0;
	arrays.vy[0] = 12.0;
	arrays.vz[0] = 13.0;
	arrays.ax[0] = 21.0;
	arrays.ay[0] = 22.0;
	arrays.az[0] = 23.0;
	arrays.type[0] = 1;
	
	a.release();
	
	// make sure when the data is re-acquired, the values read properly
	ParticleDataArraysConst arrays_const = a.acquireReadOnly();
	BOOST_CHECK(arrays_const.nparticles == 1);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[0], 1.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[0], 2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[0], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.vx[0], 11.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.vy[0], 12.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.vz[0], 13.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.ax[0], 21.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.ay[0], 22.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.az[0], 23.0, tol);
	BOOST_CHECK(arrays_const.type[0] == 1);
	BOOST_CHECK(arrays_const.rtag[0] == 0);
	BOOST_CHECK(arrays_const.tag[0] == 0);	
	
	a.release();
	
	// finally, lets check a larger ParticleData for correctness of the initialization
	const unsigned int N = 1000;
	ParticleData b(N, box);
	arrays_const = b.acquireReadOnly();
	BOOST_CHECK(arrays_const.nparticles == N);
	for (unsigned int i = 0; i < N; i++)
		{
		MY_BOOST_CHECK_CLOSE(arrays_const.x[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.y[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.z[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.vx[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.vy[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.vz[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.ax[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.ay[i], 0.0, tol);
		MY_BOOST_CHECK_CLOSE(arrays_const.az[i], 0.0, tol);
		BOOST_CHECK(arrays_const.type[i] == 0);
		BOOST_CHECK(arrays_const.rtag[i] == i);
		BOOST_CHECK(arrays_const.tag[i] == i);
		}

	b.release();
	}
	
#ifdef USE_CUDA
//! Tests the ability of the ParticleData class to copy data between CPU <-> GPU
BOOST_AUTO_TEST_CASE( ParticleData_gpu_tests )
	{
	Scalar tol = Scalar(1e-6);
		
	// This set of tests will actually check that the ParticleData class is working
	// It would be a pain in the ass to test every possible state change in going from
	// the data being on the CPU to -on the GPU to on both, etc.... so we will just check
	// basic functionality here. Any subtle bugs will just have to show up when 
	// unit tests are done that compare simulation runs on the cpu to those on the GPU
	BoxDim box(10.0,30.0,50.0);
	int N = 500;
	ParticleData pdata(N, box);
	ParticleDataArrays arrays = pdata.acquireReadWrite();
	for (int i = 0; i < N; i++)
		{
		arrays.x[i] = float(i)/100.0f;
		arrays.y[i] = float(i)/75.0f;
		arrays.z[i] = float(i)/50.0f;

		arrays.ax[i] = float(i);
		arrays.ay[i] = float(i) * 2.0f;
		arrays.az[i] = float(i) * 3.0f;
		arrays.type[i] = i;
		}
	pdata.release();
	// try accessing the data on the GPU
	gpu_pdata_arrays d_pdata = pdata.acquireReadWriteGPU();
	gpu_pdata_texread_test(&d_pdata);
	pdata.release();

	pdata.acquireReadOnly();
	for (unsigned int i = 0; i < (unsigned int)N; i++)
		{
		// check to make sure that the position copied back OK
		MY_BOOST_CHECK_CLOSE(arrays.x[i], float(i)/100.0f, tol);
		MY_BOOST_CHECK_CLOSE(arrays.y[i], float(i)/75.0f, tol);
		MY_BOOST_CHECK_CLOSE(arrays.z[i], float(i)/50.0f, tol);

		// check to make sure that the texture read worked and read back ok
		BOOST_CHECK(arrays.vx[i] == arrays.x[i]);
		BOOST_CHECK(arrays.vy[i] == arrays.y[i]);
		BOOST_CHECK(arrays.vz[i] == arrays.z[i]);
	
		// check to make sure that the accel was copied back ok
		MY_BOOST_CHECK_CLOSE(arrays.ax[i], float(i), tol);
		MY_BOOST_CHECK_CLOSE(arrays.ay[i], float(i) * 2.0f, tol);
		MY_BOOST_CHECK_CLOSE(arrays.az[i], float(i) * 3.0f, tol);

		BOOST_CHECK(arrays.type[i] == i);
		}
	pdata.release();
	}
#endif
	

//! Test operation of the simple cubic initializer class
BOOST_AUTO_TEST_CASE( SimpleCubic_test )
	{
	Scalar tol = Scalar(1e-6);

	// make a simple one-particle box
	SimpleCubicInitializer one(1, 2.0);
	ParticleData one_data(one);
	ParticleDataArraysConst arrays_const = one_data.acquireReadOnly();
	BOOST_CHECK(arrays_const.nparticles == 1);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[0], -1.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[0], -1.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[0], -1.0, tol);
	one_data.release();
	
	// now try an 8-particle one
	SimpleCubicInitializer eight(2, 2.0);
	ParticleData eight_data(eight);
	
	arrays_const = eight_data.acquireReadOnly();
	BOOST_CHECK(arrays_const.nparticles == 8);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[0], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[0], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[0], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[1], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[1], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[1], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[2], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[2], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[2], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[3], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[3], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[3], -2.0, tol);

	MY_BOOST_CHECK_CLOSE(arrays_const.x[4], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[4], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[4], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[5], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[5], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[5], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[6], -2.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[6], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[6], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.x[7], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.y[7], 0.0, tol);
	MY_BOOST_CHECK_CLOSE(arrays_const.z[7], 0.0, tol);
	eight_data.release();
	}
	
//! Tests the RandomParticleInitializer class
BOOST_AUTO_TEST_CASE( Random_test )
	{
	// create a fairly dense system with a minimum distance of 0.8
	Scalar min_dist = 0.8;
	RandomInitializer rand_init(1000, 0.4, min_dist);
	ParticleData pdata(rand_init);
	ParticleDataArraysConst	arrays = pdata.acquireReadOnly();
	
	// check that the distances between particles are OK
	BoxDim box = pdata.getBox();
	Scalar L = box.xhi - box.xlo;	
	for (unsigned int i = 0; i < arrays.nparticles; i++)
		{
		BOOST_CHECK(arrays.x[i] <= box.xhi && arrays.x[i] >= box.xlo);
		BOOST_CHECK(arrays.y[i] <= box.yhi && arrays.y[i] >= box.ylo);
		BOOST_CHECK(arrays.z[i] <= box.zhi && arrays.z[i] >= box.zlo);
		
		for (unsigned int j = 0; j < arrays.nparticles; j++)
			{
			if (i == j)
				continue;
			
			Scalar dx = arrays.x[j] - arrays.x[i];
			Scalar dy = arrays.y[j] - arrays.y[i];
			Scalar dz = arrays.z[j] - arrays.z[i];
			
			if (dx < -L/Scalar(2.0))
				dx += L;
			if (dx > L/Scalar(2.0))
				dx -= L;
			
			if (dy < -L/Scalar(2.0))
				dy += L;
			if (dy > L/Scalar(2.0))
				dy -= L;
			
			if (dz < -L/Scalar(2.0))
				dz += L;
			if (dz > L/Scalar(2.0))
				dz -= L;
			
			Scalar dr2 = dx*dx + dy*dy + dz*dz;
			BOOST_CHECK(dr2 >= min_dist*min_dist);
			}
		}
		
	pdata.release();
	}