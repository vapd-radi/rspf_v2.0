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
// $Id: rspfBandClipFilter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfBandClipFilter.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/imaging/rspfImageData.h>

RTTI_DEF1(rspfBandClipFilter, "rspfBandClipFilter", rspfImageSourceFilter);

rspfBandClipFilter::rspfBandClipFilter()
   :rspfImageSourceFilter(),
    theClipType(rspfBandClipType_NONE),
    theTile(NULL)
{
   theTile = new rspfImageData(this, // I am the owner,
                                RSPF_NORMALIZED_FLOAT,
                                1);
   theTile->initialize();
}

rspfBandClipFilter::rspfBandClipFilter(rspfImageSource* inputSource,
                                           const vector<double>& minPix,
                                           const vector<double>& maxPix,
                                           rspfBandClipType clipType)
   :rspfImageSourceFilter(inputSource),
    theClipType(clipType),
    theTile(NULL)
{
   theTile = new rspfImageData(this, // I am the owner,
                                RSPF_NORMALIZED_FLOAT,
                                1);
   theTile->initialize();
   setMinMaxPix(minPix, maxPix);
}

rspfBandClipFilter::rspfBandClipFilter(rspfImageSource* inputSource,
                                         double minPix,
                                         double maxPix,
                                         rspfBandClipType clipType)
   :rspfImageSourceFilter(inputSource),
    theClipType(clipType),
    theTile(NULL)
{
   theTile = new rspfImageData(this, // I am the owner,
                                RSPF_NORMALIZED_FLOAT,
                                1);
   theTile->initialize();
   
   theMinPix.push_back(minPix);
   theMaxPix.push_back(maxPix);
}

rspfBandClipFilter::~rspfBandClipFilter()
{
}

void rspfBandClipFilter::setClipType(rspfBandClipType clipType)
{
   theClipType = clipType;
}

rspfBandClipFilter::rspfBandClipType rspfBandClipFilter::getClipType()const
{
   return theClipType;
}

rspf_uint32 rspfBandClipFilter::getNumberOfValues()const
{
   // We make sure that all arrays have the same number
   // of elements so we can use the size of any of them.
   return (rspf_uint32)theMinPix.size();
}

void rspfBandClipFilter::setNumberOfValues(rspf_uint32 size)
{

   // lets do a non destructive set.  That way we can
   // grow or shrink the list without destroying
   // the original values.
   //
   if(!size)
   {
      theMinPix.clear();
      theMaxPix.clear();
      theMedian.clear();
      return;
   }

   
   vector<double> tempMin    = theMinPix;
   vector<double> tempMax    = theMaxPix;
   
   theMinPix.resize(size);
   theMaxPix.resize(size);
   theMedian.resize(size);
    
   rspf_uint32 upperBound = (rspf_uint32)min(theMinPix.size(), tempMin.size());
   rspf_uint32 index = 0;

   for(index = 0; index < upperBound; ++index)
   {
      theMinPix[index] = tempMin[index];
      theMaxPix[index] = tempMax[index];
      
      theMinPix[index] = theMinPix[index]>1?1:theMinPix[index];
      theMinPix[index] = theMinPix[index]<0?0:theMinPix[index];
      theMaxPix[index] = theMaxPix[index]>1?1:theMaxPix[index];
      theMaxPix[index] = theMaxPix[index]<0?0:theMaxPix[index];
      
      theMedian[index] = (theMaxPix[index] + theMinPix[index])/2.0;
   }
   
   for(;index < theMinPix.size();++index)
   {
      theMinPix[index] = RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
      theMaxPix[index] = RSPF_DEFAULT_MAX_PIX_NORM_FLOAT;
      theMedian[index] = (theMaxPix[index] + theMinPix[index])/2.0;
            
      theMedian[index] = (theMaxPix[index] + theMinPix[index])/2.0;
   }
}

void rspfBandClipFilter::setMinMaxPix(const vector<double>& minPix,
                                       const vector<double>& maxPix)
{
   theMinPix = minPix;
   theMaxPix = maxPix;

   setNumberOfValues((rspf_uint32)max(theMinPix.size(), theMaxPix.size()));
}

const std::vector<double>& rspfBandClipFilter::getMinPixList()const
{
   return theMinPix;
}

const std::vector<double> rspfBandClipFilter::getMaxPixList()const
{
   return theMaxPix;
}

double rspfBandClipFilter::getMinPix(rspf_uint32 index)const
{
   return index<theMinPix.size()?theMinPix[index]:0;
}

double rspfBandClipFilter::getMaxPix(rspf_uint32 index)const
{
   return index<theMaxPix.size()?theMaxPix[index]:0;
}

