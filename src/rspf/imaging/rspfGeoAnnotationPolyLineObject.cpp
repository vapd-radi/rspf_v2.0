//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationPolyLineObject.cpp 15766 2009-10-20 12:37:09Z gpotts $
#include <rspf/imaging/rspfGeoAnnotationPolyLineObject.h>
#include <rspf/imaging/rspfAnnotationMultiLineObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>


RTTI_DEF1(rspfGeoAnnotationPolyLineObject,
          "rspfGeoAnnotationPolyLineObject",
          rspfGeoAnnotationObject)
   
rspfGeoAnnotationPolyLineObject::rspfGeoAnnotationPolyLineObject(
   const vector<rspfGpt>& groundPts,
   rspf_uint8 r,
   rspf_uint8 g,
   rspf_uint8 b,
   rspf_uint8 thickness)
   :rspfGeoAnnotationObject(r, g, b, thickness),
    theProjectedMultiLineObject(0)
{

   thePolygon = groundPts;

   // we will initialize the projected polygon's size
   // so we don't have to reset it every time we do a
   // projection
   //
   vector<rspfDpt> projectedPoints(thePolygon.size());
   
   theProjectedMultiLineObject = new rspfAnnotationMultiLineObject(rspfPolyLine(projectedPoints),
                                                                    r,
                                                                    g,
                                                                    b,
                                                                    thickness);
}

rspfGeoAnnotationPolyLineObject::rspfGeoAnnotationPolyLineObject(const rspfGeoAnnotationPolyLineObject& rhs)
   :rspfGeoAnnotationObject(rhs),
    thePolygon(rhs.thePolygon),
    theProjectedMultiLineObject(rhs.theProjectedMultiLineObject?(rspfAnnotationMultiLineObject*)rhs.theProjectedMultiLineObject->dup():(rspfAnnotationMultiLineObject*)NULL)
{
}

rspfGeoAnnotationPolyLineObject::~rspfGeoAnnotationPolyLineObject()
{
   if(theProjectedMultiLineObject)
   {
      delete theProjectedMultiLineObject;
      theProjectedMultiLineObject = 0;
   }
}

 rspfObject* rspfGeoAnnotationPolyLineObject::dup()const
{
   return new rspfGeoAnnotationPolyLineObject(*this);
}

void rspfGeoAnnotationPolyLineObject::applyScale(double x,
                                                   double y)
{
   for(int i = 0; i < (int)thePolygon.size(); ++i)
   {
      thePolygon[i].lond(thePolygon[i].lond()*x);
      thePolygon[i].latd(thePolygon[i].latd()*y);
   }
   if(theProjectedMultiLineObject)
   {
      theProjectedMultiLineObject->applyScale(x, y);
   }
      
}

void rspfGeoAnnotationPolyLineObject::transform(rspfImageGeometry* projection)
{
   // make sure it's not null
   if(!projection)
   {
      return;
   }
   vector<rspfPolyLine>& polyList =
      theProjectedMultiLineObject->getPolyLineList();

   if(polyList.size())
   {
      vector<rspfDpt>& poly = polyList[0].getVertexList();
      
      const std::vector<rspfGpt>::size_type BOUNDS = thePolygon.size();
      
      for(std::vector<rspfGpt>::size_type index=0; index < BOUNDS; ++index)
      {
         projection->worldToLocal(thePolygon[index], poly[index]);
      }
      
      // update the bounding rect
      theProjectedMultiLineObject->computeBoundingRect();
   }
}

std::ostream& rspfGeoAnnotationPolyLineObject::print(std::ostream& out)const
{
   out << "number_of_points:  " << thePolygon.size() << endl;
   if(thePolygon.size() > 0)
   {
      for(long index =0; index < (long)(thePolygon.size()-1); ++index)
      {
         out << thePolygon[index] << endl;
      }
      out << thePolygon[thePolygon.size()-1] << endl;
   }
   out << "Projected Polygon" << endl;
   theProjectedMultiLineObject->print(out);
   return out;
}

void rspfGeoAnnotationPolyLineObject::draw(rspfRgbImage& anImage)const
{
   theProjectedMultiLineObject->draw(anImage);
}

bool rspfGeoAnnotationPolyLineObject::intersects(const rspfDrect& rect)const
{
   if(theProjectedMultiLineObject)
   {
      return theProjectedMultiLineObject->intersects(rect);
   }
   
   return false;
}

rspfAnnotationObject* rspfGeoAnnotationPolyLineObject::getNewClippedObject(const rspfDrect& rect)const
{
   if(theProjectedMultiLineObject)
   {
      return theProjectedMultiLineObject->getNewClippedObject(rect);
   }
   
   return (rspfAnnotationObject*)NULL;;
}

void rspfGeoAnnotationPolyLineObject::getBoundingRect(rspfDrect& rect)const
{
   theProjectedMultiLineObject->getBoundingRect(rect);
}

void rspfGeoAnnotationPolyLineObject::computeBoundingRect()
{
   theProjectedMultiLineObject->computeBoundingRect();
}

void rspfGeoAnnotationPolyLineObject::setColor(rspf_uint8 r,
                                                rspf_uint8 g,
                                                rspf_uint8 b)
{
   if(theProjectedMultiLineObject)
   {
      theProjectedMultiLineObject->setColor(r, g, b);
   }
}

void rspfGeoAnnotationPolyLineObject::setThickness(rspf_uint8 thickness)
{
   if(theProjectedMultiLineObject)
   {
      theProjectedMultiLineObject->setThickness(thickness);
   }
}
