//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationMultiPolyObject.cpp 18435 2010-11-17 19:41:13Z gpotts $

#include <rspf/imaging/rspfGeoAnnotationMultiPolyObject.h>
#include <rspf/imaging/rspfAnnotationMultiPolyObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfPolyArea2d.h>

RTTI_DEF1(rspfGeoAnnotationMultiPolyObject, "rspfGeoAnnotationMultiPolyObject", rspfGeoAnnotationObject);

rspfGeoAnnotationMultiPolyObject::rspfGeoAnnotationMultiPolyObject()
   :
      rspfGeoAnnotationObject(),
      theMultiPolygon(),
      theBoundingRect(),
      theFillEnabled(false),
      theProjectedPolyObject(0)
{
   allocateProjectedPolygon();   
   theBoundingRect.makeNan();
}

rspfGeoAnnotationMultiPolyObject::rspfGeoAnnotationMultiPolyObject(const vector<rspfGeoPolygon>& multiPoly,
                                                                     bool enableFill,
                                                                     unsigned char r,
                                                                     unsigned char g,
                                                                     unsigned char b,
                                                                     long thickness)
   :
      rspfGeoAnnotationObject(r, g, b, thickness),
      theMultiPolygon(multiPoly),
      theBoundingRect(),
      theFillEnabled(enableFill),
      theProjectedPolyObject(0)
{
   allocateProjectedPolygon();   
   theBoundingRect.makeNan();
}

rspfGeoAnnotationMultiPolyObject::rspfGeoAnnotationMultiPolyObject(const rspfGeoAnnotationMultiPolyObject& rhs)
   :
      rspfGeoAnnotationObject(rhs),
      theMultiPolygon(rhs.theMultiPolygon),
      theBoundingRect(rhs.theBoundingRect),
      theFillEnabled(rhs.theFillEnabled),
      theProjectedPolyObject(rhs.theProjectedPolyObject.valid()?(rspfAnnotationMultiPolyObject*)rhs.theProjectedPolyObject->dup():0)
{
}

rspfObject* rspfGeoAnnotationMultiPolyObject::dup()const
{
   return new rspfGeoAnnotationMultiPolyObject(*this);
}

rspfGeoAnnotationMultiPolyObject::~rspfGeoAnnotationMultiPolyObject()
{
   theProjectedPolyObject = 0;
}

void rspfGeoAnnotationMultiPolyObject::applyScale(double /* x */,
                                                   double /* y */)
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationPolyObject::applyScale NOT IMPLEMENTED!!!!"
      << endl;
}

void rspfGeoAnnotationMultiPolyObject::transform(rspfImageGeometry* projection)
{
   if(!projection)
   {
      return;
   }
   
   allocateProjectedPolygon();

   //---
   // NOTE:
   // allocateProjectedPolygon() will set theProjectedPolyObject to 0 if
   // theMultiPolygon is empty (theMultiPolygon.size() == 0).  So check before
   // accessing pointer to avoid a core dump.
   //---
   if (!theProjectedPolyObject)
   {
      return;
   }

   rspfDpt temp;
   std::vector<rspfPolygon> visiblePolygons;
   rspfPolygon polygon;
   for(rspf_uint32 polyI = 0; polyI < theMultiPolygon.size(); ++polyI)
   {
      polygon.clear();
      for(rspf_uint32 pointI = 0;
          pointI < theMultiPolygon[polyI].size();
          ++pointI)
      {
         projection->worldToLocal(theMultiPolygon[polyI][pointI],
                                  temp);
         if(!temp.hasNans())
         {
            polygon.addPoint(temp);
         }
      }
      theProjectedPolyObject->addPolygon(polyI, polygon);
   }
   
   //---
   // Update the bounding rect.
   //---
   theProjectedPolyObject->computeBoundingRect();
}

std::ostream& rspfGeoAnnotationMultiPolyObject::print(std::ostream& out)const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationMultiPolyObject::print NOT IMPLEMENTED"
      << endl;
   return out;
}

bool rspfGeoAnnotationMultiPolyObject::intersects(const rspfDrect& /* rect */)const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationMultiPolyObject::intersects NOT IMPLEMENTED"
      << endl;
  return false;
}

rspfAnnotationObject* rspfGeoAnnotationMultiPolyObject::getNewClippedObject(
   const rspfDrect& /* rect */)const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationMultiPolyObject::getNewClippedObject "
      << "NOT IMPLEMENTED" << endl;
   
  return (rspfAnnotationObject*)(dup());
}

void rspfGeoAnnotationMultiPolyObject::draw(rspfRgbImage& anImage)const
{
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->draw(anImage);
   }
}

void rspfGeoAnnotationMultiPolyObject::getBoundingRect(rspfDrect& rect)const
{
   //---
   // Should we make non const and call computeBoundingRect if theBoundingRect
   // is nan? (drb - 20100728)
   //---
   rect = theBoundingRect;
   if (rect.isNan()&&theProjectedPolyObject.valid())
   {
     theProjectedPolyObject->getBoundingRect(rect);
   }
}

void rspfGeoAnnotationMultiPolyObject::addPoint(rspf_uint32 polygonIndex,
                                                 const rspfGpt& pt)
{
   if(polygonIndex < theMultiPolygon.size())
   {
      theMultiPolygon[polygonIndex].addPoint(pt);
      
      // we will have to reset the projected polygon
      theProjectedPolyObject = 0;
   }
}

void rspfGeoAnnotationMultiPolyObject::setMultiPolygon(
   const vector<rspfGeoPolygon>& multiPoly)
{
   theMultiPolygon = multiPoly;
   theProjectedPolyObject = 0;
}

void rspfGeoAnnotationMultiPolyObject::setColor(unsigned char r,
                                                 unsigned char g,
                                                 unsigned char b)
{
   rspfAnnotationObject::setColor(r, g, b);
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->setColor(r, g, b);
   }
}

void rspfGeoAnnotationMultiPolyObject::setThickness(rspf_uint8 thickness)
{
   rspfAnnotationObject::setThickness(thickness);
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->setThickness(thickness);
   }
}

void rspfGeoAnnotationMultiPolyObject::computeBoundingRect()
{
   theBoundingRect.makeNan();
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->computeBoundingRect();
      theProjectedPolyObject->getBoundingRect(theBoundingRect);
   }
}

bool rspfGeoAnnotationMultiPolyObject::isPointWithin(const rspfDpt& imagePoint)const
{
   if(theProjectedPolyObject.valid())
   {
      return theProjectedPolyObject->isPointWithin(imagePoint);
   }

   return false;
}

void rspfGeoAnnotationMultiPolyObject::setFillFlag(bool flag)
{
   theFillEnabled = flag;
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->setFillFlag(flag);
   }
}

void rspfGeoAnnotationMultiPolyObject::allocateProjectedPolygon()
{
   theProjectedPolyObject = 0;
   
   if(theMultiPolygon.size())
   {
      vector<rspfPolygon> polyList( theMultiPolygon.size() );
      theProjectedPolyObject =
         new rspfAnnotationMultiPolyObject(polyList,
                                            theFillEnabled,
                                            theRed,
                                            theGreen,
                                            theBlue,
                                            theThickness);
   }
}
