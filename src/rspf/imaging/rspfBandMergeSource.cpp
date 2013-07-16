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
// $Id: rspfBandMergeSource.cpp 15766 2009-10-20 12:37:09Z gpotts $
#include <rspf/imaging/rspfBandMergeSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfIrect.h>

RTTI_DEF1(rspfBandMergeSource, "rspfBandMergeSource", rspfImageCombiner)

rspfBandMergeSource::rspfBandMergeSource()
   :rspfImageCombiner(),
    theNumberOfOutputBands(0),
    theTile(NULL)
{
}

rspfBandMergeSource::rspfBandMergeSource(rspfConnectableObject::ConnectableObjectList& inputSources)
   :rspfImageCombiner(inputSources),
    theNumberOfOutputBands(0),
    theTile(NULL)
{
   initialize();
}

rspfBandMergeSource::~rspfBandMergeSource()
{
   theNumberOfOutputBands = 0;
}

rspfRefPtr<rspfImageData> rspfBandMergeSource::getTile(const rspfIrect& tileRect,
                                              rspf_uint32 resLevel)
{
   rspf_uint32 layerIdx = 0;
   if( ( getNumberOfInputs() == 1) || !isSourceEnabled() )
   {
      return getNextTile(layerIdx, 0, tileRect, resLevel);
   }

   // test if initialized
   if(!theTile.get())
   {
      allocate();
   }
   
   if(!theTile.get())
   {
      return getNextTile(layerIdx, 0, tileRect, resLevel);
   }
   
   long w     = tileRect.width();
   long h     = tileRect.height();
   long tileW = theTile->getWidth();
   long tileH = theTile->getHeight();
   if((w != tileW)||
      (h != tileH))
   {
      theTile->setWidth(w);
      theTile->setHeight(h);
      if((w*h)!=(tileW*tileH))
      {
         theTile->initialize();
      }
      else
      {
         theTile->makeBlank();
      }
   }
   else
   {
      theTile->makeBlank();
   }
   theTile->setOrigin(tileRect.ul());
   rspf_uint32 currentBand = 0;
   rspf_uint32 maxBands = theTile->getNumberOfBands();
   rspf_uint32 inputIdx = 0;
   for(inputIdx = 0; inputIdx < getNumberOfInputs(); ++inputIdx)
   {
      rspfImageSource* input = PTR_CAST(rspfImageSource,
                                                  getInput(inputIdx));
      rspfRefPtr<rspfImageData> currentTile = 0;

      if(input)
      {
         currentTile = input->getTile(tileRect, resLevel);
      }
      else
      {
         currentTile = 0;
      }
      rspf_uint32 maxInputBands = 0;

      if(!currentTile.get())
      {
         maxInputBands = 1;
      }
      else if(currentTile->getNumberOfBands() == 0)
      {
         maxInputBands = 1;
      }
      else
      {
         maxInputBands = currentTile->getNumberOfBands();
      }
      for(rspf_uint32 band = 0;
          ( (band < maxInputBands) &&
            (currentBand < maxBands));
          ++band)
      {
         // clear the band with the actual NULL
         theTile->fill(currentBand, theTile->getNullPix(band));

         if(currentTile.valid())
         {
            if((currentTile->getDataObjectStatus() != RSPF_NULL) &&
               (currentTile->getDataObjectStatus() != RSPF_EMPTY))
            {
               memmove(theTile->getBuf(currentBand),
                       currentTile->getBuf(band),
                       currentTile->getSizePerBandInBytes());
            }
         }
         ++currentBand;
      }
   }
   theTile->validate();

   return theTile;
}

double rspfBandMergeSource::getNullPixelValue(rspf_uint32 band)const
{
   rspf_uint32 currentBandCount = 0;
   rspf_uint32 idx              = 0;

   rspf_uint32 maxBands = getNumberOfOutputBands();
   if(!maxBands) return 0.0;
   
   while((currentBandCount < maxBands)&&
	 (idx < getNumberOfInputs()))
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(idx));
      if(temp)
      {
	rspf_uint32 previousCount = currentBandCount;
	currentBandCount += temp->getNumberOfOutputBands();

         if(band < currentBandCount)
         {
            return temp->getNullPixelValue(band - previousCount);
         }
      }
      ++idx;
   }
   
   return rspf::nan();
}

double rspfBandMergeSource::getMinPixelValue(rspf_uint32 band)const
{
   rspf_uint32 currentBandCount = 0;
   rspf_uint32 idx              = 0;

   rspf_uint32 maxBands = getNumberOfOutputBands();
   if(!maxBands) return 0.0;
   
   while((currentBandCount < maxBands)&&
	 (idx < getNumberOfInputs()))
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(idx));
      if(temp)
      {
	rspf_uint32 previousCount = currentBandCount;
	currentBandCount += temp->getNumberOfOutputBands();

         if(band < currentBandCount)
         {
            return temp->getMinPixelValue(band - previousCount); 
         }
      }
      ++idx;
   }
   
   return 0.0;
}

double rspfBandMergeSource::getMaxPixelValue(rspf_uint32 band)const
{
   rspf_uint32 currentBandCount = 0;
   rspf_uint32 idx              = 0;

   rspf_uint32 maxBands = getNumberOfOutputBands();
   if(!maxBands) return 0.0;
   
   while((currentBandCount < maxBands)&&
	 (idx < getNumberOfInputs()))
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(idx));
      if(temp)
      {
	rspf_uint32 previousCount = currentBandCount;
	currentBandCount += temp->getNumberOfOutputBands();

         if(band < currentBandCount)
         {
            return temp->getMaxPixelValue(band - previousCount);
         }
      }
      ++idx;
   }
   
   return rspf::nan();
}

void rspfBandMergeSource::initialize()
{
   rspfImageCombiner::initialize();

   if(theTile.get())
   {
      theTile = NULL;
   }
   
   theNumberOfOutputBands = computeNumberOfInputBands();
}

void rspfBandMergeSource::allocate()
{
   if(theNumberOfOutputBands)
   {
      theTile = rspfImageDataFactory::instance()->create(this,
                                                          this);
      theTile->initialize();
   }
}

rspf_uint32 rspfBandMergeSource::computeNumberOfInputBands()const
{
   rspf_uint32 result = 0;
   rspf_uint32 size   = getNumberOfInputs();
   for(rspf_uint32 index = 0; index < size; ++index)
   {
      rspfImageSource* temp = PTR_CAST(rspfImageSource, getInput(index));
      if(temp)
      {
         if(temp->getNumberOfOutputBands() == 0)
         {
            ++result;
         }
         else
         {
            result += temp->getNumberOfOutputBands();
         }
      }
   }
   return result;
}

rspf_uint32 rspfBandMergeSource::getNumberOfOutputBands() const
{
   if(!theNumberOfOutputBands)
   {
      return computeNumberOfInputBands();
   }
   
   return theNumberOfOutputBands;
}
