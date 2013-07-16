//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfContainerEvent.cpp 19965 2011-08-16 18:12:15Z gpotts $

#include <rspf/base/rspfContainerEvent.h>

RTTI_DEF1(rspfContainerEvent, "rspfContainerEvent", rspfEvent);

rspfContainerEvent::rspfContainerEvent(rspfObject* obj,
                                         long id)
   :rspfEvent(obj, id)
{
}

void rspfContainerEvent::setObjectList(rspfObject* obj)
{
   m_objectList.clear();
   m_objectList.push_back(obj);
}

void rspfContainerEvent::setObjectList(ObjectList& objects)
{
   m_objectList = objects;
}
