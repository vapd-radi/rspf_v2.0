//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfEvent.cpp 13362 2008-08-01 14:02:32Z gpotts $
#include <rspf/base/rspfEvent.h>

RTTI_DEF1(rspfEvent, "rspfEvent", rspfObject);


rspfEvent::rspfEvent(rspfObject* object, long id)
:
rspfObject(),
theObject(object),
theCurrentObject(object),
theId(id),
theIsConsumedFlag(false),
thePropagationType(PROPAGATION_NONE)
{
}

rspfEvent::rspfEvent(const rspfEvent& rhs)
:
rspfObject(),
theObject(rhs.theObject),
theCurrentObject(rhs.theCurrentObject),
theId(rhs.theId),
theIsConsumedFlag(rhs.theIsConsumedFlag),
thePropagationType(PROPAGATION_NONE)

{
}

long rspfEvent::getId() const
{
   return theId;
}

void rspfEvent::setId(long id)
{
   theId = id;
}

bool rspfEvent::isConsumed() const
{
   return theIsConsumedFlag;
}

void rspfEvent::setConsumedFlag(bool flag)
{
   theIsConsumedFlag = flag;
}

void rspfEvent::consume()
{
   setConsumedFlag(true);
}

const rspfObject* rspfEvent::getObject() const
{
   return theObject;
}

rspfObject* rspfEvent::getObject()
{
   return theObject;
}

const rspfObject* rspfEvent::getCurrentObject() const
{
   return theCurrentObject;
}

rspfObject* rspfEvent::getCurrentObject()
{
   return theCurrentObject;
}

void rspfEvent::setObject(rspfObject* object)
{
   theObject = object;
}

void rspfEvent::setCurrentObject(rspfObject* object)
{
   theCurrentObject = object;
}

void rspfEvent::setPropagationType(PropagationType type)
{
   thePropagationType = type;
}

bool rspfEvent::isPropagatingToOutputs()const
{
   return thePropagationType&PROPAGATION_OUTPUT;
}

bool rspfEvent::isPropagatingToInputs()const
{
   return thePropagationType&PROPAGATION_INPUT;
}

