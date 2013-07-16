//----------------------------------------------------------------------------
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
// $Id: rspfProcessListener.cpp 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------

#include <rspf/base/rspfProcessListener.h>
#include <rspf/base/rspfProcessProgressEvent.h>

RTTI_DEF1(rspfProcessListener, "rspfProcessListener", rspfListener);

rspfProcessListener::rspfProcessListener()
   : rspfListener()
{}

rspfProcessListener::~rspfProcessListener()
{}

void rspfProcessListener::processEvent(rspfEvent& event)
{
   switch(event.getId())
   {
   case RSPF_EVENT_PROCESS_PROGRESS_ID:
   {
      rspfProcessProgressEvent* eventCast = static_cast<rspfProcessProgressEvent*>(&event);
      processProgressEvent(*eventCast);
      break;
   }
   default:
   {
      rspfListener::processEvent(event);
      break;
   }
   }
}

void rspfProcessListener::processProgressEvent(rspfProcessProgressEvent& /* event */ )
{}
