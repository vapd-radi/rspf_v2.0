//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Contributor: David A. Horner (DAH) - http://dave.thehorners.com
// 
//*************************************************************************
// $Id: rspfImageDataFactory.cpp 22135 2013-02-02 16:27:24Z dburken $

#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/imaging/rspfU11ImageData.h>
#include <rspf/imaging/rspfU16ImageData.h>
#include <rspf/imaging/rspfS16ImageData.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfScalarTypeLut.h>

// Static trace for debugging
static rspfTrace traceDebug("rspfImageDataFactory:debug");

rspfImageDataFactory* rspfImageDataFactory::theInstance = 0;
OpenThreads::Mutex rspfImageDataFactory::theInstanceMutex;
rspfImageDataFactory::rspfImageDataFactory() 
{
   theInstance = 0;
}

rspfImageDataFactory::~rspfImageDataFactory()
{
   if(theInstance)
   {
      //delete theInstance;
      theInstance = 0;
   }
}

rspfImageDataFactory* rspfImageDataFactory::instance()
{
   theInstanceMutex.lock();
   if(!theInstance)
   {
      theInstance = new rspfImageDataFactory;
   }
   theInstanceMutex.unlock();
   return theInstance;
}

rspfRefPtr<rspfImageData> rspfImageDataFactory::create(
   rspfSource* owner,
   rspfScalarType scalar,
   rspf_uint32 bands)const
{
   rspfIpt tileSize;
   rspf::defaultTileSize(tileSize);
   rspf_uint32 width  = tileSize.x;
   rspf_uint32 height = tileSize.y;
   
   // do some bounds checking and initialize to a default
   bands  = (bands>0)?bands:1;
   scalar = scalar != RSPF_SCALAR_UNKNOWN?scalar:RSPF_UINT8;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageDataFactory::create DEBUG:"
         << "\nCaller:  "
         << (owner ? owner->getClassName().c_str() : "unknown")
         << "\nbands:   " << bands
         << "\nwidth:   " << width
         << "\nheight:  " << height
         << "\nScalar type:  "
         << (rspfScalarTypeLut::instance()->getEntryString(scalar))
         << std::endl;
   }
   
   rspfRefPtr<rspfImageData> result = 0;
   switch(scalar)
   {
      case RSPF_UINT8:
      {
         result = new rspfU8ImageData(owner, bands, width, height);
         break;
      }
      case RSPF_USHORT11:
      {
         result = new rspfU11ImageData(owner, bands, width, height);
         break;
      }
      case RSPF_UINT16:
      {
         result = new rspfU16ImageData(owner, bands, width, height);
         break;
      }
      case RSPF_SINT16:
      {
         result = new rspfS16ImageData(owner, bands, width, height);
         break;
      }
      default:
      {
         // create a generic image data implementation.
         result = new rspfImageData(owner, scalar, bands, width, height);

         // Set the scalar type for stretching.
         rspfImageSource* inputSource = dynamic_cast<rspfImageSource*>(owner);
         if( inputSource )
         {
            for(rspf_uint32 band = 0; band < bands; ++band)
            {
               result->setMinPix(inputSource->getMinPixelValue(band), band);
               result->setMaxPix(inputSource->getMaxPixelValue(band), band);
               result->setNullPix(inputSource->getNullPixelValue(band), band);
            }
         }
         break;
      }
   }

   return result;
}

rspfRefPtr<rspfImageData> rspfImageDataFactory::create(
   rspfSource* owner,
   rspfScalarType scalar,
   rspf_uint32 bands,
   rspf_uint32 width,
   rspf_uint32 height)const
{
   rspfIpt tileSize;
   rspf::defaultTileSize(tileSize);
   // do some bounds checking and initialize to a default
   bands  = bands  > 0?bands:1;
   width  = width  > 0?width:tileSize.x;
   height = height > 0?height:tileSize.y;
   scalar = scalar != RSPF_SCALAR_UNKNOWN?scalar:RSPF_UINT8;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageDataFactory::create DEBUG:"
         << "\nCaller:  "
         << (owner ? owner->getClassName().c_str() : "unknown")
         << "\nbands:   " << bands
         << "\nwidth:   " << width
         << "\nheight:  " << height
         << "\nScalar type:  "
         << (rspfScalarTypeLut::instance()->getEntryString(scalar))
         << std::endl;
   }
   
   rspfRefPtr<rspfImageData> result = 0;
   switch(scalar)
   {
      case RSPF_UINT8:
      {
         result = new rspfU8ImageData(owner, bands, width, height);
         break;
      }
      case RSPF_USHORT11:
      {
         result = new rspfU11ImageData(owner, bands, width, height);
         break;
      }
      case RSPF_UINT16:
      {
         result = new rspfU16ImageData(owner, bands, width, height);
         break;
      }
      case RSPF_SINT16:
      {
         result = new rspfS16ImageData(owner, bands, width, height);
         break;
      }
      default:
      {
         // create a generic image data implementation.
         result = new rspfImageData(owner, scalar, bands, width, height);
         break;
      }
   }

   return result;
}

rspfRefPtr<rspfImageData> rspfImageDataFactory::create(
   rspfSource* owner,
   rspf_uint32 bands,
   rspfImageSource* inputSource)const
{
   rspfRefPtr<rspfImageData> result = 0;

   if(inputSource)
   {
      rspfScalarType scalar = inputSource->getOutputScalarType();
      rspf_uint32 width  = inputSource->getTileWidth();
      rspf_uint32 height = inputSource->getTileHeight();

      result = create(owner, scalar, bands, width, height);
      if ( result.valid() )
      {
         for(rspf_uint32 band = 0; band < bands; ++band)
         {
            result->setMinPix(inputSource->getMinPixelValue(band),   band);
            result->setMaxPix(inputSource->getMaxPixelValue(band),   band);
            result->setNullPix(inputSource->getNullPixelValue(band), band);
         }
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageDataFactory::create ERROR:"
         << "\nNULL input source!" << std::endl;
   }
   
   return result;
}


rspfRefPtr<rspfImageData> rspfImageDataFactory::create(
   rspfSource* owner,
   rspfImageSource* inputSource)const
{
   rspfRefPtr<rspfImageData> result = 0;

   if(inputSource)
   {
      rspfScalarType scalar = inputSource->getOutputScalarType();
      rspf_uint32 bands  = inputSource->getNumberOfOutputBands();
      rspf_uint32 width  = inputSource->getTileWidth();
      rspf_uint32 height = inputSource->getTileHeight();

      result = create(owner, scalar, bands, width, height);
      if ( result.valid() )
      {
         for(rspf_uint32 band = 0; band < bands; ++band)
         {
            result->setMinPix(inputSource->getMinPixelValue(band),   band);
            result->setMaxPix(inputSource->getMaxPixelValue(band),   band);
            result->setNullPix(inputSource->getNullPixelValue(band), band);
         }
      }
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageDataFactory::create ERROR:"
         << "\nNULL input source!" << std::endl;
   }

   return result;
}
