//*******************************************************************
// Copyright (C) 2005 SANZ Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Kenneth Melero (kmelero@sanz.com)
//
// Description: This combiner is designed to "float" the maximum pixel value
//              of all inputs to top of the mosaic output. 
//
//*************************************************************************
// $Id: rspfMaxMosaic.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfMaxMosaic.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfTrace.h>

static const rspfTrace traceDebug("rspfMaxMosaic:debug");

using namespace std;

RTTI_DEF1(rspfMaxMosaic, "rspfMaxMosaic", rspfImageCombiner)
rspfMaxMosaic::rspfMaxMosaic()
   :rspfImageCombiner(),
    theTile(NULL)
{
}

rspfMaxMosaic::rspfMaxMosaic(rspfConnectableObject::ConnectableObjectList& inputSources)
    : rspfImageCombiner(inputSources),
      theTile(NULL)
{
}


rspfMaxMosaic::~rspfMaxMosaic()
{
}

rspfRefPtr<rspfImageData> rspfMaxMosaic::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   long size = getNumberOfInputs();
   rspf_uint32 layerIdx = 0;
   // If there is only one in the mosaic then just return it.
   if(size == 1)
   {
      return getNextTile(layerIdx, 0, tileRect, resLevel);
   }
   
   rspfIpt origin = tileRect.ul();
   rspf_uint32 w = tileRect.width();
   rspf_uint32 h = tileRect.height();
   
   if(!theTile.valid())
   {
      // First time through...
      allocate();

      // If we still don't have a buffer then we will leave.
      if(!theTile.valid())
      {
         return rspfRefPtr<rspfImageData>();
      }
   }
   
   rspf_uint32 tileW = theTile->getWidth();
   rspf_uint32 tileH = theTile->getHeight();
   if((w != tileW)||
      (h != tileH))
   {
      theTile->setWidth(w);
      theTile->setHeight(h);
      if((w*h)!=(tileW*tileH))
      {
         theTile->initialize();
      }
   }
   theTile->setOrigin(origin);

   //---
   // General Note:
   //
   // Note: I will not check for disabled or enabled since we have
   // no clear way to handle this within a mosaic. The default will be
   // to do a simple a A over B type mosaic.  Derived classes should
   // check for the enabled and disabled and always
   // use this default implementation if they are disabled.
   //---
   theTile->setOrigin(origin);
   theTile->makeBlank();
   switch(theTile->getScalarType())
   {
      case RSPF_UCHAR:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_uint8>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_uint8>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_SINT8:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_sint8>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_sint8>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_FLOAT: 
      case RSPF_NORMALIZED_FLOAT:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<float>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<float>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_USHORT16:
      case RSPF_USHORT11:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_uint16>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_uint16>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_SSHORT16:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_sint16>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_sint16>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_SINT32:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_sint32>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_sint32>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_UINT32:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<rspf_uint32>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<rspf_uint32>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_DOUBLE:
      case RSPF_NORMALIZED_DOUBLE:
      {
         if(!hasDifferentInputs())
         {
            return combine(static_cast<double>(0),
                           tileRect,
                           resLevel);
         }
         else
         {
            return combineNorm(static_cast<double>(0),
                               tileRect,
                               resLevel);
         }
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Scalar type = " << theTile->getScalarType()
            << " Not supported by rspfMaxMosaic" << endl;
      }
   }

   return rspfRefPtr<rspfImageData>();
}

void rspfMaxMosaic::initialize()
{
  rspfImageCombiner::initialize();
  theTile = NULL;
}

void rspfMaxMosaic::allocate()
{
   theTile = NULL;
   
   if( (getNumberOfInputs() > 0) && getInput(0) )
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);
      theTile->initialize();
   }
}

bool rspfMaxMosaic::saveState(rspfKeywordlist& kwl,
                                 const char* prefix)const
{
   return rspfImageCombiner::saveState(kwl, prefix);
}

bool rspfMaxMosaic::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   return rspfImageCombiner::loadState(kwl, prefix);
}

