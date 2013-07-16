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
// $Id: rspfRgbToJpegYCbCrSource.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/imaging/rspfRgbToJpegYCbCrSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfJpegYCbCrVector.h>
#include <rspf/base/rspfRgbVector.h>

RTTI_DEF1(rspfRgbToJpegYCbCrSource,
          "rspfRgbToJpegYCbCrSource",
          rspfImageSourceFilter)

rspfRgbToJpegYCbCrSource::rspfRgbToJpegYCbCrSource()
   :rspfImageSourceFilter()
{
   theBlankTile = new rspfImageData(this,
                                     RSPF_UCHAR,
                                     3);
}

rspfRgbToJpegYCbCrSource::rspfRgbToJpegYCbCrSource(rspfImageSource* inputSource)
   : rspfImageSourceFilter(inputSource)
{
}

rspfRgbToJpegYCbCrSource::~rspfRgbToJpegYCbCrSource()
{
}

rspfRefPtr<rspfImageData> rspfRgbToJpegYCbCrSource::getTile(const  rspfIrect& tileRect,
                                                   rspf_uint32 resLevel)
{
   theBlankTile->setOrigin(tileRect.ul());
   theBlankTile->setWidthHeight(tileRect.width(), tileRect.height());
   
   if(theInputConnection)
   {
      rspfRefPtr<rspfImageData> imageData = theInputConnection->getTile(tileRect,
                                                              resLevel);
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
               rspfRgbVector rgb(bands[0][offset],
                                  bands[1][offset],
                                  bands[2][offset]);
               
               rspfJpegYCbCrVector ycbcr(rgb);
               
               
               bands[0][offset] = ycbcr.getY();
               bands[1][offset] = ycbcr.getCb();
               bands[2][offset] = ycbcr.getCr();
               ++offset;
            }
         }
         imageData->validate();
      }
      return imageData;
   }
   
   return theBlankTile;
}  
