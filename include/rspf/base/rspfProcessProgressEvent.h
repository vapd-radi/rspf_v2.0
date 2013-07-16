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
// $Id: rspfProcessProgressEvent.h 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------
#ifndef rspfProcessProgressEvent_HEADER
#define rspfProcessProgressEvent_HEADER

#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfString.h>

class RSPFDLLEXPORT rspfProcessProgressEvent : public rspfEvent
{
public:
   rspfProcessProgressEvent(rspfObject* owner=NULL,
                             double percentComplete=0.0,
                             const rspfString message="",
                             bool outputMessageFlag=false);

   virtual rspfObject* dup()const;
   
   double getPercentComplete()const;
   
   rspfString getMessage()const;

   void getMessage(rspfString& message)const;
   
   void setPercentComplete(double percentComplete);
   
   void setMessage(const rspfString& message);

   void setOutputMessageFlag(bool flag);

   bool getOutputMessageFlag() const;
      
protected:
   double      thePercentComplete;
   rspfString theMessage;
   bool        theOutputMessageFlag;

TYPE_DATA
};
#endif
