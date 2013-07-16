//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfBrightnessMatch.cpp 11955 2007-10-31 16:10:22Z gpotts $

#include <rspf/imaging/rspfBrightnessMatch.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNormRgbVector.h>
#include <rspf/base/rspfHsiVector.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfNumericProperty.h>

RTTI_DEF1(rspfBrightnessMatch,
          "rspfBrightnessMatch",
          rspfImageSourceFilter)
   
rspfBrightnessMatch::rspfBrightnessMatch()
   :rspfImageSourceFilter(),
    theTargetBrightness(0.5),
    theInputBrightness(0.5)
{
   theInputBrightness = rspf::nan();
   theBrightnessContrastSource = new rspfBrightnessContrastSource;
}


rspfBrightnessMatch::~rspfBrightnessMatch()
{
}

rspfRefPtr<rspfImageData> rspfBrightnessMatch::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getTile(tileRect, resLevel);
   }
   if(theInputConnection)
   {
      if(rspf::isnan(theInputBrightness))
      {
         computeInputBrightness();
      }
      return theBrightnessContrastSource->getTile(tileRect, resLevel);
   }
   return 0;
}

void rspfBrightnessMatch::initialize()
{
   theBrightnessContrastSource->connectMyInputTo(0, getInput());
   theNormTile = 0;
}

void rspfBrightnessMatch::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property)
   {
      return;
   }
   rspfString name = property->getName();
   if(name == "input_brightness")
   {
      theInputBrightness = property->valueToString().toDouble();
      theBrightnessContrastSource->setBrightness(theTargetBrightness-theInputBrightness);
   }
   else if(name == "target_brightness")
   {
      theTargetBrightness = property->valueToString().toDouble();
      theBrightnessContrastSource->setBrightness(theTargetBrightness-theInputBrightness);
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfBrightnessMatch::getProperty(const rspfString& name)const
{
   if(name == "target_brightness")
   {
      rspfNumericProperty* numeric = new rspfNumericProperty(name,
                                                               rspfString::toString(theTargetBrightness),
                                                               0.0, 1.0);
      numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT64);
      numeric->setCacheRefreshBit();
      return numeric;
   }
   else if(name == "input_brightness")
   {
      rspfNumericProperty* numeric = new rspfNumericProperty(name,
                                                               rspfString::toString(theInputBrightness),
                                                               0.0, 1.0);
      numeric->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT64);
      numeric->setCacheRefreshBit();
      return numeric;
   }

    return rspfImageSourceFilter::getProperty(name);
}

void rspfBrightnessMatch::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
   
   propertyNames.push_back("input_brightness");
   propertyNames.push_back("target_brightness");
}


bool rspfBrightnessMatch::loadState(const rspfKeywordlist& kwl,
                                              const char* prefix)
{
   const char* input_brightness = kwl.find(prefix, "input_brightness");
   const char* target_brightness = kwl.find(prefix, "target_brightness");

   if(input_brightness)
   {
      theInputBrightness = rspfString(input_brightness).toDouble();
   }
   if(target_brightness)
   {
      theTargetBrightness = rspfString(target_brightness).toDouble();
   }
   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfBrightnessMatch::saveState(rspfKeywordlist& kwl,
                                              const char* prefix)const
{
   kwl.add(prefix,
           "input_brightness",
           theInputBrightness,
           true);
   kwl.add(prefix,
           "target_brightness",
           theTargetBrightness,
            true);

   return rspfImageSourceFilter::saveState(kwl, prefix);   
}


void rspfBrightnessMatch::computeInputBrightness()
{
   if(theInputConnection)
   {
      rspfIrect inputRect = getBoundingRect();
      rspf_uint32 rlevel = 0;
      rspf_uint32 nlevels = getNumberOfDecimationLevels();
      if(nlevels>1)
      {
         while((rspf::max(inputRect.width(), inputRect.height()) > 2048)&&
               (rlevel < nlevels))
         {
            ++rlevel;
            inputRect = getBoundingRect(rlevel);
         }
      }
      rspfIpt centerPt = inputRect.midPoint();
      centerPt.x -= 1024;
      centerPt.y -= 1024;
      rspfIrect reqRect(centerPt.x,
                         centerPt.y,
                         centerPt.x + 2047,
                         centerPt.x + 2047);
      reqRect = reqRect.clipToRect(inputRect);
      rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(reqRect, rlevel);

      if(inputTile.valid())
      {
         theNormTile = new rspfImageData(0,
                                          RSPF_FLOAT32,
                                          inputTile->getNumberOfBands());
         theNormTile->initialize();
         theNormTile->setImageRectangle(reqRect);
         inputTile->copyTileToNormalizedBuffer((rspf_float32*)theNormTile->getBuf());
         theNormTile->setDataObjectStatus(inputTile->getDataObjectStatus());
         rspf_uint32 maxIdx = theNormTile->getWidth()*theNormTile->getHeight();
         rspf_float32* bands[3];
         double averageI = 0.0;
         double count = 0.0;
         rspf_uint32 offset = 0;
         
         bands[0] = (rspf_float32*)theNormTile->getBuf();
         if(theNormTile->getNumberOfBands()>2)
         {
            bands[1] = (rspf_float32*)theNormTile->getBuf(1);
            bands[2] = (rspf_float32*)theNormTile->getBuf(2);
         }
         else
         {
            bands[1] = bands[0];
            bands[2] = bands[0];
         }
         rspfHsiVector hsi;
         if(theNormTile->getDataObjectStatus() == RSPF_FULL)
         {
            count = maxIdx;
            
            for(offset = 0; offset < maxIdx; ++offset)
            {
               hsi = rspfNormRgbVector(bands[0][offset], bands[1][offset], bands[2][offset]);
               
               averageI += hsi.getI();
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
                  hsi = rspfNormRgbVector(bands[0][offset], bands[1][offset], bands[2][offset]);
                  averageI += hsi.getI();
                  
                  ++count;
               }
            }
         }
         theInputBrightness = averageI / count;
         theBrightnessContrastSource->setBrightness(theTargetBrightness-theInputBrightness);
      }
      else
      {
         theInputBrightness = .5;
      }
   }
   theNormTile = 0;
}
