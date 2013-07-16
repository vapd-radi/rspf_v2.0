//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationBitmap.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <rspf/imaging/rspfGeoAnnotationBitmap.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfIrect.h>

RTTI_DEF1(rspfGeoAnnotationBitmap,
          "rspfGeoAnnotationBitmap",
          rspfGeoAnnotationObject)
   
rspfGeoAnnotationBitmap::rspfGeoAnnotationBitmap(
   const rspfGpt& center,
   rspfRefPtr<rspfImageData> imageData,
   unsigned char r,
   unsigned char g,
   unsigned char b)
   :rspfGeoAnnotationObject(r, g, b),
    theCenterPoint(center),
    theProjectedPoint(0,0),
    theImageData(NULL)
{
   if(imageData.valid() &&
      (imageData->getScalarType()==RSPF_UCHAR))
   {
      theImageData = imageData;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfGeoAnnotationBitmap::rspfGeoAnnotationBitmap\n"
         << "Invalid image data passed to rspfGeoAnnotationBitmap "
         << "constructor" << endl;
   }
}

rspfGeoAnnotationBitmap::rspfGeoAnnotationBitmap(
   const rspfGeoAnnotationBitmap& rhs)
   :
   rspfGeoAnnotationObject(rhs),
   theCenterPoint(theCenterPoint),
   theProjectedPoint(theProjectedPoint),
   theImageData(rhs.theImageData)
{
}

rspfGeoAnnotationBitmap::~rspfGeoAnnotationBitmap()
{
}

rspfObject* rspfGeoAnnotationBitmap::dup()const
{
   return new rspfGeoAnnotationBitmap(*this);
}

bool rspfGeoAnnotationBitmap::intersects(const rspfDrect& rect) const
{
   if(theImageData.valid())
   {
      return theImageData->getImageRectangle().intersects(rect);
   }
   
   return false;
}

rspfGeoAnnotationBitmap* rspfGeoAnnotationBitmap::getNewClippedObject(
   const rspfDrect& /* rect */)const
{
   rspfGeoAnnotationBitmap* result = (rspfGeoAnnotationBitmap*)dup();
   
   rspfNotify(rspfNotifyLevel_WARN)
      << "rspfGeoAnnotationBitmap::getNewClippedObject WRNING: "
      << "not implemented" << std::endl;
   
   return result;
}

void rspfGeoAnnotationBitmap::applyScale(double /* x */, double /* y */)
{
     rspfNotify(rspfNotifyLevel_WARN)
      << "rspfGeoAnnotationBitmap::applyScale WRNING: not implemented"
      << std::endl; 
}


std::ostream& rspfGeoAnnotationBitmap::print(std::ostream& out)const
{
   out << "center:    " << theCenterPoint << endl;
   return out;
}

void rspfGeoAnnotationBitmap::draw(rspfRgbImage& anImage)const
{
   rspfRefPtr<rspfImageData> destination = anImage.getImageData();

   if(!destination)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfGeoAnnotationBitmap::draw\n"
         << "ERROR: can't draw annotation bitmap to NULL buffer" << endl;
      
      return;
   }
   rspfIrect destinationRect = destination->getImageRectangle();
   rspfIrect srcRect         = theImageData->getImageRectangle();
   if(!srcRect.intersects(destinationRect))
   {
      return;
   }
   rspfIrect clipRect        = srcRect.clipToRect(destinationRect);
   
   long clipHeight = (long)clipRect.height();
   long clipWidth  = (long)clipRect.width();


   if(clipRect.width() != 1.0 &&
      clipRect.height() != 1.0)
   {         
      long destinationOffset   = (long)(((clipRect.ul().y - destinationRect.ul().y)* destinationRect.width()) +
                                        (clipRect.ul().x - destinationRect.ul().x));
      long srcOffset   = (long)(((clipRect.ul().y - srcRect.ul().y)*srcRect.width()) +
                         (clipRect.ul().x - srcRect.ul().x));
      long destinationIndex = destinationOffset;
      long srcIndex         = srcOffset;
      long num_bands        = destination->getNumberOfBands();
      rspf_int32 s_width = (rspf_int32)srcRect.width();
      rspf_int32 d_width = (rspf_int32)destinationRect.width();
      
      num_bands = num_bands > 3? 3:num_bands;
      
      const rspf_uint8* imageDataBuf = static_cast<const rspf_uint8*>(theImageData->getBuf());
      unsigned char colorArray[3];
      colorArray[0] = theRed;
      colorArray[1] = theGreen;
      colorArray[2] = theBlue;
      for (long band=0; band<num_bands; ++band)
      {
         rspf_uint8* destinationBand = static_cast<rspf_uint8*>(destination->getBuf(band));
         destinationIndex       = destinationOffset;
         srcIndex               = srcOffset;
         for(long line = 0; line < clipHeight; ++line)
         {
            for(long col = 0; col < clipWidth; ++col)
            {
               if(imageDataBuf[srcIndex + col])
               {
                  destinationBand[destinationIndex + col] = colorArray[band];
               }
            }
            srcIndex += s_width;
            destinationIndex += d_width;
         }
      } 
   }
}

void rspfGeoAnnotationBitmap::getBoundingRect(rspfDrect& rect)const
{
   rect = rspfDrect(0,0,0,0);

   if(theImageData.valid())
   {
      rect = theImageData->getImageRectangle();
   }
}

void rspfGeoAnnotationBitmap::transform(rspfImageGeometry* projection)
{
   if(projection)
   {
      projection->worldToLocal(theCenterPoint, theProjectedPoint);
      theProjectedPoint = rspfIpt(theProjectedPoint);
      if(theImageData.valid())
      {
         rspfDpt origin(theProjectedPoint.x - theImageData->getWidth()/2.0,
                         theProjectedPoint.y - theImageData->getHeight()/2.0);
         
         theImageData->setOrigin(origin);
      }
   }
}

void rspfGeoAnnotationBitmap::setImageData(
   rspfRefPtr<rspfImageData>& imageData)
{
   theImageData = imageData;
}

void rspfGeoAnnotationBitmap::computeBoundingRect()
{
   // nothing to be done since we don't support
   // rotated bitmaps yet.
}
