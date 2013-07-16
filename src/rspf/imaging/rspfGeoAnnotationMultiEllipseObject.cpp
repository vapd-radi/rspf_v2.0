//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationMultiEllipseObject.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <rspf/imaging/rspfGeoAnnotationMultiEllipseObject.h>
#include <rspf/imaging/rspfAnnotationMultiEllipseObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>

rspfGeoAnnotationMultiEllipseObject::rspfGeoAnnotationMultiEllipseObject()
   :rspfGeoAnnotationObject(),
    theProjectedObject(new rspfAnnotationMultiEllipseObject()),
    theWidthHeight(1,1),
    theFillFlag(false)
{
   theProjectedObject->setFillFlag(false);
}

rspfGeoAnnotationMultiEllipseObject::rspfGeoAnnotationMultiEllipseObject(const std::vector<rspfGpt>& pointList,
                                                                           const rspfDpt& widthHeight,
                                                                           bool enableFill,
                                                                           unsigned char r,
                                                                           unsigned char g,
                                                                           unsigned char b,
                                                                           long thickness)
   :rspfGeoAnnotationObject(r, g, b, thickness),
    theProjectedObject(new rspfAnnotationMultiEllipseObject(widthHeight,enableFill, r, g, b, thickness )),
    thePointList(pointList),
    theWidthHeight(widthHeight),
    theFillFlag(enableFill)
{
   
}

rspfGeoAnnotationMultiEllipseObject::rspfGeoAnnotationMultiEllipseObject(const rspfDpt& widthHeight,
                                                                           bool enableFill,
                                                                           unsigned char r,
                                                                           unsigned char g,
                                                                           unsigned char b,
                                                                           long thickness)
   :rspfGeoAnnotationObject(r, g, b,thickness),
    theProjectedObject(new rspfAnnotationMultiEllipseObject(widthHeight,enableFill, r, g, b, thickness )),
    theWidthHeight(widthHeight),
    theFillFlag(enableFill)
{
}

rspfGeoAnnotationMultiEllipseObject::rspfGeoAnnotationMultiEllipseObject(const rspfGeoAnnotationMultiEllipseObject& rhs)
   :rspfGeoAnnotationObject(rhs),
    theProjectedObject((rspfAnnotationMultiEllipseObject*)rhs.theProjectedObject->dup()),
    thePointList(rhs.thePointList),
    theWidthHeight(rhs.theWidthHeight),
    theFillFlag(rhs.theFillFlag)
{
}
   
void rspfGeoAnnotationMultiEllipseObject::transform(
   rspfImageGeometry* projection)
{
   const std::vector<rspfGpt>::size_type BOUNDS = thePointList.size();
   theProjectedObject->resize((rspf_uint32)BOUNDS);
   for(std::vector<rspfGpt>::size_type i = 0; i < BOUNDS; ++i)
   {
      projection->worldToLocal(thePointList[(int)i], (*theProjectedObject)[(int)i]);
      
   }
   computeBoundingRect();
}

void rspfGeoAnnotationMultiEllipseObject::setFillFlag(bool fillFlag)
{
   theProjectedObject->setFillFlag(fillFlag);
   theFillFlag = fillFlag;
}

void rspfGeoAnnotationMultiEllipseObject::setColor(unsigned char r,
                                                    unsigned char g,
                                                    unsigned char b)
{
   rspfAnnotationObject::setColor(r,g,b);
   if(theProjectedObject)
   {
      theProjectedObject->setColor(r,g,b);
   }
}

void rspfGeoAnnotationMultiEllipseObject::setThickness(rspf_uint8 thickness)
{
   rspfAnnotationObject::setThickness(thickness);
   if(theProjectedObject)
   {
      theProjectedObject->setThickness(thickness);
   }
}


void rspfGeoAnnotationMultiEllipseObject::applyScale(double /* x */,
                                                      double /* y */)
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationMultiEllipseObject::applyScale NOT IMPLEMENTED"
      << endl;
}

void rspfGeoAnnotationMultiEllipseObject::draw(rspfRgbImage& anImage)const
{
   theProjectedObject->draw(anImage); 
}

bool rspfGeoAnnotationMultiEllipseObject::intersects(const rspfDrect& rect)const
{
   return theProjectedObject->intersects(rect);
}

void rspfGeoAnnotationMultiEllipseObject::setWidthHeight(const rspfDpt& widthHeight)
{
   theWidthHeight = widthHeight;
   theProjectedObject->setWidthHeight(widthHeight);
}

std::ostream& rspfGeoAnnotationMultiEllipseObject::print(std::ostream& out)const
{
   theProjectedObject->print(out);
   return out;
}

void rspfGeoAnnotationMultiEllipseObject::getBoundingRect(rspfDrect& rect)const
{
   theProjectedObject->getBoundingRect(rect);
}

void rspfGeoAnnotationMultiEllipseObject::computeBoundingRect()
{
   theProjectedObject->computeBoundingRect();
}

rspfObject* rspfGeoAnnotationMultiEllipseObject::dup()const
{
   return new rspfGeoAnnotationMultiEllipseObject(*this);
}

rspfAnnotationObject* rspfGeoAnnotationMultiEllipseObject::getNewClippedObject(
   const rspfDrect& /* rect */)const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfGeoAnnotationMultiEllipseObject::getNewClippedObject "
      << "NOT IMPLEMENTED" << endl;
   return (rspfAnnotationObject*)dup();
}

void rspfGeoAnnotationMultiEllipseObject::addPoint(const rspfGpt& point)
{
   thePointList.push_back(point);
}

void rspfGeoAnnotationMultiEllipseObject::setPoint(int i,
                                                    const rspfGpt& point)
{
   thePointList[i] = point;
}

void rspfGeoAnnotationMultiEllipseObject::resize(rspf_uint32 newSize)
{
   if(newSize)
   {
      thePointList.resize(newSize);
   }
}
