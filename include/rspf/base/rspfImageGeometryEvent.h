//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageGeometryEvent.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfImageGeometryEvent_HEADER
#define rspfImageGeometryEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>

class rspfImageGeometryEvent : public rspfEvent
{
public:
   rspfImageGeometryEvent(rspfObject* object=0,
                           long id=RSPF_EVENT_IMAGE_GEOMETRY_ID)
      :rspfEvent(object, id)
      {
      }
   
   rspfImageGeometryEvent(const rspfKeywordlist& geomKwl,
                           rspfObject* object=NULL,
                           long id=RSPF_EVENT_IMAGE_GEOMETRY_ID)
      :rspfEvent(object, id),
       theGeomKwl(geomKwl)
      {
      }
   rspfImageGeometryEvent(const rspfImageGeometryEvent& rhs)
      :rspfEvent(rhs),
       theGeomKwl(rhs.theGeomKwl)
      {
      }
   virtual ~rspfImageGeometryEvent(){}
   virtual rspfObject* dup()const
      {
         return new rspfImageGeometryEvent(*this);
      }
   void setGeometry(const rspfKeywordlist& geomKwl)
      {
         theGeomKwl = geomKwl;
      }
   void getGeometry(rspfKeywordlist& geomKwl)const
      {
         geomKwl = theGeomKwl;
      }
   const rspfKeywordlist& getGeometry()const
      {
         return theGeomKwl;
      }
   bool isGeometrySet()const
      {
         return (theGeomKwl.getSize() != 0);
      }
   void clearGeometry()
      {
         theGeomKwl.clear();
      }
protected:
   rspfKeywordlist theGeomKwl;

TYPE_DATA
};

#endif
