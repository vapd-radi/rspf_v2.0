//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationFontObject.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfAnnotationFontObject.h>
#include <rspf/font/rspfFontFactoryRegistry.h>

RTTI_DEF1(rspfAnnotationFontObject, "rspfAnnotationFontObject", rspfAnnotationObject);

rspfAnnotationFontObject::rspfAnnotationFontObject()
   :rspfAnnotationObject(),
    theFont(rspfFontFactoryRegistry::instance()->getDefaultFont()),
    thePosition(0,0),
    theString(""),
    theRotation(0.0),
    theHorizontalScale(0.0),
    theVerticalScale(0.0),
    theHorizontalShear(0.0),
    theVerticalShear(0.0)
{
   setFontInfo();
   theBoundingRect.makeNan();
}

rspfAnnotationFontObject::rspfAnnotationFontObject(const rspfIpt& upperLeft,
                                                     const rspfString& s,
                                                     const rspfIpt& pixelSize,
                                                     double rotation,
                                                     const rspfDpt& scale,
                                                     const rspfDpt& shear,
                                                     unsigned char r,
                                                     unsigned char g,
                                                     unsigned char b)
   :rspfAnnotationObject(r,g,b),
    theFont(rspfFontFactoryRegistry::instance()->getDefaultFont()),
    theString(s),
    thePixelSize(pixelSize),
    theRotation(rotation),
    theHorizontalScale(scale.x),
    theVerticalScale(scale.y),
    theHorizontalShear(shear.x),
    theVerticalShear(shear.y)
{
   setFontInfo();
   thePosition = upperLeft;
   computeBoundingRect();
#if 0
   if (theFont)
   {
      theFont->getBoundingBox(theBoundingRect);
   }
   theBoundingRect += upperLeft;
   thePosition = theBoundingRect.ul();
#endif
}

rspfAnnotationFontObject::rspfAnnotationFontObject(const rspfAnnotationFontObject& rhs)
   :rspfAnnotationObject(rhs),
    thePosition(rhs.thePosition),
    theString(rhs.theString),
    thePixelSize(rhs.thePixelSize),
    theRotation(rhs.theRotation),
    theHorizontalScale(rhs.theHorizontalScale),
    theVerticalScale(rhs.theVerticalScale),
    theHorizontalShear(rhs.theHorizontalShear),
    theVerticalShear(rhs.theVerticalShear),
    theBoundingRect(rhs.theBoundingRect)
{
   theFont = rhs.theFont;
   setFontInfo();
}


rspfAnnotationFontObject::~rspfAnnotationFontObject()
{
   theFont = 0;
}

void rspfAnnotationFontObject::draw(rspfRgbImage& anImage)const
{
   if(theBoundingRect.hasNans())
   {
      return;
   }
   if (!theFont)
   {
      return;
   }
   
   if(!thePixelSize.x || !thePixelSize.y)
   {
      return; // (DAH) we have to have some demension to write!
   }

   rspfRefPtr<rspfImageData> destination = anImage.getImageData();
   
   if(destination.valid())
   {
      rspfIrect destRect = anImage.getImageData()->getImageRectangle();
      
      rspfIpt shift(-theBoundingRect.ul().x,
                     -theBoundingRect.ul().y);

      destRect += shift;
      rspfIrect boundingRect = theBoundingRect + shift;
      
      if(boundingRect.intersects(destRect))
      {
         setFontInfo();
 
         rspfIrect fontBufferRect;
         rspfIrect boundingFontBox;
         
         theFont->setClippingBox(destRect);
         theFont->getBoundingBox(boundingFontBox);

         const rspf_uint8* srcBuf = theFont->rasterize();
         if (!srcBuf)
         {
            return;
         }
         
         theFont->getBufferRect(fontBufferRect);
         rspfIrect clipRect = boundingRect.clipToRect(fontBufferRect);
         
         long clipHeight = clipRect.height();
         long clipWidth  = clipRect.width();
         
         long destinationOffset   = (long)(((clipRect.ul().y - destRect.ul().y)* destRect.width()) +
                                           (clipRect.ul().x - destRect.ul().x));
         long fontBufferOffset    = (long)(((clipRect.ul().y - fontBufferRect.ul().y)*fontBufferRect.width()) +
                                           (clipRect.ul().x - fontBufferRect.ul().x));

         
         long destinationIndex = destinationOffset;
         long srcIndex         = fontBufferOffset;
         long num_bands        = destination->getNumberOfBands();
         rspf_int32 s_width = (rspf_int32)fontBufferRect.width();
         rspf_int32 d_width = (rspf_int32)destRect.width();
         
         num_bands = num_bands > 3? 3:num_bands;
         
         unsigned char colorArray[3];
         long band = 0;
         colorArray[0] = theRed;
         colorArray[1] = theGreen;
         colorArray[2] = theBlue;
         rspf_uint8** destBuf = new rspf_uint8*[num_bands];
         for(band = 0; band < num_bands;++band)
         {
            destBuf[band] = static_cast<rspf_uint8*>(destination->getBuf(band));
            if (!destBuf[band])
            {
               delete [] destBuf;
               return;
            }
         }
         
         for(long line = 0; line < clipHeight; ++line)
         {
            for(long col = 0; col < clipWidth; ++col)
            {
               if(srcBuf[srcIndex + col])
               {
                  for (band=0; band<num_bands; ++band)
                  {
                     *(destBuf[band] + destinationIndex + col) = colorArray[band];
                  }
               }
            }
            srcIndex         += s_width;
            destinationIndex += d_width;
         }
         delete [] destBuf;
      }
   }
}

