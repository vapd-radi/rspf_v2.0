//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id

#include <rspf/base/rspfDisplayEventListener.h>
#include <rspf/base/rspfDisplayListEvent.h>
#include <rspf/base/rspfDisplayRefreshEvent.h>

RTTI_DEF1(rspfDisplayEventListener, "rspfDisplayEventListener", rspfListener);

rspfDisplayEventListener::rspfDisplayEventListener()
   : rspfListener()
{
}

rspfDisplayEventListener::~rspfDisplayEventListener()
{
}

void rspfDisplayEventListener::processEvent(rspfEvent& event)
{
   rspfDisplayListEvent* displayListEvtPtr = PTR_CAST(rspfDisplayListEvent, &event);

   if(displayListEvtPtr)
   {
      displayListEvent(*displayListEvtPtr);
   }

   rspfDisplayRefreshEvent* displayRefreshEvt = PTR_CAST(rspfDisplayRefreshEvent, &event);
   if(displayRefreshEvt)
   {
      displayRefreshEvent(*displayRefreshEvt);
   }
   
}

void rspfDisplayEventListener::displayListEvent(rspfDisplayListEvent&)
{
}

void rspfDisplayEventListener::displayRefreshEvent(rspfDisplayRefreshEvent&)
{
}
