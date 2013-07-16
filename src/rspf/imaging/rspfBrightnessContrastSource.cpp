//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
// 
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfBrightnessContrastSource.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/imaging/rspfBrightnessContrastSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfHsiVector.h>
#include <rspf/base/rspfNormRgbVector.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfNumericProperty.h>

RTTI_DEF1(rspfBrightnessContrastSource,
          "rspfBrightnessContrastSource",
          rspfImageSourceFilter)
   
rspfBrightnessContrastSource::rspfBrightnessContrastSource()
   :rspfImageSourceFilter(),
    theBrightness(0.0),
    theContrast(1.0)
{  
}


rspfBrightnessContrastSource::~rspfBrightnessContrastSource()
{
}

rspfRefPtr<rspfImageData> rspfBrightnessContrastSource::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> tile = NULL;

   if(theInputConnection)
   {
      tile = theInputConnection->getTile(tileRect, resLevel);
      
      if(!tile.valid())
      {
         return tile;
      }

      if(!isSourceEnabled() ||
         (tile->getDataObjectStatus()==RSPF_NULL)||
         (tile->getDataObjectStatus()==RSPF_EMPTY)||
         ( (theBrightness == 0.0) && (theContrast   == 1.0) ) )
      {
         return tile;
      }

      if(!theTile.valid() || !theNormTile.valid())
      {
         allocate();
      }
      
      if(!theTile.valid() || !theNormTile.valid())
      {
         return tile;
      }

      theTile->setImageRectangle(tileRect);
      theNormTile->setImageRectangle(tileRect);

      tile->copyTileToNormalizedBuffer((rspf_float32*)theNormTile->getBuf());
      theNormTile->setDataObjectStatus(tile->getDataObjectStatus());

      if (theNormTile->getNumberOfBands() == 3)
      {
         processRgbTile();
      }
      else
      {
         processNBandTile();
      }
      
      theTile->copyNormalizedBufferToTile((rspf_float32*)
                                          theNormTile->getBuf());
      theTile->validate();
      return theTile;
   }
   
   return tile;
}

void rspfBrightnessContrastSource::processRgbTile()
{
   rspf_float32* bands[3];

   bands[0] = (rspf_float32*)theNormTile->getBuf(0);
   bands[1] = (rspf_float32*)theNormTile->getBuf(1);
   bands[2] = (rspf_float32*)theNormTile->getBuf(2);

   rspf_uint32 offset = 0;
   rspf_uint32 maxIdx = theNormTile->getWidth()*theNormTile->getHeight();
   rspfHsiVector hsi;
   rspf_float32 i;
   if(theNormTile->getDataObjectStatus() == RSPF_FULL)
   {
      for(offset = 0; offset < maxIdx; ++offset)
      {
         rspfNormRgbVector rgb(bands[0][offset], bands[1][offset], bands[2][offset]);
         hsi = rgb;
         i = (hsi.getI()*theContrast + theBrightness);
         if(i < RSPF_DEFAULT_MIN_PIX_NORM_FLOAT) i = RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
         if(i > 1.0) i = 1.0;
         hsi.setI(i);
         rgb = hsi;
         bands[0][offset] = rgb.getR();
         bands[1][offset] = rgb.getG();
         bands[2][offset] = rgb.getB();
      }
   }
   else
   {
      for(offset = 0; offset < maxIdx; ++offset)
      {
         if((bands[0][offset] != 0.0)&&
            (bands[1][offset] != 0.0)&&
            (bands[2][offset] != 0.0))
         {
            rspfNormRgbVector rgb(bands[0][offset], bands[1][offset], bands[2][offset]);
            hsi = rgb;
            i = (hsi.getI()*theContrast + theBrightness);
            if(i < RSPF_DEFAULT_MIN_PIX_NORM_FLOAT) i = RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
            if(i > 1.0) i = 1.0;
            hsi.setI(i);
            rgb = hsi;
            bands[0][offset] = rgb.getR();
            bands[1][offset] = rgb.getG();
            bands[2][offset] = rgb.getB();
         }
      }
   }
}