std::ostream& rspfAnnotationFontObject::print(std::ostream& out)const
{
   if(theFont.valid())
   {
      out << "Family:          " << theFont->getFamilyName() << endl
          << "Style:           " << theFont->getStyleName()  << endl;
   }
   out << "String:             " << theString << endl
       << "Upper Left Position:    " << thePosition << endl
       << "Rotation:           " << theRotation << endl
       << "Horizontal shear:   " << theHorizontalShear << endl
       << "Vertical shear:     " << theVerticalShear << endl
       << "Vertical scale:     " << theVerticalScale << endl
       << "Horizontal scale:   " << theHorizontalScale << endl
       << "Bounding rect:      " << theBoundingRect << endl;
   return out;
}

void rspfAnnotationFontObject::getBoundingRect(rspfDrect& rect)const
{
   rect =  theBoundingRect;
}

void rspfAnnotationFontObject::computeBoundingRect()
{
   setFontInfo();
   if(theFont.valid())
   {
      rspfIrect textRect;
      theFont->getBoundingBox(textRect);
      rspf_int32 w = textRect.width();
      rspf_int32 h = textRect.height();
      rspf_int32 ulx = thePosition.x;
      rspf_int32 uly = thePosition.y;
      theBoundingRect = rspfIrect(ulx,
                                   uly,
                                   ulx + w - 1,
                                   uly + h - 1);
   }
}

bool rspfAnnotationFontObject::isPointWithin(const rspfDpt& imagePoint)const
{
   return theBoundingRect.pointWithin(imagePoint);
}

void rspfAnnotationFontObject::setFont(rspfFont* font)
{
    theFont = font;

   if(!theFont)
   {
      theFont = rspfFontFactoryRegistry::instance()->getDefaultFont();
   }
}

void rspfAnnotationFontObject::setCenterPosition(const rspfIpt& position)
{
   rspfDpt pt = theBoundingRect.midPoint();
   rspfIpt delta = position-pt;
   theBoundingRect += delta;
   thePosition = theBoundingRect.ul();
}

void rspfAnnotationFontObject::setUpperLeftPosition(const rspfIpt& position)
{
   rspfDpt pt = theBoundingRect.ul();
   rspfIpt delta = position-pt;
   theBoundingRect += delta;
   thePosition = theBoundingRect.ul();
}

void rspfAnnotationFontObject::setFontInfo()const
{
   if(theFont.valid())
   {
      theFont->setString(theString);
      theFont->setRotation(theRotation);
      theFont->setScale(theHorizontalScale, theVerticalScale);
      theFont->setHorizontalVerticalShear(theHorizontalShear, theVerticalShear);
      theFont->setPixelSize(thePixelSize.x,
                            thePixelSize.y);
   }
}

void rspfAnnotationFontObject::setString(const rspfString& s)
{
   theString = s;
}

rspfString rspfAnnotationFontObject::getString()const
{
   return theString;
}

rspfAnnotationObject* rspfAnnotationFontObject::getNewClippedObject(const rspfDrect& rect)const
{
   if(intersects(rect))
   {
      return (rspfAnnotationObject*)dup();
   }
   
   return (rspfAnnotationObject*)NULL;
}

bool rspfAnnotationFontObject::intersects(const rspfDrect& rect)const
{
   return rect.intersects(theBoundingRect);
}

void rspfAnnotationFontObject::setPointSize(const rspfIpt& size)
{
   thePixelSize = size;
   setFontInfo();
   if (theFont.valid())
   {
      theFont->getBoundingBox(theBoundingRect);
   }
   theBoundingRect += (thePosition - theBoundingRect.ul());
   thePosition = theBoundingRect.ul();         
}

void rspfAnnotationFontObject::setRotation(double rotation)
{
   theRotation = rotation;
   setFontInfo();
   if (theFont.valid())
   {
      theFont->getBoundingBox(theBoundingRect);
   }
   theBoundingRect += (thePosition - theBoundingRect.ul());
   thePosition = theBoundingRect.ul();         
}

void rspfAnnotationFontObject::setScale(const rspfDpt& scale)
{
   theHorizontalScale = scale.x;
   theVerticalScale   = scale.y;
   setFontInfo();
   if (theFont.valid())
   {
      theFont->getBoundingBox(theBoundingRect);
   }
   theBoundingRect += (thePosition - theBoundingRect.ul());
   thePosition = theBoundingRect.ul();         
}

void rspfAnnotationFontObject::setShear(const rspfDpt& shear)
{
   theHorizontalShear = shear.x;
   theVerticalShear   = shear.y;
   setFontInfo();
   if (theFont.valid())
   {
      theFont->getBoundingBox(theBoundingRect);
   }
   theBoundingRect += (thePosition - theBoundingRect.ul());
   thePosition = theBoundingRect.ul();         
}

void rspfAnnotationFontObject::setGeometryInformation(const rspfFontInformation& info)
{
   thePixelSize       = info.thePointSize;
   theRotation        = info.theRotation;
   theHorizontalScale = info.theScale.x;
   theVerticalScale   = info.theScale.y;
   theHorizontalShear = info.theShear.x;
   theVerticalShear   = info.theShear.y;
   
   setFontInfo();
   if (theFont.valid())
   {
      theFont->getBoundingBox(theBoundingRect);
   }
   theBoundingRect += (thePosition - theBoundingRect.ul());
   thePosition = theBoundingRect.ul();         
}

void rspfAnnotationFontObject::applyScale(double x, double y)
{
   thePosition.x = rspf::round<int>(thePosition.x *x);
   thePosition.y = rspf::round<int>(thePosition.y *y);
   
   setFontInfo();
   computeBoundingRect();

}

rspfObject* rspfAnnotationFontObject::dup()const
{
   return new rspfAnnotationFontObject(*this);
}
