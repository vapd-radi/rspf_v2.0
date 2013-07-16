//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfBlendMosaic.cpp 20696 2012-03-19 12:36:40Z dburken $

#include <rspf/imaging/rspfBlendMosaic.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfBlendMosaic, "rspfBlendMosaic", rspfImageMosaic)

rspfBlendMosaic::rspfBlendMosaic()
   : rspfImageMosaic(),
     theNormResult(NULL)
{
}

rspfBlendMosaic::rspfBlendMosaic(rspfConnectableObject::ConnectableObjectList& inputSources)
   : rspfImageMosaic(inputSources),
     theNormResult(NULL)
{
   initialize();
}

rspfBlendMosaic::~rspfBlendMosaic()
{
}

void rspfBlendMosaic::initialize()
{   
   rspfImageMosaic::initialize();

   allocate();

   // only allocate this space if we have to
   if(hasDifferentInputs())
   {
      theNormResult = new rspfImageData(this,
                                         RSPF_NORMALIZED_FLOAT,
                                         theLargestNumberOfInputBands);
      theNormResult->initialize();
   }
      
   if(theWeights.size() < theInputObjectList.size())
   {
      for(rspf_uint32 index =(rspf_uint32)theWeights.size();
          index < theInputObjectList.size();
          ++index)
      {
         theWeights.push_back(1.0);
      }
   }
}

rspfRefPtr<rspfImageData> rspfBlendMosaic::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{   
   // rspfIpt origin = tileRect.ul();
   if(!isSourceEnabled())
   {
      return rspfImageMosaic::getTile(tileRect, resLevel);
   }
   
   rspf_uint32 size = getNumberOfInputs();
   if(!theTile.get())
   {
      // try to initialize
      initialize();

      // if we still don't have a buffer
      // then we will leave
      if(!theTile.get())
      {
         return theTile;
      }
   }

   if(size == 0)
   {
      return rspfRefPtr<rspfImageData>();
   }
   if(size == 1)
   {
      return rspfImageMosaic::getTile(tileRect, resLevel);
   }

   theTile->setImageRectangle(tileRect);
   theTile->makeBlank();
   if(theNormResult.valid())
   {
      theNormResult->setImageRectangle(tileRect);
      theNormResult->makeBlank();
   }
   
   switch(theTile->getScalarType())
   {
      case RSPF_UINT8:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_uint8>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_uint8>(0), tileRect,
                               resLevel);
         }
      }
      case RSPF_SINT8:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_sint8>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_sint8>(0), tileRect,
                               resLevel);
         }
      }
      case RSPF_FLOAT32: 
      case RSPF_NORMALIZED_FLOAT:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<float>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<float>(0), tileRect, resLevel);
         }
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_uint16>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_uint16>(0), tileRect,
                               resLevel);
         }
      }
      case RSPF_SINT16:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_sint16>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_sint16>(0), tileRect,
                               resLevel);
         }
      }
      case RSPF_SINT32:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_sint32>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_sint32>(0), tileRect,
                               resLevel);
         }
      }
      case RSPF_UINT32:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_uint32>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_uint32>(0), tileRect,
                               resLevel);
         }
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<double>(0), tileRect, resLevel);
         }
         else
         {
            return combineNorm(static_cast<double>(0), tileRect, resLevel);
         }
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfBlendMosaic::getTile NOTICE:\n"
            << "Scalar type = " << theTile->getScalarType()
            << " Not supported by rspfImageMosaic" << endl;
       }
   }

   return rspfRefPtr<rspfImageData>();   
}

