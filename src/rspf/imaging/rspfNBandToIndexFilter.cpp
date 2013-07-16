//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfNBandToIndexFilter.cpp 19732 2011-06-06 22:24:54Z dburken $

#include <rspf/imaging/rspfNBandToIndexFilter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfNotifyContext.h>


static const char* KEEP_QUANTIZED_VALUE_FLAG_KW = "keep_quantized_value_flag";

RTTI_DEF1(rspfNBandToIndexFilter,
          "rspfNBandToIndexFilter",
          rspfImageSourceFilter);

rspfNBandToIndexFilter::rspfNBandToIndexFilter()
   :rspfImageSourceFilter(),
    theLut(0),
    theTile(0),
    theKeepQuantizedValueFlag(false)
{
}

rspfNBandToIndexFilter::~rspfNBandToIndexFilter()
{
}

void rspfNBandToIndexFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   // Force allocate on next getTile.
   theTile      = 0;
   if(theLut.valid())
   {
      if(theKeepQuantizedValueFlag)
      {
         rspfNBandLutDataObject::LUT_ENTRY_TYPE minValue;
         rspfNBandLutDataObject::LUT_ENTRY_TYPE maxValue;
         rspf_uint32 bands = theLut->getNumberOfBands();
         rspf_uint32 idx   = 0;
         if(theMinValues.size() != bands)
         {
            theMinValues.resize(bands);
            theMaxValues.resize(bands);
            theNullValues.resize(bands);
         }
         std::fill(theNullValues.begin(),
                   theNullValues.end(), 0);
         for(idx = 0; idx < bands; ++idx)
         {
            theLut->getMinMax(idx, minValue, maxValue);
            theMinValues[idx] = minValue;
            theMaxValues[idx] = maxValue;
            theNullValues[idx] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(rspfImageSourceFilter::getNullPixelValue(idx));
         }
      }
      else
      {
         if(theMinValues.size() != 1)
         {
            theMinValues.resize(1);
            theMaxValues.resize(1);
            theNullValues.resize(1);
         }
         theMinValues[0]  = 0;
         theMaxValues[0]  = theLut->getNumberOfEntries()-1;
         theNullValues[0] = theLut->getNullPixelIndex();
      }
   }
}

void rspfNBandToIndexFilter::allocate()
{
   theTile      = 0;
   
   if(isSourceEnabled()&&theInputConnection)
   {
      theTile      = rspfImageDataFactory::instance()->create(this, this);
      if(theTile.valid())
      {
         theTile->initialize();
      }
   }
}

void rspfNBandToIndexFilter::disableSource()
{
   rspfImageSourceFilter::disableSource();
   theTile = 0;
}

rspfRefPtr<rspfImageData> rspfNBandToIndexFilter::getTile(const rspfIrect& origin,
                                                             rspf_uint32 resLevel)
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getTile(origin, resLevel);
   }
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
   theTile->setDataObjectStatus(RSPF_FULL);
   theTile->makeBlank();

   return convertInputTile(input.get());
}

rspfRefPtr<rspfImageData> rspfNBandToIndexFilter::convertInputTile(rspfImageData* tile)
{
   switch(getOutputScalarType())
   {
      case RSPF_UINT8:
      {
         return convertOutputTileTemplate((rspf_uint8)0,
                                          tile);
         break;
      }
      case RSPF_UINT16:
      {
         return convertOutputTileTemplate((rspf_uint16)0,
                                          tile);
         break;
      }
      case RSPF_UINT32:
      {
         return convertOutputTileTemplate((rspf_uint32)0,
                                         tile);
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN) << "rspfNBandToIndexFilter::convertInputTile: Unsupported scalar type for conversion" << std::endl;
      }
   }

   return theTile;
}

template <class T>
rspfRefPtr<rspfImageData> rspfNBandToIndexFilter::convertOutputTileTemplate(T outputDummy,
                                                                               rspfImageData* inputTile)
{
   switch(inputTile->getScalarType())
   {
      case RSPF_UINT8:
      {
         return convertInputTileToOutputTemplate((rspf_uint8)0,
                                                 outputDummy,
                                                 inputTile);
         break;
      }
      case RSPF_SINT8:
      {
         return convertInputTileToOutputTemplate((rspf_sint8)0,
                                                 outputDummy,
                                                 inputTile);
      }
      case RSPF_UINT16:
      {
         return convertInputTileToOutputTemplate((rspf_uint16)0,
                                                 outputDummy,
                                                 inputTile);
      }
      case RSPF_SINT32:
      {
         return convertInputTileToOutputTemplate((rspf_sint32)0,
                                                 outputDummy,
                                                 inputTile);
      }
      case RSPF_UINT32:
      {
         return convertInputTileToOutputTemplate((rspf_uint32)0,
                                                 outputDummy,
                                                 inputTile);
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN) << "convertInputTileToOutputTemplate::convertOutputTileTemplate: Unsupported scalar type for conversion." << std::endl;
      }
   }

   return theTile;
}

