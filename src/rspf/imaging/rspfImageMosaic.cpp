//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: implementation for image mosaic
//
//*************************************************************************
// $Id: rspfImageMosaic.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfImageMosaic.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfTrace.h>
static const rspfTrace traceDebug("rspfImageMosaic:debug");

using namespace std;

RTTI_DEF1(rspfImageMosaic, "rspfImageMosaic", rspfImageCombiner)
rspfImageMosaic::rspfImageMosaic()
   :rspfImageCombiner(),
    theTile(NULL)
{

}

rspfImageMosaic::rspfImageMosaic(rspfConnectableObject::ConnectableObjectList& inputSources)
    : rspfImageCombiner(inputSources),
      theTile(NULL)
{
}


rspfImageMosaic::~rspfImageMosaic()
{
}

rspfRefPtr<rspfImageData> rspfImageMosaic::getTile(
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
      // try to initialize
      allocate();

      // if we still don't have a buffer
      // then we will leave
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
            << " Not supported by rspfImageMosaic" << endl;
      }
   }

   return rspfRefPtr<rspfImageData>();
}

void rspfImageMosaic::initialize()
{
  rspfImageCombiner::initialize();
  theTile = NULL;
}

void rspfImageMosaic::allocate()
{
   theTile = NULL;
   
   if( (getNumberOfInputs() > 0) && getInput(0) )
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);
      theTile->initialize();
   }
}

bool rspfImageMosaic::saveState(rspfKeywordlist& kwl,
                                 const char* prefix)const
{
   return rspfImageCombiner::saveState(kwl, prefix);
}

bool rspfImageMosaic::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   return rspfImageCombiner::loadState(kwl, prefix);
}

template <class T> rspfRefPtr<rspfImageData> rspfImageMosaic::combineNorm(
   T,// dummy template variable 
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   rspf_uint32 layerIdx = 0;
   //---
   // Get the first tile from the input sources.  If this(index 0) is blank
   // that means there are no layers so go no further.
   //---
   rspfRefPtr<rspfImageData> currentImageData =
      getNextNormTile(layerIdx, 0, tileRect, resLevel);
   if(!currentImageData)
   {
      return theTile;
   }
   
   rspfRefPtr<rspfImageData> destination = theTile;
   rspfDataObjectStatus destinationStatus = theTile->getDataObjectStatus();

   
   float** srcBands         = new float*[theLargestNumberOfInputBands];
   float*  srcBandsNullPix  = new float[theLargestNumberOfInputBands];
   T**     destBands        = new T*[theLargestNumberOfInputBands];
   T*      destBandsNullPix = new T[theLargestNumberOfInputBands];
   T*      destBandsMinPix  = new T[theLargestNumberOfInputBands];
   T*      destBandsMaxPix  = new T[theLargestNumberOfInputBands];
   
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

   // Loop to copy from layers to output tile.
   while(currentImageData.valid())
   {
      //---
      // Check the status of the source tile.  If empty get the next source
      // tile and loop back.
      //---
       rspfDataObjectStatus currentStatus =
         currentImageData->getDataObjectStatus();
      if ( (currentStatus == RSPF_EMPTY) || (currentStatus == RSPF_NULL) )
      {
         currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
         continue;
      }
      
      rspf_uint32 minNumberOfBands = currentImageData->getNumberOfBands();
      
      for(band = 0; band < minNumberOfBands; ++band)
      {
         srcBands[band] = static_cast<float*>(currentImageData->getBuf(band));
         srcBandsNullPix[band] = static_cast<float>(currentImageData->getNullPix(band));
      }
      // if the src is smaller than the destination in number
      // of bands we will just duplicate the last band.
      for(;band < theLargestNumberOfInputBands; ++band)
      {
         srcBands[band] = srcBands[minNumberOfBands - 1];
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      }
      
      if ( (currentStatus == RSPF_FULL) &&
           (destinationStatus == RSPF_EMPTY) )
      {
         // Copy full tile to empty tile.
         for(band=0; band < theLargestNumberOfInputBands; ++band)
         {
            float delta = destBandsMaxPix[band] - destBandsMinPix[band];
            float minP  = destBandsMinPix[band];
            
            for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
            {
               destBands[band][offset] =
                  (T)( minP + delta*srcBands[band][offset]);
            }
         }
      }
      else // Copy tile checking all the pixels...
      {
         for(band = 0; band < theLargestNumberOfInputBands; ++band)
         {
            float delta = destBandsMaxPix[band] - destBandsMinPix[band];
            float minP  = destBandsMinPix[band];
            
            for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
            {
               if (destBands[band][offset] == destBandsNullPix[band])
               {
                  if (srcBands[band][offset] != srcBandsNullPix[band])
                  {
                     destBands[band][offset] =
                        (T)(minP + delta*srcBands[band][offset]);
                  }
               }
            }
         }
      }

      // Validate output tile and return if full.
      destinationStatus = destination->validate();
      if (destinationStatus == RSPF_FULL)
      {
         
         break;//return destination;
      }

      // If we get here we're are still not full.  Get a tile from next layer.
      currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
   }

   // Cleanup...
   delete [] srcBands;
   delete [] destBands;
   delete [] srcBandsNullPix;
   delete [] destBandsNullPix;
   delete [] destBandsMinPix;
   delete [] destBandsMaxPix;

   return destination;
}

