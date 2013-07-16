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
// $Id: rspfRgbToHsiSource.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <rspf/imaging/rspfRgbToHsiSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfHsiVector.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfRgbToHsiSource, "rspfRgbToHsiSource", rspfImageSourceFilter)

rspfRgbToHsiSource::rspfRgbToHsiSource()
   :rspfImageSourceFilter(),
    theBlankTile(NULL),
    theTile(NULL)
{
}

rspfRgbToHsiSource::rspfRgbToHsiSource(rspfImageSource* inputSource)
   : rspfImageSourceFilter(inputSource),
     theBlankTile(NULL),
     theTile(NULL)
{
}

rspfRgbToHsiSource::~rspfRgbToHsiSource()
{
}

rspfRefPtr<rspfImageData> rspfRgbToHsiSource::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();  // This filter requires an input.
   }
   
   rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(tileRect,
                                                                       resLevel);   
   if(!isSourceEnabled() || !inputTile.valid())
   {
      return inputTile;
   }

   if(!theTile)
   {
      allocate(); // First time through...
   }

   if (!theTile.valid())
   {
      return inputTile;
   }
   
   if( inputTile->getDataObjectStatus() == RSPF_NULL ||
       inputTile->getDataObjectStatus() == RSPF_EMPTY )
   {
      theBlankTile->setImageRectangle(tileRect);
      return theBlankTile;
   }
   
   if((inputTile->getNumberOfBands()==3)&&
      (inputTile->getScalarType()==RSPF_UCHAR)&&
      (inputTile->getDataObjectStatus()!=RSPF_NULL))
   {
      // Set the origin, resize if needed of the output tile.
      theTile->setImageRectangle(tileRect);

      float* outputBands[3];
      rspf_uint8* inputBands[3];
      outputBands[0] = static_cast<float*>(theTile->getBuf(0));
      outputBands[1] = static_cast<float*>(theTile->getBuf(1));
      outputBands[2] = static_cast<float*>(theTile->getBuf(2));
      inputBands[0]  = static_cast<rspf_uint8*>(inputTile->getBuf(0));
      inputBands[1]  = static_cast<rspf_uint8*>(inputTile->getBuf(1));
      inputBands[2]  = static_cast<rspf_uint8*>(inputTile->getBuf(2));
      
      long height = inputTile->getHeight();
      long width  = inputTile->getWidth();
      long offset = 0;
      for(long row = 0; row < height; ++row)
      {
         for(long col = 0; col < width; ++col)
         {
            rspfRgbVector rgb(inputBands[0][offset],
                               inputBands[1][offset],
                               inputBands[2][offset]);
            
            rspfHsiVector hsi(rgb);
            
            
            outputBands[0][offset] = hsi.getH();
            outputBands[1][offset] = hsi.getS();
            outputBands[2][offset] = hsi.getI();
            
            ++offset;
         }
      }
   }
   else // Input tile not of correct type to process...
   {
      return inputTile;
   }

   theTile->validate();
   return theTile;
}

rspfScalarType rspfRgbToHsiSource::getOutputScalarType() const
{
   return RSPF_NORMALIZED_FLOAT;
}

void rspfRgbToHsiSource::initialize()
{
   // Base class will recapture "theInputConnection".
   rspfImageSourceFilter::initialize();   
}

void rspfRgbToHsiSource::allocate()
{
   theBlankTile = NULL;
   theTile = NULL;

   if(!theInputConnection) return;
   
   theBlankTile = rspfImageDataFactory::instance()->create(this, this);
   theTile      = (rspfImageData*)theBlankTile->dup();
   theTile->initialize();
}

rspf_uint32 rspfRgbToHsiSource::getNumberOfOutputBands()const
{
   return 3;
}

double rspfRgbToHsiSource::getNullPixelValue()const
{
   return rspf::nan();
}

double rspfRgbToHsiSource::getMinPixelValue(rspf_uint32 /* band */)const
{
   return 0.0;
}

double rspfRgbToHsiSource::getMaxPixelValue(rspf_uint32 /* band */)const
{
   return 1.0;
}
