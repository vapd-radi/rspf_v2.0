//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfGeoAnnotationLineObject.cpp 15766 2009-10-20 12:37:09Z gpotts $
#include <rspf/imaging/rspfGeoAnnotationLineObject.h>
#include <rspf/imaging/rspfAnnotationLineObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>


RTTI_DEF1(rspfGeoAnnotationLineObject,
          "rspfGeoAnnotationLineObject",
          rspfGeoAnnotationObject)
   
rspfGeoAnnotationLineObject::rspfGeoAnnotationLineObject(const rspfGpt& start,
                                                           const rspfGpt& end,
                                                           unsigned char r,
                                                           unsigned char g,
                                                           unsigned char b,
                                                           long thickness)
   :rspfGeoAnnotationObject(r, g, b, thickness),
    theProjectedLineObject(0),
    theStart(start),
    theEnd(end)
{
   theProjectedLineObject = new rspfAnnotationLineObject(rspfDpt(0,0),
                                                          rspfDpt(0,0),
                                                          r,
                                                          g,
                                                          b,
                                                          thickness);
}

rspfGeoAnnotationLineObject::rspfGeoAnnotationLineObject(const rspfGeoAnnotationLineObject& rhs)
   :rspfGeoAnnotationObject(rhs),
    theProjectedLineObject(rhs.theProjectedLineObject?(rspfAnnotationLineObject*)rhs.theProjectedLineObject->dup():(rspfAnnotationLineObject*)0),
    theStart(rhs.theStart),
    theEnd(rhs.theEnd)
{
   
}


rspfGeoAnnotationLineObject::~rspfGeoAnnotationLineObject()
{
   if(theProjectedLineObject)
   {
      delete theProjectedLineObject;
      theProjectedLineObject = 0;
   }
}

rspfObject* rspfGeoAnnotationLineObject::dup()const
{
   return new rspfGeoAnnotationLineObject(*this);
}

void rspfGeoAnnotationLineObject::applyScale(double x, double y)
{
   theStart.lond(theStart.lond()*x);
   theStart.latd(theStart.latd()*y);
   theEnd.lond(theEnd.lond()*x);
   theEnd.latd(theEnd.latd()*y);

   if(theProjectedLineObject)
   {
      theProjectedLineObject->applyScale(x, y);
   }
}

void rspfGeoAnnotationLineObject::transform(rspfImageGeometry* projection)
{
   if(projection)
   {
      rspfDpt projectedStart;
      rspfDpt projectedEnd;
      
      projection->worldToLocal(theStart, projectedStart);
      projection->worldToLocal(theEnd,   projectedEnd);

      theProjectedLineObject->setLine(projectedStart, projectedEnd);
   }
}

std::ostream& rspfGeoAnnotationLineObject::print(std::ostream& out)const
{
   out << "start ground:     " << theStart << endl;
   out << "end ground:       " << theEnd   << endl;
   return out;
}

void rspfGeoAnnotationLineObject::draw(rspfRgbImage& anImage)const
{
   theProjectedLineObject->draw(anImage);
}

bool rspfGeoAnnotationLineObject::intersects(const rspfDrect& rect)const
{
   if(theProjectedLineObject)
   {
      return theProjectedLineObject->intersects(rect);
   }

   return false;
}

rspfAnnotationObject* rspfGeoAnnotationLineObject::getNewClippedObject(const rspfDrect& rect)const
{
   if(intersects(rect))
   {
      if(theProjectedLineObject)
      {
         return theProjectedLineObject->getNewClippedObject(rect);
      }
   }
   
   return (rspfAnnotationObject*)0;
}

void rspfGeoAnnotationLineObject::getBoundingRect(rspfDrect& rect)const
{
   theProjectedLineObject->getBoundingRect(rect);
}

void rspfGeoAnnotationLineObject::computeBoundingRect()
{
   theProjectedLineObject->computeBoundingRect();
}
