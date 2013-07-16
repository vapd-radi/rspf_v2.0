//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfMeanMedianFilter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfMeanMedianFilter.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/imaging/rspfImageData.h>
#include <vector>
#include <algorithm>
#include <numeric>
using namespace std;


RTTI_DEF1(rspfMeanMedianFilter,
          "rspfMeanMedianFilter",
          rspfImageSourceFilter);

// Keywords used throughout.
static const rspfString WINDOW_SIZE_KW = "window_size";
static const rspfString FILTER_TYPE_KW = "filter_type";
static const rspfString AUTO_GROW_KW   = "auto_grow_rectangle_flag";

rspfMeanMedianFilter::rspfMeanMedianFilter(rspfObject* owner)
   :rspfImageSourceFilter(owner),
    theTile(0),
    theFilterType(RSPF_MEDIAN),
    theWindowSize(3),
    theEnableFillNullFlag(false),
    theAutoGrowRectFlag(false)
{
   setDescription(rspfString("Mean Median Filter"));
}

rspfMeanMedianFilter::~rspfMeanMedianFilter()
{
}

rspfRefPtr<rspfImageData> rspfMeanMedianFilter::getTile(
   const rspfIrect& rect, rspf_uint32 resLevel)
{
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getTile(rect, resLevel);
   }

   rspf_uint32 halfSize = getWindowSize()>>1;

   rspfIrect requestRect(rect.ul().x - halfSize,
                          rect.ul().y - halfSize,
                          rect.lr().x + halfSize,
                          rect.lr().y + halfSize);

   rspfRefPtr<rspfImageData> inputData =
      rspfImageSourceFilter::getTile(requestRect, resLevel);
   if(!inputData.valid() || !inputData->getBuf())
   {
      return inputData;
   }

   if(!theTile.valid())
   {
      theTile = (rspfImageData*)inputData->dup();
      theTile->setImageRectangle(rect);
   }
   else
   {
      theTile->setImageRectangleAndBands(rect, inputData->getNumberOfBands());
   }

   applyFilter(inputData);

   theTile->setDataObjectStatus(inputData->getDataObjectStatus());
   if(theEnableFillNullFlag)
   {
      theTile->validate();
   }

   return theTile;
}

void rspfMeanMedianFilter::setWindowSize(rspf_uint32 windowSize)
{
   theWindowSize = windowSize;
}

rspf_uint32 rspfMeanMedianFilter::getWindowSize()const
{
   return theWindowSize;
}

void rspfMeanMedianFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   theTile = NULL;
}

