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
// $Id: rspfHistogramEqualization.cpp 14110 2009-03-14 15:46:40Z dburken $
#include <rspf/imaging/rspfHistogramEqualization.h>
#include <rspf/base/rspfMultiResLevelHistogram.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeyword.h>

RTTI_DEF1(rspfHistogramEqualization, "rspfHistogramEqualization", rspfImageSourceHistogramFilter)
   
static const rspfKeyword HISTOGRAM_INVERSE_FLAG_KW("inverse_flag",
                                                    "specifies whether the inverse should be done for the getTile");

static rspfTrace traceDebug("rspfHistogramEqualization:debug");

rspfHistogramEqualization::rspfHistogramEqualization()
   : rspfImageSourceHistogramFilter(),
     theTile(NULL),
     theAccumulationHistogram(NULL),
     theInverseFlag(false)
{
}

rspfHistogramEqualization::rspfHistogramEqualization(rspfImageSource* inputSource,
                                                       rspfRefPtr<rspfMultiResLevelHistogram> histogram)
   : rspfImageSourceHistogramFilter(inputSource, histogram),
     theTile(0),
     theAccumulationHistogram(0),
     theInverseFlag(false)
{
   if(getHistogram().valid())
   {
      theAccumulationHistogram = getHistogram()->createAccumulationLessThanEqual();
      initializeLuts();
   }
}

rspfHistogramEqualization::rspfHistogramEqualization(rspfImageSource* inputSource,
                                                       rspfRefPtr<rspfMultiResLevelHistogram> histogram,
                                                       bool inverseFlag)
   : rspfImageSourceHistogramFilter(inputSource, histogram),
     theTile(NULL),
     theAccumulationHistogram(0),
     theInverseFlag(inverseFlag)
{
   if(getHistogram().valid())
   {
      theAccumulationHistogram = getHistogram()->createAccumulationLessThanEqual();
      initializeLuts();
   }
}

rspfHistogramEqualization::~rspfHistogramEqualization()
{
   deleteLuts();
}

rspfRefPtr<rspfImageData> rspfHistogramEqualization::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return NULL;
   }
   
   rspfRefPtr<rspfImageData> inputTile = theInputConnection->getTile(tileRect,
                                                                       resLevel);

   if(!inputTile.valid()) return inputTile;
   
   if (!isSourceEnabled())
   {
      return inputTile;
   }
   
   if(!theTile.valid())
   {
      allocate(); // First time through...
   }
   
   // if (!theTile) throw exception!
   
   // Set the origin, resize if needed of the output tile.
   theTile->setImageRectangleAndBands(inputTile->getImageRectangle(),
				      inputTile->getNumberOfBands());
   
   if( !inputTile.valid() ||
       inputTile->getDataObjectStatus() == RSPF_NULL ||
       inputTile->getDataObjectStatus() == RSPF_EMPTY )
   {
      return theTile;
   }

   if(!theAccumulationHistogram)
   {
      computeAccumulationHistogram();
   }
   if(theForwardLut.size() == 0 ||
      theInverseLut.size() == 0)
   {
      initializeLuts();
   }
   if((theForwardLut.size() == 0) ||
      (theInverseLut.size() == 0))
   {
      return inputTile;
   }
   
   switch(inputTile->getScalarType())
   {
      case RSPF_UCHAR:
      {
         return runEqualizationAlgorithm(static_cast<rspf_uint8>(0),
                                         inputTile);
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         return runEqualizationAlgorithm(static_cast<rspf_uint16>(0),
                                         inputTile);
      }
      case RSPF_SINT16:
      {
         return runEqualizationAlgorithm(static_cast<rspf_sint16>(0),
                                         inputTile);
      }
      case RSPF_UINT32:
      {
         return runEqualizationAlgorithm(static_cast<rspf_uint32>(0),
                                         inputTile);
      }
      case RSPF_SINT32:
      {
         return runEqualizationAlgorithm(static_cast<rspf_sint32>(0),
                                         inputTile);
      }
      case RSPF_FLOAT32:
      {
         return runEqualizationAlgorithm(static_cast<rspf_float32>(0),
                                         inputTile);
      }
      case RSPF_FLOAT64:
      {
         return runEqualizationAlgorithm(static_cast<rspf_float64>(0),
                                         inputTile);
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfHistogramEqualization::getTile WARNING: Unsupported scalar type."
            << endl;
         break;
      }
   }
   
   return inputTile;
}

void rspfHistogramEqualization::setHistogram(rspfRefPtr<rspfMultiResLevelHistogram> histogram)
{
   rspfImageSourceHistogramFilter::setHistogram(histogram);
   computeAccumulationHistogram();
   initializeLuts();
}

bool rspfHistogramEqualization::saveState(rspfKeywordlist& kwl,
                                           const char* prefix)const
{
   bool result = rspfImageSourceHistogramFilter::saveState(kwl, prefix);

   kwl.add(prefix,
           HISTOGRAM_INVERSE_FLAG_KW,
           theInverseFlag,
           true);
   
   return result;
}

