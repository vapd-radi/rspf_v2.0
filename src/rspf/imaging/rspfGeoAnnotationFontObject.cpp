//*************************************************************************
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
//$Id: rspfGeoAnnotationFontObject.cpp 17108 2010-04-15 21:08:06Z dburken $

#include <sstream>

#include <rspf/imaging/rspfGeoAnnotationFontObject.h>
#include <rspf/imaging/rspfAnnotationFontObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/font/rspfFont.h>
#include <rspf/font/rspfFontFactoryRegistry.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF1(rspfGeoAnnotationFontObject,
          "rspfGeoAnnotationFontObject",
          rspfGeoAnnotationObject);


rspfGeoAnnotationFontObject::rspfGeoAnnotationFontObject()
   :
   rspfGeoAnnotationObject(),
   theCenterGround(),
   theFont(0),
   theFontInfo(),
   theAnnotationFontObject(0)
{
   theFont = (rspfFont*)rspfFontFactoryRegistry::instance()->
      getDefaultFont()->dup();
   if (!theFont)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfGeoAnnotationFontObject::rspfGeoAnnotationFontObject"
         << " WARNING: No font support..."
         << endl;
   }
   theAnnotationFontObject =
      new rspfAnnotationFontObject(rspfIpt(0,0),
                                    rspfString(""));
   
   theAnnotationFontObject->setFont(theFont.get());
   
}

rspfGeoAnnotationFontObject::rspfGeoAnnotationFontObject(
   const rspfGpt& location,
   const rspfString& s,
   const rspfIpt& pixelSize,
   double rotation,
   const rspfDpt& scale,
   const rspfDpt& shear,
   rspf_uint8 r,
   rspf_uint8 g,
   rspf_uint8 b)
   :
   rspfGeoAnnotationObject(r, g, b),
   theCenterGround(location),
   theFont(0),
   theFontInfo(),
   theAnnotationFontObject(0)
{
  rspfDpt pt;
  pt.makeNan();

  theAnnotationFontObject = new rspfAnnotationFontObject(pt,
							  s,
							  pixelSize,
							  rotation,
							  scale,
							  shear,
							  r,
							  g,
							  b);
}

rspfGeoAnnotationFontObject::rspfGeoAnnotationFontObject(
   const rspfGeoAnnotationFontObject& rhs)
   :
   rspfGeoAnnotationObject(rhs),
   theCenterGround(rhs.theCenterGround),
   theFont(rhs.theFont.valid()?(rspfFont*)rhs.theFont->dup():(rspfFont*)0),
   theFontInfo(),
   theAnnotationFontObject(
      new rspfAnnotationFontObject(
         rspfDpt(rspf::nan(), rspf::nan()),
         rhs.theAnnotationFontObject->theString,
         rhs.theAnnotationFontObject->thePixelSize,
         rhs.theAnnotationFontObject->theRotation,
         rspfDpt(rhs.theAnnotationFontObject->theHorizontalScale,
                  rhs.theAnnotationFontObject->theVerticalScale),
         rspfDpt(rhs.theAnnotationFontObject->theHorizontalShear,
                  rhs.theAnnotationFontObject->theVerticalShear),
         rhs.theRed,
         rhs.theGreen,
         rhs.theBlue))
{
   theAnnotationFontObject->setFont(theFont.get());
}

rspfGeoAnnotationFontObject::~rspfGeoAnnotationFontObject()
{
   theAnnotationFontObject = 0;
   theFont = 0;
}

rspfObject* rspfGeoAnnotationFontObject::dup()const
{
   return new rspfGeoAnnotationFontObject(*this);
}


void rspfGeoAnnotationFontObject::setFont(rspfFont* font)
{
   theFont = font;

   theAnnotationFontObject->setFont(theFont.get());
}

rspfFont* rspfGeoAnnotationFontObject::getFont()
{
   return theFont.get();
}

void rspfGeoAnnotationFontObject::setPointSize(const rspfIpt& size)
{
   if(theAnnotationFontObject.valid())
   {
      theAnnotationFontObject->setPointSize(size);
   }
}

