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
// $Id: rspfHistogramThreshholdFilter.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/imaging/rspfHistogramThreshholdFilter.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfErrorContext.h>

static const rspfKeyword MIN_VALUE_PERCENT_KW("min_percent",
                                               "min percentage clip");

static const rspfKeyword MAX_VALUE_PERCENT_KW("max_percent",
                                               "max percentage value");


RTTI_DEF1(rspfHistogramThreshholdFilter, "rspfHistogramThreshholdFilter", rspfImageSourceHistogramFilter)
   
rspfHistogramThreshholdFilter::rspfHistogramThreshholdFilter()
   : rspfImageSourceHistogramFilter(),
     theMinValuePercent(0.0),
     theMaxValuePercent(0.0)
{
}

rspfHistogramThreshholdFilter::rspfHistogramThreshholdFilter(double minValuePercent,
                                                               double maxValuePercent,
                                                               rspfImageSource* inputSource,
                                                               rspfMultiResLevelHistogram* histogram)
   : rspfImageSourceHistogramFilter(inputSource, histogram),
     theMinValuePercent(minValuePercent),
     theMaxValuePercent(maxValuePercent)
{
}

rspfHistogramThreshholdFilter::~rspfHistogramThreshholdFilter()
{
}

rspfRefPtr<rspfImageData> rspfHistogramThreshholdFilter::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection) return NULL;
   
   rspfRefPtr<rspfImageData> inputTile =
      theInputConnection->getTile(tileRect, resLevel);

   if(!isSourceEnabled())
   {
      return inputTile;
   }
   if(inputTile.valid()  &&
      inputTile->getBuf() &&
      (inputTile->getDataObjectStatus()!=RSPF_EMPTY))
   {
      switch(inputTile->getScalarType())
      {
         case RSPF_UCHAR:
         {
            return runThreshholdStretchAlgorithm((rspf_uint8)0,
                                                 inputTile);
         }
         case RSPF_USHORT16:
         case RSPF_USHORT11:
         {
            return runThreshholdStretchAlgorithm((rspf_uint16)0,
                                                 inputTile);
         }
         case RSPF_SSHORT16:
         {
            return runThreshholdStretchAlgorithm(static_cast<rspf_sint16>(0),
                                                 inputTile);
         }
         case RSPF_DOUBLE:
         case RSPF_NORMALIZED_DOUBLE:
         {
            return runThreshholdStretchAlgorithm(static_cast<double>(0),
                                                 inputTile);
         }
         case RSPF_FLOAT:
         case RSPF_NORMALIZED_FLOAT:
         {
            return runThreshholdStretchAlgorithm(static_cast<float>(0),
                                                 inputTile);
         }
         case RSPF_SCALAR_UNKNOWN:
         default:
         {
            rspfSetError("rspfHistogramThreshholdFilter",
                          rspfErrorCodes::RSPF_ERROR,
                          "Unknown scalar type");
            break;
         }
      }
   }

   return inputTile;
}


bool rspfHistogramThreshholdFilter::loadState(const rspfKeywordlist& kwl,
                                           const char* prefix)
{
   if(rspfImageSourceHistogramFilter::loadState(kwl, prefix))
   {
      const char* minPercent = kwl.find(prefix, MIN_VALUE_PERCENT_KW);
      const char* maxPercent = kwl.find(prefix, MAX_VALUE_PERCENT_KW);

      if(minPercent)
      {
         theMinValuePercent = rspfString(minPercent).toDouble();
      }
      else
      {
         theMinValuePercent = 0.0;
      }
      if(maxPercent)
      {
         theMaxValuePercent = rspfString(maxPercent).toDouble();
      }
      else
      {
         theMaxValuePercent = 0.0;
      }
   }
   else
   {
      return false;
   }

   return true;
}

bool rspfHistogramThreshholdFilter::saveState(rspfKeywordlist& kwl,
                                           const char* prefix)const
{
   if(rspfImageSourceHistogramFilter::saveState(kwl, prefix))
   {
      kwl.add(prefix,
              MIN_VALUE_PERCENT_KW,
              theMinValuePercent,
              true);
      kwl.add(prefix,
              MAX_VALUE_PERCENT_KW,
              theMaxValuePercent,
              true);
   }
   else
   {
      return false;
   }

   return true;
}

void rspfHistogramThreshholdFilter::initialize()
{
   rspfImageSourceHistogramFilter::initialize();
}

template <class T>
rspfRefPtr<rspfImageData> rspfHistogramThreshholdFilter::runThreshholdStretchAlgorithm(
   T /* dummyVariable */,
   rspfRefPtr<rspfImageData>& tile)
{
   if(!getHistogram())
   {
      return tile;
   }
   
   rspfRefPtr<rspfMultiBandHistogram> histo    = getHistogram()->getMultiBandHistogram(0);
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
         
         if(h.valid()&&buf)
         {
            T np     = static_cast<T>(tile->getNullPix(band));
            T minPix = static_cast<T>(tile->getMinPix(band));
            T maxPix = static_cast<T>(tile->getMaxPix(band));
            double range  = (maxPix - minPix);
            double maxClip = (h->HighClipVal(theMaxValuePercent/100.0)/(double)h->GetRes());
            double minClip = (h->LowClipVal(theMinValuePercent/100.0)/(double)h->GetRes());
            rspf_float64 normPix;
            double delta   = fabs(maxClip - minClip);
            if(delta > 0.0)
            {
               for(long offset = 0; offset < offsetUpperBound; ++offset)
               {
                  if(buf[offset] != np)
                  {
                     normPix = ((double)buf[offset]-minPix)/range;
                     if(normPix <= minClip)
                     {
                        buf[offset] = minPix;
                     }
                     else if(normPix >= maxClip)
                     {
                        buf[offset] = maxPix;
                     }
                     else 
                     {
                        double t = (normPix - minClip)/delta;
                        T value = static_cast<T>(minPix + range*t);
                        buf[offset] = value;
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

void rspfHistogramThreshholdFilter::setMinValuePercent(double minValue)
{
   theMinValuePercent = minValue;
}

void rspfHistogramThreshholdFilter::setMaxValuePercent(double maxValue)
{
   theMaxValuePercent = maxValue;
}

double rspfHistogramThreshholdFilter::getMinValuePercent()const
{
   return theMinValuePercent;
}

double rspfHistogramThreshholdFilter::getMaxValuePercent()const
{
   return theMaxValuePercent;
}
