//*******************************************************************
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class definition for rspfNotify.
//*******************************************************************
//  $Id: rspfNotify.cpp 22149 2013-02-11 21:36:10Z dburken $

#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <stack>
#include <cstddef>

#include <rspf/base/rspfNotify.h>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

static std::ostream* theOssimFatalStream  = &std::cerr;
static std::ostream* theOssimWarnStream   = &std::cerr;
static std::ostream* theOssimInfoStream   = &std::cout;
static std::ostream* theOssimNoticeStream = &std::cout;
static std::ostream* theOssimDebugStream  = &std::cout;
static std::ostream* theOssimAlwaysStream = &std::cout;

static OpenThreads::Mutex theMutex;
static rspfNotifyFlags theNotifyFlags     = rspfNotifyFlags_ALL;
std::stack<rspfNotifyFlags> theNotifyFlagsStack;

template <class charT, class traits = std::char_traits<charT> >
class rspfNullBufferStream : public std::basic_streambuf<charT, traits>
{
public:
   rspfNullBufferStream(){};


protected:

   std::streamsize xsputn(const charT * /* pChar */, std::streamsize /* n */)
      {
         return 0;
      }

private:
   rspfNullBufferStream(const rspfNullBufferStream&);
   rspfNullBufferStream& operator=(const rspfNullBufferStream&);
};

template <class charT, class traits = std::char_traits<charT> >
class rspfLogFileBufferStream : public std::basic_streambuf<charT, traits>
{
public:
   rspfLogFileBufferStream(){};

   void setLogFilename(const rspfFilename& file)
      {
         theLogFilename = file;
      }
   rspfFilename getLogFilename()const
      {
         return theLogFilename;
      }

protected:
   rspfFilename theLogFilename;
   virtual int overflow(int c)
      {
         if(!traits::eq_int_type(c, traits::eof()))
         {
            tempString = tempString + rspfString(c);
         }

         return c;
      }

   virtual std::streamsize xsputn(const charT * pChar, std::streamsize n)
      {
         tempString = tempString + rspfString(pChar, pChar + n);

         return n;
      }

   virtual int sync()
      {
         if(theLogFilename != "")
         {
            std::ofstream outFile(theLogFilename.c_str(),
                                  std::ios::app|std::ios::out);
            if(outFile)
            {
               outFile.write(tempString.c_str(), (std::streamsize)tempString.length());
            }
            
            tempString = "";
         }
         return 0;
      }

private:
   rspfString tempString;

   rspfLogFileBufferStream(const rspfLogFileBufferStream&);
   rspfLogFileBufferStream& operator=(const rspfLogFileBufferStream&);
};


class rspfNullStream : public std::ostream
{
public:
   rspfNullStream() : std::ostream(&nullBufferStream){}
   virtual ~rspfNullStream()
      {
         nullBufferStream.pubsync();
      }

private:
   rspfNullBufferStream<char> nullBufferStream;
   // Copy & assignment are undefined in iostreams
   rspfNullStream(const rspfNullStream&);
   rspfNullStream & operator=(const rspfNullStream&);
};

class rspfLogFileStream : public std::ostream
{
public:
   rspfLogFileStream() : std::ostream(&theLogFileBufferStream){}
   virtual ~rspfLogFileStream()
      {
         theLogFileBufferStream.pubsync();
      }
   void setLogFilename(const rspfFilename& filename)
      {
         theLogFileBufferStream.setLogFilename(filename);
      }
   rspfFilename getLogFilename()const
      {
         return theLogFileBufferStream.getLogFilename();
      }

private:
   rspfLogFileBufferStream<char> theLogFileBufferStream;
   // Copy & assignment are undefined in iostreams
   rspfLogFileStream(const rspfLogFileStream&);
   rspfLogFileStream & operator=(const rspfLogFileStream&);
};

static rspfNullStream    theOssimNullStream;
static rspfLogFileStream theLogFileStream;

void rspfSetDefaultNotifyHandlers()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theOssimFatalStream  = &std::cerr;
   theOssimWarnStream   = &std::cout;
   theOssimInfoStream   = &std::cout;
   theOssimNoticeStream = &std::cout;
   theOssimDebugStream  = &std::cout;
   theOssimAlwaysStream = &std::cout;
}

void rspfSetNotifyStream(std::ostream* outputStream,
                          rspfNotifyFlags whichLevelsToRedirect)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   if(whichLevelsToRedirect&rspfNotifyFlags_FATAL)
   {
      theOssimFatalStream = outputStream;
   }
   if(whichLevelsToRedirect&rspfNotifyFlags_WARN)
   {
      theOssimWarnStream = outputStream;
   }
   if(whichLevelsToRedirect&rspfNotifyFlags_INFO)
   {
      theOssimInfoStream = outputStream;
   }
   if(whichLevelsToRedirect&rspfNotifyFlags_NOTICE)
   {
      theOssimNoticeStream = outputStream;
   }
   if(whichLevelsToRedirect&rspfNotifyFlags_DEBUG)
   {
      theOssimDebugStream = outputStream;
   }
}

