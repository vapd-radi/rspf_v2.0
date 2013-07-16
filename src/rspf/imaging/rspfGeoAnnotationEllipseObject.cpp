//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationEllipseObject.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <sstream>

#include <rspf/imaging/rspfGeoAnnotationEllipseObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfUnitConversionTool.h>

RTTI_DEF1(rspfGeoAnnotationEllipseObject,
          "rspfGeoAnnotationEllipseObject",
          rspfGeoAnnotationObject);
   
rspfGeoAnnotationEllipseObject::rspfGeoAnnotationEllipseObject(
   const rspfGpt& center,
   const rspfDpt& widthHeight,
   bool enableFill,
   rspf_uint8 r,
   rspf_uint8 g,
   rspf_uint8 b,
   rspf_uint8 thickness)
   :rspfGeoAnnotationObject(r, g, b, thickness),
    theProjectedEllipse(0),
    theCenter(center),
    theWidthHeight(widthHeight),
    theEllipseWidthHeightUnitType(RSPF_PIXEL) // default to image space
{
   theProjectedEllipse = new rspfAnnotationEllipseObject(rspfDpt(0,0),
							  rspfDpt(0,0),
							  enableFill,
							  r,
							  g,
							  b,
							  thickness);
}

rspfGeoAnnotationEllipseObject::rspfGeoAnnotationEllipseObject(
   const rspfGeoAnnotationEllipseObject& rhs)
   :rspfGeoAnnotationObject(rhs),
    theProjectedEllipse(rhs.theProjectedEllipse.valid()?(rspfAnnotationEllipseObject*)rhs.theProjectedEllipse->dup():(rspfAnnotationEllipseObject*)0),
    theCenter(rhs.theCenter),
    theWidthHeight(rhs.theWidthHeight),
    theEllipseWidthHeightUnitType(rhs.theEllipseWidthHeightUnitType)
{
}

rspfGeoAnnotationEllipseObject::~rspfGeoAnnotationEllipseObject()
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse = 0;
   }
}

rspfObject* rspfGeoAnnotationEllipseObject::dup()const
{
   return new rspfGeoAnnotationEllipseObject(*this);
}

void rspfGeoAnnotationEllipseObject::applyScale(double x,
                                                 double y)
{
   if(theProjectedEllipse.valid()) theProjectedEllipse->applyScale(x, y);
   theCenter.lond(theCenter.lond()*x);
   theCenter.latd(theCenter.latd()*y);
   theWidthHeight .x *= x;
   theWidthHeight .y *= y;
}

std::ostream& rspfGeoAnnotationEllipseObject::print(std::ostream& out)const
{
   out << "center ground:                  " <<  theCenter << endl
       << "width height image space flag = " << theEllipseWidthHeightUnitType
       << endl
       << *theProjectedEllipse;
   return out;
}

void rspfGeoAnnotationEllipseObject::draw(rspfRgbImage& anImage)const
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse->draw(anImage);
   }
}

bool rspfGeoAnnotationEllipseObject::intersects(const rspfDrect& rect)const
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse->intersects(rect);
   }

   return false;
}

rspfAnnotationObject* rspfGeoAnnotationEllipseObject::getNewClippedObject(const rspfDrect& rect)const
{
   if(intersects(rect))
   {
      if(theProjectedEllipse.valid())
      {
         return theProjectedEllipse->getNewClippedObject(rect);
      }
   }
   
   return (rspfAnnotationObject*)0;
}

void rspfGeoAnnotationEllipseObject::getBoundingRect(rspfDrect& rect)const
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse->getBoundingRect(rect);
   }
   else
   {
      rect.makeNan();
   }
}

void rspfGeoAnnotationEllipseObject::computeBoundingRect()
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse->computeBoundingRect();
   }
}

void rspfGeoAnnotationEllipseObject::transform(rspfImageGeometry* projection)
{
   if(!projection)
   {
      return;
   }

   // Ellipse center, height and width in image space.
   rspfDpt projectedCenter;
   rspfDpt projectedWidthHeight;

   // first get the center projected
   projection->worldToLocal(theCenter, projectedCenter);

   getWidthHeightInPixels(projectedWidthHeight, projection);

   theProjectedEllipse->setCenterWidthHeight(projectedCenter,
                                             projectedWidthHeight);      
}

bool rspfGeoAnnotationEllipseObject::saveState(rspfKeywordlist& kwl,
                                                const char* prefix) const
{
   kwl.add(prefix, "center", theCenter.toString().c_str());
   kwl.add(prefix, "ellipse_width", theWidthHeight.x);
   kwl.add(prefix, "ellipse_height", theWidthHeight.y);
   kwl.add(prefix, "azimuth", getAzimuth());
   kwl.add(prefix, "fill", rspfString::toString(getFillFlag()));
   kwl.add(prefix, "draw_axes", rspfString::toString(getDrawAxesFlag()));
   kwl.add(prefix, "units",
           rspfUnitTypeLut::instance()->getEntryString(theEllipseWidthHeightUnitType).c_str());
   
   return rspfGeoAnnotationObject::saveState(kwl, prefix);
}