void rspfBrightnessContrastSource::processNBandTile()
{
   const rspf_uint32  BANDS = theNormTile->getNumberOfBands();
   const rspf_uint32  PPB   = theNormTile->getSizePerBand();
   const rspf_float32 MP    = theNormTile->getMinNormalizedPix();
   
   for (rspf_uint32 band = 0; band < BANDS; ++band)
   {
      rspf_float32* buf = theNormTile->getFloatBuf(band);

      for (rspf_uint32 i = 0; i < PPB; ++i)
      {
         if (buf[i] != 0.0)
         {
            rspf_float32 p = buf[i] * theContrast + theBrightness;
            buf[i] = (p < 1.0) ? ( (p > MP) ? p : 0.0 ) : 1.0;
         }
      }
   }
}

void rspfBrightnessContrastSource::initialize()
{
   rspfImageSourceFilter::initialize();

   theTile     = 0;
   theNormTile = 0;
}

void rspfBrightnessContrastSource::allocate()
{
   if( isSourceEnabled() && theInputConnection )
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);
      if(theTile.valid())
      {
         theNormTile = rspfImageDataFactory::instance()->create(this,
                                                                 RSPF_FLOAT32,
                                                                 theTile->getNumberOfBands());
      }
      if(theTile.valid() && theNormTile.valid())
      {
         theTile->initialize();
         theNormTile->initialize();
      }
      else
      {
         theTile     = 0;
         theNormTile = 0;
      }
   }
}

void rspfBrightnessContrastSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property)
   {
      return;
   }
   rspfString name = property->getName();
   if(name == "brightness")
   {
      theBrightness = property->valueToString().toDouble();
   }
   else if(name == "contrast")
   {
      theContrast = property->valueToString().toDouble();
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfBrightnessContrastSource::getProperty(const rspfString& name)const
{
   if(name == "brightness")
   {
      rspfNumericProperty* numeric = new rspfNumericProperty(name,
                                                               rspfString::toString(getBrightness()),
                                                               -1.0, 1.0);
      numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT64);
      numeric->setCacheRefreshBit();
      return numeric;
   }
   else if(name == "contrast")
   {
      rspfNumericProperty* numeric = new rspfNumericProperty(name,
                                                               rspfString::toString(getContrast()),
                                                               0.0, 20.0);
      numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT64);
      numeric->setCacheRefreshBit();
      return numeric;
   }

    return rspfImageSourceFilter::getProperty(name);
}

void rspfBrightnessContrastSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   
   propertyNames.push_back("brightness");
   propertyNames.push_back("contrast");
}


bool rspfBrightnessContrastSource::loadState(const rspfKeywordlist& kwl,
                                              const char* prefix)
{
   const char* brightness = kwl.find(prefix, "brightness");
   const char* contrast = kwl.find(prefix, "contrast");

   if(brightness)
   {
      theBrightness = rspfString(brightness).toDouble();
   }
   if(contrast)
   {
      theContrast   = rspfString(contrast).toDouble();
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfBrightnessContrastSource::saveState(rspfKeywordlist& kwl,
                                              const char* prefix)const
{
   kwl.add(prefix,
           "brightness",
           theBrightness,
           true);
   kwl.add(prefix,
           "contrast",
           theContrast,
           true);

   return rspfImageSourceFilter::saveState(kwl, prefix);   
}


void rspfBrightnessContrastSource::setBrightnessContrast(
   rspf_float64 brightness, rspf_float64 contrast)
{
   theBrightness = brightness;
   theContrast   = contrast;
}

void rspfBrightnessContrastSource::setBrightness(rspf_float64 brightness)
{
   setBrightnessContrast(brightness, getContrast());
}

void rspfBrightnessContrastSource::setContrast(rspf_float64 contrast)
{
   setBrightnessContrast(getBrightness(), contrast);
}

rspf_float64 rspfBrightnessContrastSource::getBrightness()const
{
   return theBrightness;
}

rspf_float64 rspfBrightnessContrastSource::getContrast()const
{
   return theContrast;
}
