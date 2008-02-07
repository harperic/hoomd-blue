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

#include <iostream>

//! Name the unit test module
#define BOOST_TEST_MODULE UtilityClassesTests
#include "boost_utf_configure.h"

#include <math.h>
#include "ClockSource.h"
#include "Profiler.h"

/*! \file utils_test.cc
	\brief Unit tests for ClockSource and Profiler
	\ingroup unit_tests
*/

using namespace std;

//! perform some simple checks on the clock source code
BOOST_AUTO_TEST_CASE( ClockSource_test )
	{
	ClockSource c1;
	int64_t t = c1.getTime();
	// c.getTime() should read 0, but we can't expect it to be exact, so allow a tolerance
	BOOST_CHECK(abs(int(t)) <= 1000000);
	
	// test timing a whole second
	ClockSource c2;
	int64_t t1 = c2.getTime();
	Sleep(1000);
	int64_t t2 = c2.getTime();
	BOOST_CHECK(abs(int(t2 - t1 - 1000000000)) <= 10000000);
	
	// unfortunately, testing of microsecond timing with a sleep routine is out of the question
	// the following test code tests the ability of the timer to read nearby values
	/*ClockSource c4;
	int64_t times[100];
	for (int i = 0; i < 100; i++)
		{
		times[i] = c4.getTime();
		}
	for (int i = 0; i < 100; i++)
		{
		cout << times[i] << endl;
		}*/
		
	// test copying timers
	// operator=
	c1 = c2;
	t1 = c1.getTime();
	t2 = c2.getTime();
	BOOST_CHECK(abs(int(t1-t2)) <= 1000000);
	// copy constructor
	ClockSource c3(c1);
	t1 = c1.getTime();
	t2 = c3.getTime();
	BOOST_CHECK(abs(int(t1-t2)) <= 1000000);

	// test the ability of the clock source to format values
	BOOST_CHECK_EQUAL(ClockSource::formatHMS(0), string("00:00:00"));
	BOOST_CHECK_EQUAL(ClockSource::formatHMS(int64_t(1000000000)), string("00:00:01"));
	BOOST_CHECK_EQUAL(ClockSource::formatHMS(int64_t(1000000000)*int64_t(11)), string("00:00:11"));
	BOOST_CHECK_EQUAL(ClockSource::formatHMS(int64_t(1000000000)*int64_t(65)), string("00:01:05"));
	BOOST_CHECK_EQUAL(ClockSource::formatHMS(int64_t(1000000000)*int64_t(3678)), string("01:01:18"));
	}
	
//! perform some simple checks on the profiler code
BOOST_AUTO_TEST_CASE ( Profiler_test )
	{
	// ProfileDataElem tests
	// constructor test
	ProfileDataElem p;
	BOOST_CHECK(p.getChildElapsedTime() == 0);
	BOOST_CHECK(p.getTotalFlopCount() == 0);
	BOOST_CHECK(p.getTotalMemByteCount() == 0);

	// build up a tree and test its getTotal members
	p.m_elapsed_time = 1;
	p.m_flop_count = 2;
	p.m_mem_byte_count = 3;
	BOOST_CHECK(p.getChildElapsedTime() == 0);
	BOOST_CHECK(p.getTotalFlopCount() == 2);
	BOOST_CHECK(p.getTotalMemByteCount() == 3);
	
	p.m_children["A"].m_elapsed_time = 4;
	p.m_children["A"].m_flop_count = 5;
	p.m_children["A"].m_mem_byte_count = 6;
	BOOST_CHECK(p.getChildElapsedTime() == 4);
	BOOST_CHECK(p.getTotalFlopCount() == 7);
	BOOST_CHECK(p.getTotalMemByteCount() == 9);
	
	p.m_children["B"].m_elapsed_time = 7;
	p.m_children["B"].m_flop_count = 8;
	p.m_children["B"].m_mem_byte_count = 9;
	BOOST_CHECK(p.getChildElapsedTime() == 4+7);
	BOOST_CHECK(p.getTotalFlopCount() == 7+8);
	BOOST_CHECK(p.getTotalMemByteCount() == 9+9);
	
	p.m_children["A"].m_children["C"].m_elapsed_time = 10;
	p.m_children["A"].m_children["C"].m_flop_count = 11;
	p.m_children["A"].m_children["C"].m_mem_byte_count = 12;
	BOOST_CHECK(p.getChildElapsedTime() == 4+7);
	BOOST_CHECK(p.getTotalFlopCount() == 7+8+11);
	BOOST_CHECK(p.getTotalMemByteCount() == 9+9+12);	

	Profiler prof("Main");
	prof.push("Loading");
	Sleep(500);
	prof.pop();
	prof.push("Neighbor");
	Sleep(1000);
	prof.pop(int64_t(1e6), int64_t(1e6));
	
	prof.push("Pair");
	prof.push("Load");
	Sleep(1000);
	prof.pop(int64_t(1e9), int64_t(1e9));
	prof.push("Work");
	Sleep(1000);
	prof.pop(int64_t(10e9), int64_t(100));
	prof.push("Unload");
	Sleep(1000);
	prof.pop(int64_t(100), int64_t(1e9));
	prof.pop();

	std::cout << prof;

	// This code attempts to reproduce the problem found in ticket #50
	Profiler prof2("test");
	prof2.push("test1");
	//Changing this slep value much lower than 100 results in the bug.
	Sleep(000);
	prof2.pop(100, 100);
	std::cout << prof2;
	
	}