bool rspfGeoAnnotationEllipseObject::loadState(const rspfKeywordlist& kwl,
                                                const char* prefix)
{
   if (!theProjectedEllipse)
   {
      return false;
   }
   
   //---
   // Base class state must be called first to pick up colors...
   //---
   bool status =  rspfGeoAnnotationObject::loadState(kwl, prefix);
   if (status == true)
   {
      theProjectedEllipse->setColor(theRed, theGreen, theBlue);
      theProjectedEllipse->setThickness(theThickness);
   }

   const char* center         = kwl.find(prefix, "center");
   const char* ellipse_width  = kwl.find(prefix, "ellipse_width");
   const char* ellipse_height = kwl.find(prefix, "ellipse_height");
   const char* azimuth        = kwl.find(prefix, "azimuth");
   const char* fill           = kwl.find(prefix, "fill");
   const char* draw_axes      = kwl.find(prefix, "draw_axes");
   const char* units          = kwl.find(prefix, "units");
   
   if(units)
   {
      theEllipseWidthHeightUnitType =
         static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->getEntryNumber(units));
      if (theEllipseWidthHeightUnitType == RSPF_UNIT_UNKNOWN)
      {
         theEllipseWidthHeightUnitType = RSPF_PIXEL;
      }
   }

   if (center)
   {
      std::istringstream is(center);
      is >> theCenter;
   }
   if(ellipse_width)
   {
      theWidthHeight.x = rspfString(ellipse_width).toDouble();
   }
   if(ellipse_height)
   {
      theWidthHeight.y = rspfString(ellipse_height).toDouble();
   }

   if (azimuth)
   {
      setAzimuth(rspfString(azimuth).toFloat64());
   }

   if (fill)
   {
      setFillFlag(rspfString(fill).toBool());
   }

   if (draw_axes)
   {
      setDrawAxesFlag(rspfString(draw_axes).toBool());
   }

   return status;
}

void rspfGeoAnnotationEllipseObject::setEllipseWidthHeightUnitType(
   rspfUnitType type)
{
   theEllipseWidthHeightUnitType = type;
}

void rspfGeoAnnotationEllipseObject::setWidthHeight(const rspfDpt& pt)
{
   theWidthHeight = pt;
}

void rspfGeoAnnotationEllipseObject::setAzimuth(rspf_float64 azimuth)
{
   if (theProjectedEllipse.valid())
   {
      theProjectedEllipse->setAzimuth(azimuth);
   }
}
   
rspf_float64 rspfGeoAnnotationEllipseObject::getAzimuth() const
{
   if (theProjectedEllipse.valid())
   {
      return theProjectedEllipse->getAzimuth();
   }
   return 0.0;
}

void rspfGeoAnnotationEllipseObject::getWidthHeightInPixels(
   rspfDpt& widthHeight, const rspfImageGeometry* projection) const
{
   switch (theEllipseWidthHeightUnitType)
   {
      case RSPF_PIXEL:
      {
         widthHeight = theWidthHeight;

         break;
      }
      
      case RSPF_DEGREES:
      case RSPF_MINUTES:
      case RSPF_SECONDS:
      {
         rspfGpt origin = projection->hasProjection()?projection->getProjection()->origin():rspfGpt();
         rspfDpt gsd    = projection->getMetersPerPixel();
         
         rspfUnitConversionTool uct(origin,
                                     theWidthHeight.x,
                                     theEllipseWidthHeightUnitType);

         double x = uct.getValue(RSPF_METERS);

         uct.setValue(theWidthHeight.y, theEllipseWidthHeightUnitType);
         
         double y = uct.getValue(RSPF_METERS);

         widthHeight.x = ceil(x/gsd.x);
         widthHeight.y = ceil(y/gsd.y);
         
         break;
      }

      case RSPF_METERS:
      {
         rspfDpt gsd = projection->getMetersPerPixel();
         widthHeight.x = static_cast<rspf_int32>(theWidthHeight.x/gsd.x+0.5);
         widthHeight.y = static_cast<rspf_int32>(theWidthHeight.y/gsd.y+0.5);

         break;
      }

      default:
      {
         rspfDpt gsd = projection->getMetersPerPixel();
         
         rspfUnitConversionTool uct(theWidthHeight.x,
                                     theEllipseWidthHeightUnitType);

         double x = uct.getValue(RSPF_METERS);
         
         uct.setValue(theWidthHeight.y, theEllipseWidthHeightUnitType);
         
         double y = uct.getValue(RSPF_METERS);
         
         widthHeight.x = ceil(x/gsd.x);
         widthHeight.y = ceil(y/gsd.y);
         
         break;
      }
   }
}
void rspfGeoAnnotationEllipseObject::setColor(rspf_uint8 r,
                                               rspf_uint8 g,
                                               rspf_uint8 b)
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse->setColor(r, g, b);
   }
   rspfGeoAnnotationObject::setColor(r, g, b);
}

void rspfGeoAnnotationEllipseObject::setThickness(rspf_uint8 thickness)
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse->setThickness(thickness);
   }
}

void rspfGeoAnnotationEllipseObject::setFillFlag(bool flag)
{
   if(theProjectedEllipse.valid())
   {
      theProjectedEllipse->setFillFlag(flag);
   }
}

bool rspfGeoAnnotationEllipseObject::getFillFlag() const
{
   if (theProjectedEllipse.valid())
   {
      return theProjectedEllipse->getFillFlag();
   }
   return false;
}

void rspfGeoAnnotationEllipseObject::setDrawAxesFlag(bool flag)
{
   if (theProjectedEllipse.valid())
   {
      theProjectedEllipse->setDrawAxesFlag(flag);
   }
}

bool rspfGeoAnnotationEllipseObject::getDrawAxesFlag() const
{
   if (theProjectedEllipse.valid())
   {
      return theProjectedEllipse->getDrawAxesFlag();
   }
   return false;
}

void rspfGeoAnnotationEllipseObject::setCenter(const rspfGpt& gpt)
{
   theCenter = gpt;
}

void rspfGeoAnnotationEllipseObject::getCenter(rspfGpt& gpt) const
{
   gpt = theCenter;
}

