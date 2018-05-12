/*
   Precise timer

   Copyright (C) 2008,2009:
         Daniel Roggen, droggen@gmail.com

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __PRECISETIMER_H
#define __PRECISETIMER_H

#ifdef WIN32

/*
typedef __int64 LONGLONG;
typedef struct  _LARGE_INTEGER
{
	LONGLONG QuadPart;
}	LARGE_INTEGER;


long QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
long QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
*/
//#include "winnt.h"
#include <wtypes.h>

#endif
#ifdef LINUX
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#endif //LINUX


class PreciseTimer {
public:
	static bool IsInit;
#ifdef WIN32
	static LARGE_INTEGER frq;
#endif

	static double QueryTimer();
//	static double QueryTimer2();
};


#endif