template <class T> rspfRefPtr<rspfImageData> rspfMaxMosaic::combineNorm(
   T,// dummy template variable 
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   rspf_uint32 layerIdx = 0;
   rspfRefPtr<rspfImageData> destination = theTile;
   
   rspfRefPtr<rspfImageData> currentImageData =
      getNextNormTile(layerIdx, 0, tileRect, resLevel);
      
   if(!currentImageData)
   {
      return currentImageData;
   }
   
   float** srcBands        = new float*[theLargestNumberOfInputBands];
   float* srcBandsNullPix  = new float[theLargestNumberOfInputBands];
   T** destBands = new T*[theLargestNumberOfInputBands];
   T* destBandsNullPix = new T[theLargestNumberOfInputBands];
   T* destBandsMinPix = new T[theLargestNumberOfInputBands];
   T* destBandsMaxPix = new T[theLargestNumberOfInputBands];
      
   rspf_uint32 band;
   rspf_uint32 upperBound = destination->getWidth()*destination->getHeight();
   rspf_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
   for(band = 0; band < minNumberOfBands; ++band)
   {
      srcBands[band]  = static_cast<float*>(currentImageData->getBuf(band));
      srcBandsNullPix[band]  = static_cast<float>(currentImageData->getNullPix(band));
      
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
      destBandsMinPix[band] = static_cast<T>(theTile->getMinPix(band));
      destBandsMaxPix[band] = static_cast<T>(theTile->getMaxPix(band));
   }
   // if the src is smaller than the destination in number
   // of bands we will just duplicate the last band.
   for(;band < theLargestNumberOfInputBands; ++band)
   {
      srcBands[band]  = static_cast<float*>(srcBands[minNumberOfBands - 1]);
      srcBandsNullPix[band] = static_cast<float>(currentImageData->getNullPix(minNumberOfBands - 1));
      
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
      destBandsMinPix[band] = static_cast<T>(theTile->getMinPix(band));
      destBandsMaxPix[band] = static_cast<T>(theTile->getMaxPix(band));
   }
   // most of the time we will not overlap so let's
   // copy the first tile into destination and check later.
   //
   rspf_uint32 tempBandIdx = 0;
   for(band = 0; band < theTile->getNumberOfBands();++band)
   {
      if(band < currentImageData->getNumberOfBands())
      {
         theTile->copyNormalizedBufferToTile(band,
                                             (float*)currentImageData->getBuf(band));
         ++tempBandIdx;
      }
      else
      {
         if(tempBandIdx)
         {
            theTile->copyNormalizedBufferToTile(band,
                                                (float*)currentImageData->getBuf(tempBandIdx-1));
         }
      }
   }
   destination->validate();
   
   currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);

   while(currentImageData.valid())
   {  
      rspf_uint32 minNumberOfBands           = currentImageData->getNumberOfBands();
      rspfDataObjectStatus currentStatus     = currentImageData->getDataObjectStatus();
      rspfDataObjectStatus destinationStatus = destination->getDataObjectStatus();
      
      if(destinationStatus == RSPF_FULL)
      {
         return destination;
      }
      for(band = 0; band < minNumberOfBands; ++band)
      {
         srcBands[band]        = static_cast<float*>(currentImageData->getBuf(band));
         srcBandsNullPix[band] = static_cast<float>(currentImageData->getNullPix(band));
      }
      // if the src is smaller than the destination in number
      // of bands we will just duplicate the last band.
      for(;band < theLargestNumberOfInputBands; ++band)
      {
         srcBands[band] = srcBands[minNumberOfBands - 1];
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      }

      if((destinationStatus == RSPF_EMPTY)&&
         (currentStatus     != RSPF_EMPTY)&&
         (currentStatus     != RSPF_NULL))
      {
         rspf_uint32 upperBound = destination->getWidth()*destination->getHeight();
         for(band=0; band < theLargestNumberOfInputBands; ++band)
         {
            float delta = destBandsMaxPix[band] - destBandsMinPix[band];
            float minP  = destBandsMinPix[band];
            for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
            {
               destBands[band][offset] = (T)( minP + delta*srcBands[band][offset]);
            }
         }
      }
      else if((destinationStatus == RSPF_PARTIAL)&&
              (currentStatus     != RSPF_EMPTY)&&
              (currentStatus     != RSPF_NULL))
      {
         for(band = 0; band < theLargestNumberOfInputBands; ++band)
         {
            
            float delta = destBandsMaxPix[band] - destBandsMinPix[band];
            float minP  = destBandsMinPix[band];
            for(rspf_uint32 offset = 0;
                offset < upperBound;
                ++offset)
            {
               
               if(destBands[band][offset] == destBandsNullPix[band])
               {
                  destBands[band][offset] = (T)(minP + delta*srcBands[band][offset]);
               }
            }
         }
      }
      destination->validate();
      
      currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
   }
   // Cleanup...
   delete [] srcBands;
   delete [] srcBandsNullPix;
   delete [] destBands;
   delete [] destBandsNullPix;
   delete [] destBandsMinPix;
   delete [] destBandsMaxPix;

   return destination;
}

