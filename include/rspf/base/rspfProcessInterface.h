//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfProcessInterface.h 14789 2009-06-29 16:48:14Z dburken $
#ifndef rspfProcessInterface_HEADER
#define rspfProcessInterface_HEADER
#include <iosfwd>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/base/rspfListenerManager.h>

class rspfObject;
class rspfListenerManager;

class RSPFDLLEXPORT rspfProcessInterface
{
public:

   enum rspfProcessStatus
   {
      PROCESS_STATUS_UNKNOWN       = 0,
      PROCESS_STATUS_EXECUTING     = 1,
      PROCESS_STATUS_ABORTED       = 2,
      PROCESS_STATUS_ABORT_REQUEST = 3,
      PROCESS_STATUS_NOT_EXECUTING = 4
   };
   
   rspfProcessInterface();
   
   virtual ~rspfProcessInterface();
   
   virtual rspfObject* getObject()=0;
   virtual const rspfObject* getObject()const=0;
   virtual bool execute()=0;
   
   virtual void abort();

   bool needsAborting()const;

   bool isAbortRequested()const;

   bool isAborted()const;

   bool isExecuting()const;

   rspfProcessStatus getProcessStatus()const;

   virtual void setProcessStatus(rspfProcessStatus processStatus);

   virtual double getPercentComplete()const;

   virtual void setPercentComplete(double percentComplete);

   virtual rspfListenerManager* getManager();

   void enableEvents();
   
   void disableEvents();

   void setCurrentMessage(const rspfString& message);

   /*!
    *  Called by friend operator<< function, derived classes should override
    *  if something different is desired.
    */
   virtual std::ostream& print(std::ostream& out) const;
   
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfProcessInterface& data);
   
protected:
   double              thePercentComplete;
   rspfProcessStatus  theProcessStatus;
   rspfString         theMessage;
   bool                theEventFlag;

TYPE_DATA
};

#endif
