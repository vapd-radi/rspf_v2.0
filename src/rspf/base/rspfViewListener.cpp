//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfViewListener.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/base/rspfViewListener.h>

RTTI_DEF1(rspfViewListener, "rspfViewListener", rspfListener);
void rspfViewListener::processEvent(rspfEvent& event)
{
   rspfViewEvent* evt = PTR_CAST(rspfViewEvent, &event);

   if(evt)
   {
      switch(evt->getViewEventType())
      {
      case  rspfViewEvent::RSPF_VIEW_EVENT_SCALE_CHANGE:
      {
         viewScaleChangeEvent(*evt);
         break;
      }
      case  rspfViewEvent::RSPF_VIEW_EVENT_TYPE_GENERIC:
      {
         viewEvent(*evt);
         break;
      }
      case rspfViewEvent::RSPF_VIEW_EVENT_TRANSFORM_CHANGE:
      {
         viewTransformChangeEvent(*evt);
         break;
      }
      case rspfViewEvent::RSPF_VIEW_EVENT_VIEW_TYPE_CHANGE:
      {
         viewTypeChangeEvent(*evt);
         break;
      }
      }
      allViewEvents(*evt);
   }
}