bool rspfHistogramEqualization::loadState(const rspfKeywordlist& kwl,
                                           const char* prefix)
{
   static const char MODULE[] = "rspfHistogramEqualization::loadState";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << " Entered..."
         << "\nprefix:  " << prefix << endl;
   }

   const char* lookup = kwl.find(prefix,
                                 HISTOGRAM_INVERSE_FLAG_KW);
   if(lookup)
   {
      theInverseFlag = rspfString(lookup).toBool();
   }
   if(rspfImageSourceHistogramFilter::loadState(kwl, prefix))
   {
//      computeAccumulationHistogram();
//      initializeLuts();
   }
   else
   {
      return false;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG:" << MODULE;
      this->print(rspfNotify(rspfNotifyLevel_DEBUG));
      rspfNotify(rspfNotifyLevel_DEBUG) << "\nExited..." << endl;
   }
   
   return true;
}

void rspfHistogramEqualization::computeAccumulationHistogram()
{
   if(getHistogram().valid())
   {
      theAccumulationHistogram = getHistogram()->createAccumulationLessThanEqual();
   }
}

template <class T>
rspfRefPtr<rspfImageData> rspfHistogramEqualization::runEqualizationAlgorithm(T, rspfRefPtr<rspfImageData> tile)
{
   
   if(!theAccumulationHistogram ||
      !getHistogram())
   {
      return tile;
   }

   // for now we will always pull from res 0 information
   rspfRefPtr<rspfMultiBandHistogram> histo = getHistogram()->getMultiBandHistogram(0);

   if(histo.valid())
   {
      rspf_uint32 maxBands = ( (histo->getNumberOfBands() >
                                 tile->getNumberOfBands())?
                                tile->getNumberOfBands():
                                histo->getNumberOfBands());
      
      long offsetUpperBound = tile->getHeight()*tile->getWidth();

      for(rspf_uint32 band = 0; band < maxBands; ++band)
      {
         rspfRefPtr<rspfHistogram> bandHisto = histo->getHistogram(band);
         T* buf = static_cast<T*>(tile->getBuf(band));
         double *histoLut = band<theForwardLut.size()?theForwardLut[band]:NULL;
         rspf_uint32 actualBand = theBandList[band];
         if(bandHisto.valid())
         {
            if(buf&&histoLut&&(actualBand <  histo->getNumberOfBands()))
            {
               if(theInverseFlag)
               {
                  histoLut = theInverseLut[actualBand];
               }
               if(histoLut)
               {
                  if(tile->getDataObjectStatus() == RSPF_FULL)
                  {
                     T minPix = (T)tile->getMinPix(actualBand);
                     T maxPix = (T)tile->getMaxPix(actualBand);
                     for(long offset = 0; offset < offsetUpperBound; ++offset)
                     {
                        rspf_int32 idx = bandHisto->GetIndex(buf[offset]);
                           
                        if(idx>=0)
                        {
                           T value = (T)(histoLut[idx]);

                           //---
                           // Assign clamping to min max.
                           // 
                           // ESH 03/2009 -- Clamping to within min-max fixed
                           //--- 
                           buf[offset] = value < minPix ? minPix :
                              (value > maxPix ? maxPix : value);
                        }
                     }
                  }
                  else
                  {
                     T minPix  = (T)tile->getMinPix(actualBand);
                     T maxPix  = (T)tile->getMaxPix(actualBand);
                     T nullPix = (T)tile->getNullPix(actualBand);
                     for(long offset = 0; offset < offsetUpperBound; ++offset)
                     {
                        rspf_int32 idx = bandHisto->GetIndex(buf[offset]);
                        
                        if((buf[offset]!=nullPix)&&(idx>=0))
                        {
                           T value = (T)(histoLut[idx]);

                           //---
                           // Assign clamping to min max.
                           // 
                           // ESH 03/2009 -- Clamping to within min-max fixed
                           //--- 
                           buf[offset] = value < minPix ? minPix :
                              (value > maxPix ? maxPix : value);
                        }
                        else
                        {
                           buf[offset] = nullPix;
                        }
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

void rspfHistogramEqualization::initialize()
{
   // Base class will recapture "theInputConnection".
   rspfImageSourceHistogramFilter::initialize();
   if(theTile.valid())
   {
      theTile = NULL;
   }
}

void rspfHistogramEqualization::allocate()
{
   theBandList.clear();
   
   if(theTile.valid())
   {
      theTile = NULL;
   }
   
   if(theInputConnection)
   {
      theInputConnection->getOutputBandList(theBandList);
      theTile = rspfImageDataFactory::instance()->create(this, this);
      if(theTile.valid())
      {
         theTile->initialize();
      }
   }
}

void rspfHistogramEqualization::initializeLuts()
{
   if(theForwardLut.size() != 0 ||
      theInverseLut.size() != 0)
   {
      deleteLuts();
   }
   if(!theAccumulationHistogram||!getHistogram())
   {
      return;
   }
   rspfRefPtr<rspfMultiBandHistogram> accumHisto = theAccumulationHistogram->getMultiBandHistogram(0);
   rspfRefPtr<rspfMultiBandHistogram> histogram  = getHistogram()->getMultiBandHistogram(0);
   rspfKeywordlist kwl;
   theAccumulationHistogram->saveState(kwl);

   if(accumHisto.valid()&&histogram.valid())
   {
      long maxBands = accumHisto->getNumberOfBands();
      
      for(long band = 0; band < maxBands; ++band)
      {
         // first we grab pointers to the histogram and the accumulation
         // histogram
         rspfRefPtr<rspfHistogram> h  = accumHisto->getHistogram(band);
         rspfRefPtr<rspfHistogram> h2 = histogram->getHistogram(band);

         if(h.valid()&&h2.valid())
         {
            // lets get the number of indices.
            rspf_uint32 numberOfIndices = (rspf_uint32)h2->GetRes();
            vector<double> countForInverse(numberOfIndices);
            
            theForwardLut.push_back(new double[numberOfIndices]);
            theInverseLut.push_back(new double[numberOfIndices]);
            
            // let's grab the counts array
            const float* histoCounts = h->GetCounts();

//         double maxIntensity      = h2->GetMaxVal();
            double maxIntensity      = h2->GetRangeMax();
            double maxCount          = h->GetMaxCount();
            
            // now pre compute the transforms
            double *forwardLut = theForwardLut[band];
            double *inverseLut = theInverseLut[band];
            
//         double minIntensity  = h2->GetMinVal();
            double minIntensity  = h2->GetRangeMin();
            double delta = maxIntensity-minIntensity;
            rspf_uint32 idx = 0;
            
            // clear out the inverse
            //
            for(idx = 0; idx < numberOfIndices; ++ idx)
            {
               inverseLut[idx] = rspf::nan();
            }
            for(idx = 0; idx < numberOfIndices; ++ idx)
            {
               forwardLut[idx]   = minIntensity + (histoCounts[idx]/maxCount)*delta;
               
               rspf_int32 inverseIdx = h2->GetIndex(forwardLut[idx]);
               if(inverseIdx >= 0)
               {
                  inverseLut[inverseIdx]   = minIntensity + delta*(idx/(rspf_float32)numberOfIndices);
               }
            }
            
            // now solve the inverse lut
            //
            rspf_uint32 idxStart = 0;
            rspf_uint32 idxEnd = 0;
            
            while((rspf::isnan(inverseLut[idxEnd]))&&(idxEnd <numberOfIndices)){ ++idxEnd;}
            if((idxStart!=idxEnd)&&(idxEnd<numberOfIndices))
            {
               std::fill(inverseLut,
                         inverseLut+idxEnd,
                         inverseLut[idxEnd]);
            }
            idxStart = numberOfIndices-1;
            while((rspf::isnan(inverseLut[idxStart]))&&(idxStart > 0)){ --idxStart;}
            if(idxStart !=0)
            {
               std::fill(inverseLut+idxStart,
                         inverseLut+numberOfIndices,
                         inverseLut[idxStart]);
            }
            
            idxStart = 0;
            idxEnd   = 0;
            rspf_float32 valueStart = 0.0;
            rspf_float32 valueEnd = 0.0;
            
            while(idxStart < numberOfIndices)
            {
               idxEnd = idxStart;
               if(rspf::isnan(inverseLut[idxStart]))
               {
                  while(rspf::isnan(inverseLut[idxEnd])&&(idxEnd < (numberOfIndices-1))) ++idxEnd;
                  double length = (idxEnd-idxStart)+1;
                  valueEnd = inverseLut[idxEnd];
                  double deltaVal = (valueEnd-valueStart);
                  rspf_uint32 tempIdx = idxStart;
                  rspf_float32 count = 1.0;
                  double t = 0.0;
                  while(tempIdx < idxEnd)
                  {
                     t = (count/length);
                     t = t>1.0?1.0:t;
                     inverseLut[tempIdx] = valueStart + deltaVal*t;
                     ++count;
                     ++tempIdx;
                  }
                  idxStart = idxEnd;
                  valueStart = valueEnd;
               }
               else
               {
                  valueStart = inverseLut[idxStart];
                  ++idxStart;
               }
            }
         }
      }
   }
}

bool rspfHistogramEqualization::setHistogram(const rspfFilename& filename)
{
   return rspfImageSourceHistogramFilter::setHistogram(filename);
}

bool rspfHistogramEqualization::getInverseFlag()const
{
   return theInverseFlag;
}

void rspfHistogramEqualization::setInverseFlag(bool inverseFlag)
{
   theInverseFlag = inverseFlag;
}

void rspfHistogramEqualization::deleteLuts()
{
   long band = 0;
   
   for(band = 0; band < (long)theForwardLut.size(); ++band)
   {
      delete [] theForwardLut[band];
   }
   theForwardLut.clear();
   
   for(band = 0; band < (long)theInverseLut.size(); ++band)
   {
      delete [] theInverseLut[band];      
   }
   
   theInverseLut.clear();
}

const rspfFilename& rspfImageSourceHistogramFilter::getHistogramFilename() const
{
   return theFilename;
}

std::ostream& rspfHistogramEqualization::print(std::ostream& out) const
{
   // Base class...
   return rspfImageSourceHistogramFilter::print(out);
}