template <class T1, class T2>
rspfRefPtr<rspfImageData> rspfNBandToIndexFilter::convertInputTileToOutputTemplate(
   T1 /* inputDummy */,
   T2 /* outputDummy */,
   rspfImageData* inputTile)
{
   if (!inputTile)
   {
      return theTile;
   }
   
   const rspf_uint32 BANDS = inputTile->getNumberOfBands();

   if (!BANDS)
   {
      return theTile;
   }
//   T2** outBand = (T2*)theTile->getBuf();

   rspf_uint32 numberOfBands = theLut->getNumberOfBands();
   
   rspfNBandLutDataObject::LUT_ENTRY_TYPE* bandValues = new rspfNBandLutDataObject::LUT_ENTRY_TYPE[numberOfBands];

   memset(bandValues, 0, sizeof(rspfNBandLutDataObject::LUT_ENTRY_TYPE)*numberOfBands);
   numberOfBands = rspf::min(BANDS,
                            numberOfBands);
   if(!numberOfBands) return theTile;
   T1** band = new T1*[numberOfBands];
   T2** outBand = new T2*[numberOfBands];
   rspf_int32 idx   = 0;

   for(idx = 0; idx < (rspf_int32)numberOfBands; ++idx)
   {
      band[idx] = (T1*)inputTile->getBuf(idx);
      outBand[idx] = (T2*)theTile->getBuf(idx);
   }
   if(band[0])
   {
      if(inputTile->getDataObjectStatus() == RSPF_FULL)
      {
         rspf_uint32 upper = inputTile->getWidth()*inputTile->getHeight();
         rspf_uint32 idx2=0;
         for(rspf_uint32 offset = 0; offset < upper; ++offset)
         {
            for(idx2 = 0; idx2 < numberOfBands; ++idx2)
            {
               bandValues[idx2] = *band[idx2];
               ++band[idx2];
            }
            //---
            // Note call the rspfNBandLutDataObject::findIndex that takes a size as the input
            // data may have dropped the alpha channel.
            //---
            idx = theLut->findIndex(bandValues, numberOfBands);

            if(!theKeepQuantizedValueFlag)
            {
               *outBand[0] = (T2)idx;
               ++outBand[0];
            }
            else
            {
               for(idx2 = 0; idx2 < numberOfBands; ++idx2)
               {
                  if(idx >=0)
                  {
                     *outBand[idx2] = (T2)(*theLut)[idx][idx2];
                     ++outBand[idx2];
                  }
               }
            }
         }
      }
      else if(inputTile->getDataObjectStatus() == RSPF_PARTIAL)
      {
         rspf_uint32 upper = inputTile->getWidth()*inputTile->getHeight();
         rspf_uint32 idx2=0;
         for(rspf_uint32 offset = 0; offset < upper; ++offset)
         {
            for(idx2 = 0; idx2 < numberOfBands; ++idx2)
            {
               bandValues[idx2] = *band[idx2];
               ++band[idx2];
            }
            idx = -1;
            if(!inputTile->isNull(offset))
            {
               //---
               // Note call the rspfNBandLutDataObject::findIndex that takes a size as the input
               // data may have dropped the alpha channel.
               //---
               idx = theLut->findIndex(bandValues, numberOfBands);
            }
            if(!theKeepQuantizedValueFlag)
            {
               if(idx>=0)
               {
                  *outBand[0] = (T2)idx;
               }
               ++outBand[0];
            }
            else
            {
               for(idx2 = 0; idx2 < numberOfBands; ++idx2)
               {
                  if(idx >=0)
                  {
                     if(idx>=0)
                     {
                        *outBand[idx2] = (T2)(*theLut)[idx][idx2];
                     }
                  }
                  ++outBand[idx2];
               }
            }
         }
      }
   }
   if(theKeepQuantizedValueFlag)
   {
      theTile->validate();
   }
   else
   {
      theTile->setDataObjectStatus(RSPF_FULL);
   }
   return theTile;
}

bool rspfNBandToIndexFilter::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   rspfString newPrefix = prefix;
   newPrefix = newPrefix + "lut.";

   if(theLut.valid())
   {
      theLut->saveState(kwl, newPrefix.c_str());
   }

   kwl.add(prefix,
           KEEP_QUANTIZED_VALUE_FLAG_KW,
           theKeepQuantizedValueFlag?"true":"false",
           true);
           
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfNBandToIndexFilter::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   rspfString newPrefix = prefix;
   newPrefix = newPrefix + "lut.";

   if(!theLut.valid())
   {
      theLut = new rspfNBandLutDataObject;
   }
   theLut->loadState(kwl, newPrefix.c_str());

   const char* flag = kwl.find(prefix, KEEP_QUANTIZED_VALUE_FLAG_KW);
   if(flag)
   {
      theKeepQuantizedValueFlag = rspfString(flag).toBool();
   }
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

