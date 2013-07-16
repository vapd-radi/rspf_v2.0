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
// $Id: rspfJpegYCbCrToRgbSource.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/imaging/rspfJpegYCbCrToRgbSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfJpegYCbCrVector.h>
#include <rspf/base/rspfRgbVector.h>

RTTI_DEF1(rspfJpegYCbCrToRgbSource,
          "rspfJpegYCbCrToRgbSource" ,
          rspfImageSourceFilter)

rspfJpegYCbCrToRgbSource::rspfJpegYCbCrToRgbSource()
   :rspfImageSourceFilter(),
    theBlankTile()
{
}

rspfJpegYCbCrToRgbSource::rspfJpegYCbCrToRgbSource(rspfImageSource* inputSource)
   : rspfImageSourceFilter(inputSource),
     theBlankTile()
{
}

rspfJpegYCbCrToRgbSource::~rspfJpegYCbCrToRgbSource()
{
}

void rspfJpegYCbCrToRgbSource::initialize()
{
   rspfImageSourceFilter::initialize();
}

void rspfJpegYCbCrToRgbSource::allocate()
{
   theBlankTile = new rspfImageData(this,
                                     RSPF_UCHAR,
                                     3);
}

rspfRefPtr<rspfImageData> rspfJpegYCbCrToRgbSource::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if (!theBlankTile.valid())
   {
      allocate(); // first time through.
   }
   
   if(theBlankTile.valid())
   {
      theBlankTile->setOrigin(tileRect.ul());
      theBlankTile->setWidthHeight(tileRect.width(), tileRect.height());
   }
   
   if(theInputConnection)
   {
      rspfRefPtr<rspfImageData> imageData =
         theInputConnection->getTile(tileRect, resLevel);

      if(!imageData.valid())
      {
         return theBlankTile;
      }
      if((isSourceEnabled())&&
         (imageData->getNumberOfBands()==3)&&
         (imageData->getScalarType()==RSPF_UCHAR)&&
         (imageData->getDataObjectStatus()!=RSPF_NULL)&&
         (imageData->getDataObjectStatus()!=RSPF_EMPTY))
      {
         rspf_uint8* bands[3];
         
         bands[0] = static_cast<rspf_uint8*>(imageData->getBuf(0));
         bands[1] = static_cast<rspf_uint8*>(imageData->getBuf(1));
         bands[2] = static_cast<rspf_uint8*>(imageData->getBuf(2));
         
         long height = imageData->getHeight();
         long width  = imageData->getWidth();
         long offset = 0;
         for(long row = 0; row < height; ++row)
         {
            for(long col = 0; col < width; ++col)
            {
               
               rspfJpegYCbCrVector ycbcr(bands[0][offset],
                                          bands[1][offset],
                                          bands[2][offset]);
               rspfRgbVector rgb(ycbcr);
               
               
               bands[0][offset] = rgb.getR();
               bands[1][offset] = rgb.getG();
               bands[2][offset] = rgb.getB();
               
               ++offset;
            }
         }
         imageData->validate();
      }
      return imageData;
   }
   
   return theBlankTile;
}  
