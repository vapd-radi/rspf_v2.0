//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationMultiEllipseObject.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <rspf/imaging/rspfAnnotationMultiEllipseObject.h>

rspfAnnotationMultiEllipseObject::rspfAnnotationMultiEllipseObject
   (const std::vector<rspfDpt>& pointList,
    const rspfDpt& widthHeight,
    bool enableFill,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    long thickness)
   :
         rspfAnnotationObject(r, g, b, thickness),
         thePointList(pointList),
         theWidthHeight(widthHeight),
         theFillFlag(enableFill)
{
   computeBoundingRect();
}

rspfAnnotationMultiEllipseObject::rspfAnnotationMultiEllipseObject
   (const rspfDpt& widthHeight,
    bool enableFill,
    unsigned char r,
    unsigned char g,
    unsigned char b,
    long thickness)
   :
         rspfAnnotationObject(r, g, b, thickness),
         theWidthHeight(widthHeight),
         theFillFlag(enableFill)
{
   theBoundingRect.makeNan();
}

rspfAnnotationMultiEllipseObject::rspfAnnotationMultiEllipseObject(const rspfAnnotationMultiEllipseObject& rhs)
      :thePointList(rhs.thePointList),
       theWidthHeight(rhs.theWidthHeight),
       theFillFlag(rhs.theFillFlag),
       theBoundingRect(rhs.theBoundingRect)
{
}

rspfObject* rspfAnnotationMultiEllipseObject::dup()const
{
   return new rspfAnnotationMultiEllipseObject(*this);
}

rspfAnnotationMultiEllipseObject::~rspfAnnotationMultiEllipseObject()
{}

rspfAnnotationObject* rspfAnnotationMultiEllipseObject::getNewClippedObject(
   const rspfDrect& /* rect */)const
{
   rspfNotify(rspfNotifyLevel_WARN)
      << "rspfAnnotationMultiEllipseObject::getNewClippedObject WARNING: Not implemented"
      << std::endl;
   return NULL;
}

void rspfAnnotationMultiEllipseObject::applyScale(double x, double y)
{
   int i;
   int upper = (int)thePointList.size();
   for(i = 0; i < upper; ++i)
   {
      thePointList[i].x *= x;
      thePointList[i].y *= y;
   }
   theWidthHeight.x *= x;
   theWidthHeight.y *= y;
   
   computeBoundingRect();
}

void rspfAnnotationMultiEllipseObject::draw(rspfRgbImage& anImage)const
{
   anImage.setDrawColor(theRed, theGreen, theBlue);
   anImage.setThickness(theThickness);
   rspfDrect imageRect = anImage.getImageData()->getImageRectangle();

   if(theBoundingRect.intersects(imageRect))
   {
      int i;
      int upper = (int)thePointList.size();
      if(theFillFlag)
      {
         for(i = 0; i < upper; ++i)
         {
            rspfDpt tempPoint = thePointList[i];
            
            rspfDrect tempRect(rspfDpt(tempPoint.x - theWidthHeight.x,
                                         tempPoint.y - theWidthHeight.y),
                                rspfDpt(tempPoint.x + theWidthHeight.x,
                                         tempPoint.y + theWidthHeight.y));
            if(tempRect.intersects(imageRect))
            {
               anImage.drawFilledArc(rspf::round<int>(tempPoint.x),
                                     rspf::round<int>(tempPoint.y),
                                     rspf::round<int>(theWidthHeight.x),
                                     rspf::round<int>(theWidthHeight.y),
                                     0,
                                     360);
            }
         }
      }
      else
      {
         for(i = 0; i < upper; ++i)
         {
            rspfDpt tempPoint = thePointList[i];
            
            rspfDrect tempRect(rspfDpt(tempPoint.x - theWidthHeight.x,
                                         tempPoint.y - theWidthHeight.y),
                                rspfDpt(tempPoint.x + theWidthHeight.x,
                                         tempPoint.y + theWidthHeight.y));
            if(tempRect.intersects(imageRect))
            {
               anImage.drawArc(rspf::round<int>(tempPoint.x),
                               rspf::round<int>(tempPoint.y),
                               rspf::round<int>(theWidthHeight.x),
                               rspf::round<int>(theWidthHeight.y),
                               0,
                               360);
            }
         }
      }
   }
}

bool rspfAnnotationMultiEllipseObject::intersects(const rspfDrect& rect)const
{
   return rect.intersects(theBoundingRect);
}

std::ostream& rspfAnnotationMultiEllipseObject::print(std::ostream& out)const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfAnnotionMultiEllipseObject::print NOT IMPLEMENTED"
      << endl;
   return out;
}

void rspfAnnotationMultiEllipseObject::getBoundingRect(rspfDrect& rect)const
{
   rect =  theBoundingRect;
}

void rspfAnnotationMultiEllipseObject::computeBoundingRect()
{
   theBoundingRect.makeNan();

   int i;
   int upper = (int)thePointList.size();
   for(i = 0; i < upper; ++i)
   {
      rspfDpt tempPoint = thePointList[i];
      if(!tempPoint.hasNans())
      {
         rspfDrect tempRect(rspfDpt(tempPoint.x - theWidthHeight.x,
                                      tempPoint.y - theWidthHeight.y),
                             rspfDpt(tempPoint.x + theWidthHeight.x,
                                      tempPoint.y + theWidthHeight.y));
         if(!theBoundingRect.hasNans())
         {
            theBoundingRect.combine(tempRect);
         }
         else
         {
            theBoundingRect = tempRect;
         }
      }
   }
}

bool rspfAnnotationMultiEllipseObject::isPointWithin(const rspfDpt& imagePoint)const
{
   if(!theBoundingRect.hasNans())
   {
      return theBoundingRect.pointWithin(imagePoint);
   }
   return false;
}

void rspfAnnotationMultiEllipseObject::setFillFlag(bool flag)
{
   theFillFlag = flag;
}

void rspfAnnotationMultiEllipseObject::resize(rspf_uint32 size)
{
   if(size)
   {
      thePointList.resize(size);
   }
   else
   {
      thePointList.clear();
   }
}

void rspfAnnotationMultiEllipseObject::setWidthHeight(const rspfDpt& widthHeight)
{
   theWidthHeight = widthHeight;
   computeBoundingRect();
}

rspfDpt& rspfAnnotationMultiEllipseObject::operator[](int i)
{
   return thePointList[i];
}

const rspfDpt& rspfAnnotationMultiEllipseObject::operator[](int i)const
{
   return thePointList[i];
}
