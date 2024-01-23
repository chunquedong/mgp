/*
 * Copyright (c) 2012-2016, chunquedong
 *
 * This file is part of cppfan project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE version 3.0
 *
 * History:
 *   2012-12-23  Jed Young  Creation
 */
#include "System.h"

#include <stdio.h>

using namespace mgp;

#ifndef WIN32

  #include <sys/types.h>
  #include <time.h>
  #include <unistd.h>

#ifdef P__MACH__
  #include <mach/mach.h>
  #include <mach/mach_time.h>
  //#include <sys/_types/_timespec.h>
  #include <mach/mach.h>
  #include <mach/clock.h>
  #include <sys/timeb.h>
  #include <sys/time.h>
  #include <sys/sysctl.h>

  int64_t System::nanoTicks(void) {
    //            clock_serv_t cclock;
    //            mach_timespec_t mts;
    //
    //            host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    //            clock_get_time(cclock, &mts);
    //            mach_port_deallocate(mach_task_self(), cclock);
    //
    //            return (mts.tv_sec * NanoPerSecond) + mts.tv_nsec;
    //            mach_timebase_info_data_t timebase;
    //            mach_timebase_info(&timebase);
    //            int64_t now = mach_absolute_time();
    //            int64_t elapsedNanoSeconds = now * timebase.numer / timebase.denom;
    //            return elapsedNanoSeconds;
    
    struct timeval boottime;
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    size_t size = sizeof(boottime);
    time_t now;
    time_t uptime = -1;
    (void)time(&now);
    if (sysctl(mib, 2, &boottime, &size, NULL, 0) != -1 && boottime.tv_sec != 0) {
      uptime = now - boottime.tv_sec;
    }
    return uptime * NanoPerSecond;
  }
#else
  #include <sys/timeb.h>
  #include <sys/time.h>

  int64_t System::nanoTicks(void) {
    //  return clock() / (CLOCKS_PER_SECOND * 1000);
    struct timespec ts;
    static time_t startTime;
    int rc = clock_gettime(CLOCK_MONOTONIC, &ts);
    //获取CLOCK_BOOTTIME失败的时候使用time函数代替
    if (rc != 0) {
      if (startTime == 0) {
        time(&startTime);
      }
      time_t now;
      time(&now);
      return ((int64_t)(now - startTime))*NanoPerSecond;
    }
    return (ts.tv_sec * NanoPerSecond) + ts.tv_nsec;
  }
#endif

int64_t System::currentTimeMillis() {
  //        struct timeb val;
  //        ftime(&val);
  //        return val.time * (int64_t)1000+ val.millitm;
  
  struct timeval tv;
  gettimeofday (&tv, NULL);
  return ((int64_t) tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

#else
/*========================================================================
 * Windows
 */

#include <Windows.h>
#include <time.h>

int64_t System::nanoTicks() {
  /*
   LARGE_INTEGER m_nBeginTime;
   LARGE_INTEGER m_nFreq;
   QueryPerformanceFrequency(&m_nFreq);
   QueryPerformanceCounter(&m_nBeginTime);
   return (m_nBeginTime.QuadPart*NanoPerSecond)/m_nFreq.QuadPart;
   */
  int64_t t = (int64_t)timeGetTime();
  return t * 1000000L;
}

int64_t System::currentTimeMillis() {
    struct timeval tv;
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tv.tv_sec = clock;
    tv.tv_usec = wtm.wMilliseconds * 1000;
    int64_t millis = ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
    //printf("%lld\n", millis);
    return millis;
}
#endif



#ifndef WIN32

  #include <sys/types.h>
  #include <time.h>
  #include <unistd.h>

  void System::sleep(MillisTime millitm) {
    usleep(millitm * 1000);
  }

  bool System::getSelfPath(char *selfname) {
    const char *sysfile = "/proc/self/exe";
    int  namelen = 256;
    //memset(selfname, 0, 256);
    if ( -1 != readlink( sysfile, selfname, namelen) ) {
      return true;
    }
    return false;
  }

#else
/*========================================================================
 * Windows
 */

#include <Windows.h>

  void System::sleep(MillisTime millitm) {
    Sleep(millitm);
  }

  bool System::getSelfPath(char *selfname) {
    //TCHAR szFileName[MAX_PATH];
    GetModuleFileNameA(NULL,selfname,MAX_PATH);
    return true;
  }

#endif //CF_WIN

#include <thread>

uint64_t System::currentThreadId() {
#ifdef WIN32
  //unsigned long GetCurrentThreadId(void);
  return GetCurrentThreadId();
#else
  //unsigned long pthread_self(void);
  return (uint64_t)pthread_self();
#endif
}