void rspfMeanMedianFilter::applyFilter(rspfRefPtr<rspfImageData>& input)
{
   switch(input->getScalarType())
   {
      case RSPF_UINT8:
      {
         switch (theFilterType)
         {
            case RSPF_MEDIAN:
            case RSPF_MEDIAN_FILL_NULLS:
               applyMedian(rspf_uint8(0), input);
               break;
               
            case RSPF_MEDIAN_NULL_CENTER_ONLY:
               applyMedianNullCenterOnly(rspf_uint8(0), input);
               break;
               
            case RSPF_MEAN:
            case RSPF_MEAN_FILL_NULLS:
               applyMean(rspf_uint8(0), input);
               break;

            case RSPF_MEAN_NULL_CENTER_ONLY:
               applyMeanNullCenterOnly(rspf_uint8(0), input);
               
            default:
               break;
         }
         break;
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         switch (theFilterType)
         {
            case RSPF_MEDIAN:
            case RSPF_MEDIAN_FILL_NULLS:
               applyMedian(rspf_uint16(0), input);
               break;
               
            case RSPF_MEDIAN_NULL_CENTER_ONLY:
               applyMedianNullCenterOnly(rspf_uint16(0), input);
               break;
               
            case RSPF_MEAN:
            case RSPF_MEAN_FILL_NULLS:
               applyMean(rspf_uint16(0), input);
               break;

            case RSPF_MEAN_NULL_CENTER_ONLY:
               applyMeanNullCenterOnly(rspf_uint16(0), input);
               break;
               
            default:
               break;
         }
         break;
      }
      case RSPF_SINT16:
      {
         switch (theFilterType)
         {
            case RSPF_MEDIAN:
            case RSPF_MEDIAN_FILL_NULLS:
               applyMedian(rspf_sint16(0), input);
               break;
               
            case RSPF_MEDIAN_NULL_CENTER_ONLY:
               applyMedianNullCenterOnly(rspf_sint16(0), input);
               break;
               
            case RSPF_MEAN:
            case RSPF_MEAN_FILL_NULLS:
               applyMean(rspf_sint16(0), input);
               break;

            case RSPF_MEAN_NULL_CENTER_ONLY:
               applyMeanNullCenterOnly(rspf_sint16(0), input);
               break;
               
            default:
               break;
         }
         break;
      }
      case RSPF_UINT32:
      {
         switch (theFilterType)
         {
            case RSPF_MEDIAN:
            case RSPF_MEDIAN_FILL_NULLS:
               applyMedian(rspf_uint32(0), input);
               break;
               
            case RSPF_MEDIAN_NULL_CENTER_ONLY:
               applyMedianNullCenterOnly(rspf_uint32(0), input);
               break;
               
            case RSPF_MEAN:
            case RSPF_MEAN_FILL_NULLS:
               applyMean(rspf_uint32(0), input);
               break;

            case RSPF_MEAN_NULL_CENTER_ONLY:
               applyMeanNullCenterOnly(rspf_uint32(0), input);
               break;
               
            default:
               break;
         }
      }
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         switch (theFilterType)
         {
            case RSPF_MEDIAN:
            case RSPF_MEDIAN_FILL_NULLS:
               applyMedian(rspf_float32(0.0), input);
               break;
               
            case RSPF_MEDIAN_NULL_CENTER_ONLY:
               applyMedianNullCenterOnly(rspf_float32(0.0), input);
               break;
               
            case RSPF_MEAN:
            case RSPF_MEAN_FILL_NULLS:
               applyMean(rspf_float32(0.0), input);
               break;

            case RSPF_MEAN_NULL_CENTER_ONLY:
               applyMeanNullCenterOnly(rspf_float32(0.0), input);
               break;
               
            default:
               break;
         }
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         switch (theFilterType)
         {
            case RSPF_MEDIAN:
            case RSPF_MEDIAN_FILL_NULLS:
               applyMedian(rspf_float64(0.0), input);
               break;
               
            case RSPF_MEDIAN_NULL_CENTER_ONLY:
               applyMedianNullCenterOnly(rspf_float64(0.0), input);
               break;
               
            case RSPF_MEAN:
            case RSPF_MEAN_FILL_NULLS:
               applyMean(rspf_float64(0.0), input);
               break;

            case RSPF_MEAN_NULL_CENTER_ONLY:
               applyMeanNullCenterOnly(rspf_float64(0.0), input);
               break;
               
            default:
               break;
         }
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfMeanMedianFilter::applyFilter WARNING:\n"
            << "Unhandled scalar type!" << endl;
      }
   }
}