rspfRefPtr<rspfImageData> rspfBandClipFilter::getTile(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;
   }
   
   rspfRefPtr<rspfImageData> data =
      theInputConnection->getTile(rect, resLevel);
   if(!data.get())
   {
      return data;
   }
   rspfDataObjectStatus status = data->getDataObjectStatus();
   if((status != RSPF_NULL) &&
      (status != RSPF_EMPTY))
   {
      rspf_uint32 dw = data->getWidth();
      rspf_uint32 dh = data->getHeight();
      rspf_uint32 db = data->getNumberOfBands();

      rspf_uint32 tw = theTile->getWidth();
      rspf_uint32 th = theTile->getHeight();
      rspf_uint32 tb = theTile->getNumberOfBands();

      if(((tw*th)!=(dw*dh))||
         (tb != db))
      {
         theTile = new rspfImageData(this,
                                      RSPF_NORMALIZED_FLOAT,
                                      db,
                                      dw,
                                      dh);
         theTile->initialize();
      }

      if(getNumberOfValues() != theTile->getNumberOfBands())
      {
         // Should this go on??? (drb)
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfBandClipFilter::getTile\n"
            << "getNumberOfValues() != theTile->getNumberOfBands"
            << endl;
      }
      
      data->copyTileToNormalizedBuffer(static_cast<float*>(theTile->getBuf()));
      theTile->setDataObjectStatus(data->getDataObjectStatus());
      
      switch(theClipType)
      {
         case rspfBandClipType_CLIP:
         {
            runClip();
            break;
         }
         case rspfBandClipType_CLAMP:
         {
            runClamp();
            break;
         }
         case rspfBandClipType_LINEAR_STRETCH:
         {
            runLinearStretch();
            break;
         }
         case rspfBandClipType_MEDIAN_STRETCH:
         {
            runMedianStretch();
            break;
         }
         default:
            break;
      }
      data->copyNormalizedBufferToTile(static_cast<float*>(theTile->getBuf()));
   }
   
   return data;
}

void rspfBandClipFilter::runClip()
{
   rspf_uint32 offset = 0;
   rspf_uint32 upperBound = theTile->getWidth()*theTile->getHeight();
   for(rspf_uint32 band =0; band < theTile->getNumberOfBands(); ++band)
   {
      float *buf     = static_cast<float*>(theTile->getBuf(band));
      float  minPix  = theMinPix[band];
      float  maxPix  = theMaxPix[band];
      float  nullPix = theTile->getNullPix(band);
      
      if(buf)
      {
         for(offset = 0; offset < upperBound; ++offset)
         {
            if((*buf < minPix)||
               (*buf > maxPix))
            {
               *buf = nullPix;
            }
            ++buf;
         }
      }
   }
   theTile->validate();
}

void rspfBandClipFilter::runClamp()
{
   rspf_uint32 offset = 0;
   rspf_uint32 upperBound = theTile->getWidth()*theTile->getHeight();
   for(rspf_uint32 band =0; band < theTile->getNumberOfBands(); ++band)
   {
      float *buf     = static_cast<float*>(theTile->getBuf(band));
      float  minPix  = theMinPix[band];
      float  maxPix  = theMaxPix[band];
      float  nullPix = theTile->getNullPix(band);
      
      if(buf)
      {
         if(theTile->getDataObjectStatus() == RSPF_PARTIAL)
         {
            for(offset = 0; offset < upperBound; ++offset)
            {
               if(*buf != nullPix)
               {
                  if(*buf < minPix)
                  {
                     *buf = minPix;
                  }
                  else if(*buf > maxPix)
                  {
                     *buf = maxPix;
                  }
               }
               ++buf;
            }
         }
         else
         {
            for(offset = 0; offset < upperBound; ++offset)
            {
               if(*buf < minPix)
               {
                  *buf = minPix;
               }
               else if(*buf > maxPix)
               {
                  *buf = maxPix;
               }
               ++buf;
            }
         }
      }
   }
}