std::ostream* rspfGetNotifyStream(rspfNotifyLevel whichLevel)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   std::ostream* notifyStream = &theOssimNullStream;

   switch(whichLevel)
   {
      case rspfNotifyLevel_ALWAYS:
      {
         notifyStream = theOssimAlwaysStream;
         break;
      }
      case rspfNotifyLevel_FATAL:
      {
         notifyStream = theOssimFatalStream;
         break;
      }
      case rspfNotifyLevel_WARN:
      {
         notifyStream = theOssimWarnStream;
         break;
      }
      case rspfNotifyLevel_INFO:
      {
         notifyStream = theOssimInfoStream;
         break;
      }
      case rspfNotifyLevel_NOTICE:
      {
         notifyStream = theOssimNoticeStream;
         break;
      }
      case rspfNotifyLevel_DEBUG:
      {
         notifyStream = theOssimDebugStream;
         break;
      }
   }
   return notifyStream;
}

RSPFDLLEXPORT std::ostream& rspfNotify(rspfNotifyLevel level)
{
   if(rspfIsReportingEnabled())
   {
      theMutex.lock();
      if(theLogFileStream.getLogFilename() != "")
      {
         theMutex.unlock();
         return theLogFileStream;
      }
      else
      {
         bool reportMessageFlag = false;
         switch(level)
         {
            case rspfNotifyLevel_ALWAYS:
            {
               reportMessageFlag = true;
               break;
            }
            case rspfNotifyLevel_FATAL:
            {
               if(theNotifyFlags&rspfNotifyFlags_FATAL)
               {
                  reportMessageFlag = true;
               }
               break;
            }
            case rspfNotifyLevel_WARN:
            {
               if(theNotifyFlags&rspfNotifyFlags_WARN)
               {
                  reportMessageFlag = true;
               }
               break;
            }
            case rspfNotifyLevel_INFO:
            {
               if(theNotifyFlags&rspfNotifyFlags_INFO)
               {
                  reportMessageFlag = true;
               }
               break;
            }
            case rspfNotifyLevel_NOTICE:
            {
               if(theNotifyFlags&rspfNotifyFlags_NOTICE)
               {
                  reportMessageFlag = true;
               }
               break;
            }
            case rspfNotifyLevel_DEBUG:
            {
               if(theNotifyFlags&rspfNotifyFlags_DEBUG)
               {
                  reportMessageFlag = true;
               }
               break;
            }
         }
         if(reportMessageFlag)
         {
            theMutex.unlock();
            return *rspfGetNotifyStream(level);
         }
      }

      theMutex.unlock();
      
   } // matches: if(rspfIsReportingEnabled())

   return theOssimNullStream;
}

void rspfSetLogFilename(const rspfFilename& filename)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theLogFileStream.setLogFilename(filename);
}

/*
const char* rspfGetLogFilename()
{
   return theLogFileStream.getLogFilename().c_str();
}
*/

void rspfGetLogFilename(rspfFilename& logFile)
{
   logFile = theLogFileStream.getLogFilename();
}

rspfString rspfErrorV(const char *fmt, va_list args )
{
   char temp[2024];
   if(fmt)
   {
      vsprintf(temp, fmt, args);
   }
   else
   {
      sprintf(temp,"%s", "");
   }

   return temp;
}

void rspfEnableNotify(rspfNotifyFlags flags)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theNotifyFlags = (rspfNotifyFlags)(theNotifyFlags | flags);
}

void rspfDisableNotify(rspfNotifyFlags flags)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theNotifyFlags = (rspfNotifyFlags)((rspfNotifyFlags_ALL^flags)&
                                       theNotifyFlags);
}

void rspfSetNotifyFlag(rspfNotifyFlags notifyFlags)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theNotifyFlags = notifyFlags;
}

void rspfPushNotifyFlags()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theNotifyFlagsStack.push(theNotifyFlags);
}

void rspfPopNotifyFlags()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   if(theNotifyFlagsStack.empty())
   {
      return;
   }
   theNotifyFlags = theNotifyFlagsStack.top();
   theNotifyFlagsStack.pop();
}

rspfNotifyFlags rspfGetNotifyFlags()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   return theNotifyFlags;
}



bool rspfIsReportingEnabled()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   return  (theNotifyFlags != rspfNotifyFlags_NONE);
}


void rspfNotify(rspfString msg,
                 rspfNotifyLevel notifyLevel)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfNotify(notifyLevel) << msg << "\n";
}

void rspfSetError( const char* /* className */,
                    rspf_int32 /* error */,
                    const char *fmtString, ...)
{
   // NOTE: This code has an infinite loop in it!!! (drb)
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   va_list args;
   
   va_start(args, fmtString);
   rspfString result = rspfErrorV(fmtString, args );
   va_end(args);
   rspfNotify(rspfNotifyLevel_WARN) << result << "\n";
}

void rspfSetInfo( const char* /* className */,
                   const char *fmtString, ...)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   va_list args;

   va_start(args, fmtString);
   rspfString result = rspfErrorV(fmtString, args );
   va_end(args);
   rspfNotify(rspfNotifyLevel_WARN) << result << "\n";
}
