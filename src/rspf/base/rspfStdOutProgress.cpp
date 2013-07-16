//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfStdOutProgress.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <iomanip>
#include <rspf/base/rspfStdOutProgress.h>

RTTI_DEF1(rspfStdOutProgress, "rspfStdOutProgress", rspfProcessListener);

rspfStdOutProgress theStdOutProgress;

rspfStdOutProgress::rspfStdOutProgress(rspf_uint32 precision,
                                         bool flushStream)
   :
      rspfProcessListener(),
      thePrecision(precision),
      theFlushStreamFlag(flushStream)
{
}

void rspfStdOutProgress::processProgressEvent(rspfProcessProgressEvent& event)
{
   if (event.getOutputMessageFlag())
   {
      rspfString s;
      event.getMessage(s);
      if (!s.empty())
      {
		  rspfNotify(rspfNotifyLevel_NOTICE) << s.c_str() << std::endl;
      }
      return; // Don't output percentage on a message update.
   }

   
   double p = event.getPercentComplete();
   rspfNotify(rspfNotifyLevel_NOTICE)
	   << std::setiosflags(std::ios::fixed)
      << std::setprecision(thePrecision)
      << p << "%\r";
   
   if(theFlushStreamFlag)
   {
      (p != 100.0) ?
         rspfNotify(rspfNotifyLevel_NOTICE).flush() :
         rspfNotify(rspfNotifyLevel_NOTICE) << "\n";
   }
}

void rspfStdOutProgress::setFlushStreamFlag(bool flag)
{
   theFlushStreamFlag = flag;
}