void rspfBandClipFilter::runLinearStretch()
{   
   rspf_uint32 bands = (rspf_uint32)min(theTile->getNumberOfBands(), (rspf_uint32)theMinPix.size());
   rspf_uint32 w     = theTile->getWidth();
   rspf_uint32 h     = theTile->getHeight();
   rspf_uint32 upperBound = w*h;
   const double* minPixArray = theTile->getMinPix();
   const double* maxPixArray = theTile->getMaxPix();
      
   float* *bandArray = new float*[bands];
   rspf_uint32 band = 0;
   for(band = 0; band < bands; ++band)
   {
      bandArray[band] = static_cast<float*>(theTile->getBuf(band));
   }
   
   if(theTile->getDataObjectStatus() == RSPF_PARTIAL)
   {
      for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
      {
         if(!theTile->isNull(offset))
         {
            for(band = 0; band < bands; ++band)
            {
               double delta = theMaxPix[band] - theMinPix[band];
               double t     = (bandArray[band][offset] - theMinPix[band])/delta;
               if(t < 0)
               {
                 bandArray[band][offset] = minPixArray[band];
               }
               else if(t > 1)
               {
                 bandArray[band][offset] = maxPixArray[band];
               }
               else
               {
                  bandArray[band][offset] = t;
               }
            }
         }
      }
   }
   else if(theTile->getDataObjectStatus() == RSPF_FULL)
   {
      for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            for(band = 0; band < bands; ++band)
            {
               double delta = theMaxPix[band] - theMinPix[band];
               double t     = (bandArray[band][offset] - theMinPix[band])/delta;
               
              if(t < 0)
              {
                 bandArray[band][offset] = minPixArray[band];
              }
              else if(t > 1)
              {
                 bandArray[band][offset] = maxPixArray[band];
              }
              else
              {
                 bandArray[band][offset] = t;
              }
            }
         }
   }
   delete [] bandArray;
   
}

void rspfBandClipFilter::runMedianStretch()
{
   rspf_uint32 bands = theTile->getNumberOfBands();
   float* *bandArray = new float*[bands];
   rspf_uint32 band = 0;

   for(band = 0; band < bands; ++band)
   {
      bandArray[band] = static_cast<float*>(theTile->getBuf(band));
   }
   rspf_uint32 upperBound = theTile->getWidth()*theTile->getHeight();
   
   if(theTile->getDataObjectStatus() == RSPF_PARTIAL)
   {
      for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
      {
         if(!theTile->isNull(offset))
         {
            for(band = 0; band < bands; ++band)
            {
               double side = (theMedian[band] - (double)bandArray[band][offset]);
               
               if(side > 0) // on the left
               {
                  double delta = fabs(theMedian[band] - theMinPix[band]);
                  double t     = ((double)bandArray[band][offset] - theMinPix[band])/delta;
                  if(t < 0)
                  {
                     bandArray[band][offset] = RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
                  }
                  else if(t > 1)
                  {
                     bandArray[band][offset] = theMedian[band];
                  }
                  else
                  {
                     bandArray[band][offset] = theMedian[band]*t;
                  }
               }
               else // on the right
               {
                  double delta = theMaxPix[band] - theMedian[band];
                  double t = ((double)bandArray[band][offset] - theMedian[band])/delta;
                  
                  if(t < 0)
                  {
                     bandArray[band][offset] = theMedian[band];
                  }
                  else if(t > 1)
                  {
                     bandArray[band][offset] = RSPF_DEFAULT_MAX_PIX_NORM_FLOAT;
                  }
                  else
                  {
                     bandArray[band][offset] = theMedian[band]+delta*t;
                  }
               }
            }
         }
      }
   }
   else if(theTile->getDataObjectStatus() == RSPF_FULL)
   {
      for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
      {
         for(band = 0; band < bands; ++band)
         {
            double side = (theMedian[band] - (double)bandArray[band][offset]);
            
            if(side > 0) // on the left
            {
               double delta = fabs(theMedian[band] - theMinPix[band]);
               double t     = ((double)bandArray[band][offset] - theMinPix[band])/delta;
               if(t < 0)
               {
                  bandArray[band][offset] = RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
               }
               else if(t > 1)
               {
                  bandArray[band][offset] = theMedian[band];
               }
               else
               {
                  bandArray[band][offset] = theMedian[band]*t;
               }
            }
            else // on the right
            {
               double delta = theMaxPix[band] - theMedian[band];
               double t = ((double)bandArray[band][offset] - theMedian[band])/delta;
               
               if(t < 0)
               {
                  bandArray[band][offset] = theMedian[band];
               }
               else if(t > 1)
               {
                  bandArray[band][offset] = RSPF_DEFAULT_MAX_PIX_NORM_FLOAT;
               }
               else
               {
                  bandArray[band][offset] = theMedian[band]+delta*t;
               }
            }
         }
      }
   }

   delete [] bandArray;
}   

void rspfBandClipFilter::initialize()
{
   rspfImageSourceFilter::initialize();
   
   if(theInputConnection)
   {
      rspf_uint32 bands = theInputConnection->getNumberOfOutputBands();

      if(bands)
      {
         if(theMinPix.size() != bands)
         {
            setNumberOfValues(bands);
         }
      }
   }
}

