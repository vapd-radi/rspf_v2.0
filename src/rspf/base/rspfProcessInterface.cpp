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
// $Id: rspfProcessInterface.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <ostream>

#include <rspf/base/rspfProcessInterface.h>
#include <rspf/base/rspfListenerManager.h>

RTTI_DEF(rspfProcessInterface, "rspfProcessInterface");

rspfProcessInterface::rspfProcessInterface()
   :thePercentComplete(0.0),
    theProcessStatus(PROCESS_STATUS_NOT_EXECUTING),
    theMessage(""),
    theEventFlag(true)
{
}

rspfProcessInterface::~rspfProcessInterface()
{
}

void rspfProcessInterface::abort()
{
   if(theProcessStatus == PROCESS_STATUS_EXECUTING)
   {
      setProcessStatus(PROCESS_STATUS_ABORT_REQUEST);
   }
}

bool rspfProcessInterface::needsAborting() const
{
   return ( isAborted()|| isAbortRequested() );
}

bool rspfProcessInterface::isAbortRequested()const
{
   return (theProcessStatus == PROCESS_STATUS_ABORT_REQUEST);
}

bool rspfProcessInterface::isAborted()const
{
   return (theProcessStatus==PROCESS_STATUS_ABORTED);
}

bool rspfProcessInterface::isExecuting()const
{
   return ((theProcessStatus==PROCESS_STATUS_EXECUTING)||
           (theProcessStatus==PROCESS_STATUS_ABORT_REQUEST));
}

rspfProcessInterface::rspfProcessStatus rspfProcessInterface::getProcessStatus()const
{
   return theProcessStatus;
}

void rspfProcessInterface::setProcessStatus(rspfProcessStatus processStatus)
{
   theProcessStatus = processStatus;
}

double rspfProcessInterface::getPercentComplete()const
{
   return thePercentComplete;
}

void rspfProcessInterface::setPercentComplete(double percentComplete)
{
   thePercentComplete = percentComplete;

   rspfListenerManager* manager = getManager();
   if(theEventFlag&&manager)
   {
      rspfProcessProgressEvent event(getObject(),
                                      thePercentComplete,
                                      theMessage,
                                      false);
      manager->fireEvent(event);
   }
}

rspfListenerManager* rspfProcessInterface::getManager()
{
   return PTR_CAST(rspfListenerManager, getObject());
}

void rspfProcessInterface::enableEvents()
{
   theEventFlag = true;
}
   
void rspfProcessInterface::disableEvents()
{
   theEventFlag = false;
}

void rspfProcessInterface::setCurrentMessage(const rspfString& message)
{
   theMessage = message;

   rspfListenerManager* manager = getManager();
   if(theEventFlag&&manager)
   {
      rspfProcessProgressEvent event(getObject(),
                                      thePercentComplete,
                                      theMessage,
                                      true);
      manager->fireEvent(event);
   }   
}

std::ostream& rspfProcessInterface::print(std::ostream& out) const
{
   out << "process status: ";
   if(theProcessStatus == PROCESS_STATUS_EXECUTING)
   {
      out << "executing" << std::endl;
   }
   else if(theProcessStatus == PROCESS_STATUS_ABORTED)
   {
      out << "aborted" << std::endl;
   }
   else if(theProcessStatus == PROCESS_STATUS_NOT_EXECUTING)
   {
      out << "not executing" << std::endl;
   }
   out << "percent_complete: " << thePercentComplete;
   
   return out;
}

std::ostream& operator<<(std::ostream& out, const rspfProcessInterface& data)
{
   return data.print(out);
}