template <class T> rspfRefPtr<rspfImageData> rspfMaxMosaic::combine(
   T,// dummy template variable 
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   rspf_uint32 layerIdx = 0;
   rspfRefPtr<rspfImageData> destination = theTile;

   rspfRefPtr<rspfImageData> currentImageData =
      getNextTile(layerIdx, 0, tileRect, resLevel);
      
   T** srcBands         = new T*[theLargestNumberOfInputBands];
   T*  srcBandsNullPix  = new T[theLargestNumberOfInputBands];
   T** destBands        = new T*[theLargestNumberOfInputBands];
   T*  destBandsNullPix = new T[theLargestNumberOfInputBands];
      
   rspf_uint32 band;
   rspf_uint32 upperBound = destination->getWidth()*destination->getHeight();
   rspf_uint32 bandIndex  = 0;
   if(!currentImageData)
   {
      return currentImageData;
   }
   rspf_uint32 offset=0;
   rspf_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
   for(band = 0; band < minNumberOfBands; ++band)
   {
      srcBands[band]  = static_cast<T*>(currentImageData->getBuf(band));
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      srcBandsNullPix[band]  = static_cast<T>(currentImageData->getNullPix(band));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
   }
   // if the src is smaller than the destination in number
   // of bands we will just duplicate the last band.
   for(;band < theLargestNumberOfInputBands; ++band)
   {
      srcBands[band]  = static_cast<T*>(srcBands[minNumberOfBands - 1]);
      destBands[band] = static_cast<T*>(theTile->getBuf(band));
      srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      destBandsNullPix[band] = static_cast<T>(theTile->getNullPix(band));
   }
   // most of the time we will not overlap so let's
   // copy the first tile into destination and check later.
   //
   for(band = 0; band < theTile->getNumberOfBands();++band)
   {
      T* destBand = destBands[band];
      T* srcBand  = srcBands[band];
      if(destBand&&srcBand)
      {
         for(offset = 0; offset < upperBound;++offset)
         {
	   *destBand = *srcBand;
	   ++srcBand; ++destBand;
         }
      }
   }
   destination->setDataObjectStatus(currentImageData->getDataObjectStatus());

   currentImageData = getNextTile(layerIdx,
                                  tileRect,
                                  resLevel);

   while(currentImageData.valid())
   {  
      rspf_uint32 minNumberOfBands           = currentImageData->getNumberOfBands();
      rspfDataObjectStatus currentStatus     = currentImageData->getDataObjectStatus();
      rspfDataObjectStatus destinationStatus = destination->getDataObjectStatus();
      
      for(band = 0; band < minNumberOfBands; ++band)
      {
         srcBands[band]        = static_cast<T*>(currentImageData->getBuf(band));
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(band));
      }
      // if the src is smaller than the destination in number
      // of bands we will just duplicate the last band.
      for(;band < theLargestNumberOfInputBands; ++band)
      {
         srcBands[band] = srcBands[minNumberOfBands - 1];
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      }


      if((destinationStatus == RSPF_PARTIAL)&&
	 (currentStatus     != RSPF_EMPTY)&&
	 (currentStatus     != RSPF_NULL))
	{
	  for(bandIndex = 0; bandIndex < theLargestNumberOfInputBands; ++bandIndex)
	    {
	      
	      for(rspf_uint32 offset = 0;
		  offset < upperBound;
		  ++offset)
		{
		  if(srcBands[bandIndex][offset] > destBands[bandIndex][offset])
		    {
		      destBands[bandIndex][offset] = srcBands[bandIndex][offset];
		    }
		}
	    }
	}
      else
	{
	  rspf_uint32 upperBound = destination->getWidth()*destination->getHeight();
	  for(rspf_uint32 band=0; band < theLargestNumberOfInputBands; ++band)
	    {
	      for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
		{
		  if(srcBands[band][offset] > destBands[band][offset])
		    {
		      destBands[band][offset] = srcBands[band][offset];
		    }
		}
	    }
	}
      
      destination->validate();
      
      currentImageData = getNextTile(layerIdx,tileRect, resLevel);
   }
   // Cleanup...
   delete [] srcBands;
   delete [] srcBandsNullPix;
   delete [] destBands;
   delete [] destBandsNullPix;
   return destination;
}

rspfString rspfMaxMosaic::getShortName()const
{
   return rspfString("rspfMaxMosaic");
}

rspfString rspfMaxMosaic::getLongName()const
{
   return rspfString("Max Mosaic");
}

rspfString rspfMaxMosaic::getDescription()const
{
   return rspfString("Combiner which puts maximum dn value on image.");
}
