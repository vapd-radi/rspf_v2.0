//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfImageAoiListener.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/base/rspfImageAoiListener.h>
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfImageRectangleEvent.h>
#include <rspf/base/rspfImagePolygonEvent.h>

void rspfImageAoiListener::processEvent(rspfEvent& event)
{
   switch(event.getId())
   {
   case RSPF_EVENT_AOI_RECTANGLE_ID:
   {
      rspfImageRectangleEvent* eventCast = static_cast<rspfImageRectangleEvent*>(&event);
      imageRectangleEvent(*eventCast);
      break;
   }
   case RSPF_EVENT_AOI_POLYGON_ID:
   {
      rspfImagePolygonEvent* eventCast = static_cast<rspfImagePolygonEvent*>(&event);
      imagePolygonEvent(*eventCast);
      break;
   }
   default:
   {
      rspfListener::processEvent(event);
      break;
   }
   }
}