template <class T>
void rspfMeanMedianFilter::applyMean(T /* dummyVariable */,
                                      rspfRefPtr<rspfImageData>& inputData)
{
   rspf_uint32 halfWindow = (theWindowSize >> 1);
   rspf_uint32 bandIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 kernelX   = 0;
   rspf_uint32 kernelY   = 0;
   rspf_uint32 kernelIdx = 0;
   rspf_uint32 iw  = inputData->getWidth();
   rspf_uint32 ow  = theTile->getWidth();
   rspf_uint32 oh = theTile->getHeight();
   rspf_uint32 numberOfBands = rspf::min(theTile->getNumberOfBands(),
                                         inputData->getNumberOfBands());
   rspfDataObjectStatus status = inputData->getDataObjectStatus();
   std::vector<double> values(theWindowSize*theWindowSize);

   if(status == RSPF_FULL)
   {
      for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
      {
         T* inputBuf     = (T*)inputData->getBuf(bandIdx);
         T* outputBuf    = (T*)theTile->getBuf(bandIdx);

         if(inputBuf&&outputBuf)
         {
            for(y = 0; y < oh; ++y)
            {
               for(x = 0; x < ow; ++x)
               {
                  kernelIdx = 0;
                  for(kernelY = 0; kernelY < theWindowSize; ++kernelY)
                  {
                     for(kernelX = 0; kernelX < theWindowSize;++kernelX)
                     {
                        values[kernelIdx] = *(inputBuf+kernelX + kernelY*iw);
                        ++kernelIdx;
                     }
                  }


                  if(values.size() > 0)
                  {
                     double sum = std::accumulate(values.begin(),
                                                  values.end(),
                                                  0.0);
                     double average = sum/(double)values.size();
                     (*outputBuf) = (T)average;
                  }
                  ++inputBuf;
                  ++outputBuf;
               }

               inputBuf+=(halfWindow<<1);
            }
         }
      }
   }
   else
   {
      for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
      {
         T* inputBuf     = (T*)inputData->getBuf(bandIdx);
         T* outputBuf    = (T*)theTile->getBuf(bandIdx);
         T np            = (T)inputData->getNullPix(bandIdx);
         if(inputBuf&&outputBuf)
         {
            for(y = 0; y < oh; ++y)
            {
               for(x = 0; x < ow; ++x)
               {
                  values.clear();
                  for(kernelY = 0; kernelY < theWindowSize; ++kernelY)
                  {
                     for(kernelX = 0; kernelX < theWindowSize;++kernelX)
                     {
                        T tempValue = *(inputBuf+kernelX + kernelY*iw);

                        if(tempValue != np)
                        {
                           values.push_back((double)tempValue);
                        }
                     }
                  }


                  if(values.size() > 0)
                  {
                     double accumulate = std::accumulate(values.begin(),
                                                         values.end(),
                                                         0.0);
                     double average = accumulate/(double)values.size();
                     if(*(inputBuf+halfWindow + halfWindow*iw) == np)
                     {
                        if(theEnableFillNullFlag)
                        {
                           (*outputBuf) = (T)average;
                        }
                        else
                        {
                           (*outputBuf) = np;
                        }
                     }
                     else
                     {
                        (*outputBuf) = (T)average;
                     }
                  }
                  else
                  {
                     *outputBuf = np;
                  }
                  ++inputBuf;
                  ++outputBuf;
               }

               inputBuf+=(halfWindow<<1);
            }
         }
      }
   }
}

template <class T> void rspfMeanMedianFilter::applyMeanNullCenterOnly(
   T /* dummyVariable */,
   rspfRefPtr<rspfImageData>& inputData)
{
   rspf_uint32 halfWindow = (theWindowSize >> 1);
   rspf_uint32 bandIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 kernelX   = 0;
   rspf_uint32 kernelY   = 0;
   rspf_uint32 iw  = inputData->getWidth();
   rspf_uint32 ow  = theTile->getWidth();
   rspf_uint32 oh = theTile->getHeight();
   rspf_uint32 numberOfBands = rspf::min(theTile->getNumberOfBands(),
                                         inputData->getNumberOfBands());
   rspfDataObjectStatus status = inputData->getDataObjectStatus();
   std::vector<double> values;

   if(status == RSPF_FULL)
   {
      // Nothing to do just copy the tile.
      theTile->loadTile(inputData.get());
   }
   else
   {
      // Partial tile with nulls in it.
      for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
      {
         T* inputBuf     = (T*)inputData->getBuf(bandIdx);
         T* outputBuf    = (T*)theTile->getBuf(bandIdx);
         if (!inputBuf || !outputBuf)
         {
            return; // Shouldn't happen...
         }
         
         const T NP = (T)inputData->getNullPix(bandIdx);

         for(y = 0; y < oh; ++y)
         {
            for(x = 0; x < ow; ++x)
            {
               // Get the center input pixel.
               const T CP = *(inputBuf+halfWindow + halfWindow*iw);
               if (CP == NP)
               {
                  values.clear();
                  for(kernelY = 0; kernelY < theWindowSize; ++kernelY)
                  {
                     for(kernelX = 0; kernelX < theWindowSize;++kernelX)
                     {
                        T tempValue = *(inputBuf+kernelX + kernelY*iw);
                        
                        if(tempValue != NP)
                        {
                           values.push_back((double)tempValue);
                        }
                     }
                  }
                  
                  if(values.size() > 0)
                  {
                     double accumulate = std::accumulate(values.begin(),
                                                         values.end(),
                                                         0.0);
                     double average = accumulate/(double)values.size();
                     (*outputBuf) = (T)average;
                  }
                  else
                  {
                     (*outputBuf) = NP;
                  }
                  
               }
               else // Center pixel (CP) not null.
               {
                  (*outputBuf) = CP;
               }
               
               // Move over...
               ++inputBuf;
               ++outputBuf;
               
            } // End of loop in x direction.

            // Move down...
            inputBuf+=(halfWindow<<1);
            
         }  // End of loop in y direction.
         
      }  // End of band loop.
      
   }  // End of else "partial tile" block.
}

