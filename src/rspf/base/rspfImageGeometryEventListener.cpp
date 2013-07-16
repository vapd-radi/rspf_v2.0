//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageGeometryEventListener.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/base/rspfImageGeometryEventListener.h>

RTTI_DEF1(rspfImageGeometryEventListener,
          "rspfImageGeometryEventListener",
          rspfListener);

void rspfImageGeometryEventListener::processEvent(rspfEvent& event)
{
   switch(event.getId())
   {
   case  RSPF_EVENT_IMAGE_GEOMETRY_ID:
   {
      rspfImageGeometryEvent* tempCast = (rspfImageGeometryEvent*)(&event);
      imageGeometryEvent(*tempCast);
      
      break;
   }
   default:
   {
      rspfListener::processEvent(event);
   }
   }
}
