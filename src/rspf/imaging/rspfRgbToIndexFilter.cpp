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
// $Id: rspfRgbToIndexFilter.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <rspf/imaging/rspfRgbToIndexFilter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>


RTTI_DEF1(rspfRgbToIndexFilter,
          "rspfRgbToIndexFilter",
          rspfImageSourceFilter);

rspfRgbToIndexFilter::rspfRgbToIndexFilter()
   :rspfImageSourceFilter(),
    theLut(new rspfRgbLutDataObject()),
    theTile(NULL)
{
}

rspfRgbToIndexFilter::rspfRgbToIndexFilter(rspfImageSource* inputSource,
                                             const rspfRgbLutDataObject& rgbLut)
   :rspfImageSourceFilter(inputSource),
    theLut((rspfRgbLutDataObject*)rgbLut.dup()),
    theTile(NULL)
{
}

rspfRgbToIndexFilter::~rspfRgbToIndexFilter()
{
   theLut = 0;
}

void rspfRgbToIndexFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   // Force allocate on next getTile.
   theTile      = NULL;
}

void rspfRgbToIndexFilter::allocate()
{
   theTile      = NULL;
   
   if(isSourceEnabled())
   {
      theTile      = rspfImageDataFactory::instance()->create(this, this);
      theTile->initialize();
   }
}

void rspfRgbToIndexFilter::disableSource()
{
   rspfImageSourceFilter::disableSource();
   theTile = NULL;
}

rspfRefPtr<rspfImageData> rspfRgbToIndexFilter::getTile(
   const rspfIrect& origin,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();
   }

   if(!theTile.valid())
   {
      allocate();
   }
   
   rspfRefPtr<rspfImageData> input = theInputConnection->getTile(origin,
                                                                   resLevel);

   if(!theTile.valid() || !input.valid())
   {
      return input;
   }

   theTile->setImageRectangle(origin);

   if(input->getScalarType() == RSPF_UCHAR)
   {
      theTile->setDataObjectStatus(RSPF_FULL);
      return convertInputTile(input);
   }
   else
   {
      theTile->makeBlank();
   }

   theTile->setDataObjectStatus(RSPF_FULL);
   
   return theTile;
}

rspfRefPtr<rspfImageData> rspfRgbToIndexFilter::convertInputTile(rspfRefPtr<rspfImageData>& tile)
{
   if (!tile)
   {
      return theTile;
   }
   
   const rspf_uint32 BANDS = tile->getNumberOfBands();

   if (!BANDS)
   {
      return theTile;
   }
   
   rspf_uint8* band[3];
   rspf_uint8* outBand = (rspf_uint8*)theTile->getBuf();

   if(BANDS >= 3)
   {
      band[0] = (rspf_uint8*)(tile->getBuf(0));
      band[1] = (rspf_uint8*)(tile->getBuf(1));
      band[2] = (rspf_uint8*)(tile->getBuf(2));
   }
   else
   {
      band[0] = (rspf_uint8*)(tile->getBuf(0));
      band[1] = (rspf_uint8*)(tile->getBuf(0));
      band[2] = (rspf_uint8*)(tile->getBuf(0));
   }

   if(band[0])
   {
      rspf_uint32 upper = tile->getWidth()*tile->getHeight();

      for(rspf_uint32 offset = 0; offset < upper; ++offset)
      {
         *outBand = theLut->findIndex(*band[0], *band[1], *band[2]);
         
         ++outBand;
         ++band[0];
         ++band[1];
         ++band[2];
      }
   }
   
   return theTile;
}

bool rspfRgbToIndexFilter::saveState(rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   rspfString newPrefix = prefix;
   newPrefix = newPrefix + "lut.";

   theLut->saveState(kwl, newPrefix.c_str());

   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfRgbToIndexFilter::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   rspfString newPrefix = prefix;
   newPrefix = newPrefix + "lut.";

   theLut->loadState(kwl, newPrefix.c_str());
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

rspf_uint32 rspfRgbToIndexFilter::getNumberOfOutputBands() const
{
   if(isSourceEnabled())
   {
      return 1;
   }
   
   return rspfImageSourceFilter::getNumberOfOutputBands();
}
   
rspfScalarType rspfRgbToIndexFilter::getOutputScalarType() const
{
   if(isSourceEnabled())
   {
      return RSPF_UCHAR;
   }
   
   return rspfImageSourceFilter::getOutputScalarType();
}

void rspfRgbToIndexFilter::setLut(rspfRgbLutDataObject& lut)
{
   theLut = (rspfRgbLutDataObject*) lut.dup();
}

double rspfRgbToIndexFilter::getNullPixelValue(rspf_uint32 band)const
{
   if(isSourceEnabled())
   {
      return 0.0;
   }
   
   return rspfImageSourceFilter::getNullPixelValue(band);
}

double rspfRgbToIndexFilter::getMinPixelValue(rspf_uint32 /* band */)const
{
   if(isSourceEnabled())
   {
      return 1.0;
   }
   
   return rspfImageSourceFilter::getMinPixelValue();
}

double rspfRgbToIndexFilter::getMaxPixelValue(rspf_uint32 /* band */)const
{
   if(isSourceEnabled())
   {
      return 255.0;
   }
   
   return rspfImageSourceFilter::getMaxPixelValue();
}
