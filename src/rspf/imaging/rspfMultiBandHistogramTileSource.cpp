//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfMultiBandHistogramTileSource.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/imaging/rspfMultiBandHistogramTileSource.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <algorithm>

static const char* MIN_VALUE_PERCENT_KW = "min_percent";
static const char* MAX_VALUE_PERCENT_KW = "max_percent";
static const char* ALGORITHM_KW         = "algorithm";

RTTI_DEF1(rspfMultiBandHistogramTileSource,
          "rspfMultiBandHistogramTileSource",
          rspfImageSourceHistogramFilter)
   
rspfMultiBandHistogramTileSource::rspfMultiBandHistogramTileSource()
   : rspfImageSourceHistogramFilter(),
     theTile(0),
     theAlgorithm(RSPF_HISTOGRAM_LINEAR_STRETCH_ALGORITHM)
{
   disableSource();
}

rspfMultiBandHistogramTileSource::rspfMultiBandHistogramTileSource(
   double /* minValuePercent */,
   double /* maxValuePercent */,
   rspfImageSource* inputSource,
   rspfMultiResLevelHistogram* histogram)
   : rspfImageSourceHistogramFilter(inputSource, histogram),
     theTile(0),
     theAlgorithm(RSPF_HISTOGRAM_LINEAR_STRETCH_ALGORITHM)
{
   disableSource();
}

rspfMultiBandHistogramTileSource::~rspfMultiBandHistogramTileSource()
{
}

rspfRefPtr<rspfImageData> rspfMultiBandHistogramTileSource::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection) return 0;
   
   rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(tileRect,
                                                                       resLevel);
   
   if(!isSourceEnabled() || !inputTile.valid())
   {
      return inputTile;
   }

   if(!theTile.valid())
   {
      allocate(); // First time through...
   }
   
   if( !theTile.valid() ||
       !theMinValuePercentArray.size() ||
       !theMaxValuePercentArray.size() )
   {
      return inputTile;
   }
   
   theTile->setImageRectangle(tileRect);
   if(inputTile->getBuf()&&
      (inputTile->getDataObjectStatus()!=RSPF_EMPTY))
   {
      theTile->loadTile(inputTile.get());
      switch(inputTile->getScalarType())
      {
         case RSPF_UCHAR:
         {
            return runLinearStretchAlgorithm((rspf_uint8)0,
                                             theTile);
         }
         case RSPF_USHORT16:
         case RSPF_USHORT11:
         {
            return runLinearStretchAlgorithm((rspf_uint16)0,
                                             theTile);
         }
         case RSPF_SSHORT16:
         {
            return runLinearStretchAlgorithm(static_cast<rspf_sint16>(0),
                                             theTile);
         }
         case RSPF_DOUBLE:
         case RSPF_NORMALIZED_DOUBLE:
         {
            return runLinearStretchAlgorithm(static_cast<double>(0),
                                             theTile);
         }
         case RSPF_FLOAT:
         case RSPF_NORMALIZED_FLOAT:
         {
            return runLinearStretchAlgorithm(static_cast<float>(0),
                                             theTile);
         }
         case RSPF_SCALAR_UNKNOWN:
         default:
         {
            rspfSetError("rspfMultiBandHistogramTileSource",
                          rspfErrorCodes::RSPF_ERROR,
                          "Unknown scalar type");
            break;
         }
      }
      theTile->validate();
      return theTile;
   }
   
   return inputTile;
}

void rspfMultiBandHistogramTileSource::initialize()
{
   rspfImageSourceHistogramFilter::initialize();

   // Force an allocate on the next getTile.
   theTile = 0;
}

void rspfMultiBandHistogramTileSource::allocate()
{
   theTile = rspfImageDataFactory::instance()->create(this, this);

   if(theTile.valid())
   {
      theTile->initialize();

      rspf_uint32 numberOfBands = theTile->getNumberOfBands();
      rspf_uint32 i = 0;
      if(numberOfBands)
      {
         std::vector<double> copyArray;
         
         if(numberOfBands > theMinValuePercentArray.size())
         {
            copyArray = theMinValuePercentArray;
            theMinValuePercentArray.resize(numberOfBands);
            if(numberOfBands < copyArray.size())
            {
               std::copy(copyArray.begin(),
                         copyArray.end(),
                         theMinValuePercentArray.begin());
            }
            else
            {
               std::copy(copyArray.begin(),
                         copyArray.begin() + copyArray.size(),
                         theMinValuePercentArray.begin());
            }
         }
         if(numberOfBands != theMaxValuePercentArray.size())
         {
            copyArray = theMaxValuePercentArray;
            theMaxValuePercentArray.resize(numberOfBands);
            if(numberOfBands < copyArray.size())
            {
               std::copy(copyArray.begin(),
                         copyArray.end(),
                         theMaxValuePercentArray.begin());
            }
            else
            {
               std::copy(copyArray.begin(),
                         copyArray.begin() + copyArray.size(),
                         theMaxValuePercentArray.begin());
            }
         }
         if(numberOfBands > theMinValuePercentArray.size())
         {
            for(i = (rspf_uint32)theMinValuePercentArray.size(); i < numberOfBands; ++i)
            {
               theMinValuePercentArray[i] = 0.0;
               theMaxValuePercentArray[i] = 0.0;
            }
         }
      }
   }
}

