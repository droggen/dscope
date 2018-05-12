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

#include <QtGlobal>
#include "precisetimer.h"


bool PreciseTimer::IsInit=false;


//************************************
#ifdef Q_OS_WIN
LARGE_INTEGER PreciseTimer::frq;
double PreciseTimer::QueryTimer()
{
    LARGE_INTEGER t1;
    double dt;

    if(!IsInit) {
        QueryPerformanceFrequency(&frq);
        IsInit=true;
    }
    QueryPerformanceCounter(&t1);

    dt = ((double)t1.QuadPart)/(double)frq.QuadPart;

    return dt;
}
#endif


//************************************
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)

double PreciseTimer::QueryTimer()
{
    timeval t1;
    double dt;

    gettimeofday(&t1,0);

    dt = (((double)t1.tv_sec)+((double)t1.tv_usec)/1000000.0);

    return dt;
}
/*double PreciseTimer::QueryTimer2()
{
    double dt;
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC,&now);
    dt = ((double)now.tv_sec)+((double)now.tv_nsec)/1000000000.0;

    return dt;
}
*/
#endif //LINUX

#ifdef Q_OS_ANDROID
double PreciseTimer::QueryTimer()
{
    timeval t1;
    double dt;

    gettimeofday(&t1,0);

    dt = (((double)t1.tv_sec)+((double)t1.tv_usec)/1000000.0);

    return dt;
    return 0;
}
#endif
