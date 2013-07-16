//----------------------------------------------------------------------------
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Event for process progress.
//
// $Id: rspfProcessProgressEvent.cpp 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------
#include <rspf/base/rspfProcessProgressEvent.h>

RTTI_DEF1(rspfProcessProgressEvent, "rspfProcessProgressEvent", rspfEvent);

rspfProcessProgressEvent::rspfProcessProgressEvent(rspfObject* owner,
                                                     double percentComplete,
                                                     const rspfString message,
                                                     bool outputMessageFlag)
   :
      rspfEvent(owner, RSPF_EVENT_PROCESS_PROGRESS_ID),
      thePercentComplete(percentComplete),
      theMessage(message),
      theOutputMessageFlag(outputMessageFlag)
{
}

rspfObject* rspfProcessProgressEvent::dup()const
{
   return new rspfProcessProgressEvent(*this);
}

double rspfProcessProgressEvent::getPercentComplete()const
{
   return thePercentComplete;
}
   
rspfString rspfProcessProgressEvent::getMessage()const
{
   return theMessage;
}

void rspfProcessProgressEvent::getMessage(rspfString& message)const
{
   message = theMessage;
}
   
void rspfProcessProgressEvent::setPercentComplete(double percentComplete)
{
   thePercentComplete = percentComplete;
}

void rspfProcessProgressEvent::setMessage(const rspfString& message)
{
   theMessage = message;
}

void rspfProcessProgressEvent::setOutputMessageFlag(bool flag)
{
   theOutputMessageFlag = flag;
}

bool rspfProcessProgressEvent::getOutputMessageFlag() const
{
   return theOutputMessageFlag;
}