bool rspfMultiBandHistogramTileSource::loadState(const rspfKeywordlist& kwl,
                                           const char* prefix)
{
   theMinValuePercentArray.clear();
   theMaxValuePercentArray.clear();
   if(rspfImageSourceHistogramFilter::loadState(kwl, prefix))
   {
      rspfNotify(rspfNotifyLevel_INFO) << "rspfMultiBandHistogramTileSource::loadState INFO: histogram filter was loaded" << std::endl;
      rspfString regExpression = rspfString("^(") + rspfString(prefix) + rspfString(MIN_VALUE_PERCENT_KW) + rspfString("[0-9]+)");

      long numberOfSources = kwl.getNumberOfSubstringKeys(regExpression);
      
      unsigned long numberOfMatches = 0;
      rspf_uint32 i = 0;
      while(numberOfMatches < static_cast<unsigned long>(numberOfSources))
      {
         rspfString key = rspfString(MIN_VALUE_PERCENT_KW) + rspfString::toString(i);
         const char* value = kwl.find(prefix, key); 
         if(value)
         {
            theMinValuePercentArray.push_back(rspfString(value).toDouble());
            ++numberOfMatches;
         }      
         ++i;
      }

      numberOfMatches = 0;
      i               = 0;
      regExpression = rspfString("^(") + rspfString(prefix) + rspfString(MAX_VALUE_PERCENT_KW) + rspfString("[0-9]+)");
      numberOfSources = kwl.getNumberOfSubstringKeys(regExpression);
      while(numberOfMatches < static_cast<unsigned long>(numberOfSources))
      {
         rspfString key = rspfString(MAX_VALUE_PERCENT_KW) + rspfString::toString(i);
         const char* value = kwl.find(prefix, key); 

         if(value)
         {
            theMaxValuePercentArray.push_back(rspfString(value).toDouble());
            ++numberOfMatches;
         }      
         ++i;
      }

      rspfString algorithm = kwl.find(prefix,
                                       ALGORITHM_KW);
      algorithm = algorithm.downcase();
      theAlgorithm = RSPF_HISTOGRAM_LINEAR_STRETCH_ALGORITHM;
      if(algorithm != "")
      {
         if(algorithm.contains("linear"))
         {
           theAlgorithm = RSPF_HISTOGRAM_LINEAR_STRETCH_ALGORITHM;
         }
      }
   }
   return true;
}

bool rspfMultiBandHistogramTileSource::saveState(rspfKeywordlist& kwl,
                                           const char* prefix)const
{
   rspfImageSourceHistogramFilter::saveState(kwl, prefix);
   rspf_int32 i = 0;

   for(i = 0; i< (rspf_int32)theMinValuePercentArray.size();++i)
   {
      kwl.add(prefix,
              rspfString(MIN_VALUE_PERCENT_KW) + rspfString::toString(i),
              theMinValuePercentArray[i],
              true);
   }
   for(i = 0; i< (rspf_int32)theMaxValuePercentArray.size();++i)
   {
      kwl.add(prefix,
              rspfString(MAX_VALUE_PERCENT_KW) + rspfString::toString(i),
              theMaxValuePercentArray[i],
              true);
   }
   rspfString algorithmString;

   switch(theAlgorithm)
   {
   case RSPF_HISTOGRAM_LINEAR_STRETCH_ALGORITHM:
   {
      algorithmString = "linear_stretch";
      break;
   }
   case RSPF_HISTOGRAM_NO_ALGORITHM:
   {
      algorithmString = "none";
      break;
   }
   default:
   {
      algorithmString = "none";
      break;
   }
   }

   kwl.add(prefix,
           ALGORITHM_KW,
           algorithmString.c_str(),
           true);
   
   return true;
}

template <class T>
rspfRefPtr<rspfImageData> rspfMultiBandHistogramTileSource::runLinearStretchAlgorithm(
   T /* dummyVariable */, rspfRefPtr<rspfImageData>& tile)
{
   if(!getHistogram())
   {
      return tile;
   }

   rspfRefPtr<rspfMultiBandHistogram> histo    = getHistogram()->getMultiBandHistogram(theCurrentResLevel);
   if(histo.valid())
   {
      rspf_uint32 maxBands = ( (histo->getNumberOfBands() >
                           tile->getNumberOfBands())?
                          tile->getNumberOfBands():
                          histo->getNumberOfBands());

      long offsetUpperBound = tile->getHeight()*tile->getWidth();

      for(rspf_uint32 band = 0; band < maxBands; ++band)
      {
         rspfRefPtr<rspfHistogram> h  = histo->getHistogram(band);
         
         T* buf   = static_cast<T*>(tile->getBuf(band));
         T np     = static_cast<T>(tile->getNullPix(band));
         T minPix = static_cast<T>(tile->getMinPix(band));
         T maxPix = static_cast<T>(tile->getMaxPix(band));
         T range  = (maxPix - minPix);
         if(h.valid()&&buf)
         {
            
            double maxClip = minPix + (h->HighClipVal(theMaxValuePercentArray[band])/(double)h->GetRes())*range;
            double minClip = minPix + (h->LowClipVal(theMinValuePercentArray[band])/(double)h->GetRes())*range;
            double delta   = (maxClip - minClip);
            
            if(fabs(delta) > 0.0)
            {
               for(long offset = 0; offset < offsetUpperBound; ++offset)
               {
                  if(buf[offset] != np)
                  {
                     if(buf[offset] < minClip)
                     {
                        buf[offset] = minPix;
                     }
                     else if(buf[offset] > maxClip)
                     {
                        buf[offset] = maxPix;
                     }
                     else
                     {
                        T value = static_cast<T>(minPix + ((buf[offset] - minClip)/delta)*range);
                        buf[offset] = value>maxPix?maxPix:value;
                     }
                  }
               }
            }
         }
      }

      tile->validate();
   }
   
   return tile;
}
