//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt.
//
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfImagePolygonEvent.h 13016 2008-06-10 16:06:58Z dburken $
#ifndef rspfImagePolygonEvent_HEADER
#define rspfImagePolygonEvent_HEADER
#include <vector>
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>


class RSPFDLLEXPORT rspfImagePolygonEvent : public rspfEvent
{
public:
   rspfImagePolygonEvent(const std::vector<rspfIpt>& polygon,
                          rspfObject* obj=0)
      : rspfEvent(obj,RSPF_EVENT_AOI_POLYGON_ID) ,
        thePolygon(polygon)
      {
      }
      
   rspfImagePolygonEvent(const std::vector<rspfDpt>& polygon,
                          rspfObject* obj=0)
      : rspfEvent(obj,RSPF_EVENT_AOI_POLYGON_ID),
        thePolygon(polygon.size())
      {
         for (std::vector<rspfDpt>::size_type i = 0; i < polygon.size(); ++i)
         {
            thePolygon[i] = polygon[i];
         }
      }
   virtual rspfObject* dup()const
      {
         return new rspfImagePolygonEvent(*this);
      }
   
   const std::vector<rspfIpt>& getpolygon()const
      {
         return thePolygon;
      }
   const std::vector<rspfIpt>& getPolygon()const
      {
         return thePolygon;
      }
   void setPolygon(const std::vector<rspfIpt>& polygon)
      {
         thePolygon = polygon;
      }
   void setPolygon(const std::vector<rspfDpt>& polygon)
      {
         thePolygon.resize(polygon.size());
         for (std::vector<rspfDpt>::size_type i = 0; i < polygon.size(); ++i)
         {
            thePolygon[i] = polygon[i];
         }
      }
   
protected:
   std::vector<rspfIpt> thePolygon;
   
};

#endif
