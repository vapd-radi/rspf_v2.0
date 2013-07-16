//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfMouseListener.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <rspf/base/rspfMouseListener.h>
#include <rspf/base/rspfMouseEvent.h>

RTTI_DEF1(rspfMouseListener,"rspfMouseListener",rspfListener);

void rspfMouseListener::processEvent(rspfEvent& event)
{
   rspfMouseEvent* mEvent = PTR_CAST(rspfMouseEvent, &event);

   if(mEvent)
   {
      mouseEvent(*mEvent);
   }
   else
   {
      rspfListener::processEvent(event);
   }
//    switch(event.getId())
//    {
//    case RSPF_MOUSE_EVENT_ID:
//    {
//        = static_cast<rspfMouseEvent*>(&event);
//       mouseEvent(*mEvent);
//       break;
//    }
//    default:
//    {
//       rspfListener::processEvent(event);
//    }
//    }
}