template <class T> rspfRefPtr<rspfImageData> rspfBlendMosaic::combine(
   T,
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
  rspfRefPtr<rspfImageData> currentImageData=NULL;
  rspf_uint32 band;
  double currentWeight = 1.0;
  double previousWeight = 1.0;
  // double sumOfWeights   = 1;
  long offset = 0;
  long row    = 0;
  long col    = 0;
  rspf_uint32 layerIdx = 0;
  currentImageData = getNextTile(layerIdx, 0, tileRect, resLevel);
  
  if(!currentImageData.get()) // if we don't have one then return theTile
  {
     return theTile;
  }
  
  T** srcBands  = new T*[theLargestNumberOfInputBands];
  T** destBands = new T*[theLargestNumberOfInputBands];
  T*  nullPix   = new T[theTile->getNumberOfBands()];
  
  previousWeight = theWeights[layerIdx];
  //    // now get the previous weight and then combine the two into one.
	 // let's assign the bands
  for(band = 0; band < theLargestNumberOfInputBands; ++band)
  {
     destBands[band] = static_cast<T*>(theTile->getBuf(band));
     nullPix[band]   = static_cast<T>(theTile->getNullPix(band));
  }
  while(currentImageData.get())
  {
      rspfDataObjectStatus currentStatus =
         currentImageData->getDataObjectStatus();
      
      // set the current weight for the current tile.
      currentWeight = theWeights[layerIdx];

      // sumOfWeights = previousWeight+currentWeight;
      if( (currentStatus != RSPF_EMPTY) &&
	  (currentStatus != RSPF_NULL))
       {	 
	 long h = (long)currentImageData->getHeight();
	 long w = (long)currentImageData->getWidth();
	 offset = 0;
	 rspf_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
	 
	 // let's assign the bands
	 for(band = 0; band < minNumberOfBands; ++band)
	   {
	     srcBands[band]  = static_cast<T*>(currentImageData->getBuf(band));
	   }
	 for(;band < theLargestNumberOfInputBands; ++band)
	   {
	     srcBands[band]  = srcBands[minNumberOfBands - 1];
	   }

 	 if(currentStatus == RSPF_PARTIAL)
         {
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                  if(!currentImageData->isNull(offset))
                  {
                     for(band = 0; band < theLargestNumberOfInputBands; ++band)
                     {
                        if(destBands[band][offset] != nullPix[band])
                        {
//                            destBands[band][offset] = static_cast<T>((destBands[band][offset]*(1.0-currentWeight) +  
//                                                                      srcBands[band][offset]*currentWeight));
                           destBands[band][offset] = static_cast<T>((destBands[band][offset]*previousWeight + 
                                                                     srcBands[band][offset]*currentWeight)/(previousWeight+currentWeight));
                        }
                        else
                        {
                           destBands[band][offset] = srcBands[band][offset];
                        }
                     }	 
                  }
                  ++offset;
               }
            }
         }
	 else
         {
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                  for(band = 0; band < theLargestNumberOfInputBands; ++band)
                  {			
                     if(destBands[band][offset] != nullPix[band])
                     {
                        destBands[band][offset] = static_cast<T>((destBands[band][offset]*previousWeight+ 
                                                                  srcBands[band][offset]*currentWeight)/(previousWeight + currentWeight));
                     }
                     else
                     {
                        destBands[band][offset] = srcBands[band][offset];
                     }
                  }
                  ++offset;
               }
            }
         }
       }
      currentImageData = getNextTile(layerIdx, tileRect, resLevel);
      previousWeight   = (previousWeight+currentWeight)/2.0;
    }
  delete [] srcBands;
  delete [] destBands;
  delete [] nullPix;
  theTile->validate();

  return theTile;   
}

template <class T> rspfRefPtr<rspfImageData> rspfBlendMosaic::combineNorm(
   T, const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> currentImageData=NULL;
   rspf_uint32 band;
   double currentWeight = 1.0;
   double previousWeight = 1.0;
   // double sumOfWeights   = 1;
   long offset = 0;
   long row    = 0;
   long col    = 0;
   rspf_uint32 layerIdx = 0;
   currentImageData = getNextNormTile(layerIdx, 0, tileRect, resLevel);
  
   if(!currentImageData.get()) // if we don't have one then return theTile
   {
      return theTile;
   }
   theNormResult->makeBlank();
   float** srcBands  = new float*[theLargestNumberOfInputBands];
   float** destBands = new float*[theLargestNumberOfInputBands];
   float*  nullPix   = new float[theTile->getNumberOfBands()];
  
   previousWeight = theWeights[layerIdx];
   //    // now get the previous weight and then combine the two into one.
   // let's assign the bands
   for(band = 0; band < theLargestNumberOfInputBands; ++band)
   {
      destBands[band] = static_cast<float*>(theNormResult->getBuf(band));
      nullPix[band]   = static_cast<float>(theNormResult->getNullPix(band));
   }
   while(currentImageData.get())
   {
      rspfDataObjectStatus currentStatus     = currentImageData->getDataObjectStatus();
     
      // set the current weight for the current tile.
      currentWeight = theWeights[layerIdx];
     
      // sumOfWeights = previousWeight+currentWeight;
      if( (currentStatus != RSPF_EMPTY) &&
          (currentStatus != RSPF_NULL))
      {	 
         long h = (long)currentImageData->getHeight();
         long w = (long)currentImageData->getWidth();
         offset = 0;
         rspf_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
        
         // let's assign the bands
         for(band = 0; band < minNumberOfBands; ++band)
         {
            srcBands[band]  = static_cast<float*>(currentImageData->getBuf(band));
         }
         for(;band < theLargestNumberOfInputBands; ++band)
         {
            srcBands[band]  = srcBands[minNumberOfBands - 1];
         }
        
         if(currentStatus == RSPF_PARTIAL)
         {
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                  if(!currentImageData->isNull(offset))
                  {
                     for(band = 0; band < theLargestNumberOfInputBands; ++band)
                     {
                        if(destBands[band][offset] != nullPix[band])
                        {
                           destBands[band][offset] = static_cast<float>((destBands[band][offset]*previousWeight + 
                                                                         srcBands[band][offset]*currentWeight)/(previousWeight+currentWeight));
                        }
                        else
                        {
                           destBands[band][offset] = srcBands[band][offset];
                        }
                     }	 
                  }
                  ++offset;
               }
            }
         }
         else
         {
            for(row = 0; row < h; ++row)
            {
               for(col = 0; col < w; ++col)
               {
                  for(band = 0; band < theLargestNumberOfInputBands; ++band)
                  {			
                     if(destBands[band][offset] != nullPix[band])
                     {
                        destBands[band][offset] = static_cast<float>((destBands[band][offset]*previousWeight+ 
                                                                      srcBands[band][offset]*currentWeight)/(previousWeight + currentWeight));
                     }
                     else
                     {
                        destBands[band][offset] = srcBands[band][offset];
                     }
                  }
                  ++offset;
               }
            }
         }
      }
      currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
      previousWeight   = (previousWeight+currentWeight)/2.0;
   }
   theNormResult->validate();
   theTile->copyNormalizedBufferToTile((float*)theNormResult->getBuf());
   delete [] srcBands;
   delete [] destBands;
   delete [] nullPix;
   theTile->validate();

   return theTile;   
}

