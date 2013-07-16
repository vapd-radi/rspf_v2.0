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
// $Id: rspfConnectionEvent.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfConnectionEvent_HEADER
#define rspfConnectionEvent_HEADER

#include <vector>

#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfConnectableObject.h>

class RSPFDLLEXPORT rspfConnectionEvent : public rspfEvent
{
public:
   enum rspfConnectionDirectionType
   {
      RSPF_DIRECTION_UNKNOWN  = 0,
      RSPF_INPUT_DIRECTION    = 1,
      RSPF_OUTPUT_DIRECTION   = 2,
      RSPF_INPUT_OUTPUT_DIRECTION = 3
   };
   
   rspfConnectionEvent(rspfObject* object=NULL,
                        long id=RSPF_EVENT_NULL_ID);

   rspfConnectionEvent(rspfObject* object,
                        long id,
                        const rspfConnectableObject::ConnectableObjectList& newList,
                        const rspfConnectableObject::ConnectableObjectList& oldList,
                        rspfConnectionDirectionType whichDirection);

   rspfConnectionEvent(rspfObject* object,
                        long id,
                        rspfConnectableObject* newConnectableObject,
                        rspfConnectableObject* oldConnectableObject,
                        rspfConnectionDirectionType whichDirection);
   
   rspfConnectionEvent(const rspfConnectionEvent& rhs);

   rspfObject* dup()const;
   
   virtual void setDirection(rspfConnectionDirectionType direction);
   
   virtual rspfConnectionDirectionType getDirection()const;

   virtual rspf_uint32 getNumberOfNewObjects()const;
   
   virtual rspf_uint32 getNumberOfOldObjects()const;
   
   virtual rspfConnectableObject* getOldObject(rspf_uint32 i=0);

   virtual rspfConnectableObject* getNewObject(rspf_uint32 i=0);

   virtual bool isDisconnect()const;
   
   virtual bool isConnect()const;
      
   virtual bool isInputDirection()const;
   
   virtual bool isOutputDirection()const;
   
protected:
   rspfConnectableObject::ConnectableObjectList theNewObjectList;
   rspfConnectableObject::ConnectableObjectList theOldObjectList;
   rspfConnectionDirectionType         theDirectionType;

TYPE_DATA
};

#endif
