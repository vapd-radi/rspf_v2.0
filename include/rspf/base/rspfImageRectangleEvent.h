//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageRectangleEvent.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfImageRectangleEvent_HEADER
#define rspfImageRectangleEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfIrect.h>

class RSPFDLLEXPORT rspfImageRectangleEvent : public rspfEvent
{
public:
   rspfImageRectangleEvent(const rspfIrect& rect,
                            rspfObject* obj=NULL)
      : rspfEvent(obj,RSPF_EVENT_AOI_RECTANGLE_ID) ,
        theRectangle(rect)
      {
      }
   virtual rspfObject* dup()const
      {
         return new rspfImageRectangleEvent(*this);
      }
   
   const rspfIrect& getRectangle()const
      {
         return theRectangle;
      }
   void setRectangle(const rspfIrect& rect)
      {
         theRectangle = rect;
      }
   
protected:
   rspfIrect theRectangle;
   
};

#endif