bool rspfBlendMosaic::saveState(rspfKeywordlist& kwl,
                                 const char* prefix)const
{
   bool result = rspfImageMosaic::saveState(kwl, prefix);

   rspfString copyPrefix    = prefix;
   for(rspf_uint32 index = 0; index < theWeights.size(); ++index)
   {
      rspfString weightStr = rspfString("weight") +
                              rspfString::toString(index);
      kwl.add(copyPrefix.c_str(),
              weightStr.c_str(),
              theWeights[index],
              true);
   }
   
   return result;
}

bool rspfBlendMosaic::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   bool result = rspfImageMosaic::loadState(kwl, prefix);
   rspf_uint32 count = 0;
   theWeights.clear();
   if(result)
   {
      rspfString copyPrefix    = prefix;
      rspfString regExpression =  rspfString("^(") + copyPrefix + "weight[0-9]+)";
      rspf_uint32 result = kwl.getNumberOfSubstringKeys(regExpression);
            
      rspf_uint32 numberOfMatches = 0;
      while(numberOfMatches < result)
      {
         rspfString value = rspfString("weight") + rspfString::toString(count);

         const char* lookup = kwl.find(copyPrefix.c_str(),
                                       value.c_str());
         
         if(lookup)
         {
            ++numberOfMatches;
            theWeights.push_back(rspfString(lookup).toDouble());
         }

         ++count;
      }
   }
   
   return result;
}


void rspfBlendMosaic::setNumberOfWeights(rspf_uint32 numberOfWeights)
{
   if(numberOfWeights > theWeights.size())
   {
      rspf_uint32 length = numberOfWeights - (rspf_uint32) theWeights.size();
      for(rspf_uint32 index= 0; index < length; ++ index)
      {
         theWeights.push_back(1.0);
      }
   }
   else if(numberOfWeights < theWeights.size())
   {
      // copy the list
      vector<double> temp(theWeights.begin(),
                          theWeights.begin()+numberOfWeights);
      // now shrink
      theWeights = temp;
   }
}

void rspfBlendMosaic::setAllWeightsTo(double value)
{
   for(rspf_uint32 index = 0; index < theWeights.size(); ++index)
   {
      theWeights[index] = value;
   }
}

void rspfBlendMosaic::normalizeWeights()
{
  double sumWeights=0.0;
  rspf_uint32 index = 0;
  for(index = 0; index < theWeights.size(); ++index)
    {
      sumWeights+=theWeights[index];
    }
  if(sumWeights > 0.0)
    {
      for(index = 0; index < theWeights.size(); ++index)
	{
	  theWeights[index] /= sumWeights;
	}
    }
}

void rspfBlendMosaic::findMinMax(double& minValue, double& maxValue)const
{
   if(theWeights.size() > 0)
   {
      minValue = maxValue = theWeights[0];

      for(rspf_uint32 index = 1; index < theWeights.size(); ++index)
      {
         minValue = minValue < theWeights[index]?minValue:theWeights[index];
         maxValue = maxValue > theWeights[index]?maxValue:theWeights[index];
      }
   }
}

void rspfBlendMosaic::setWeight(rspf_uint32 index, double weight)
{
   if(index < theWeights.size())
   {
      theWeights[index] = weight;
   }
}

void rspfBlendMosaic::setWeights(const std::vector<double>& weights)
{
   theWeights = weights;
}

const vector<double>& rspfBlendMosaic::getWeights()const
{
   return theWeights;
}

double rspfBlendMosaic::getWeight(rspf_uint32 index) const
{
   if(index < theWeights.size())
   {
      return theWeights[index];
   }
   if(theWeights.size())
   {
      return theWeights[theWeights.size()-1];
   }
   return 0;
}
