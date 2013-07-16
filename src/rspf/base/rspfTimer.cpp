/* 
 * This code was taken directly from the OpenSceneGraph
 */
//#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTimer.h>

rspfTimer* rspfTimer::m_instance = 0;

// follows are the constructors of the Timer class, once version
// for each OS combination.  The order is WIN32, FreeBSD, Linux, IRIX,
// and the rest of the world.
//
// all the rest of the timer methods are implemented within the header.


rspfTimer* rspfTimer::instance()
{
   if(!m_instance)
   {
      m_instance = new rspfTimer;
   }
   return m_instance;
}

// ---
// From:  http://msdn.microsoft.com/en-us/library/b0084kay.aspx
// Defined for applications for Win32 and Win64. Always defined.
// ---
#if defined(_WIN32)

#include <sys/types.h>
#include <fcntl.h>
#include <windows.h>
#include <winbase.h>
rspfTimer::rspfTimer()
{
   LARGE_INTEGER frequency;
   if(QueryPerformanceFrequency(&frequency))
   {
      m_secsPerTick = 1.0/(double)frequency.QuadPart;
   }
   else
   {
      m_secsPerTick = 1.0;
      rspfNotify(rspfNotifyLevel_NOTICE)<<"Error: rspfTimer::rspfTimer() unable to use QueryPerformanceFrequency, "<<std::endl;
      rspfNotify(rspfNotifyLevel_NOTICE)<<"timing code will be wrong, Windows error code: "<<GetLastError()<<std::endl;
   }
   
   setStartTick();        
}

rspfTimer::Timer_t rspfTimer::tick() const
{
   LARGE_INTEGER qpc;
   if (QueryPerformanceCounter(&qpc))
   {
      return qpc.QuadPart;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_NOTICE)<<"Error: rspfTimer::rspfTimer() unable to use QueryPerformanceCounter, "<<std::endl;
      rspfNotify(rspfNotifyLevel_NOTICE)<<"timing code will be wrong, Windows error code: "<<GetLastError()<<std::endl;
      return 0;
   }
}

#else

#include <sys/time.h>

rspfTimer::rspfTimer(  )
{
   m_secsPerTick = (1.0 / (double) 1000000);
   
   setStartTick();        
}

rspfTimer::Timer_t rspfTimer::tick() const
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return ((rspfTimer::Timer_t)tv.tv_sec)*1000000+(rspfTimer::Timer_t)tv.tv_usec;
}

#endif