bool rspfBandClipFilter::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   bool result = rspfImageSourceFilter::loadState(kwl, prefix);

   if(result)
   {
      rspfString minRegExpression    = rspfString("^(") +
                                        rspfString(prefix) +
                                        "min[0-9]+)";
      rspfString maxRegExpression    = rspfString("^(") +
                                        rspfString(prefix) +
                                        "max[0-9]+)";
      rspfString medianRegExpression = rspfString("^(") +
                                        rspfString(prefix) +
                                        "median[0-9]+)";
      
      rspf_uint32 numberOfMins    = kwl.getNumberOfSubstringKeys(minRegExpression);
      rspf_uint32 numberOfMaxs    = kwl.getNumberOfSubstringKeys(maxRegExpression);
      rspf_uint32 numberOfMedians = kwl.getNumberOfSubstringKeys(medianRegExpression);
      theMinPix.clear();
      theMaxPix.clear();
      theMedian.clear();

      if(numberOfMins != numberOfMaxs)
      {
         rspf_int32 temp = (rspf_int32)min(numberOfMins, numberOfMaxs);
         numberOfMins = temp;
         numberOfMaxs = temp;
      }
      if(numberOfMins> 0)
      {
         rspfString prefixMin    = rspfString(prefix) + "min";
         rspfString prefixMax    = rspfString(prefix) + "max";
         rspfString prefixMedian = rspfString(prefix) + "median";
         
         rspf_uint32 numberOfMatches = 0;
         rspf_uint32 index = 0;
         const char* value=(const char*)NULL;
         
         while(numberOfMatches < numberOfMins)
         {
            value = kwl.find(prefixMin.c_str(),
                             rspfString::toString(index).c_str());
            if(value)
            {
               theMinPix.push_back(rspfString(value).toDouble());
               ++numberOfMatches;
            }
            ++index;
         }
         index = 0;
         numberOfMatches = 0;
         while(numberOfMatches < numberOfMaxs)
         {
            value = kwl.find(prefixMax.c_str(),
                             rspfString::toString(index).c_str());
            if(value)
            {
               theMaxPix.push_back(rspfString(value).toDouble());
               ++numberOfMatches;
            }
            ++index;
         }
         if(numberOfMedians != numberOfMins)
         {
            for(index = 0; index <  theMaxPix.size(); ++index)
            {
               theMedian.push_back((theMinPix[index]+theMaxPix[index])/2.0);
            }
         }
         else
         {
            index = 0;
            numberOfMatches = 0;
            while(numberOfMatches < numberOfMedians)
            {
               value = kwl.find(prefixMedian.c_str(),
                                rspfString::toString(index).c_str());
               if(value)
               {
                  theMedian.push_back(rspfString(value).toDouble());
                  ++numberOfMatches;
               }
               ++index;
            }
         }
      }
   }
   const char* clip_type = kwl.find(prefix, "clip_type");
   if(clip_type)
   {
     rspfString clipType = clip_type;
      clipType = clipType.upcase().trim();

      if(clipType == "NONE")
      {
         theClipType = rspfBandClipType_NONE;
      }
      else if(clipType == "CLIP")
      {
         theClipType = rspfBandClipType_CLIP;
      }
      else if(clipType == "LINEAR_STRETCH")
      {
         theClipType = rspfBandClipType_LINEAR_STRETCH;
      }
      else if(clipType == "MEDIAN_STRETCH")
      {
         theClipType = rspfBandClipType_MEDIAN_STRETCH;
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "rspfBandClipFilter::loadState NOTICE:\n"
         << "Clip type not given defaulting to NONE" << endl;
      theClipType = rspfBandClipType_NONE;
   }

   return result;
}

bool rspfBandClipFilter::saveState(rspfKeywordlist& kwl,
                                    const char* prefix)const
{
   rspfString minPrefix    = rspfString("min");
   rspfString maxPrefix    = rspfString("max");
   rspfString medianPrefix = rspfString("median");

   for(rspf_uint32 index = 1; index <= getNumberOfValues(); ++index)
   {
      rspfString value = (minPrefix+rspfString::toString(index));
      kwl.add(prefix,
              value.c_str(),
              theMinPix[index-1],
              true);
      value = (maxPrefix+rspfString::toString(index));
      kwl.add(prefix,
              value.c_str(),
              theMaxPix[index-1],
              true);
      value = (medianPrefix+rspfString::toString(index));
      kwl.add(prefix,
              value.c_str(),
              theMedian[index-1],
              true);
   }
   rspfString clipType;
   switch(theClipType)
   {
      case rspfBandClipType_NONE:
      {
         clipType = "NONE";
         break;
      }
      case rspfBandClipType_CLIP:
      {
         clipType = "CLIP";
         break;
      }
      case rspfBandClipType_LINEAR_STRETCH:
      {
         clipType = "LINEAR_STRETCH";
         break;
      }
      case rspfBandClipType_MEDIAN_STRETCH:
      {
         clipType = "MEDIAN_STRETCH";
         break;
      }
      default:
         break;
   }
   kwl.add(prefix,
           "clip_type",
           clipType.c_str(),
           true);
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}