template <class T>
void rspfMeanMedianFilter::applyMedian(T /* dummyVariable */,
                                        rspfRefPtr<rspfImageData>& inputData)
{
   rspf_uint32 halfWindow = (theWindowSize >> 1);
   rspf_uint32 bandIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 kernelX   = 0;
   rspf_uint32 kernelY   = 0;
   rspf_uint32 kernelIdx = 0;
   rspf_uint32 iw  = inputData->getWidth();
   rspf_uint32 ow  = theTile->getWidth();
   rspf_uint32 oh = theTile->getHeight();
   rspf_uint32 numberOfBands = rspf::min(theTile->getNumberOfBands(),
                                         inputData->getNumberOfBands());
   rspfDataObjectStatus status = inputData->getDataObjectStatus();
   std::vector<T> values(theWindowSize*theWindowSize);

   if(status == RSPF_FULL)
   {
      for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
      {
         T* inputBuf     = (T*)inputData->getBuf(bandIdx);
         T* outputBuf    = (T*)theTile->getBuf(bandIdx);

         if(inputBuf&&outputBuf)
         {
            for(y = 0; y < oh; ++y)
            {
               for(x = 0; x < ow; ++x)
               {
                  kernelIdx = 0;
                  for(kernelY = 0; kernelY < theWindowSize; ++kernelY)
                  {
                     for(kernelX = 0; kernelX < theWindowSize;++kernelX)
                     {
                        values[kernelIdx] = *(inputBuf+kernelX + kernelY*iw);
                        ++kernelIdx;
                     }
                  }

                  std::sort(values.begin(),
                            values.end());

                  if(values.size() > 0)
                  {
                     (*outputBuf) = values[values.size()>>1];
                  }
                  ++inputBuf;
                  ++outputBuf;
               }

               inputBuf+=(halfWindow<<1);
            }
         }
      }
   }
   else
   {
      for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
      {
         T* inputBuf     = (T*)inputData->getBuf(bandIdx);
         T* outputBuf    = (T*)theTile->getBuf(bandIdx);
         T np            = (T)inputData->getNullPix(bandIdx);
         if(inputBuf&&outputBuf)
         {
            for(y = 0; y < oh; ++y)
            {
               for(x = 0; x < ow; ++x)
               {
                  values.clear();
                  for(kernelY = 0; kernelY < theWindowSize; ++kernelY)
                  {
                     for(kernelX = 0; kernelX < theWindowSize;++kernelX)
                     {
                        T tempValue = *(inputBuf+kernelX + kernelY*iw);

                        if(tempValue != np)
                        {
                           values.push_back(tempValue);
                        }
                     }
                  }

                  std::sort(values.begin(),
                            values.end());

                  if(values.size() > 0)
                  {
                     if(*(inputBuf+halfWindow + halfWindow*iw) == np)
                     {
                        if(theEnableFillNullFlag)
                        {
                           (*outputBuf) = values[values.size()>>1];
                        }
                        else
                        {
                           (*outputBuf) = np;
                        }
                     }
                     else
                     {
                        (*outputBuf) = values[values.size()>>1];
                     }
                  }
                  else
                  {
                     *outputBuf = np;
                  }
                  ++inputBuf;
                  ++outputBuf;
               }

               inputBuf+=(halfWindow<<1);
            }
         }
      }
   }
}

