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
// $Id: rspfHsvToRgbSource.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/imaging/rspfHsvToRgbSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfHsvVector.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfHsvToRgbSource, "rspfHsvToRgbSource", rspfImageSourceFilter)

rspfHsvToRgbSource::rspfHsvToRgbSource()
   :rspfImageSourceFilter(),
    theBlankTile(NULL),
    theTile(NULL)
{
}

rspfHsvToRgbSource::rspfHsvToRgbSource(rspfImageSource* inputSource)
   : rspfImageSourceFilter(inputSource),
     theBlankTile(NULL),
     theTile(NULL)
{
}

rspfHsvToRgbSource::~rspfHsvToRgbSource()
{
}

rspfRefPtr<rspfImageData> rspfHsvToRgbSource::getTile(
   const  rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(theInputConnection)
   {
      rspfRefPtr<rspfImageData> imageData =
         theInputConnection->getTile(tileRect, resLevel);

      if(!imageData.valid()) return theBlankTile;
      
      if(!isSourceEnabled())
      {
         return imageData;
      }

      if(!theTile.valid()) allocate();
      if(!theTile.valid())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Unable to initialize rspfHsvToRgbSource in getTile"
            << std::endl;
         return rspfRefPtr<rspfImageData>();
      }
      
      long w  = tileRect.width();
      long h  = tileRect.height();
      long tw = theTile->getWidth();
      long th = theTile->getHeight();
      
      theBlankTile->setOrigin(tileRect.ul());
      theBlankTile->setWidthHeight(w, h);
      theTile->setWidthHeight(w, h);
      theTile->setOrigin(tileRect.ul());
      if( (tw*th) != (w*h))
      {
         theTile->initialize();
      }
      else
      {
         theTile->makeBlank();
      }
      
      if((imageData->getNumberOfBands()==3)&&
         (imageData->getScalarType()==RSPF_NORMALIZED_FLOAT)&&
         (imageData->getDataObjectStatus()!=RSPF_NULL))
      {
         rspf_uint8* outputBands[3];
         float* inputBands[3];
         outputBands[0] = static_cast<rspf_uint8*>(theTile->getBuf(0));
         outputBands[1] = static_cast<rspf_uint8*>(theTile->getBuf(1));
         outputBands[2] = static_cast<rspf_uint8*>(theTile->getBuf(2));
         inputBands[0] = static_cast<float*>(imageData->getBuf(0));
         inputBands[1] = static_cast<float*>(imageData->getBuf(1));
         inputBands[2] = static_cast<float*>(imageData->getBuf(2));
         
         long height = imageData->getHeight();
         long width  = imageData->getWidth();
         long offset = 0;
         for(long row = 0; row < height; ++row)
         {
            for(long col = 0; col < width; ++col)
            {
               rspfHsvVector hsv(inputBands[0][offset],
                                  inputBands[1][offset],
                                  inputBands[2][offset]);
               
               rspfRgbVector rgb(hsv);

               outputBands[0][offset] = rgb.getR();
               outputBands[1][offset] = rgb.getG();
               outputBands[2][offset] = rgb.getB();

               ++offset;
            }
         }
      }
      else
      {
         return imageData;
      }
      theTile->validate();
      return theTile;
   }
   
   return theBlankTile;
}

void rspfHsvToRgbSource::initialize()
{
   theBlankTile = NULL;
   theTile      = NULL;
}

void rspfHsvToRgbSource::allocate()
{
   theBlankTile = rspfImageDataFactory::instance()->create(this, this);
   theTile = (rspfImageData*)theBlankTile->dup();
   theTile->initialize();
}

rspfScalarType rspfHsvToRgbSource::getOutputScalarType() const
{
   return RSPF_UINT8;
}

double rspfHsvToRgbSource::getNullPixelValue()const
{
   return 0;
}

double rspfHsvToRgbSource::getMinPixelValue(rspf_uint32 /* band */)const
{
   return RSPF_DEFAULT_MIN_PIX_UCHAR;
}

double rspfHsvToRgbSource::getMaxPixelValue(rspf_uint32 /* band */)const
{
   return RSPF_DEFAULT_MAX_PIX_UCHAR;
}  