void rspfGeoAnnotationFontObject::setRotation(double rotation)
{
   if(theAnnotationFontObject.valid())
   {
      theAnnotationFontObject->setRotation(rotation);
   }
}

void rspfGeoAnnotationFontObject::setScale(const rspfDpt& scale)
{
   if(theAnnotationFontObject.valid())
   {
      theAnnotationFontObject->setScale(scale);
   }
}

void rspfGeoAnnotationFontObject::setShear(const rspfDpt& shear)
{
   if(theAnnotationFontObject.valid())
   {
      theAnnotationFontObject->setShear(shear);
   }
}

void rspfGeoAnnotationFontObject::setColor(rspf_uint8 r,
                                            rspf_uint8 g,
                                            rspf_uint8 b)
{
   if(theAnnotationFontObject.valid())
   {
      theAnnotationFontObject->setColor(r, g, b);
   }
}

void rspfGeoAnnotationFontObject::transform(rspfImageGeometry* projection)
{
   if(projection)
   {
      rspfDpt ipt;

      projection->worldToLocal(theCenterGround, ipt);
      theAnnotationFontObject->setCenterPosition(ipt);
      theAnnotationFontObject->computeBoundingRect();
   }
}

void rspfGeoAnnotationFontObject::setCenterGround(const rspfGpt& gpt)
{
   theCenterGround = gpt;
}

rspfAnnotationFontObject* rspfGeoAnnotationFontObject::getFontObject()
{
   return theAnnotationFontObject.get();
}

void rspfGeoAnnotationFontObject::draw(rspfRgbImage& anImage)const
{
   theAnnotationFontObject->draw(anImage);
}

void rspfGeoAnnotationFontObject::computeBoundingRect()
{
   theAnnotationFontObject->computeBoundingRect();
}

void rspfGeoAnnotationFontObject::getBoundingRect(rspfDrect& rect)const
{
   theAnnotationFontObject->getBoundingRect(rect);
}

rspfAnnotationObject* rspfGeoAnnotationFontObject::getNewClippedObject(
   const rspfDrect& /* rect */)const
{
   return (rspfAnnotationObject*)0;
}

bool rspfGeoAnnotationFontObject::intersects(const rspfDrect& rect)const
{
   return theAnnotationFontObject->intersects(rect);
}

void rspfGeoAnnotationFontObject::applyScale(double x, double y)
{
   theAnnotationFontObject->applyScale(x,y);
}

bool rspfGeoAnnotationFontObject::saveState(rspfKeywordlist& kwl,
                                             const char* prefix) const
{
   kwl.add(prefix, "center", theCenterGround.toString().c_str());
   kwl.add(prefix, "text", theAnnotationFontObject->getString().c_str());
   theFontInfo.saveState(kwl, prefix);
   
   return rspfGeoAnnotationObject::saveState(kwl, prefix);
}

bool rspfGeoAnnotationFontObject::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   //---
   // Base class state must be called first to pick up colors...
   //---
   bool status =  rspfGeoAnnotationObject::loadState(kwl, prefix);

   theAnnotationFontObject->setColor(theRed, theGreen, theBlue);
   theAnnotationFontObject->setThickness(theThickness);
   
   const char* lookup;

   lookup = kwl.find(prefix, "text");
   if (lookup)
   {
      theAnnotationFontObject->setString(rspfString(lookup));
   }

   lookup = kwl.find(prefix, "center");
   if (lookup)
   {
      std::istringstream is(lookup);
      is >> theCenterGround;
   }

   // Get the font information.
   theFontInfo.loadState(kwl, prefix);

   // See if we can make a font.
   rspfRefPtr<rspfFont> f = rspfFontFactoryRegistry::instance()->
      createFont(theFontInfo);
   if (f.valid())
   {
      theFont = f;
      theAnnotationFontObject->setFont(theFont.get());
   }

   theAnnotationFontObject->setGeometryInformation(theFontInfo);
   
   return status;
}