template <class T> void rspfMeanMedianFilter::applyMedianNullCenterOnly (
   T /* dummyVariable */,
   rspfRefPtr<rspfImageData>& inputData)
{
   rspf_uint32 halfWindow = (theWindowSize >> 1);
   rspf_uint32 bandIdx = 0;
   rspf_uint32 x = 0;
   rspf_uint32 y = 0;
   rspf_uint32 kernelX   = 0;
   rspf_uint32 kernelY   = 0;
   rspf_uint32 iw  = inputData->getWidth();
   rspf_uint32 ow  = theTile->getWidth();
   rspf_uint32 oh = theTile->getHeight();
   rspf_uint32 numberOfBands = rspf::min(theTile->getNumberOfBands(),
                                         inputData->getNumberOfBands());
   rspfDataObjectStatus status = inputData->getDataObjectStatus();
   std::vector<T> values;

   if(status == RSPF_FULL)
   {
      // Nothing to do just copy the tile.
      theTile->loadTile(inputData.get());
   }
   else
   {
       // Partial tile with nulls in it.
      for(bandIdx = 0; bandIdx < numberOfBands; ++bandIdx)
      {
         T* inputBuf     = (T*)inputData->getBuf(bandIdx);
         T* outputBuf    = (T*)theTile->getBuf(bandIdx);
         if (!inputBuf || !outputBuf)
         {
            return; // Shouldn't happen...
         }
         
         const T NP = (T)inputData->getNullPix(bandIdx);

         for(y = 0; y < oh; ++y)
         {
            for(x = 0; x < ow; ++x)
            {
               // Get the center input pixel.
               const T CP = *(inputBuf+halfWindow + halfWindow*iw);
               if (CP == NP)
               {
                  values.clear();
                  for(kernelY = 0; kernelY < theWindowSize; ++kernelY)
                  {
                     for(kernelX = 0; kernelX < theWindowSize;++kernelX)
                     {
                        T tempValue = *(inputBuf+kernelX + kernelY*iw);
                        
                        if(tempValue != NP)
                        {
                           values.push_back(tempValue);
                        }
                     }
                  }

                  std::sort(values.begin(),
                            values.end());
                  
                  if(values.size() > 0)
                  {
                     (*outputBuf) = values[values.size()>>1];
                  }
                  else
                  {
                     (*outputBuf) = NP;
                  }
               }
               else // Center pixel (CP) not null.
               {
                  (*outputBuf) = CP;
               }

               // Move over...
               ++inputBuf;
               ++outputBuf;
               
            }  // End of loop in x direction.

            // Move down...
            inputBuf+=(halfWindow<<1);
            
         }  // End of loop in y direction.
         
      }  // End of band loop.
      
   }  // End of else "partial tile" block.
}

void rspfMeanMedianFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid())
   {
      return;
   }

   rspfString name = property->getName();

   if (name == WINDOW_SIZE_KW)
   {
      theWindowSize = property->valueToString().toUInt32();
   }
   else if (name == FILTER_TYPE_KW)
   {
      rspfString value = property->valueToString();
      setFilterType(value);
   }
   else if (name == AUTO_GROW_KW)
   {
      rspfString value;
      property->valueToString(value);
      setAutoGrowRectFlag(value.toBool());
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfMeanMedianFilter::getProperty(const rspfString& name)const
{
   if (name == WINDOW_SIZE_KW)
   {
      rspfProperty* prop =
         new rspfNumericProperty(WINDOW_SIZE_KW,
                                  rspfString::toString(theWindowSize),
                                  3,
                                  25);
      prop->setCacheRefreshBit();

      return prop;
   }
   else if (name == FILTER_TYPE_KW)
   {
      std::vector<rspfString> constraintList;
      getFilterTypeList(constraintList);
      rspfString value = getFilterTypeString();
      rspfProperty* prop = new rspfStringProperty(FILTER_TYPE_KW,
                                                    value,
                                                    false,
                                                    constraintList);
      prop->setCacheRefreshBit();
      
      return prop;
   }
   else if (name == AUTO_GROW_KW)
   {
      rspfRefPtr<rspfProperty> p = new rspfBooleanProperty(
         AUTO_GROW_KW, getAutoGrowRectFlag());
      p->setFullRefreshBit();
      return p;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfMeanMedianFilter::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(WINDOW_SIZE_KW);
   propertyNames.push_back(FILTER_TYPE_KW);
   propertyNames.push_back(AUTO_GROW_KW);

   rspfImageSourceFilter::getPropertyNames(propertyNames);
}

bool rspfMeanMedianFilter::saveState(rspfKeywordlist& kwl,
                                      const char* prefix)const
{
   kwl.add(prefix,
           WINDOW_SIZE_KW.c_str(),
           theWindowSize,
           true);
   kwl.add(prefix,
           FILTER_TYPE_KW.c_str(),
           getFilterTypeString(),
           true);
   kwl.add(prefix,
           AUTO_GROW_KW.c_str(),
           (theAutoGrowRectFlag?"true":"false"),
           true);   

   return rspfImageSourceFilter::saveState(kwl, prefix);
}

bool rspfMeanMedianFilter::loadState(const rspfKeywordlist& kwl,
                                      const char* prefix)
{
   const char* lookup = NULL;
   
   lookup = kwl.find(prefix, WINDOW_SIZE_KW.c_str());
   if(lookup)
   {
      theWindowSize = rspfString(lookup).toUInt32();
   }
   
   lookup = kwl.find(prefix, FILTER_TYPE_KW.c_str());
   if(lookup)
   {
      rspfString type = lookup;
      setFilterType(type);
   }

   lookup = kwl.find(prefix, AUTO_GROW_KW.c_str());
   if(lookup)
   {
      rspfString flag = lookup;
      setAutoGrowRectFlag(flag.toBool());
   }

   return rspfImageSourceFilter::loadState(kwl, prefix);
}
void rspfMeanMedianFilter::setFilterType(rspfMeanMedianFilterType type)
{
   theFilterType = type;
}

void rspfMeanMedianFilter::setFilterType(const rspfString& type)
{
   rspfString s = type;
   s.downcase();

   std::vector<rspfString> list;
   getFilterTypeList(list);
   
   for (rspf_uint32 i = 0; i < list.size(); ++i)
   {
      if (s == list[i])
      {
         theFilterType = static_cast<rspfMeanMedianFilterType>(i);
      }
   }

   if ( (theFilterType == RSPF_MEDIAN_FILL_NULLS) ||
        (theFilterType == RSPF_MEAN_FILL_NULLS) )
   {
      theEnableFillNullFlag = true;
   }
   else
   {
      theEnableFillNullFlag = false;
   }
}

void rspfMeanMedianFilter::setAutoGrowRectFlag(bool flag)
{
   theAutoGrowRectFlag = flag;
}

bool rspfMeanMedianFilter::getAutoGrowRectFlag() const
{
   return theAutoGrowRectFlag;
}

rspfString rspfMeanMedianFilter::getFilterTypeString() const
{
   std::vector<rspfString> list;
   getFilterTypeList(list);
   return list[theFilterType];
}

void rspfMeanMedianFilter::getFilterTypeList(
   std::vector<rspfString>& list) const
{
   list.resize(RSPF_MEAN_NULL_CENTER_ONLY+1);

   list[0] = rspfString("median");
   list[1] = rspfString("median_fill_nulls");
   list[2] = rspfString("median_null_center_only");
   list[3] = rspfString("mean");
   list[4] = rspfString("mean_fill_nulls");
   list[5] = rspfString("mean_null_center_only");
}

rspfIrect rspfMeanMedianFilter::getBoundingRect(rspf_uint32 resLevel)const
{
   // Get the input rectangle.
   rspfIrect rect = rspfImageSourceFilter::getBoundingRect(resLevel);
   if (!theAutoGrowRectFlag || rect.hasNans())
   {
      return rect; // Not in autogrow mode or no input connection yet...
   }

   if ( (theFilterType == RSPF_MEDIAN_FILL_NULLS)       ||
        (theFilterType == RSPF_MEDIAN_NULL_CENTER_ONLY) ||
        (theFilterType == RSPF_MEAN_FILL_NULLS)         ||
        (theFilterType == RSPF_MEAN_NULL_CENTER_ONLY) )
   {
      rspfIpt pt = rect.ul();
      const rspf_int32 HW = (theWindowSize >> 1); // half window size.

      // Adjust the upper left.
      pt.x = pt.x-HW;
      pt.y = pt.y-HW;
      rect.set_ul(pt);

      // Adjust the lower right.
      pt = rect.lr();
      pt.x = pt.x+HW;
      pt.y = pt.y+HW;
      rect.set_lr(pt);
   }

   return rect;
}