template <class T> rspfRefPtr<rspfImageData> rspfImageMosaic::combine(
   T,// dummy template variable 
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   rspf_uint32 layerIdx = 0;
   //---
   // Get the first tile from the input sources.  If this(index 0) is blank
   // that means there are no layers so go no further.
   //---
   rspfRefPtr<rspfImageData> currentImageData =
      getNextTile(layerIdx, 0, tileRect, resLevel);
   if (!currentImageData)
   {
      return theTile;
   }

   rspfRefPtr<rspfImageData> destination = theTile;
   rspfDataObjectStatus destinationStatus = theTile->getDataObjectStatus();

   T** srcBands         = new T*[theLargestNumberOfInputBands];
   T*  srcBandsNullPix  = new T[theLargestNumberOfInputBands];
   T** destBands        = new T*[theLargestNumberOfInputBands];
   T*  destBandsNullPix = new T[theLargestNumberOfInputBands];
      
   rspf_uint32 band;
   rspf_uint32 upperBound = destination->getWidth()*destination->getHeight();
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

   // Loop to copy from layers to output tile.
   while(currentImageData.valid())
   {
      //---
      // Check the status of the source tile.  If empty get the next source
      // tile and loop back.
      //---
      rspfDataObjectStatus currentStatus =
         currentImageData->getDataObjectStatus();
      if ( (currentStatus == RSPF_EMPTY) || (currentStatus == RSPF_NULL) )
      {
         currentImageData = getNextNormTile(layerIdx, tileRect, resLevel);
         continue;
      }
      
      rspf_uint32 minNumberOfBands = currentImageData->getNumberOfBands();

      for(band = 0; band < minNumberOfBands; ++band)
      {
         srcBands[band] = static_cast<T*>(currentImageData->getBuf(band));
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(band));
      }
      // if the src is smaller than the destination in number
      // of bands we will just duplicate the last band.
      for(;band < theLargestNumberOfInputBands; ++band)
      {
         srcBands[band] = srcBands[minNumberOfBands - 1];
         srcBandsNullPix[band] = static_cast<T>(currentImageData->getNullPix(minNumberOfBands - 1));
      }

      if ( (currentStatus == RSPF_FULL) &&
           (destinationStatus == RSPF_EMPTY) )
      {
         // Copy full tile to empty tile.
         for(rspf_uint32 band=0; band < theLargestNumberOfInputBands; ++band)
         {
            for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
            {
               destBands[band][offset] = srcBands[band][offset];
            }
         }
      }
      else // Copy tile checking all the pixels...
      {
         for(band = 0; band < theLargestNumberOfInputBands; ++band)
         {
            
            for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
            {
               if(destBands[band][offset] == destBandsNullPix[band])
               {
                  destBands[band][offset] = srcBands[band][offset];
               }
            }
         }
      }

      // Validate output tile and return if full.
      destinationStatus = destination->validate();
      if (destinationStatus == RSPF_FULL)
      {
         break;//return destination;
      }

      // If we get here we're are still not full.  Get a tile from next layer.
      currentImageData = getNextTile(layerIdx, tileRect, resLevel);
   }
   
   // Cleanup...
   delete [] srcBands;
   delete [] destBands;
   delete [] srcBandsNullPix;
   delete [] destBandsNullPix;
   
   return destination;
}
