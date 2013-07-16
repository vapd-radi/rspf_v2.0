//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfConnectionEvent.cpp 15833 2009-10-29 01:41:53Z eshirschorn $

#include <rspf/base/rspfConnectionEvent.h>

RTTI_DEF1(rspfConnectionEvent, "rspfConnectionEvent", rspfEvent);

rspfConnectionEvent::rspfConnectionEvent(rspfObject* object, long id)
   :rspfEvent(object, id),
    theDirectionType(RSPF_DIRECTION_UNKNOWN)
{
}

rspfConnectionEvent::rspfConnectionEvent(
   rspfObject* object,
   long id,
   const rspfConnectableObject::ConnectableObjectList& newList,
   const rspfConnectableObject::ConnectableObjectList& oldList,
   rspfConnectionDirectionType whichDirection)
   : rspfEvent(object, id),
     theNewObjectList(newList),
     theOldObjectList(oldList),
     theDirectionType(whichDirection)
{
}

rspfConnectionEvent::rspfConnectionEvent(
   rspfObject* object,
   long id,
   rspfConnectableObject* newConnectableObject,
   rspfConnectableObject* oldConnectableObject,
   rspfConnectionDirectionType whichDirection)
   : rspfEvent(object, id),
     theDirectionType(whichDirection)
{
   if(newConnectableObject)
   {
      theNewObjectList.push_back(newConnectableObject);
   }
   if(oldConnectableObject)
   {
      theOldObjectList.push_back(oldConnectableObject);
   }
}
   
rspfConnectionEvent::rspfConnectionEvent(const rspfConnectionEvent& rhs)
   : rspfEvent(rhs),
     theNewObjectList(rhs.theNewObjectList),
     theOldObjectList(rhs.theOldObjectList),
     theDirectionType(rhs.theDirectionType)
{
}

rspfObject* rspfConnectionEvent::dup()const
{
   return new rspfConnectionEvent(*this);
}

void rspfConnectionEvent::setDirection(
   rspfConnectionDirectionType direction)
{
   theDirectionType   = direction;
}

rspfConnectionEvent::rspfConnectionDirectionType rspfConnectionEvent::getDirection()const
{
   return theDirectionType;
}

rspf_uint32 rspfConnectionEvent::getNumberOfNewObjects()const
{
   return (rspf_uint32)theNewObjectList.size();
}

rspf_uint32 rspfConnectionEvent::getNumberOfOldObjects()const
{
   return (rspf_uint32)theOldObjectList.size();
}

rspfConnectableObject* rspfConnectionEvent::getOldObject(rspf_uint32 i)
{
   if(i < getNumberOfOldObjects())
   {
      return theOldObjectList[i].get();
   }
   
   return (rspfConnectableObject*)NULL;
}

rspfConnectableObject* rspfConnectionEvent::getNewObject(rspf_uint32 i)
{
   if(i < getNumberOfNewObjects())
   {
      return theNewObjectList[i].get();
   }
   
   return (rspfConnectableObject*)NULL;
}

bool rspfConnectionEvent::isDisconnect()const
{
   return (getId()==RSPF_EVENT_CONNECTION_DISCONNECT_ID);
}

bool rspfConnectionEvent::isConnect()const
{
   return (getId()==RSPF_EVENT_CONNECTION_CONNECT_ID);
}

bool rspfConnectionEvent::isInputDirection()const
{
   return ((long)theDirectionType  & (long)RSPF_INPUT_DIRECTION);
}

bool rspfConnectionEvent::isOutputDirection()const
{
   return ((long)theDirectionType & RSPF_OUTPUT_DIRECTION);
}
