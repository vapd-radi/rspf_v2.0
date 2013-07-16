//*******************************************************************
// Copyright (C) 2010 Radiant Blue Technologies Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: William Watkins
// 
// Description: A brief description of the contents of the file.
//
//*************************************************************************

#include <rspf/imaging/rspfSICDToDetectedImage.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfTrace.h>

RTTI_DEF1(rspfSICDToDetectedImage,
          "rspfSICDToDetectedImage",
          rspfImageSourceFilter)

rspfSICDToDetectedImage::rspfSICDToDetectedImage()
:rspfImageSourceFilter()
{  
}

rspfSICDToDetectedImage::~rspfSICDToDetectedImage()
{
}

rspfRefPtr<rspfImageData> rspfSICDToDetectedImage::getTile(
                                                              const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> tile = NULL;
   
   if(theInputConnection)
   {
      tile = theInputConnection->getTile(tileRect, resLevel);
      
      if(!tile.valid())
      {
         return tile;
      }
      
      if(!isSourceEnabled() ||
         (tile->getDataObjectStatus()==RSPF_NULL)||
         (tile->getDataObjectStatus()==RSPF_EMPTY))
      {
         return tile;
      }
      
      if(!theTile.valid()) 
      {
         allocate();
      }
      
      if(!theTile.valid())
      {
         return tile;
      }
      
      theTile->setImageRectangle(tileRect);
      
      if (tile->getNumberOfBands() == 2)
      {
         switch(tile->getScalarType())
         {
            case RSPF_UINT8:				// SICD 8 Bit data is Amplitude and Phase
            {
               processAmplitudeAngleTile((rspf_uint8)0,
                                         tile.get());
               break;
            }
            case RSPF_SINT8:
            {
               processAmplitudeAngleTile((rspf_sint8)0,
                                         tile.get());
               break;
            }
            case RSPF_UINT16:			// SICD 16 Bit data is complex
            {
               processComplexTile((rspf_uint16)0,
                                  tile.get());
               break;
            }
            case RSPF_SINT16:
            {
               processComplexTile((rspf_sint16)0,
                                  tile.get());
               break;
            }			  
            case RSPF_FLOAT32:
            {
               processComplexTile((rspf_float32)0,
                                  tile.get());
               break;
            }
            default:
            {
               break;
            }
         }
         theTile->validate();
      }
      
      return theTile;
   }
   
   return theTile;
}

template<class T>
void rspfSICDToDetectedImage::processComplexTile(T /* dummy */, rspfImageData* tile)
{
	T* bands[2];		
	T* result = (T*)theTile->getBuf();		
   
   bands[0] = (T*)tile->getBuf(0);
   bands[1] = (T*)tile->getBuf(1);
   
   rspf_uint32 offset = 0;
   rspf_uint32 maxIdx = tile->getWidth()*tile->getHeight();
   
   for(offset = 0; offset < maxIdx; ++offset)
   {
      result[offset] = sqrt((rspf_float64)bands[0][offset]*(rspf_float64)bands[0][offset] + (rspf_float64)bands[1][offset]*(rspf_float64)bands[1][offset]);
   }
   
}

template<class T>
void rspfSICDToDetectedImage::processAmplitudeAngleTile(T /* dummy */, rspfImageData* tile)
{
	T* bands[2];		
	T* result = (T*)theTile->getBuf();		
	
	bands[0] = (T*)tile->getBuf(0);
	bands[1] = (T*)tile->getBuf(1);
	
	rspf_uint32 offset = 0;
	rspf_uint32 maxIdx = tile->getWidth()*tile->getHeight();
	
   
	for(offset = 0; offset < maxIdx; ++offset)
	{
		result[offset] = bands[0][offset];  // The SICD spec says 8 bit data is Mag 8 bits and Phase 8 bits (Not complex)
	}
}


void rspfSICDToDetectedImage::initialize()
{
   rspfImageSourceFilter::initialize();
   theTile     = 0;
}

void rspfSICDToDetectedImage::allocate()
{
   if( isSourceEnabled() && theInputConnection )
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);
      if(theTile.valid())
      {
      }
	   if(theTile.valid())
      {
         theTile->initialize();
      }
      else
      {
         theTile     = 0;
      }
   }
}

void rspfSICDToDetectedImage::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property)
   {
      return;
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfSICDToDetectedImage::getProperty(const rspfString& name)const
{
   return rspfImageSourceFilter::getProperty(name);
}

void rspfSICDToDetectedImage::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
}