bool rspfNBandToIndexFilter::isSourceEnabled()const
{
   bool result = rspfSource::isSourceEnabled()&&theInputConnection;

   // only support 3 band integral values, no floating point for now.
   //
   if(result)
   {
      
      rspfScalarType inputScalarType = theInputConnection->getOutputScalarType();
      if(!theLut.valid() ||
         (theLut->getNumberOfEntries()<1)||
         (inputScalarType==RSPF_FLOAT32)||
         (inputScalarType==RSPF_FLOAT64)||
         (theInputConnection->getNumberOfOutputBands()!=3))
      {
         result = false;
      }
   }
   
   return result;
}

rspf_uint32 rspfNBandToIndexFilter::getNumberOfOutputBands() const
{
   if(isSourceEnabled())
   {
      if(theKeepQuantizedValueFlag)
      {
         if(theLut.valid())
         {
            return theLut->getNumberOfBands();
         }
      }
      else
      {
         return 1;
      }
   }
   
   return rspfImageSourceFilter::getNumberOfOutputBands();
}
   
rspfScalarType rspfNBandToIndexFilter::getOutputScalarType() const
{
   if(isSourceEnabled())
   {
      
      rspf_uint32 numberOfEntries = theLut->getNumberOfEntries();
      
      if(numberOfEntries < 257)
      {
         return RSPF_UCHAR;
      }
      else if(numberOfEntries < 65537)
      {
         return RSPF_UINT16;
      }
      else
      {
         return RSPF_UINT32;
      }
   }
   
   return rspfImageSourceFilter::getOutputScalarType();
}

void rspfNBandToIndexFilter::setLut(rspfNBandLutDataObject& lut)
{
   theLut = new rspfNBandLutDataObject(lut);
}

double rspfNBandToIndexFilter::getNullPixelValue(rspf_uint32 band)const
{
   if(isSourceEnabled())
   {
      if(theLut->getNullPixelIndex() >= 0)
      {
         return theLut->getNullPixelIndex();
      }
   }
   
   return rspfImageSourceFilter::getNullPixelValue(band);
}

double rspfNBandToIndexFilter::getMinPixelValue(rspf_uint32 band)const
{
   if(isSourceEnabled())
   {
      if(band < theMinValues.size())
      {
         return (double)theMinValues[band];
      }
   }
//    if(isSourceEnabled())
//    {
//       if(getNullPixelValue() != 0)
//       {
//          return 0;
//       }
//       else
//       {
//          return 1;
//       }
//       return 0;
//    }
   
   return rspfImageSourceFilter::getMinPixelValue();
}

double rspfNBandToIndexFilter::getMaxPixelValue(rspf_uint32 band)const
{
   if(isSourceEnabled())
   {
      if(band < theMaxValues.size())
      {
         return (double)theMaxValues[band];
      }
//       if(theLut->getNullPixelIndex() != ((rspf_int32)(theLut->getNumberOfEntries())-1))
//       {
//          return theLut->getNumberOfEntries();
//       }
//       else
//       {
//          return (theLut->getNumberOfEntries()-1);
//       }
   }
   
   return rspfImageSourceFilter::getMaxPixelValue();
}

void rspfNBandToIndexFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(property->getName() == rspfKeywordNames::FILENAME_KW)
   {
      rspfKeywordlist kwl;
      if(kwl.addFile(rspfFilename(property->valueToString())))
      {
         theLut = new rspfNBandLutDataObject;
         theLut->loadState(kwl);
      }
   }
   else if(property->getName() == KEEP_QUANTIZED_VALUE_FLAG_KW)
   {
      theKeepQuantizedValueFlag = property->valueToString().toBool();
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfNBandToIndexFilter::getProperty(const rspfString& name)const
{
   if(name == rspfKeywordNames::FILENAME_KW)
   {
      rspfFilenameProperty* property = new rspfFilenameProperty(name, theLutFilename);
      property->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);
      property->setCacheRefreshBit();
      
      return property;
   }
   else if(name == KEEP_QUANTIZED_VALUE_FLAG_KW)
   {
      rspfBooleanProperty* property = new rspfBooleanProperty(name, theKeepQuantizedValueFlag);
      property->setCacheRefreshBit();

      return property;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfNBandToIndexFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);

   propertyNames.push_back(rspfKeywordNames::FILENAME_KW);
   propertyNames.push_back(KEEP_QUANTIZED_VALUE_FLAG_KW);
}
