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
// $Id: rspfHsiToRgbSource.cpp 17206 2010-04-25 23:20:40Z dburken $
#include <rspf/imaging/rspfHsiToRgbSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfHsiVector.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfHsiToRgbSource, "rspfHsiToRgbSource", rspfImageSourceFilter)

rspfHsiToRgbSource::rspfHsiToRgbSource()
   :rspfImageSourceFilter(),
    theBlankTile(NULL),
    theTile(NULL)
{
}

rspfHsiToRgbSource::rspfHsiToRgbSource(rspfImageSource* inputSource)
   : rspfImageSourceFilter(inputSource),
    theBlankTile(NULL),
    theTile(NULL)
{
}

rspfHsiToRgbSource::~rspfHsiToRgbSource()
{
}

rspfRefPtr<rspfImageData> rspfHsiToRgbSource::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;  // This filter requires an input.
   }
   
   rspfRefPtr<rspfImageData> inputTile =
      theInputConnection->getTile(tileRect, resLevel);   
   if(!isSourceEnabled())
   {
      return inputTile;
   }
   
   if(!theTile.valid())
   {
      allocate(); // First time through...
   }
   
   if( !inputTile.valid() ||
       inputTile->getDataObjectStatus() == RSPF_NULL ||
       inputTile->getDataObjectStatus() == RSPF_EMPTY )
   {
      theBlankTile->setImageRectangle(tileRect);
      return theBlankTile;
   }

   if((inputTile->getNumberOfBands()==3)&&
      (inputTile->getScalarType()==RSPF_NORMALIZED_FLOAT)&&
      (inputTile->getDataObjectStatus()!=RSPF_NULL))
   {
      // Set the origin, resize if needed of the output tile.
      theTile->setImageRectangle(tileRect);
      
      rspf_uint8* outputBands[3];
      float* inputBands[3];
      outputBands[0] = static_cast<rspf_uint8*>(theTile->getBuf(0));
      outputBands[1] = static_cast<rspf_uint8*>(theTile->getBuf(1));
      outputBands[2] = static_cast<rspf_uint8*>(theTile->getBuf(2));
      inputBands[0] = static_cast<float*>(inputTile->getBuf(0));
      inputBands[1] = static_cast<float*>(inputTile->getBuf(1));
      inputBands[2] = static_cast<float*>(inputTile->getBuf(2));
      
      long height = inputTile->getHeight();
      long width  = inputTile->getWidth();
      long offset = 0;
      for(long row = 0; row < height; ++row)
      {
         for(long col = 0; col < width; ++col)
         {
            rspfHsiVector hsi(inputBands[0][offset],
                               inputBands[1][offset],
                               inputBands[2][offset]);
            
            rspfRgbVector rgb(hsi);
            
            
            outputBands[0][offset] = rgb.getR();
            outputBands[1][offset] = rgb.getG();
            outputBands[2][offset] = rgb.getB();
            
            ++offset;
         }
      }
   }
   else
   {
      return inputTile;
   }

   theTile->validate();

   return theTile;
}

void rspfHsiToRgbSource::initialize()
{
   rspfImageSourceFilter::initialize();
}

void rspfHsiToRgbSource::allocate()
{
   theBlankTile = rspfImageDataFactory::instance()->create(this, this);
   theTile = (rspfImageData*)theBlankTile->dup();
   theTile->initialize();
}

rspfScalarType rspfHsiToRgbSource::getOutputScalarType() const
{
   return RSPF_UCHAR;
}

double rspfHsiToRgbSource::getNullPixelValue()const
{
   return 0.0;
}

double rspfHsiToRgbSource::getMinPixelValue(rspf_uint32 /* band */)const
{
   return RSPF_DEFAULT_MIN_PIX_UCHAR;
}

double rspfHsiToRgbSource::getMaxPixelValue(rspf_uint32 /* band */)const
{
   return RSPF_DEFAULT_MAX_PIX_UCHAR;
}
