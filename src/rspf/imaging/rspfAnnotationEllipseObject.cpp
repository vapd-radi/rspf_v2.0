//*******************************************************************
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationEllipseObject.cpp 13964 2009-01-14 16:30:07Z gpotts $
#include <rspf/imaging/rspfAnnotationEllipseObject.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfCommon.h>

RTTI_DEF1(rspfAnnotationEllipseObject,
          "rspfAnnotationEllipseObject",
          rspfAnnotationObject)
   
rspfAnnotationEllipseObject::rspfAnnotationEllipseObject(
   const rspfDpt& center,
   const rspfDpt& widthHeight,
   rspf_float64 azimuthInDegrees,
   bool enableFill,
   rspf_uint8 r,
   rspf_uint8 g,
   rspf_uint8 b,
   rspf_uint8 thickness)
   : rspfAnnotationObject(r, g, b, thickness),
     theCenter(center),
     theWidthHeight(widthHeight),
     theAzimuthInDegrees(azimuthInDegrees),
     theFillEnabled(enableFill),
     theDrawAxesFlag(false),
     theBoundingRect(0,0,0,0)
{
   computeBoundingRect();
}

rspfAnnotationEllipseObject::rspfAnnotationEllipseObject(
   const rspfAnnotationEllipseObject& rhs)
   :rspfAnnotationObject(rhs),
   theCenter(rhs.theCenter),
   theWidthHeight(rhs.theWidthHeight),
   theAzimuthInDegrees(rhs.theAzimuthInDegrees),
   theFillEnabled(rhs.theFillEnabled),
   theDrawAxesFlag(rhs.theDrawAxesFlag),
   theBoundingRect(rhs.theBoundingRect)
{
}

rspfObject* rspfAnnotationEllipseObject::dup()const
{
   return new rspfAnnotationEllipseObject(*this);
}

rspfAnnotationEllipseObject::~rspfAnnotationEllipseObject()
{
}

void rspfAnnotationEllipseObject::applyScale(double x, double y)
{
   theCenter.x      *= x;
   theCenter.y      *= y;
   theWidthHeight.x *= x;
   theWidthHeight.y *= y;
   computeBoundingRect();
}



void rspfAnnotationEllipseObject::draw(rspfRgbImage& anImage)const
{
   anImage.setDrawColor(theRed, theGreen, theBlue);
   anImage.setThickness(theThickness);

   if(theBoundingRect.intersects(anImage.getImageData()->getImageRectangle()))
   {
      if(theFillEnabled)
      {
         anImage.drawFilledEllipse((int)(theCenter.x),
                                   (int)(theCenter.y),
                                   (int)(theWidthHeight.x),
                                   (int)(theWidthHeight.y),
                                   theAzimuthInDegrees*RAD_PER_DEG);
      }
      else
      {
         anImage.drawEllipse((int)(theCenter.x),
                             (int)(theCenter.y),
                             (int)(theWidthHeight.x),
                             (int)(theWidthHeight.y),
                             theAzimuthInDegrees*RAD_PER_DEG,
                             theDrawAxesFlag); 
      }
   }
}

bool rspfAnnotationEllipseObject::intersects(const rspfDrect& rect)const
{
   return rect.intersects(theBoundingRect);
}

rspfAnnotationObject* rspfAnnotationEllipseObject::getNewClippedObject(
   const rspfDrect& rect)const
{
   if(intersects(rect))
   {
      return (rspfAnnotationObject*)dup();
   }
   
   return (rspfAnnotationEllipseObject*)NULL;
}

std::ostream& rspfAnnotationEllipseObject::print(std::ostream& out)const
{
   rspfAnnotationObject::print(out);
   out << endl;
   out << "ellipse_center:        " << theCenter
       << "\nellipse_height:        " << theWidthHeight.y
       << "\nellipse_width:         " << theWidthHeight.x
       << "\nfill_enabled:          " << theFillEnabled
       << "\ndraw_axes:             " << theDrawAxesFlag
       << "\nellipse_bounding_rect: " << theBoundingRect;
   return out;
}

void rspfAnnotationEllipseObject::getBoundingRect(rspfDrect& rect)const
{
   rect = theBoundingRect;
}

void rspfAnnotationEllipseObject::computeBoundingRect()
{
   rspf_float64 maxDimension =
      rspf::max(theWidthHeight.x, theWidthHeight.y)/2.0 + 1.0;
   theBoundingRect = rspfDrect(theCenter.x - maxDimension,
                                theCenter.y - maxDimension,
                                theCenter.x + maxDimension,
                                theCenter.y + maxDimension);
}

bool rspfAnnotationEllipseObject::isPointWithin(const rspfDpt& point)const
{
  return theBoundingRect.pointWithin(point);
}

void rspfAnnotationEllipseObject::setCenterWidthHeight(const rspfDpt& center,
                                                        const rspfDpt& widthHeight)
{
   theCenter          = center;
   theWidthHeight     = widthHeight;

   computeBoundingRect();
}

void rspfAnnotationEllipseObject::setAzimuth(rspf_float64 azimuth)
{
   theAzimuthInDegrees = azimuth;
}
   
rspf_float64 rspfAnnotationEllipseObject::getAzimuth() const
{
   return theAzimuthInDegrees;
}

void rspfAnnotationEllipseObject::setFillFlag(bool enabled)
{
   theFillEnabled = enabled;
}

bool rspfAnnotationEllipseObject::getFillFlag() const
{
   return theFillEnabled;
}

void rspfAnnotationEllipseObject::setDrawAxesFlag(bool flag)
{
   theDrawAxesFlag = flag;
}

bool rspfAnnotationEllipseObject::getDrawAxesFlag() const
{
   return theDrawAxesFlag;
}

bool rspfAnnotationEllipseObject::saveState(rspfKeywordlist& kwl,
                                             const char* prefix)const
{
   return rspfAnnotationObject::saveState(kwl, prefix);
}

bool rspfAnnotationEllipseObject::loadState(const rspfKeywordlist& kwl,
                                                const char* prefix)
{
   return rspfAnnotationObject::loadState(kwl, prefix);
}
