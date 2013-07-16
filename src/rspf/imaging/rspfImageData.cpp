//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageData.cpp 22161 2013-02-25 12:10:04Z gpotts $

#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>
#include <rspf/base/rspfScalarTypeLut.h>

#include <iterator>
#include <ostream>


RTTI_DEF1(rspfImageData, "rspfImageData", rspfRectilinearDataObject)

rspfImageData::rspfImageData()
   : rspfRectilinearDataObject(2,            // 2d
                                0,         // owner
                                1,            // bands
                                RSPF_UINT8), // scalar
     m_nullPixelValue(0),
     m_minPixelValue(0),
     m_maxPixelValue(0),
     m_alpha(0),
     m_origin(0, 0),
     m_indexedFlag(false)
{
   rspfIpt tileSize;
   rspf::defaultTileSize(tileSize);
   m_spatialExtents[0] = tileSize.x;
   m_spatialExtents[1] = tileSize.y;
   initializeDefaults();
}

rspfImageData::rspfImageData(rspfSource*    owner,
                               rspfScalarType scalar,
                               rspf_uint32    bands)
   : rspfRectilinearDataObject(2,
                                owner,
                                bands,
                                scalar),
     m_nullPixelValue(0),
     m_minPixelValue(0),
     m_maxPixelValue(0),
     m_alpha(0),
     m_origin(0, 0),
     m_indexedFlag(false)
{
   rspfIpt tileSize;
   rspf::defaultTileSize(tileSize);
   m_spatialExtents[0] = tileSize.x;
   m_spatialExtents[1] = tileSize.y;
   initializeDefaults();
}

rspfImageData::rspfImageData(rspfSource* owner,
                               rspfScalarType scalar,
                               rspf_uint32 bands,
                               rspf_uint32 width,
                               rspf_uint32 height)
   : rspfRectilinearDataObject(owner,
                               bands,
                               width, // 2-D array
                               height,
                               scalar),
     m_nullPixelValue(0),
     m_minPixelValue(0),
     m_maxPixelValue(0),
     m_alpha(0),
     m_origin(0, 0),
     m_indexedFlag(false)
{   
   m_spatialExtents[0] = width;
   m_spatialExtents[1] = height;
   initializeDefaults();
}

rspfImageData::rspfImageData(const rspfImageData &rhs)
   : rspfRectilinearDataObject(rhs),
     m_nullPixelValue(rhs.m_nullPixelValue),
     m_minPixelValue(rhs.m_minPixelValue),
     m_maxPixelValue(rhs.m_maxPixelValue),
     m_alpha(rhs.m_alpha),
     m_origin(rhs.m_origin),
     m_indexedFlag(rhs.m_indexedFlag)
{
}

const rspfImageData& rspfImageData::operator=(const rspfImageData& rhs)
{
   if (this != &rhs)
   {
      // rspfRectilinearDataObject initialization:
      rspfRectilinearDataObject::operator=(rhs);
      
      // rspfImageData (this) members:
      m_nullPixelValue = rhs.m_nullPixelValue;
      m_minPixelValue  = rhs.m_minPixelValue;
      m_maxPixelValue  = rhs.m_maxPixelValue;
      m_alpha          = rhs.m_alpha;
      m_origin         = rhs.m_origin;
      m_indexedFlag    = rhs.m_indexedFlag;
   }
   return *this;
}

rspfImageData::~rspfImageData()
{
}

bool rspfImageData::isValidBand(rspf_uint32 band) const
{
   return (band<getNumberOfDataComponents());
}

rspf_uint32 rspfImageData::getSize() const
{
   return (getSizePerBand() * getNumberOfDataComponents());
}

rspf_uint32 rspfImageData::getSizePerBand() const
{
   return (getHeight() * getWidth());
}

rspf_uint32 rspfImageData::getSizeInBytes() const
{
   return (getSizePerBandInBytes() * getNumberOfDataComponents());
}

rspf_uint32 rspfImageData::getSizePerBandInBytes() const
{
   return (getHeight() * getWidth() * getScalarSizeInBytes());
}

const rspf_uint8* rspfImageData::getAlphaBuf() const
{
   if (m_alpha.size() > 0)
   {
      return &m_alpha.front();
   }
   return 0;
}

rspf_uint8* rspfImageData::getAlphaBuf()
{
   if (m_alpha.size() > 0)
   {
      return &m_alpha.front();
   }
   return 0;
}

const void* rspfImageData::getBuf() const
{
   if (m_dataBuffer.size() > 0)
   {
      return static_cast<const void*>(&m_dataBuffer.front());
   }
   return 0;
}

void* rspfImageData::getBuf()
{
   if (m_dataBuffer.size() > 0)
   {
      return static_cast<void*>(&m_dataBuffer.front());
   }
   return 0;
}

const void* rspfImageData::getBuf(rspf_uint32 band) const
{
   const rspf_uint8* b = static_cast<const rspf_uint8*>(getBuf());
   
   if (isValidBand(band) && b != 0)
   {
      b += (band * getSizePerBandInBytes());
      return static_cast<const void*>(b);
   }
   return 0;
}

void* rspfImageData::getBuf(rspf_uint32 band)
{
   rspf_uint8* b = static_cast<rspf_uint8*>(getBuf());
   
   if (isValidBand(band) && b != 0)
   {
      b += (band * getSizePerBandInBytes());
      return static_cast<void*>(b);
   }
   return 0;
}

const rspf_uint8* rspfImageData::getUcharBuf() const
{
   if (m_scalarType == RSPF_UINT8)
   {
      return static_cast<const rspf_uint8*>(getBuf());
   }
   return 0;
}

const rspf_uint16* rspfImageData::getUshortBuf() const
{
   if (m_scalarType == RSPF_UINT16 ||
       m_scalarType == RSPF_USHORT11)
   {
      return static_cast<const rspf_uint16*>(getBuf());
   }
   return 0;
}

const rspf_sint16* rspfImageData::getSshortBuf() const
{
   if (m_scalarType == RSPF_SINT16)
   {
      return static_cast<const rspf_sint16*>(getBuf());
   }
   return 0;
}

const rspf_float32* rspfImageData::getFloatBuf() const
{
   if (m_scalarType == RSPF_FLOAT32 ||
       m_scalarType == RSPF_NORMALIZED_FLOAT)
   {
      return static_cast<const rspf_float32*>(getBuf());
   }
   return 0;
}

const rspf_float64* rspfImageData::getDoubleBuf() const
{
   if (m_scalarType == RSPF_FLOAT64 ||
       m_scalarType == RSPF_NORMALIZED_DOUBLE)
   {
      return static_cast<const rspf_float64*>(getBuf());
   }
   return 0;
}

rspf_uint8* rspfImageData::getUcharBuf() 
{
   if (m_scalarType == RSPF_UINT8)
   {
      return static_cast<rspf_uint8*>(getBuf());
   }
   return 0;
}

rspf_uint16* rspfImageData::getUshortBuf() 
{
   if (m_scalarType == RSPF_UINT16 ||
       m_scalarType == RSPF_USHORT11)
   {
      return static_cast<rspf_uint16*>(getBuf());
   }
   return 0;
}

rspf_sint16* rspfImageData::getSshortBuf() 
{
   if (m_scalarType == RSPF_SINT16)
   {
      return static_cast<rspf_sint16*>(getBuf());
   }
   return 0;
}

rspf_float32* rspfImageData::getFloatBuf() 
{
   if (m_scalarType == RSPF_FLOAT32 ||
       m_scalarType == RSPF_NORMALIZED_FLOAT)
   {
      return static_cast<rspf_float32*>(getBuf());
   }
   return 0;
}

rspf_float64* rspfImageData::getDoubleBuf() 
{
   if (m_scalarType == RSPF_FLOAT64 ||
       m_scalarType == RSPF_NORMALIZED_DOUBLE)
   {
      return static_cast<rspf_float64*>(getBuf());
   }
   return 0;
}

const rspf_uint8* rspfImageData::getUcharBuf(rspf_uint32 band) const
{
   if (m_scalarType == RSPF_UINT8)
   {
      return static_cast<const rspf_uint8*>(getBuf(band));
   }
   return 0;
}

const rspf_uint16* rspfImageData::getUshortBuf(rspf_uint32 band) const
{
   if (m_scalarType == RSPF_UINT16 ||
       m_scalarType == RSPF_USHORT11)
   {
      return static_cast<const rspf_uint16*>(getBuf(band));
   }
   return 0;
}

const rspf_sint16* rspfImageData::getSshortBuf(rspf_uint32 band) const
{
   if (m_scalarType == RSPF_SINT16)
   {
      return static_cast<const rspf_sint16*>(getBuf(band));
   }
   return 0;
}

const rspf_float32* rspfImageData::getFloatBuf(rspf_uint32 band) const
{
   if (m_scalarType == RSPF_FLOAT32 ||
       m_scalarType == RSPF_NORMALIZED_FLOAT)
   {
      return static_cast<const rspf_float32*>(getBuf(band));
   }
   return 0;
}

const rspf_float64* rspfImageData::getDoubleBuf(rspf_uint32 band) const
{
   if (m_scalarType == RSPF_FLOAT64 ||
       m_scalarType == RSPF_NORMALIZED_DOUBLE)
   {
      return static_cast<const rspf_float64*>(getBuf(band));
   }
   return 0;
}

rspf_uint8* rspfImageData::getUcharBuf(rspf_uint32 band) 
{
   if (m_scalarType == RSPF_UINT8)
   {
      return static_cast<rspf_uint8*>(getBuf(band));
   }
   return 0;
}

rspf_uint16* rspfImageData::getUshortBuf(rspf_uint32 band) 
{
   if (m_scalarType == RSPF_UINT16 ||
       m_scalarType == RSPF_USHORT11)
   {
      return static_cast<rspf_uint16*>(getBuf(band));
   }
   return 0;
}

rspf_sint16* rspfImageData::getSshortBuf(rspf_uint32 band) 
{
   if (m_scalarType == RSPF_SINT16)
   {
      return static_cast<rspf_sint16*>(getBuf(band));
   }
   return 0;
}

rspf_float32* rspfImageData::getFloatBuf(rspf_uint32 band) 
{
   if (m_scalarType == RSPF_FLOAT32 ||
       m_scalarType == RSPF_NORMALIZED_FLOAT)
   {
      return static_cast<rspf_float32*>(getBuf(band));
   }
   return 0;
}

rspf_float64* rspfImageData::getDoubleBuf(rspf_uint32 band) 
{
   if (m_scalarType == RSPF_FLOAT64 ||
       m_scalarType == RSPF_NORMALIZED_DOUBLE)
   {
      return static_cast<rspf_float64*>(getBuf(band));
   }
   return 0;
}

void rspfImageData::getNormalizedFloat(rspf_uint32 offset,
                                        rspf_uint32 bandNumber,
                                        rspf_float32& result)const
{
   // Make sure that the types and width and height are good.
   if( (getDataObjectStatus() != RSPF_NULL) && (bandNumber < getNumberOfDataComponents()) )
   {
      rspf_float32 p = 0.0;

      switch (getScalarType())
      {
         case RSPF_UINT8:
         {
            const unsigned char* sourceBuf = getUcharBuf(bandNumber);
            p = sourceBuf[offset];
            break;
         }
         case RSPF_SINT8:
         {
            const rspf_sint8* sourceBuf = static_cast<const rspf_sint8*>(getBuf(bandNumber));
            p = sourceBuf[offset];
            break;
         }
         case RSPF_USHORT11:
         case RSPF_UINT16:
         {
            const rspf_uint16* sourceBuf = getUshortBuf(bandNumber);
            p = sourceBuf[offset];
            break;
         }
         case RSPF_SINT16:
         {
            const rspf_sint16* sourceBuf = getSshortBuf(bandNumber);
            p = sourceBuf[offset];
            break;
         }
         case RSPF_UINT32:
         {
            const rspf_uint32* sourceBuf =
               static_cast<const rspf_uint32*>(getBuf(bandNumber));
            p = sourceBuf[offset];
            break;
         }
         case RSPF_SINT32:
         {
            const rspf_sint32* sourceBuf = static_cast<const rspf_sint32*>(getBuf(bandNumber));
            p = sourceBuf[offset];
            break;
         }
         case RSPF_NORMALIZED_FLOAT:
         case RSPF_FLOAT32:
         {
            const rspf_float32* sourceBuf = getFloatBuf(bandNumber);
            p = sourceBuf[offset];
            break;
         }
         case RSPF_FLOAT64:
         case RSPF_NORMALIZED_DOUBLE:
         {
            const rspf_float64* sourceBuf = getDoubleBuf(bandNumber);
            p = sourceBuf[offset];
            break;
         }
         case RSPF_SCALAR_UNKNOWN:
         default:
         {
            // Shouldn't hit this.
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfImageData::setNormalizedFloat Unsupported scalar type!"
               << std::endl;
         }
         
      } // Matches: switch (getScalarType())

      if ( p != m_nullPixelValue[bandNumber] )
      {
         const rspf_float32 DELTA = m_maxPixelValue[bandNumber] - m_minPixelValue[bandNumber] - 1;
         const rspf_float32 OFFSET_TO_ONE = 1 - m_minPixelValue[bandNumber];
         
         result = ( p <= m_maxPixelValue[bandNumber] ) ?
            ( ( p >= m_minPixelValue[bandNumber] ) ? ( p + OFFSET_TO_ONE ) / DELTA : 0.0 ) : 1.0;
      }
      else
      {
         result = 0.0;
      }
      
   } // Matches: if( (getDataObjectStatus() ...
   
} // End: rspfImageData::getNormalizedFloat

void rspfImageData::setNormalizedFloat(rspf_uint32 offset,
                                        rspf_uint32 bandNumber,
                                        rspf_float32 inputValue)
{
   // Make sure that the types and width and height are good.
   if( (getDataObjectStatus() != RSPF_NULL) && (bandNumber < getNumberOfDataComponents()) )
   {

      rspf_float32 p = 0.0;

      if ( inputValue )
      {
         const rspf_float32 DELTA = m_maxPixelValue[bandNumber] - m_minPixelValue[bandNumber] - 1;
         const rspf_float32 OFFSET_TO_MIN = m_minPixelValue[bandNumber] - 1;
         p = inputValue * DELTA + OFFSET_TO_MIN + 0.5;
         if ( p > m_maxPixelValue[bandNumber] )
         {
            p = m_maxPixelValue[bandNumber];
         }
      }
      else
      {
         p = m_nullPixelValue[bandNumber];
      }
   
      switch (getScalarType())
      {
         case RSPF_UINT8:
         {
            unsigned char* sourceBuf = getUcharBuf(bandNumber);
            sourceBuf[offset] = static_cast<rspf_uint8>( p );
            break;
         }
         case RSPF_SINT8:
         {
            rspf_sint8* sourceBuf = static_cast<rspf_sint8*>(getBuf(bandNumber));
            sourceBuf[offset] = static_cast<rspf_sint8>( p );
            break;
         }
         case RSPF_USHORT11:
         case RSPF_UINT16:
         {
            rspf_uint16* sourceBuf = getUshortBuf(bandNumber);
            sourceBuf[offset] = static_cast<rspf_uint16>( p );
            break;
         }
         case RSPF_SINT16:
         {
            rspf_sint16* sourceBuf = getSshortBuf(bandNumber);
            sourceBuf[offset] = static_cast<rspf_sint16>( p );
            break;
         }
         case RSPF_UINT32:
         {
            rspf_uint32* sourceBuf = static_cast<rspf_uint32*>(getBuf(bandNumber));
            sourceBuf[offset] = static_cast<rspf_uint32>( p );
            break;
         }
         case RSPF_SINT32:
         {
            rspf_sint32* sourceBuf = static_cast<rspf_sint32*>(getBuf(bandNumber));
            sourceBuf[offset] = static_cast<rspf_sint32>( p );
            break;
         }
         case RSPF_NORMALIZED_FLOAT:
         case RSPF_FLOAT32:
         {
            rspf_float32* sourceBuf = getFloatBuf(bandNumber);
            sourceBuf[offset] = p;
            break;
         }
         case RSPF_FLOAT64:
         case RSPF_NORMALIZED_DOUBLE:
         {
            rspf_float64* sourceBuf = getDoubleBuf(bandNumber);
            sourceBuf[offset] = p;
            break;
         }
         case RSPF_SCALAR_UNKNOWN:
         default:
            // Shouldn't hit this.
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfImageData::setNormalizedFloat Unsupported scalar type!"
               << std::endl;
            
      } // Matches: switch (getScalarType())

   } // Matches: if( (getDataObjectStatus() ...
   
} // End: rspfImageData::setNormalizedFloat

void rspfImageData::convertToNormalizedFloat(rspfImageData* result)const
{
   if(!result)
   {
      return;
   }
   // make sure that the types and width and height are
   // good.
   if( (result->getScalarType() != RSPF_NORMALIZED_FLOAT) ||
       (result->getNumberOfBands() != this->getNumberOfBands())||
       (result->getWidth() != this->getWidth()) ||
       (result->getHeight() != this->getHeight())||
       (result->getDataObjectStatus() == RSPF_NULL) ||
       (getDataObjectStatus() == RSPF_NULL))
   {
      return;
   }

   copyTileToNormalizedBuffer((rspf_float32*)result->getBuf());
   result->setDataObjectStatus(getDataObjectStatus());
}

rspfRefPtr<rspfImageData> rspfImageData::newNormalizedFloat()const
{
   rspfRefPtr<rspfImageData> result =
      new rspfImageData(0,
                         RSPF_NORMALIZED_FLOAT,
                         this->getNumberOfBands(),
                         this->getWidth(),
                         this->getHeight());
   
   result->initialize();
   
   convertToNormalizedFloat(result.get());
   
   return result;
}

void rspfImageData::convertToNormalizedDouble(rspfImageData* result)const
{
   if(!result)
   {
      return;
   }
   // make sure that the types and width and height are
   // good.
   if( (result->getScalarType() != RSPF_NORMALIZED_DOUBLE) ||
       (result->getNumberOfBands() != this->getNumberOfBands())||
       (result->getWidth() != this->getWidth()) ||
       (result->getHeight() != this->getHeight())||
       (result->getDataObjectStatus() == RSPF_NULL) ||
       (getDataObjectStatus() == RSPF_NULL))
   {
      return;
   }

   copyTileToNormalizedBuffer((rspf_float64*)result->getBuf());
   result->setDataObjectStatus(getDataObjectStatus());
}

rspfImageData* rspfImageData::newNormalizedDouble()const
{
   rspfImageData* result = new rspfImageData(0,
                                               RSPF_NORMALIZED_DOUBLE,
                                               this->getNumberOfBands(),
                                               this->getWidth(),
                                               this->getHeight());
   result->initialize();

   convertToNormalizedDouble(result);
   
   return result;   
}


void rspfImageData::unnormalizeInput(rspfImageData* normalizedInput)
{
   if((normalizedInput->getScalarType() != RSPF_NORMALIZED_FLOAT) &&
      (normalizedInput->getScalarType() != RSPF_NORMALIZED_DOUBLE) )
   {
      //ERROR
      return;
   }

   if(normalizedInput->getScalarType() == RSPF_NORMALIZED_DOUBLE)
   {
      copyNormalizedBufferToTile((rspf_float64*)normalizedInput->getBuf());
   }
   else
   {
      copyNormalizedBufferToTile((rspf_float32*)normalizedInput->getBuf());
   }
}

rspf_float64 rspfImageData::computeMeanSquaredError(
   rspf_float64 meanValue,
   rspf_uint32 bandNumber) const
{
   rspf_float64 result = -1; // invalid MSE
   
   switch (getScalarType())
   {
      case RSPF_UINT8:
      {
         result = computeMeanSquaredError(rspf_uint8(0),
                                          meanValue,
                                          bandNumber);
         break;
      }
      case RSPF_SINT8:
      {
         result = computeMeanSquaredError(rspf_sint8(0),
                                          meanValue,
                                          bandNumber);
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         result = computeMeanSquaredError(rspf_uint16(0),
                                          meanValue,
                                          bandNumber);
         break;
      }  
      case RSPF_SINT16:
      {
         result = computeMeanSquaredError(rspf_sint16(0),
                                          meanValue,
                                          bandNumber);
         break;
      }  
      case RSPF_UINT32:
      {
         result = computeMeanSquaredError(rspf_uint32(0),
                                          meanValue,
                                          bandNumber);
         break;
      }  
      case RSPF_SINT32:
      {
         result = computeMeanSquaredError(rspf_sint32(0),
                                          meanValue,
                                          bandNumber);
         break;
      }  
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         result = computeMeanSquaredError(rspf_float32(0.0),
                                          meanValue,
                                          bandNumber);
         break;
      }  
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         result = computeMeanSquaredError(rspf_float64(0.0),
                                          meanValue,
                                          bandNumber);
         break;
      }  
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         setDataObjectStatus(RSPF_STATUS_UNKNOWN);
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::computeMeanSquaredError File %s line %d\n\
Invalid scalar type:  %d",
                       __FILE__,
                       __LINE__,
                       getScalarType());
         break;
      }
   }

   return result;
}
   
template <class T> rspf_float64 rspfImageData::computeMeanSquaredError(
   T, /* dummyTemplate */
   rspf_float64 meanValue,
   rspf_uint32 bandNumber) const
{
   rspf_float64  result               = -1.0; // invalid MSE

   if ( (getDataObjectStatus() == RSPF_NULL) ||
        (getDataObjectStatus() == RSPF_EMPTY) )
   {
      return result;
   }
   
   rspf_uint32  index           = 0;
   rspf_float64 delta           = 0.0;
   rspf_uint32  validPixelCount = 0;

   const T* BUFFER = static_cast<const T*>(getBuf(bandNumber));
   if(BUFFER)
   {
      const rspf_uint32 BOUNDS = getSizePerBand();
      for(index = 0; index < BOUNDS; ++index)
      {
         if(!isNull(index))
         {
            delta = BUFFER[index] - meanValue;
            result += (delta*delta);
            ++validPixelCount;
         }
      }
      if(validPixelCount > 0)
      {
         result /= validPixelCount;
      }
   }

   return result;
}


//******************************************************************
//
// NOTE: I was checking for null and not adding it to the histogram.
//       this was messing up the equalization algorithms since the
//       accumulation histogram did not represent the area of the
//       image.  For now I will leave out the check for "is null" and
//       add this to the count so that the total accumulation is the
//       area of the image.
//
//******************************************************************
void rspfImageData::populateHistogram(rspfRefPtr<rspfMultiBandHistogram> histo)
{
   rspf_uint32 numberOfBands = getNumberOfBands();

   if( (getDataObjectStatus() == RSPF_NULL) ||
       (getDataObjectStatus() == RSPF_EMPTY)||
       (!histo))
   {
      return;
   }
   switch(getScalarType())
   {
      case RSPF_UINT8:
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            rspfRefPtr<rspfHistogram> currentHisto = histo->getHistogram(band);
            if(currentHisto.valid())
            {
               float* histoBins = currentHisto->GetCounts();
               int binCount = currentHisto->GetRes();
               rspf_uint8* buffer = (rspf_uint8*)getBuf(band);
               rspf_uint32 upperBound = getWidth()*getHeight();
               if ( binCount == 256 )
               {
                  for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
                  {
                     ++histoBins[ buffer[offset] ];
                  }
               }
               else
               {
                 for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
                  {
                     currentHisto->UpCount((float)buffer[offset]);
                  }
               }
            }
         }
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            rspfRefPtr<rspfHistogram> currentHisto = histo->getHistogram(band);
	 
             rspf_uint16* buffer = (rspf_uint16*)getBuf(band);
             if(currentHisto.valid())
             {
                rspf_uint32 upperBound = getWidth()*getHeight();
                for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
                {
                  currentHisto->UpCount((float)buffer[offset]);
                }
             }
         }
         break;
      }
      case RSPF_SINT16:
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            rspfRefPtr<rspfHistogram> currentHisto = histo->getHistogram(band);
            rspf_sint16* buffer = (rspf_sint16*)getBuf(band);
            
            if(currentHisto.valid())
            {
               rspf_uint32 upperBound = getWidth()*getHeight();
               for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
               {
                  currentHisto->UpCount((float)buffer[offset]);
               }
            }
         }
         break;
      }
      case RSPF_SINT32:
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            rspfRefPtr<rspfHistogram> currentHisto = histo->getHistogram(band);
            rspf_sint32* buffer = (rspf_sint32*)getBuf(band);
            
            if(currentHisto.valid())
            {
               rspf_uint32 upperBound = getWidth()*getHeight();
               for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
               {
                  currentHisto->UpCount((float)buffer[offset]);
               }
            }
         }
         break;
      }
      case RSPF_UINT32:
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            rspfRefPtr<rspfHistogram> currentHisto = histo->getHistogram(band);
            rspf_uint32* buffer = (rspf_uint32*)getBuf(band);
            
            if(currentHisto.valid())
            {
               rspf_uint32 upperBound = getWidth()*getHeight();
               for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
               {
                  currentHisto->UpCount((float)buffer[offset]);
               }
            }
         }
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            rspfRefPtr<rspfHistogram> currentHisto = histo->getHistogram(band);
            rspf_float64* buffer = (rspf_float64*)getBuf(band);
            
            if(currentHisto.valid())
            {
               rspf_uint32 upperBound = getWidth()*getHeight();
               for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
               {
                  currentHisto->UpCount((float)buffer[offset]);
               }
            }
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         for(rspf_uint32 band = 0; band < numberOfBands; ++band)
         {
            rspfRefPtr<rspfHistogram> currentHisto = histo->getHistogram(band);
            rspf_float32* buffer = (rspf_float32*)getBuf(band);
            
            if(currentHisto.valid())
            {
               rspf_uint32 upperBound = getWidth()*getHeight();
               for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
               {
                  currentHisto->UpCount((float)buffer[offset]);
               }
            }
         }
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         //ERROR
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::populateHistogram\n"
            << "Unknown scalar type." << std::endl;
      }
   }  // end of switch
}

rspf_float64 rspfImageData::computeAverageBandValue(rspf_uint32 bandNumber) const
{
   rspf_float64 result = 0.0;
   
   switch (getScalarType())
   {
      case RSPF_UINT8:
      {
         result = computeAverageBandValue(rspf_uint8(0),
                                          bandNumber);
         break;
      }  
      case RSPF_SINT8:
      {
         result = computeAverageBandValue(rspf_sint8(0),
                                          bandNumber);
         break;
      }  
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         result = computeAverageBandValue(rspf_uint16(0),
                                          bandNumber);
         break;
      }  
      case RSPF_SINT16:
      {
         result = computeAverageBandValue(rspf_sint16(0),
                                          bandNumber);
         break;
      }  
      case RSPF_UINT32:
      {
         result = computeAverageBandValue(rspf_uint32(0),
                                          bandNumber);
         break;
      }  
      case RSPF_SINT32:
      {
         result = computeAverageBandValue(rspf_sint32(0),
                                          bandNumber);
         break;
      }  
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         result = computeAverageBandValue(rspf_float32(0.0),
                                          bandNumber);
         break;
      }  
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         result = computeAverageBandValue(rspf_float64(0.0),
                                          bandNumber);
         break;
      }  
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         setDataObjectStatus(RSPF_STATUS_UNKNOWN);
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::computeAverageBandValue File %s line %d\n\
Invalid scalar type:  %d",
                       __FILE__,
                       __LINE__,
                       getScalarType());
         break;
      }
   }

   return result;
}

template <class T> rspf_float64 rspfImageData::computeAverageBandValue(
   T, /* dummy */
   rspf_uint32 bandNumber) const
{
   rspf_float64  result = 0.0;

   if ( (getDataObjectStatus() == RSPF_NULL) ||
        (getDataObjectStatus() == RSPF_EMPTY) )
   {
      return result;
   }
   
   rspf_uint32 index = 0;
   rspf_uint32 validPixelCount = 0;

   const T* BUFFER = static_cast<const T*>(getBuf(bandNumber));
   if(BUFFER)
   {
      const rspf_uint32 BOUNDS = getSizePerBand();
      for(index = 0; index < BOUNDS; ++index)
      {
         if(!isNull(index))
         {
            result += BUFFER[index];
            ++validPixelCount;
         }
      }
      if(validPixelCount > 0)
      {
         result /= validPixelCount;
      }
   }

   return result;
}

rspfDataObjectStatus rspfImageData::validate() const
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
      {
         return validate(rspf_uint8(0));
      }
      case RSPF_SINT8:
      {
         return validate(rspf_sint8(0));
      }
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         return validate(rspf_uint16(0));
      }  
      case RSPF_SINT16:
      {
         return validate(rspf_sint16(0));
      }
         
      case RSPF_UINT32:
      {
         return validate(rspf_uint32(0));
      }  
      case RSPF_SINT32:
      {
         return validate(rspf_sint32(0));
      }  
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         return validate(rspf_float32(0.0));
      }
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         return validate(rspf_float64(0.0));
      }  
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         setDataObjectStatus(RSPF_STATUS_UNKNOWN);
         break;
      }
   }

   return RSPF_STATUS_UNKNOWN;
}

template <class T>
rspfDataObjectStatus rspfImageData::validate(T /* dummyTemplate */ ) const
{
   if (m_dataBuffer.size() == 0)
   {
      setDataObjectStatus(RSPF_NULL);
      return RSPF_NULL;
   }

   rspf_uint32       count           = 0;
   const rspf_uint32 SIZE            = getSize();
   const rspf_uint32 BOUNDS          = getSizePerBand();
   const rspf_uint32 NUMBER_OF_BANDS = getNumberOfBands();
   
   for(rspf_uint32 band = 0; band < NUMBER_OF_BANDS; ++band)
   {
      const T NP = static_cast<T>(m_nullPixelValue[band]);
      const T* p = static_cast<const T*>(getBuf(band));
      
      for (rspf_uint32 i = 0; i < BOUNDS; ++i)
      {
         // check if the band is null
         if (p[i] != NP) ++count;         
      }
   }

   if (!count)
      setDataObjectStatus(RSPF_EMPTY);
   else if (count == SIZE)
      setDataObjectStatus(RSPF_FULL);
   else
      setDataObjectStatus(RSPF_PARTIAL);

   return getDataObjectStatus();
}

void rspfImageData::makeBlank()
{
   if ( (m_dataBuffer.size() == 0) || (getDataObjectStatus() == RSPF_EMPTY) )
   {
      return; // nothing to do...
   }

   switch (getScalarType())
   {
      case RSPF_UINT8:
      {
         makeBlank(rspf_uint8(0));
         return;
      }  
      case RSPF_SINT8:
      {
         makeBlank(rspf_sint8(0));
         return;
      }  
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         makeBlank(rspf_uint16(0));
         return;
      }  
      case RSPF_SINT16:
      {
         makeBlank(rspf_sint16(0));
         return;
      }  
      case RSPF_UINT32:
      {
         makeBlank(rspf_uint32(0));
         return;
      }
      case RSPF_SINT32:
      {
         makeBlank(rspf_sint32(0));
         return;
      }  
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
      {
         makeBlank(rspf_float32(0.0));
         return;
      }  
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         makeBlank(rspf_float64(0.0));
         return;
      }  
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         setDataObjectStatus(RSPF_STATUS_UNKNOWN);
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::makeBlank File %s line %d\n\
Invalid scalar type:  %d",
                       __FILE__,
                       __LINE__,
                       getScalarType());
         break;
      }
   }
}

template <class T> void rspfImageData::makeBlank(T /* dummyTemplate */ )
{
   // Note: Empty buffer or already RSPF_EMPTY checked in public method.
   
   const rspf_uint32 BANDS = getNumberOfBands();
   const rspf_uint32 SPB   = getSizePerBand();
   rspf_uint32 sizePerBandInBytes = getSizePerBandInBytes();
   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const T NP = static_cast<T>(m_nullPixelValue[band]);
      if(NP == 0.0)
      {
         // do an optimize byte fill
         memset(getBuf(band), '\0', sizePerBandInBytes);
      }
      else
      {
         // slower assign call
         T* p = static_cast<T*>(getBuf(band));
         for (rspf_uint32 i = 0; i < SPB; ++i)
         {
            p[i] = NP;
         }
      }
   }
   
   setDataObjectStatus(RSPF_EMPTY);
}

void rspfImageData::initialize()
{
   // let the base class allocate a buffer
   rspfRectilinearDataObject::initialize();
   
   if (m_dataBuffer.size() > 0)
   {
      makeBlank();  // Make blank will set the status.
   }
}

// Write the tile out to disk with a general raster header file.
bool rspfImageData::write(const rspfFilename& f) const
{
   bool result = false;

   std::ofstream os;
   os.open(f.c_str(), ios::out | ios::binary);
   if (os.good())
   {
      // Write the tile out.
      os.write(static_cast<const char*>(getBuf()),
               static_cast<std::streamsize>(getSizeInBytes()));
      
      result = os.good();
      if (result)
      {
         // Write a header file that we can use to read the tile.
         os.close();
         rspfFilename hdrFile = f;
         hdrFile.setExtension("hdr");
         os.open(hdrFile.c_str(), ios::out);
         result = os.good();
         if (result)
         {
            os << "filename: " << f.c_str()
               << "\nimage_type:  general_raster_bsq"
               << "\nindexed: " << m_indexedFlag               
               << "\ninterleave_type:  bsq"
               << "\norigin: " << m_origin
               << "\nnumber_bands: " << rspfString::toString(getNumberOfBands())
               << "\nnumber_lines: " << rspfString::toString(getHeight())
               << "\nnumber_samples: " << rspfString::toString(getWidth())
               << "\nscalar_type: "
               << rspfScalarTypeLut::instance()->getEntryString(getScalarType())
               << "\n";
            for(rspf_uint32 band=0; band < getNumberOfBands(); ++band)
            {
               rspfString bs = "band";
               bs += rspfString::toString(band+1); 
               os << bs.c_str() << ".min_value: " << m_minPixelValue[band] << "\n"
                  << bs.c_str() << ".max_value: " << m_maxPixelValue[band] << "\n"
                  << bs.c_str() << ".null_value: " << m_nullPixelValue[band]
                  << std::endl;
            }
         }
      }
   }
   os.close();
   
   return result;
}

bool rspfImageData::isWithin(rspf_int32 x, rspf_int32 y)
{
   return ((x >= m_origin.x) &&
           (x <  m_origin.x + static_cast<rspf_int32>(m_spatialExtents[0])) &&
           (y >= m_origin.y) &&
           (y <  m_origin.y + static_cast<rspf_int32>(m_spatialExtents[1])));
           
}

void rspfImageData::setValue(rspf_int32 x, rspf_int32 y, rspf_float64 color)
{
   if(m_dataBuffer.size() > 0 && isWithin(x, y))
   {
      rspf_uint32 band=0;

      //***
      // Compute the offset into the buffer for (x,y).  This should always
      // come out positive.
      //***
      rspf_uint32 ux = static_cast<rspf_uint32>(x - m_origin.x);
      rspf_uint32 uy = static_cast<rspf_uint32>(y - m_origin.y);
      
      rspf_uint32 offset = uy * m_spatialExtents[0] + ux;
      
      switch (getScalarType())
      {
         case RSPF_UINT8:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               unsigned char* buf = static_cast<unsigned char*>(getBuf(band))+
                  offset;
               *buf = (unsigned char)color;
            }
            break;
         }
         case RSPF_SINT8:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               rspf_sint8* buf = static_cast<rspf_sint8*>(getBuf(band))+
                  offset;
               *buf = (rspf_sint8)color;
            }
            break;
         }
         case RSPF_UINT16:
         case RSPF_USHORT11:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               rspf_uint16* buf = static_cast<rspf_uint16*>(getBuf(band))+
                  offset;
               *buf = (rspf_uint16)color;
            }
            break;
         }
         case RSPF_SINT16:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               signed short* buf = static_cast<signed short*>(getBuf(band))+
                  offset;
               *buf = (signed short)color;
            }
            break;
         }
         case RSPF_UINT32:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               rspf_uint32* buf = static_cast<rspf_uint32*>(getBuf(band))+
                  offset;
               *buf = (rspf_uint32)color;
            }
            break;
         }
         case RSPF_SINT32:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               rspf_sint32* buf = static_cast<rspf_sint32*>(getBuf(band))+
                  offset;
               *buf = (rspf_sint32)color;
            }
            break;
         }
         case RSPF_NORMALIZED_FLOAT:
         case RSPF_FLOAT32:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               rspf_float32* buf = static_cast<rspf_float32*>(getBuf(band))+offset;
               *buf = (rspf_float32)color;
            }
            break;
         }
         case RSPF_FLOAT64:
         case RSPF_NORMALIZED_DOUBLE:
         {
            for(band = 0; band < m_numberOfDataComponents; band++)
            {
               rspf_float64* buf = static_cast<rspf_float64*>(getBuf(band))+offset;
               *buf = color;
            }
            break;
         }
         case RSPF_SCALAR_UNKNOWN:
         default:
         {
            //ERROR
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfImageData::setValue Unsupported scalar type!"
               << std::endl;
            
         }
         
      } // End of:  switch (getScalarType())
   }
}

void rspfImageData::initializeDefaults()
{
   initializeMinDefault();
   initializeMaxDefault();
   initializeNullDefault();
}

void rspfImageData::initializeMinDefault()
{
   if(!m_numberOfDataComponents)
   {
      return;
   }

   m_minPixelValue.resize(m_numberOfDataComponents);

   rspf_float64 value = rspf::defaultMin( getScalarType() );
   
   for(rspf_uint32 band = 0; band < m_numberOfDataComponents; ++band)
   {
      m_minPixelValue[band]  = value;
   }
}

void rspfImageData::initializeMaxDefault()
{
   if(!m_numberOfDataComponents)
   {
      return;
   }

   m_maxPixelValue.resize(m_numberOfDataComponents);

   rspf_float64 value = rspf::defaultMax( getScalarType() );
      
   for(rspf_uint32 band = 0; band < m_numberOfDataComponents; ++band)
   {
      m_maxPixelValue[band]  = value;
   }
}

void rspfImageData::initializeNullDefault()
{
   if(!m_numberOfDataComponents)
   {
      return;
   }

   m_nullPixelValue.resize(m_numberOfDataComponents);

   rspf_float64 value = rspf::defaultNull( getScalarType() );
   
   for(rspf_uint32 band = 0; band < m_numberOfDataComponents; ++band)
   {
      m_nullPixelValue[band]  = value;
   }
}

bool rspfImageData::isEqualTo(const rspfDataObject& rhs,
                               bool deepTest)const
{
   rspfImageData* rhsPtr = PTR_CAST(rspfImageData, &rhs);
   if(!(&rhs)||(!rhsPtr)) return false;
   bool result = ( (m_scalarType         == rhsPtr->m_scalarType)&&
                   (m_numberOfDataComponents == rhsPtr->m_numberOfDataComponents)&&
                   (m_origin             == rhsPtr->m_origin)&&
                   (getWidth()            == rhsPtr->getWidth())&&
                   (getHeight()           == rhsPtr->getHeight()));

   if(result)
   {
      bool test=true;
      for(rspf_uint32 index = 0; index < m_numberOfDataComponents; ++index)
      {
         if(m_minPixelValue[index] != rhsPtr->m_minPixelValue[index])
         {
            test = false;
            break;
         }
         if(m_maxPixelValue[index] != rhsPtr->m_maxPixelValue[index])
         {
            test = false;
            break;
         }
         if(m_nullPixelValue[index] != rhsPtr->m_nullPixelValue[index])
         {
            test = false;
            break;
         }
      }
      result = test;
   }
   if(deepTest&&result)
   {
      if(getBuf() != 0 && rhsPtr->getBuf() != 0)
      {
         if(memcmp(getBuf(), rhsPtr->getBuf(), getSizeInBytes()) != 0)
         {
            result = false;
         }
      }
      else if(getBuf() == 0 && rhsPtr->getBuf() == 0)
      {
         // nothing both are null so don't change the result.
      }
      else // one is null so not equal.
      {
         result = false;
      }
   }

   return result;
}

rspfString rspfImageData::getScalarTypeAsString() const
{
   return rspfScalarTypeLut::instance()->getEntryString(getScalarType());
}

rspf_uint32 rspfImageData::getNumberOfBands() const
{
   return getNumberOfDataComponents();
}

rspf_float64 rspfImageData::getPix(const rspfIpt& position,
                              rspf_uint32 band) const
{
   rspfIpt relative( position.x - m_origin.x,
                      position.y - m_origin.y);
   return getPix((m_spatialExtents[0])*relative.y + relative.x, band);
}

rspf_float64 rspfImageData::getPix(rspf_uint32 offset, rspf_uint32 band) const
{
   switch(m_scalarType)
   {
      case RSPF_UINT8:
      {
         const rspf_uint8* buf = getUcharBuf(band);
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }
      }
      case RSPF_SINT8:
      {
         const rspf_sint8* buf = static_cast<const rspf_sint8*>(getBuf(band));
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         const rspf_uint16* buf = getUshortBuf(band);
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }
         break;
      }
      case RSPF_SINT16:
      {
         const rspf_sint16* buf = getSshortBuf(band);
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }
         break;
      }
      case RSPF_SINT32:
      {
         const rspf_sint32* buf = static_cast<const rspf_sint32*>(getBuf(band));
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }
         break;
      }
      case RSPF_UINT32:
      {
         const rspf_uint32* buf = static_cast<const rspf_uint32*>(getBuf(band));
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         const rspf_float64* buf = getDoubleBuf(band);
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }      
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         const rspf_float32* buf = getFloatBuf(band);
         if(buf)
         {
            return (rspf_float64)buf[offset];
         }
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
         // Shouldn't hit this.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::fill Unsupported scalar type!"
            << std::endl;   
   }

   return 0.0;
}

void rspfImageData::fill(rspf_uint32 band, rspf_float64 value)
{
   void* s         = getBuf(band);

   if (s == 0) return; // nothing to do...

   rspf_uint32 size_in_pixels = getWidth()*getHeight();
   
   switch (getScalarType())
   {
      case RSPF_UINT8:
      {
         rspf_uint8* p = getUcharBuf(band);
         rspf_uint8 np = static_cast<rspf_uint8>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_SINT8:
      {
         rspf_sint8* p = static_cast<rspf_sint8*>(getBuf(band));
         rspf_sint8 np = static_cast<rspf_sint8>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         rspf_uint16* p = getUshortBuf(band);
         rspf_uint16 np = static_cast<rspf_uint16>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_SINT16:
      {
         rspf_sint16* p = getSshortBuf(band);
         rspf_sint16 np = static_cast<rspf_sint16>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_UINT32:
      {
         rspf_uint32* p = static_cast<rspf_uint32*>(getBuf(band));
         rspf_uint32 np = static_cast<rspf_uint32>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_SINT32:
      {
         rspf_sint32* p = static_cast<rspf_sint32*>(getBuf(band));
         rspf_sint32 np = static_cast<rspf_sint32>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         rspf_float32* p = getFloatBuf(band);
         rspf_float32 np = static_cast<rspf_float32>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         rspf_float64* p = getDoubleBuf(band);
         rspf_float64 np = static_cast<rspf_float64>(value);
         for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;
         
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::makeBlank File %s line %d\n\
Invalid scalar type:  %d",
                       __FILE__,
                       __LINE__,
                       getScalarType());
         return;
   }
   
   setDataObjectStatus(RSPF_EMPTY);
   
}

void rspfImageData::fill(rspf_float64 value)
{
   rspf_uint32 valueNullCount= 0;
   for(rspf_uint32 band=0; band < getNumberOfBands(); ++band)
   {
      if (value == m_nullPixelValue[band])
      {
         ++valueNullCount;
      }
      
      fill(band, value);
   }

   if (valueNullCount==0)
   {
      setDataObjectStatus(RSPF_FULL);
   }
   else if(valueNullCount==getNumberOfBands())
   {
      setDataObjectStatus(RSPF_EMPTY);
   }
   else
   {
      setDataObjectStatus(RSPF_PARTIAL);
   }
}

bool rspfImageData::isNull(rspf_uint32 offset)const
{
   rspf_uint32 numberOfBands = getNumberOfBands();
   rspf_uint32 band=0;
   if(!getBuf())
   {
      return true;
   }

   switch(getScalarType())
   {
      case RSPF_UINT8:
      {
         for(band = 0; band < numberOfBands; ++band)  
         {
            const rspf_uint8* buf = static_cast<const rspf_uint8*>(getBuf(band))+offset;
            if((*buf) != (rspf_uint8)getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_SINT8:
      {
         for(band = 0; band < numberOfBands; ++band)  
         {
            const rspf_sint8* buf = static_cast<const rspf_sint8*>(getBuf(band))+offset;
            if((*buf) != (rspf_uint8)getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_uint16* buf = static_cast<const rspf_uint16*>(getBuf(band))+offset;
            if((*buf) != (rspf_uint16)getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_SINT16:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_sint16* buf = static_cast<const rspf_sint16*>(getBuf(band))+offset;
            if((*buf) != (rspf_sint16)getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_UINT32:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_uint32* buf = static_cast<const rspf_uint32*>(getBuf(band))+offset;
            if((*buf) != (rspf_uint32)getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_SINT32:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_sint32* buf = static_cast<const rspf_sint32*>(getBuf(band))+offset;
            if((*buf) != (rspf_sint32)getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_float32* buf = static_cast<const rspf_float32*>(getBuf(band))+offset;
            if((*buf) != 0.0)
            {
               return false;
            }
         }
         break;
      }
      case RSPF_FLOAT32:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_float32* buf = static_cast<const rspf_float32*>(getBuf(band))+offset;
            if((*buf) != (rspf_float32)getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_float64* buf = static_cast<const rspf_float64*>(getBuf(band))+offset;
            if((*buf) != 0.0)
            {
               return false;
            }
         }
         break;
      }
      case RSPF_FLOAT64:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            const rspf_float64* buf = static_cast<const rspf_float64*>(getBuf(band))+offset;
            if((*buf) != getNullPix(band))
            {
               return false;
            }
         }
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         //ERROR
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::isNull Unsupported scalar type!"
            << std::endl;
      }
   }
   return true;
}

bool rspfImageData::isNull(rspf_uint32 offset, rspf_uint32 band)const
{
   switch(getScalarType())
   {
      case RSPF_UINT8:
      {
         const rspf_uint8* buf =
            static_cast<const rspf_uint8*>(getBuf(band))+offset;
         
         if((*buf) != (rspf_uint8)getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_SINT8:
      {
         const rspf_sint8* buf =
            static_cast<const rspf_sint8*>(getBuf(band))+offset;
         
         if((*buf) != (rspf_sint8)getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         const rspf_uint16* buf =
            static_cast<const rspf_uint16*>(getBuf(band))+offset;
         if((*buf) != (rspf_uint16)getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_SINT16:
      {
         const rspf_sint16* buf =
            static_cast<const rspf_sint16*>(getBuf(band))+offset;
         if((*buf) != (rspf_sint16)getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_UINT32:
      {
         const rspf_uint32* buf =
            static_cast<const rspf_uint32*>(getBuf(band))+offset;
         if((*buf) != (rspf_uint32)getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_SINT32:
      {
         const rspf_sint32* buf =
            static_cast<const rspf_sint32*>(getBuf(band))+offset;
         if((*buf) != (rspf_sint32)getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         const rspf_float32* buf = static_cast<const rspf_float32*>(getBuf(band))+offset;
         if((*buf) != (rspf_float32)getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         const rspf_float64* buf = static_cast<const rspf_float64*>(getBuf(band))+offset;
         if((*buf) != getNullPix(band))
         {
            return false;
         }
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         //ERROR
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::isNull Unsupported scalar type!"
            << std::endl;
      }
   }
   return true;
}

bool rspfImageData::isNull(const rspfIpt& pt)const
{
   rspf_int32 xNew = (pt.x - m_origin.x);
   rspf_int32 yNew = (pt.y - m_origin.y);
   if(xNew < 0 || xNew >= static_cast<rspf_int32>(m_spatialExtents[0]) ||
      yNew < 0 || yNew >= static_cast<rspf_int32>(m_spatialExtents[1]) )
   {
      return true;
   }
   rspf_uint32 offset = getWidth()*yNew + xNew;
   
   return isNull(offset);
}

bool rspfImageData::isNull(const rspfIpt& pt, rspf_uint32 band)const
{
   rspf_int32 xNew = (pt.x - m_origin.x);
   rspf_int32 yNew = (pt.y - m_origin.y);
   if(xNew < 0 || xNew >= static_cast<rspf_int32>(m_spatialExtents[0]) ||
      yNew < 0 || yNew >= static_cast<rspf_int32>(m_spatialExtents[1]) )
   {
      return true;
   }
   rspf_uint32 offset = getWidth()*yNew + xNew;

   return isNull(offset, band);
}

void rspfImageData::setNull(rspf_uint32 offset)
{
   rspf_uint32 numberOfBands = getNumberOfBands();
   rspf_uint32 band=0;
   switch(getScalarType())
   {
      case RSPF_UINT8:
      {
         for(band = 0; band < numberOfBands; ++band)  
         {
            rspf_uint8* buf = static_cast<rspf_uint8*>(getBuf(band))+offset;
            *buf       = (rspf_uint8)getNullPix(band);
         }
         break;
      }
      case RSPF_SINT8:
      {
         for(band = 0; band < numberOfBands; ++band)  
         {
            rspf_sint8* buf = static_cast<rspf_sint8*>(getBuf(band))+offset;
            *buf       = (rspf_sint8)getNullPix(band);
         }
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            rspf_uint16* buf = static_cast<rspf_uint16*>(getBuf(band))+
               offset;
            *buf = (rspf_uint16)getNullPix(band);
         }
         break;
      }
      case RSPF_SINT16:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            rspf_sint16* buf = static_cast<rspf_sint16*>(getBuf(band))+
               offset;
            *buf = (rspf_sint16)getNullPix(band);
         }
         break;
      }
      case RSPF_UINT32:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            rspf_uint32* buf = static_cast<rspf_uint32*>(getBuf(band))+
               offset;
            *buf = (rspf_uint32)getNullPix(band);
         }
         break;
      }
      case RSPF_SINT32:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            rspf_sint32* buf = static_cast<rspf_sint32*>(getBuf(band))+
               offset;
            *buf = (rspf_sint32)getNullPix(band);
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            rspf_float32* buf = static_cast<rspf_float32*>(getBuf(band))+offset;
            *buf = (rspf_float32)getNullPix(band);
         }
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         for(band = 0; band < numberOfBands; band++)
         {
            rspf_float64* buf = static_cast<rspf_float64*>(getBuf(band))+offset;
            *buf = getNullPix(band);
         }
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         //ERROR
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::isNull Unsupported scalar type!"
            << std::endl;
      }
   }
}

void rspfImageData::setNull(rspf_uint32 offset, rspf_uint32 band)
{
   switch(getScalarType())
   {
      case RSPF_UINT8:
      {
         rspf_uint8* buf = static_cast<rspf_uint8*>(getBuf(band))+offset;
         *buf       = (rspf_uint8)getNullPix(band);
         break;
      }
      case RSPF_SINT8:
      {
         rspf_sint8* buf = static_cast<rspf_sint8*>(getBuf(band))+offset;
         *buf       = (rspf_sint8)getNullPix(band);
         break;
      }
      case RSPF_UINT16:
      case RSPF_USHORT11:
      {
         rspf_uint16* buf = static_cast<rspf_uint16*>(getBuf(band))+offset;
         *buf = (rspf_uint16)getNullPix(band);
         break;
      }
      case RSPF_SINT16:
      {
         rspf_sint16* buf = static_cast<rspf_sint16*>(getBuf(band))+offset;
         *buf = (rspf_sint16)getNullPix(band);
         break;
      }
      case RSPF_UINT32:
      {
         rspf_uint32* buf = static_cast<rspf_uint32*>(getBuf(band))+offset;
         *buf       = (rspf_uint32)getNullPix(band);
         break;
      }
      case RSPF_SINT32:
      {
         rspf_sint32* buf = static_cast<rspf_sint32*>(getBuf(band))+offset;
         *buf       = (rspf_sint32)getNullPix(band);
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         rspf_float32* buf = static_cast<rspf_float32*>(getBuf(band))+offset;
         *buf = (rspf_float32)getNullPix(band);
         break;
      }
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
      {
         rspf_float64* buf = static_cast<rspf_float64*>(getBuf(band))+offset;
         *buf = getNullPix(band);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         //ERROR
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::isNull Unsupported scalar type!"
            << std::endl;
      }
   }
}

void rspfImageData::setNull(const rspfIpt& pt)
{
   rspf_int32 xNew = (pt.x - m_origin.x);
   rspf_int32 yNew = (pt.y - m_origin.y);
   
   if(xNew < 0 || xNew >= (int)m_spatialExtents[0] ||
      yNew < 0 || yNew >= (int)m_spatialExtents[1])
   {
      return;
   }
   rspf_uint32 offset = ((int)getWidth())*yNew + xNew;
   
   setNull(offset);
}

void rspfImageData::setNull(const rspfIpt& pt, rspf_uint32 band)
{
   rspf_int32 xNew = (pt.x - m_origin.x);
   rspf_int32 yNew = (pt.y - m_origin.y);
   
   if(xNew < 0 || xNew >= (int)m_spatialExtents[0] ||
      yNew < 0 || yNew >= (int)m_spatialExtents[1])
   {
      return;
   }
   rspf_uint32 offset = ((int)getWidth())*yNew + xNew;
   
   setNull(offset, band);
}

void rspfImageData::setNullPix(rspf_float64 null_pix)
{
   if(!m_numberOfDataComponents)
   {
      return;
   }
   m_nullPixelValue.resize(m_numberOfDataComponents);
   for(rspf_uint32 band = 0; band < m_numberOfDataComponents; ++band)
   {
      m_nullPixelValue[band] = null_pix;
   }
}

void rspfImageData::setNullPix(rspf_float64 null_pix, rspf_uint32 band)
{
   if( !m_numberOfDataComponents || (band >= m_numberOfDataComponents) )
   {
      return;
   }
   if (m_nullPixelValue.size() != m_numberOfDataComponents)
   {
      initializeNullDefault();
   }
   m_nullPixelValue[band] = null_pix;
}

void rspfImageData::setNullPix(const rspf_float64* nullPixArray,
                                rspf_uint32 numberOfValues)
{
   if(!nullPixArray || !m_numberOfDataComponents)
   {
      return;
   }

   if (m_nullPixelValue.size() != m_numberOfDataComponents)
   {
      initializeNullDefault();
   }
   
   for(rspf_uint32 band = 0;
       (band < numberOfValues) && (band < m_numberOfDataComponents);
       ++band)
   {
      m_nullPixelValue[band] = nullPixArray[band];
   }
}

void rspfImageData::setMinPix(rspf_float64 min_pix)
{
   if(!m_numberOfDataComponents)
   {
      return;
   }
   m_minPixelValue.resize(m_numberOfDataComponents);
   for(rspf_uint32 band = 0; band < m_minPixelValue.size(); ++band)
   {
      m_minPixelValue[band] = min_pix;
   }
}

void rspfImageData::setMinPix(rspf_float64 min_pix, rspf_uint32 band)
{
   if( !m_numberOfDataComponents || (band >= m_numberOfDataComponents) )
   {
      return;
   }
   if (m_minPixelValue.size() != m_numberOfDataComponents)
   {
      initializeMinDefault();
   }
   m_minPixelValue[band] = min_pix;
}

void rspfImageData::setMinPix(const rspf_float64* minPixArray,
                               rspf_uint32 numberOfValues)
{
   if( !minPixArray || !m_numberOfDataComponents )
   {
      return;
   }

   if (m_minPixelValue.size() != m_numberOfDataComponents)
   {
      initializeMinDefault();
   }
   
   for(rspf_uint32 band = 0;
       (band < numberOfValues) && (band < m_numberOfDataComponents);
       ++band)
   {
      m_minPixelValue[band] = minPixArray[band];
   }
}

void rspfImageData::setMaxPix(rspf_float64 max_pix)
{
   if(!m_numberOfDataComponents)
   {
      return;
   }
   m_maxPixelValue.resize(m_numberOfDataComponents);
   for(rspf_uint32 band = 0; band < m_numberOfDataComponents; ++band)
   {
      m_maxPixelValue[band] = max_pix;
   }
}

void rspfImageData::setMaxPix(rspf_float64 max_pix, rspf_uint32 band)
{
   if( !m_numberOfDataComponents || (band >= m_numberOfDataComponents) )
   {
      return;
   }
   if (m_maxPixelValue.size() != m_numberOfDataComponents)
   {
      initializeMaxDefault();
   }
   m_maxPixelValue[band] = max_pix;
}

void rspfImageData::setMaxPix(const rspf_float64* maxPixArray,
                               rspf_uint32 /* numberOfValues */)
{
   if(!maxPixArray || !m_numberOfDataComponents )
   {
      return;
   }

   if (m_maxPixelValue.size() != m_numberOfDataComponents)
   {
      initializeMaxDefault();
   }
   
   for(rspf_uint32 band = 0;
       (band < m_numberOfDataComponents) &&
          (band < m_numberOfDataComponents);
       ++band)
   {
      m_maxPixelValue[band] = maxPixArray[band];
   }
}

void rspfImageData::setNumberOfBands(rspf_uint32 bands,
                                      bool reallocate)
{
   rspf_uint32 b  = getNumberOfBands();
   if(bands && (b != bands))
   {
      setNumberOfDataComponents(bands);
      if(reallocate)
      {
         rspfRectilinearDataObject::initialize();
      }
      
      rspf_uint32 minBands = rspf::min(b, bands);


      vector<rspf_float64> newNull(bands);
      vector<rspf_float64> newMin(bands);
      vector<rspf_float64> newMax(bands);

      rspf_uint32 i = 0;
      while (i < minBands)
      {
         newNull[i] = m_nullPixelValue[i];
         newMin[i]  = m_minPixelValue[i];
         newMax[i]  = m_maxPixelValue[i];
         ++i;
      }

      if(b)
      {
        while (i < bands)
        {
           newNull[i] = m_nullPixelValue[b-1];
           newMin[i]  = m_minPixelValue[b-1];
           newMax[i]  = m_maxPixelValue[b-1];
           ++i;
        }
      }
      
      m_nullPixelValue = newNull;
      m_minPixelValue  = newMin;
      m_maxPixelValue  = newMax;
   }
}

void rspfImageData::setImageRectangleAndBands(const rspfIrect& rect,
                                               rspf_uint32 numberOfBands)
{
   if(rect.hasNans())
   {
      return;
   }
   
   rspf_uint32 bands   = getNumberOfBands();
   rspf_uint32 w       = getWidth();
   rspf_uint32 h       = getHeight();
   rspf_uint32 nw      = rect.width();
   rspf_uint32 nh      = rect.height();

   setOrigin(rect.ul());
   setWidthHeight(nw, nh);
   setNumberOfBands(numberOfBands, false);
   
   // we will try to be non destructive on the resize of the number of bands
   if( ( (w*h != nw*nh)  || ( bands != numberOfBands) ) &&
       (theDataObjectStatus != RSPF_NULL) )
   {
      initialize();
   }
}

void rspfImageData::setImageRectangle(const rspfIrect& rect)
{
   if(rect.hasNans())
   {
      return;
   }
   
   rspf_uint32 w  = getWidth();
   rspf_uint32 h  = getHeight();
   rspf_uint32 nw = rect.width();
   rspf_uint32 nh = rect.height();

   setOrigin(rect.ul());
   setWidthHeight(nw, nh);

   // we will try to be non destructive on the resize of the number of bands
   if ( (w*h != nw*nh) && (theDataObjectStatus != RSPF_NULL) )
   {
      initialize();
   }
}

void rspfImageData::assign(const rspfDataObject* data)
{
   if(!data) return;
   rspfImageData* d = PTR_CAST(rspfImageData, data);
   if(d)
   {
      assign(d);
   }
}

void rspfImageData::assign(const rspfImageData* data)
{
   rspfSource* tmp_owner = getOwner();
   
   rspfRectilinearDataObject::assign(data);

   //***
   // The data member "theSource" will be overwritten so capture it and then
   // set it back.
   //***
   setOwner(tmp_owner);

   if(this != data)
   {
      rspf_uint32 numberOfBands = getNumberOfBands();
      if(!numberOfBands)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::assign\n"
            << "Number of components is 0, can't assign" << std::endl;
         return;
      }
      m_origin = data->m_origin;

      m_minPixelValue  = data->m_minPixelValue;
      m_maxPixelValue  = data->m_maxPixelValue;
      m_nullPixelValue = data->m_nullPixelValue;

      if (m_minPixelValue.size() == 0 ||
          m_maxPixelValue.size() == 0 ||
          m_nullPixelValue.size() == 0)
      {
         initializeDefaults();
      }
   }
}

void rspfImageData::assignBand(const rspfImageData* data,
                                rspf_uint32 source_band,
                                rspf_uint32 output_band)
{
   //***
   // This method requires this image data to be initialized to the same
   // size as the source data being copied.
   //***

   // Some basic error checking...
   if ( !data->isValidBand(source_band) )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::assignBand ERROR:"
         << "\nInvalid source band!" << std::endl;
      return;
   }
   if ( !isValidBand(output_band) )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::assignBand ERROR:"
         << "\nInvalid output band!" << std::endl;
      return;
   }
   if (data->getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::assignBand ERROR:"
         << "\nSource data status is null!" << std::endl;
      return;
   }
   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::assignBand ERROR:"
         << "\nThis status is null!" << std::endl;
      return;
   }
   
   //***
   // Make sure this buffer is the same size; if not, return.
   //***
   rspf_uint32 source_size = data->getSizePerBandInBytes();
      
   if ( source_size != getSizePerBandInBytes() )
   {
      return;
   }

   // Get the pointers to the bands.
   const void*  s = data->getBuf(source_band);
   void*        d = getBuf(output_band);

   // One last check.
   if (s == 0 || d == 0)
   {
      return;
   }

   // Ok copy the image data...
   memcpy(d, s, source_size);
}

void rspfImageData::loadBand(const void* src,
                              const rspfIrect& src_rect,
                              rspf_uint32 band)
{
   // Call the appropriate load method.
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadBandTemplate(rspf_uint8(0), src, src_rect, band);
         return;

      case RSPF_SINT8:
         loadBandTemplate(rspf_sint8(0), src, src_rect, band);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadBandTemplate(rspf_uint16(0), src, src_rect, band);
         return;
         
      case RSPF_SINT16:
         loadBandTemplate(rspf_sint16(0), src, src_rect, band);
         return;
         
      case RSPF_UINT32:
         loadBandTemplate(rspf_uint32(0), src, src_rect, band);
         return;

      case RSPF_SINT32:
         loadBandTemplate(rspf_sint32(0), src, src_rect, band);
         return;
         
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
         loadBandTemplate(rspf_float32(0), src, src_rect, band);
         return;
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadBandTemplate(rspf_float64(0), src, src_rect, band);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadBand\n\
File %s line %d\nUnknown scalar type!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::loadBand(const void* src,
                              const rspfIrect& src_rect,
                              const rspfIrect& clip_rect,
                              rspf_uint32 band)
{
   // Call the appropriate load method.
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadBandTemplate(rspf_uint8(0), src, src_rect, clip_rect, band);
         return;

      case RSPF_SINT8:
         loadBandTemplate(rspf_sint8(0), src, src_rect, clip_rect, band);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadBandTemplate(rspf_uint16(0), src, src_rect, clip_rect, band);
         return;
         
      case RSPF_SINT16:
         loadBandTemplate(rspf_sint16(0), src, src_rect, clip_rect, band);
         return;
         
      case RSPF_UINT32:
         loadBandTemplate(rspf_uint32(0), src, src_rect, clip_rect, band);
         return;

      case RSPF_SINT32:
         loadBandTemplate(rspf_sint32(0), src, src_rect, clip_rect, band);
         return;
         
      case RSPF_FLOAT32:
      case RSPF_NORMALIZED_FLOAT:
         loadBandTemplate(rspf_float32(0), src, src_rect, clip_rect, band);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadBandTemplate(rspf_float64(0), src, src_rect, clip_rect, band);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadBand\n\
File %s line %d\nUnknown scalar type!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

template <class T> void rspfImageData::loadBandTemplate(T, // dummy template variable
                                                         const void* src,
                                                         const rspfIrect& src_rect,
                                                         rspf_uint32 band)
{
   const rspfIrect img_rect = getImageRectangle();

   // Get the clip rect.
   const rspfIrect clip_rect = img_rect.clipToRect(src_rect);

   
   loadBandTemplate(T(0), src, src_rect, clip_rect, band);
}

template <class T> void rspfImageData::loadBandTemplate(T, // dummy template variable
                                                         const void* src,
                                                         const rspfIrect& src_rect,
                                                         const rspfIrect& clip_rect,
                                                         rspf_uint32 band)
{
   static const char  MODULE[] = "rspfImageData::loadBand";
   
   // Check the pointer.
   if (!src)
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "%s File %s line %d\nNULL pointer passed to method!",
                    MODULE,
                    __FILE__,
                    __LINE__);
      return;
   }
   
   // Check the band.
   if (!isValidBand(band))
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "%s File %s line %d\nInvalid band:  %d",
                    MODULE,
                    __FILE__,
                    __LINE__,
                    band);
      return;
   }
   
   const rspfIrect img_rect = getImageRectangle();
   
   // Check for intersect.
   if ( ! img_rect.intersects(src_rect) )
   {
      return; // Nothing to do here.
   }

   // Check the clip rect.
   if (!clip_rect.completely_within(img_rect))
   {
      return;
   }
   
   // Check the status and allocate memory if needed.
   if (getDataObjectStatus() == RSPF_NULL) initialize();

   // Get the width of the buffers.
   rspf_uint32 s_width = src_rect.width();
   rspf_uint32 d_width = getWidth();
   
   const T* s = static_cast<const T*>(src);
   T* d = static_cast<T*>(getBuf(band));

   // Move the pointers to the first valid pixel.
   s += (clip_rect.ul().y - src_rect.ul().y) * s_width +
        clip_rect.ul().x - src_rect.ul().x;
   
   d += (clip_rect.ul().y - img_rect.ul().y) * d_width +
        clip_rect.ul().x - img_rect.ul().x;
   
   // Copy the data.
   rspf_uint32 clipHeight = clip_rect.height();
   rspf_uint32 clipWidth  = clip_rect.width();
  
   for (rspf_uint32 line = 0; line < clipHeight; ++line)
   {
      for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
      {
         d[sample] = s[sample];
      }

      s += s_width;
      d += d_width;
   }
}

void rspfImageData::loadTile(const rspfImageData* src)
{
   if (!src)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::loadTile ERROR:"
         << "Null tile passed to method!" << std::endl;
      return;
   }

   if (!src->getBuf())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::loadTile ERROR:"
         << "Source tile buff is null!" << std::endl;
      return;
   }

   if (!this->getBuf())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::loadTile ERROR:"
         << "This tile not initialized!" << std::endl;
      return;
   }

   if (src->getNumberOfBands() != this->getNumberOfBands())
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::loadTile ERROR:"
         << "Tiles do not have same number of bands!" << std::endl;
      return;
   }

   //***
   // Set the status of this tile to the status of the source tile.
   // Do this in place of validate.
   //***
   setDataObjectStatus(src->getDataObjectStatus());
   
   if(getScalarType() == src->getScalarType())
   {      
      loadTile((void*)(src->getBuf()),
               src->getImageRectangle(),
               RSPF_BSQ);
      setNullPix(src->getNullPix(), src->getNumberOfBands());
   }
   else // do a slow generic normalize to unnormalize copy
   {
      // Check the pointer.
      rspfIrect src_rect = src->getImageRectangle();
      const rspfIrect img_rect = getImageRectangle();
      
      // Check for intersect.
      if ( !img_rect.intersects(src_rect) )
      {
         return; // Nothing to do here.
      }

      // Get the clip rect.
      const rspfIrect clip_rect = img_rect.clipToRect(src_rect);

      // Check the status and allocate memory if needed.
      if (getDataObjectStatus() == RSPF_NULL) initialize();
      
      // Get the width of the buffers.
      rspf_uint32 num_bands     = getNumberOfBands();
      rspf_uint32 s_width       = src_rect.width();
      rspf_uint32 d_width       = getWidth();
            
      rspf_uint32 band;
      
      rspf_uint32 sourceOffset = (clip_rect.ul().y - src_rect.ul().y) *
         s_width + (clip_rect.ul().x - src_rect.ul().x);      
      
      rspf_uint32 destinationOffset = (clip_rect.ul().y - img_rect.ul().y) *
         d_width + (clip_rect.ul().x - img_rect.ul().x);
      
      rspf_uint32 clipHeight = clip_rect.height();
      rspf_uint32 clipWidth  = clip_rect.width();

      // Copy the data.
      for (band=0; band<num_bands; ++band)
      {
         rspf_uint32 sourceIndex      = sourceOffset;
         rspf_uint32 destinationIndex = destinationOffset;
         rspf_float32 tempResult      = 0.0;

         for (rspf_uint32 line = 0; line < clipHeight; ++line)
         {
            for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
            {
               src->getNormalizedFloat(sourceIndex + sample,
                                       band,
                                       tempResult);
               
               this->setNormalizedFloat(destinationIndex + sample,
                                        band,
                                        tempResult);
            }
            sourceIndex      += s_width;
            destinationIndex += d_width;
         }
      }
   }
}

void rspfImageData::loadTile(const void* src,
                              const rspfIrect& src_rect,
                              rspfInterleaveType il_type)
{
   switch (il_type)
   {
   case RSPF_BIP:
      loadTileFromBip(src, src_rect);
      return;
   case RSPF_BIL:
      loadTileFromBil(src, src_rect);
      return;
   case RSPF_BSQ:
      loadTileFromBsq(src, src_rect);
      return;
   default:
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::loadTile ERROR:  unsupported interleave type!"
         << std::endl;
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfImageData::loadTile\n\
File %s line %d\nUnknown interleave type!",
                    __FILE__,
                    __LINE__);      
      return;
   } // End of "switch (type)"
}

void rspfImageData::loadTile(const void* src,
                              const rspfIrect& src_rect,
                              const rspfIrect& clip_rect,
                              rspfInterleaveType il_type)
{
   switch (il_type)
   {
   case RSPF_BIP:
      loadTileFromBip(src, src_rect, clip_rect);
      return;
   case RSPF_BIL:
      loadTileFromBil(src, src_rect, clip_rect);
      return;
   case RSPF_BSQ:
      loadTileFromBsq(src, src_rect, clip_rect);
      return;
   default:
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::loadTile ERROR:  unsupported interleave type!"
         << std::endl;
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfImageData::loadTile\n\
File %s line %d\nUnknown interleave type!",
                    __FILE__,
                    __LINE__);      
      return;
   } // End of "switch (type)"
}


void rspfImageData::nullTileAlpha(const rspf_uint8* src,
                                   const rspfIrect& src_rect,
                                   bool multiplyAlphaFlag)
{
   const rspfIrect img_rect = getImageRectangle();
   
   // Get the clip rect.
   const rspfIrect clip_rect = img_rect.clipToRect(src_rect);

   nullTileAlpha(src, src_rect, clip_rect, multiplyAlphaFlag);
}

void rspfImageData::nullTileAlpha(const rspf_uint8* src,
                                   const rspfIrect& src_rect,
                                   const rspfIrect& clip_rect,
                                   bool multiplyAlphaFlag)
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         nullTileAlphaTemplate(rspf_uint8(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;

      case RSPF_SINT8:
         nullTileAlphaTemplate(rspf_sint8(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         nullTileAlphaTemplate(rspf_uint16(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;
         
      case RSPF_SINT16:
         nullTileAlphaTemplate(rspf_sint16(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;
         
      case RSPF_UINT32:
         nullTileAlphaTemplate(rspf_uint32(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;

      case RSPF_SINT32:
         nullTileAlphaTemplate(rspf_sint32(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;
         
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         nullTileAlphaTemplate(rspf_float32(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         nullTileAlphaTemplate(rspf_float64(0), src, src_rect, clip_rect, multiplyAlphaFlag);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::nullTileAlpha\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
   
}

void rspfImageData::loadTileFromBip(const void* src,
                                     const rspfIrect& src_rect)
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadTileFromBipTemplate(rspf_uint8(0), src, src_rect);
         return;

      case RSPF_SINT8:
         loadTileFromBipTemplate(rspf_sint8(0), src, src_rect);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadTileFromBipTemplate(rspf_uint16(0), src, src_rect);
         return;
         
      case RSPF_SINT16:
         loadTileFromBipTemplate(rspf_sint16(0), src, src_rect);
         return;
         
      case RSPF_UINT32:
         loadTileFromBipTemplate(rspf_uint32(0), src, src_rect);
         return;

      case RSPF_SINT32:
         loadTileFromBipTemplate(rspf_sint32(0), src, src_rect);
         return;
         
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         loadTileFromBipTemplate(rspf_float32(0), src, src_rect);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadTileFromBipTemplate(rspf_float64(0), src, src_rect);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadTileFromBip\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::loadTileFromBip(const void* src,
                                     const rspfIrect& src_rect,
                                     const rspfIrect& clip_rect)
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadTileFromBipTemplate(rspf_uint8(0), src, src_rect, clip_rect);
         return;

      case RSPF_SINT8:
         loadTileFromBipTemplate(rspf_sint8(0), src, src_rect, clip_rect);
         return;

      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadTileFromBipTemplate(rspf_uint16(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_SINT16:
         loadTileFromBipTemplate(rspf_sint16(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_UINT32:
         loadTileFromBipTemplate(rspf_uint32(0), src, src_rect, clip_rect);
         return;

      case RSPF_SINT32:
         loadTileFromBipTemplate(rspf_sint32(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         loadTileFromBipTemplate(rspf_float32(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadTileFromBipTemplate(rspf_float64(0), src, src_rect, clip_rect);
         return;

      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadTileFromBip\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::loadTileFromBil(const void* src,
                                     const rspfIrect& src_rect)
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadTileFromBilTemplate(rspf_uint8(0), src, src_rect);
         return;

      case RSPF_SINT8:
         loadTileFromBilTemplate(rspf_sint8(0), src, src_rect);
         return;

      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadTileFromBilTemplate(rspf_uint16(0), src, src_rect);
         return;
      
      case RSPF_SINT16:
         loadTileFromBilTemplate(rspf_sint16(0), src, src_rect);
         return;
      
      case RSPF_UINT32:
         loadTileFromBilTemplate(rspf_uint32(0), src, src_rect);
         return;

      case RSPF_SINT32:
         loadTileFromBilTemplate(rspf_sint32(0), src, src_rect);
         return;
      
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         loadTileFromBilTemplate(rspf_float32(0), src, src_rect);
         return;
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadTileFromBilTemplate(rspf_float64(0), src, src_rect);
         return;

      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadTileFromBil\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::loadTileFromBil(const void* src,
                                     const rspfIrect& src_rect,
                                     const rspfIrect& clip_rect)
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadTileFromBilTemplate(rspf_uint8(0), src, src_rect, clip_rect);
         return;

      case RSPF_SINT8:
         loadTileFromBilTemplate(rspf_sint8(0), src, src_rect, clip_rect);
         return;

      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadTileFromBilTemplate(rspf_uint16(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_SINT16:
         loadTileFromBilTemplate(rspf_sint16(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_UINT32:
         loadTileFromBilTemplate(rspf_uint32(0), src, src_rect, clip_rect);
         return;

      case RSPF_SINT32:
         loadTileFromBilTemplate(rspf_sint32(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         loadTileFromBilTemplate(rspf_float32(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadTileFromBilTemplate(rspf_float64(0), src, src_rect, clip_rect);
         return;

      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadTileFromBil\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::loadTileFromBsq(const void* src,
                                     const rspfIrect& src_rect)
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadTileFromBsqTemplate(rspf_uint8(0), src, src_rect);
         return;

      case RSPF_SINT8:
         loadTileFromBsqTemplate(rspf_sint8(0), src, src_rect);
         return;

      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadTileFromBsqTemplate(rspf_uint16(0), src, src_rect);
         return;
      
      case RSPF_SINT16:
         loadTileFromBsqTemplate(rspf_sint16(0), src, src_rect);
         return;
      
      case RSPF_UINT32:
         loadTileFromBsqTemplate(rspf_uint32(0), src, src_rect);
         return;

      case RSPF_SINT32:
         loadTileFromBsqTemplate(rspf_sint32(0), src, src_rect);
         return;
      
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         loadTileFromBsqTemplate(rspf_float32(0), src, src_rect);
         return;
      
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadTileFromBsqTemplate(rspf_float64(0), src, src_rect);
         return;
      
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadTileFromBsq\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::loadTileFromBsq(const void* src,
                                     const rspfIrect& src_rect,
                                     const rspfIrect& clip_rect)
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         loadTileFromBsqTemplate(rspf_uint8(0), src, src_rect, clip_rect);
         return;

      case RSPF_SINT8:
         loadTileFromBsqTemplate(rspf_sint8(0), src, src_rect, clip_rect);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         loadTileFromBsqTemplate(rspf_uint16(0), src, src_rect, clip_rect);
         return;
         
      case RSPF_SINT16:
         loadTileFromBsqTemplate(rspf_sint16(0), src, src_rect, clip_rect);
         return;
         
      case RSPF_UINT32:
         loadTileFromBsqTemplate(rspf_uint32(0), src, src_rect, clip_rect);
         return;

      case RSPF_SINT32:
         loadTileFromBsqTemplate(rspf_sint32(0), src, src_rect, clip_rect);
         return;
      
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         loadTileFromBsqTemplate(rspf_float32(0), src, src_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         loadTileFromBsqTemplate(rspf_float64(0), src, src_rect, clip_rect);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::loadTileFromBsq\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::computeMinMaxPix(vector<rspf_float64>& minBands,
                                      vector<rspf_float64>& maxBands) const
{
   if ( (getDataObjectStatus() == RSPF_NULL) || 
        (getDataObjectStatus() == RSPF_EMPTY) )
   {
      return;
   }

   switch(m_scalarType)
   {
      case RSPF_UINT8:
      {
         computeMinMaxPix((rspf_uint8)0,
                          minBands,
                          maxBands);
         break;
      }
      case RSPF_SINT8:
      {
         computeMinMaxPix((rspf_sint8)0,
                          minBands,
                          maxBands);
         break;
         
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         computeMinMaxPix((rspf_uint16)0,
                          minBands,
                          maxBands);
         break;
      }
      case RSPF_SINT16:
      {
         computeMinMaxPix((rspf_sint16)0,
                          minBands,
                          maxBands);
         break;
      }
      case RSPF_UINT32:
      {
         computeMinMaxPix((rspf_uint32)0,
                          minBands,
                          maxBands);
         break;
      }
      case RSPF_SINT32:
      {
         computeMinMaxPix((rspf_sint32)0,
                          minBands,
                          maxBands);
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         computeMinMaxPix((rspf_float32)0.0,
                          minBands,
                          maxBands);
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         computeMinMaxPix((rspf_float64)0.0,
                          minBands,
                          maxBands);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         // Shouldn't hit this.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::computeMinPix Unsupported scalar type!"
            << std::endl;
         break;
      }
   }
}

template <class T>
void rspfImageData::computeMinMaxPix(T /* dummyValue */,
                                      vector<rspf_float64>& minBands,
                                      vector<rspf_float64>& maxBands) const
{
   const rspf_uint32 BANDS = getNumberOfBands();
   const rspf_uint32 SPB   = getSizePerBand(); 
   
   if( minBands.size() != BANDS ||
       maxBands.size() != BANDS )
   {
      minBands.resize(BANDS);
      maxBands.resize(BANDS);
      for (rspf_uint32 band = 0; band < BANDS; ++band)
      {
         // Set min to max and max to min for starters.
         minBands[band] = getMaxPix(band);
         maxBands[band] = getMinPix(band);
      }
   }
   
   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const T* bandBuffer = (const T*)getBuf(band);
      if(bandBuffer)
      {
         const T NP   = static_cast<T>(getNullPix(band));
         rspf_float64 currentMin = minBands[band];
         rspf_float64 currentMax = maxBands[band];
         for(rspf_uint32 offset = 0; offset < SPB; ++offset)
         {
            T p = bandBuffer[offset];
            if(p != NP)
            {
               if(p < currentMin)
               {
                  currentMin = p;
               }
               else if(p > currentMax)
               {
                  currentMax = p;
               }
            }
         }
         minBands[band] = currentMin;
         maxBands[band] = currentMax;
      }
   }
}

void rspfImageData::computeMinMaxNulPix(vector<rspf_float64>& minBands,
                                         vector<rspf_float64>& maxBands,
                                         vector<rspf_float64>& nulBands) const
{
   if ( getDataObjectStatus() == RSPF_NULL )
   {
      return;
   }
   
   switch(m_scalarType)
   {
      case RSPF_UINT8:
      {
         computeMinMaxNulPix((rspf_uint8)0,
                             minBands,
                             maxBands,
                             nulBands);
         break;
      }
      case RSPF_SINT8:
      {
         computeMinMaxNulPix((rspf_sint8)0,
                             minBands,
                             maxBands,
                             nulBands);
         break;  
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         computeMinMaxNulPix((rspf_uint16)0,
                             minBands,
                             maxBands,
                             nulBands);
         break;
      }
      case RSPF_SINT16:
      {
         computeMinMaxNulPix((rspf_sint16)0,
                             minBands,
                             maxBands,
                             nulBands);
         break;
      }
      case RSPF_UINT32:
      {
         computeMinMaxNulPix((rspf_uint32)0,
                             minBands,
                             maxBands,
                             nulBands);
         break;
      }
      case RSPF_SINT32:
      {
         computeMinMaxNulPix((rspf_sint32)0,
                             minBands,
                             maxBands,
                             nulBands);
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         computeMinMaxNulPix((rspf_float32)0.0,
                             minBands,
                             maxBands,
                             nulBands);
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         computeMinMaxNulPix((rspf_float64)0.0,
                             minBands,
                             maxBands,
                             nulBands);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         // Shouldn't hit this.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::computeMinNulPix Unsupported scalar type!"
            << std::endl;
         break;
      }
   }
}

template <class T>
void rspfImageData::computeMinMaxNulPix(T /* dummyValue */,
                                         vector<rspf_float64>& minBands,
                                         vector<rspf_float64>& maxBands,
                                         vector<rspf_float64>& nulBands) const
{
   const rspf_uint32 BANDS = getNumberOfBands();
   const rspf_uint32 SPB   = getSizePerBand(); 
   T DEFAULT_NULL = static_cast<T>(rspf::defaultNull(getScalarType()));   

   if( minBands.size() != BANDS ||
       maxBands.size() != BANDS ||
       nulBands.size() != BANDS )
   {
      // First time through.
      minBands.resize(BANDS);
      maxBands.resize(BANDS);
      nulBands.resize(BANDS);
      for (rspf_uint32 band = 0; band < BANDS; ++band)
      {
         // Set min to max and max to min for starters.
         minBands[band] = getMaxPix(band);
         maxBands[band] = getMinPix(band);
         nulBands[band] = getMaxPix(band);
      }
   }
   
   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const T* bandBuffer = (const T*)getBuf(band);
      if(bandBuffer)
      {
         rspf_float64 currentMin = minBands[band];
         rspf_float64 currentMax = maxBands[band];
         rspf_float64 currentNul = nulBands[band];
         for(rspf_uint32 offset = 0; offset < SPB; ++offset)
         {
            T p = bandBuffer[offset];

            //---
            // Since we are scanning for nulls this is making an assumption that the default
            // null is incorrect and should be ignored in this scan as it could have been
            // introduced by a make blank on a partial tile so ignore it.
            //---
            if ( p != DEFAULT_NULL )
            {
               // Must do null first as min depends on null.
               if ( p < currentNul )
               {
                  currentNul = p;
               }
               if( ( p < currentMin ) && ( p > currentNul ) )
               {
                  currentMin = p;
               }
               else if( p > currentMax )
               {
                  currentMax = p;
               }
            }
         }
         minBands[band] = currentMin;
         maxBands[band] = currentMax;
         nulBands[band] = currentNul;
      }
   }
}

template <class T>
void rspfImageData::loadTileFromBipTemplate(T, // dummy template variable
                                             const void* src,
                                             const rspfIrect& src_rect)
{
   const rspfIrect img_rect = getImageRectangle();
   
   // Get the clip rect.
   const rspfIrect clip_rect = img_rect.clipToRect(src_rect);

   loadTileFromBipTemplate(T(0), src, src_rect, clip_rect);
}

template <class T>
void rspfImageData::loadTileFromBipTemplate(T, // dummy template variable
                                             const void* src,
                                             const rspfIrect& src_rect,
                                             const rspfIrect& clip_rect)
{
   static const char  MODULE[] = "rspfImageData::loadTileFromBip";
   
   // Check the pointer.
   if (!src)
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                     "%s File %s line %d\nNULL pointer passed to method!",
                    MODULE,
                    __FILE__,
                    __LINE__);
      return;
   }
   
   const rspfIrect img_rect = getImageRectangle();
   
   // Check for intersect.
   if ( ! img_rect.intersects(src_rect) )
   {
      return; // Nothing to do here.
   }
   
   // Check the clip rect.
   if (!clip_rect.completely_within(img_rect))
   {
      return;
   }
   
   // Check the status and allocate memory if needed.
   if (getDataObjectStatus() == RSPF_NULL) initialize();

   // Get the width of the buffers.
   rspf_uint32 num_bands  = getNumberOfBands();
   rspf_uint32 s_width    = (src_rect.width()) * num_bands;
   rspf_uint32 d_width    = getWidth();
   rspf_uint32 band       = 0;
   const T* s        = static_cast<const T*>(src);
   T** d             = new T*[num_bands];
   
   // Make destination pointers to each one.
   for (band=0; band<num_bands; band++)
   {
      d[band] = static_cast<T*>(getBuf(band));
      
      // Move the pointers to the first valid pixel.
      d[band] += (clip_rect.ul().y - img_rect.ul().y) * d_width +
                 clip_rect.ul().x - img_rect.ul().x;
   }
   
   // Move the source pointer to the first valid pixel.
   s += (clip_rect.ul().y - src_rect.ul().y) * s_width +
        (clip_rect.ul().x - src_rect.ul().x) * num_bands;
   
   // Copy the data.
   rspf_uint32 clipHeight = clip_rect.height();
   rspf_uint32 clipWidth  = clip_rect.width();
   
   for (rspf_uint32 line = 0; line < clipHeight; ++line)
   {
      rspf_uint32 j = 0;
      for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
      {
         for (band=0; band<num_bands; band++)
         {
            d[band][sample] = s[j+band];
         }
         j += num_bands;
      }
      
      s += s_width;
      for (band=0; band<num_bands; band++)
      {
         d[band] += d_width;
      }
   }
   
   delete [] d;
}
   
template <class T>
void rspfImageData::loadTileFromBilTemplate(T, // dummy template variable
                                             const void* src,
                                             const rspfIrect& src_rect)
{
   const rspfIrect img_rect = getImageRectangle();
   
   // Get the clip rect.
   const rspfIrect clip_rect = img_rect.clipToRect(src_rect);

   loadTileFromBilTemplate(T(0), src, src_rect, clip_rect);
}
   
template <class T>
void rspfImageData::loadTileFromBilTemplate(T, // dummy template variable
                                             const void* src,
                                             const rspfIrect& src_rect,
                                             const rspfIrect& clip_rect)
{
   static const char  MODULE[] = "rspfImageData::loadTileFromBil";
   
   // Check the pointer.
   if (!src)
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "%s File %s line %d\nNULL pointer passed to method!",
                    MODULE,
                    __FILE__,
                    __LINE__);
      return;
   }
   
   const rspfIrect img_rect = getImageRectangle();
   
   // Check for intersect.
   if ( ! img_rect.intersects(src_rect) )
   {
      return; // Nothing to do here.
   }
   
   // Check the clip rect.
   if (!clip_rect.completely_within(img_rect))
   {
      return;
   }

   // Check the status and allocate memory if needed.
   if (getDataObjectStatus() == RSPF_NULL) initialize();
   
   // Get the width of the buffers.
   rspf_uint32 num_bands = getNumberOfBands();
   rspf_uint32 s_width = (src_rect.lr().x - src_rect.ul().x + 1);
   rspf_uint32 d_width = getWidth();
   rspf_uint32 band = 0;
   rspf_uint32 d_offset = (clip_rect.ul().y - img_rect.ul().y) * d_width +
                     clip_rect.ul().x - img_rect.ul().x;
   
   const T* s = static_cast<const T*>(src);

   // Get the number of bands and make destination pointers to each one.
   T** d = new T*[num_bands];
   
   for (band=0; band<num_bands; band++)
   {
      d[band] = static_cast<T*>(getBuf(band));
      
      // Move the pointers to the first valid pixel.
      d[band] += d_offset;
   }
   
   // Move the source pointer to the first valid pixel.
   s += (clip_rect.ul().y - src_rect.ul().y) * s_width * num_bands +
        clip_rect.ul().x - src_rect.ul().x;
   
   // Copy the data.
   rspf_uint32 clipHeight = clip_rect.height();
   rspf_uint32 clipWidth  = clip_rect.width();

   for (rspf_uint32 line = 0; line < clipHeight; ++line)
   {
      for (band = 0; band < num_bands; ++band)
      {
         for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
         {
            d[band][sample] = s[sample];
         }
         s       += s_width;
         d[band] += d_width;
      }
   }

   delete [] d;
}
   
template <class T>
void rspfImageData::loadTileFromBsqTemplate(T, // dummy template variable
                                             const void* src,
                                             const rspfIrect& src_rect)
{
   const rspfIrect img_rect = getImageRectangle();
   
   // Get the clip rect.
   const rspfIrect clip_rect = img_rect.clipToRect(src_rect);

   loadTileFromBsqTemplate(T(0), src, src_rect, clip_rect);
}

template <class T>
void rspfImageData::loadTileFromBsqTemplate(T, // dummy template variable
                                             const void* src,
                                             const rspfIrect& src_rect,
                                             const rspfIrect& clip_rect)
{
   static const char  MODULE[] = "rspfImageData::loadTileFromBsq";
   
   // Check the pointer.
   if (!src)
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "%s File %s line %d\nNULL pointer passed to method!",
                    MODULE,
                    __FILE__,
                    __LINE__);
      return;
   }
   
   const rspfIrect img_rect = getImageRectangle();
   
   // Check for intersect.
   if ( ! img_rect.intersects(src_rect) )
   {
      return; // Nothing to do here.
   }

   // Check the clip rect.
   if (!clip_rect.completely_within(img_rect))
   {
      return;
   }
   
   // Check the status and allocate memory if needed.
   if (getDataObjectStatus() == RSPF_NULL) initialize();
   
   // Get the width of the buffers.
   rspf_uint32 num_bands = getNumberOfBands();
   rspf_uint32 s_width = src_rect.width();
   rspf_uint32 d_width = getWidth();
   rspf_uint32 s_band_offset = s_width * src_rect.height();
   
   const T* s = static_cast<const T*>(src);
   
   rspf_uint32 band;
   rspf_uint32 destinationOffset = (clip_rect.ul().y - img_rect.ul().y) * d_width +
                              (clip_rect.ul().x - img_rect.ul().x);
   rspf_uint32 destinationIndex = destinationOffset;
   rspf_uint32 sourceOffset = (clip_rect.ul().y - src_rect.ul().y) * s_width +
                         (clip_rect.ul().x - src_rect.ul().x);
   rspf_uint32 sourceIndex = sourceOffset;
   rspf_uint32 clipHeight = clip_rect.height();
   rspf_uint32 clipWidth = clip_rect.width();

   // Copy the data.
   for (band=0; band<num_bands; band++)
   {
      T* destinationBand = static_cast<T*>(getBuf(band));
      destinationIndex = destinationOffset;
      sourceIndex = sourceOffset + s_band_offset*band;
      
      for (rspf_uint32 line = 0; line < clipHeight; ++line)
      {
         for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
         {
            destinationBand[destinationIndex + sample]
               = s[sourceIndex+sample];
         }
         sourceIndex += s_width;
         destinationIndex += d_width;
      }
   }
}

void rspfImageData::loadTileFrom1Band(const void* src,
				       const rspfIrect& src_rect)
{
   rspf_uint32 bands = getNumberOfBands();
   for(rspf_uint32 band = 0; band < bands; ++band)
   {
      loadBand(src, src_rect, band);
   }
}

void rspfImageData::loadTileFrom1Band(const rspfImageData* src)
{
  if(!src) return;
  loadTileFrom1Band((const void*)src->getBuf(),
		    src->getImageRectangle());
}

void rspfImageData::loadShortBand(const void* src,
                                   const rspfIrect& src_rect,
                                   rspf_uint32 band,
                                   bool swap_bytes)
{
   static const char  MODULE[] = "rspfImageData::loadShortBand";

   // Check the pointer.
   if (!src)
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "%s File %s line %d\nNULL pointer passed to method!",
                    MODULE,
                    __FILE__,
                    __LINE__);
      return;
   }

   // Check the band.
   if (!isValidBand(band))
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "%s File %s line %d\nInvalid band:  %d",
                    MODULE,
                    __FILE__,
                    __LINE__,
                    band);
      return;
   }

   const rspfIrect img_rect = getImageRectangle();
   
   // Check for intersect.
   if ( ! img_rect.intersects(src_rect) )
   {
      return; // Nothing to do here.
   }

   // Get the clip rect.
   const rspfIrect clip_rect = img_rect.clipToRect(src_rect);

   // Check the status and allocate memory if needed.
   if (getDataObjectStatus() == RSPF_NULL) initialize();

   // Get the width of the buffers.
   rspf_uint32 s_width = src_rect.width();
   rspf_uint32 d_width = getWidth();

   const rspf_uint16* s = static_cast<const rspf_uint16*>(src);
   rspf_uint16* d = getUshortBuf(band);

   // Move the pointers to the first valid pixel.
   s += (clip_rect.ul().y - src_rect.ul().y) * s_width +
        clip_rect.ul().x - src_rect.ul().x;
   
   d += (clip_rect.ul().y - img_rect.ul().y) * d_width +
        clip_rect.ul().x - img_rect.ul().x;
   
   // Copy the data.
   for (rspf_int32 line=clip_rect.ul().y; line<=clip_rect.lr().y; line++)
   {
      rspf_uint32 i = 0;
      for (rspf_int32 sample=clip_rect.ul().x; sample<=clip_rect.lr().x; sample++)
      {
         d[i] = (swap_bytes ? ( (s[i] << 8) | (s[i] >> 8) ) :
                 s[i]);
         ++i;
      }

      s += s_width;
      d += d_width;
   }

   validate();
}

bool rspfImageData::isPointWithin(const rspfIpt& point)const
{
   return ((point.x >= m_origin.x)&&
           (point.y >= m_origin.y)&&
           ((point.x - m_origin.x)<static_cast<rspf_int32>(m_spatialExtents[0]))&&
           ((point.y - m_origin.y)<static_cast<rspf_int32>(m_spatialExtents[1])));
}

bool rspfImageData::isPointWithin(rspf_int32 x, rspf_int32 y)const
{
   return ((x >= m_origin.x)&&
           (y >= m_origin.y)&&
           ((x - m_origin.x) < static_cast<rspf_int32>(m_spatialExtents[0]))&&
           ((y - m_origin.y) < static_cast<rspf_int32>(m_spatialExtents[1])));
}

void rspfImageData::unloadTile(void* dest,
                                const rspfIrect& dest_rect,
                                rspfInterleaveType type) const
{
   unloadTile(dest, dest_rect, getImageRectangle(), type);
}

void rspfImageData::unloadTile(void* dest,
                                const rspfIrect& dest_rect,
                                const rspfIrect& clip_rect,
                                rspfInterleaveType type) const
{
   switch (type)
   {
      case RSPF_BIP:
         unloadTileToBip(dest, dest_rect, clip_rect);
         return;
      case RSPF_BIL:
         unloadTileToBil(dest, dest_rect, clip_rect);
         return;
      case RSPF_BSQ:
         unloadTileToBsq(dest, dest_rect, clip_rect);
         return;
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::unloadTile\n\
File %s line %d\nUnknown scalar type!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::unloadTileToBip(void* dest,
                                     const rspfIrect& dest_rect,
                                     const rspfIrect& clip_rect) const
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         unloadTileToBipTemplate(rspf_uint8(0), dest, dest_rect, clip_rect);
         return;

      case RSPF_SINT8:
         unloadTileToBipTemplate(rspf_sint8(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         unloadTileToBipTemplate(rspf_uint16(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SINT16:
         unloadTileToBipTemplate(rspf_sint16(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_UINT32:
         unloadTileToBipTemplate(rspf_uint32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SINT32:
         unloadTileToBipTemplate(rspf_sint32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         unloadTileToBipTemplate(rspf_float32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         unloadTileToBipTemplate(rspf_float64(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::unloadTileToBip\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::unloadTileToBipAlpha(void* dest,
                                          const rspfIrect& dest_rect,
                                          const rspfIrect& clip_rect) const
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         unloadTileToBipAlphaTemplate(rspf_uint8(0), dest, dest_rect, clip_rect);
         return;

      case RSPF_SINT8:
         unloadTileToBipAlphaTemplate(rspf_sint8(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         unloadTileToBipAlphaTemplate(rspf_uint16(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SINT16:
         unloadTileToBipAlphaTemplate(rspf_sint16(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_UINT32:
         unloadTileToBipAlphaTemplate(rspf_uint32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SINT32:
         unloadTileToBipAlphaTemplate(rspf_sint32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         unloadTileToBipAlphaTemplate(rspf_float32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         unloadTileToBipAlphaTemplate(rspf_float64(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::unloadTileToBipAlpha\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::unloadTileToBil(void* dest,
                                     const rspfIrect& dest_rect,
                                     const rspfIrect& clip_rect) const
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         unloadTileToBilTemplate(rspf_uint8(0), dest, dest_rect, clip_rect);
         return;

      case RSPF_SINT8:
         unloadTileToBilTemplate(rspf_sint8(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         unloadTileToBilTemplate(rspf_uint16(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SINT16:
         unloadTileToBilTemplate(rspf_sint16(0), dest, dest_rect, clip_rect);
         return;

      case RSPF_UINT32:
         unloadTileToBilTemplate(rspf_uint32(0), dest, dest_rect, clip_rect);
         return;

      case RSPF_SINT32:
         unloadTileToBilTemplate(rspf_sint32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         unloadTileToBilTemplate(rspf_float32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         unloadTileToBilTemplate(rspf_float64(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::unloadTileToBil\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::unloadTileToBsq(void* dest,
                                     const rspfIrect& dest_rect,
                                     const rspfIrect& clip_rect) const
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         unloadTileToBsqTemplate(rspf_uint8(0), dest, dest_rect, clip_rect);
         return;

      case RSPF_SINT8:
         unloadTileToBsqTemplate(rspf_sint8(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         unloadTileToBsqTemplate(rspf_uint16(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SINT16:
         unloadTileToBsqTemplate(rspf_sint16(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_UINT32:
         unloadTileToBsqTemplate(rspf_uint32(0), dest, dest_rect, clip_rect);
         return;

      case RSPF_SINT32:
         unloadTileToBsqTemplate(rspf_sint32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         unloadTileToBsqTemplate(rspf_float32(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         unloadTileToBsqTemplate(rspf_float64(0), dest, dest_rect, clip_rect);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::unloadTileToBsq\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::unloadBand(void* dest,
                                const rspfIrect& dest_rect,
                                rspf_uint32 band) const
{
   unloadBand(dest, dest_rect, getImageRectangle(), band);
}

void rspfImageData::unloadBand(void* dest,
                                const rspfIrect& dest_rect,
                                const rspfIrect& clip_rect,
                                rspf_uint32 band) const
{
   // Call the appropriate load method.
   switch (getScalarType())
   {
      case RSPF_UINT8:
         unloadBandTemplate(rspf_uint8(0), dest, dest_rect, clip_rect, band);
         return;

      case RSPF_SINT8:
         unloadBandTemplate(rspf_sint8(0), dest, dest_rect, clip_rect, band);
         return;
         
      case RSPF_UINT16:
      case RSPF_USHORT11:
         unloadBandTemplate(rspf_uint16(0), dest, dest_rect, clip_rect, band);
         return;
         
      case RSPF_SINT16:
         unloadBandTemplate(rspf_sint16(0), dest, dest_rect, clip_rect, band);
         return;
         
      case RSPF_UINT32:
         unloadBandTemplate(rspf_uint32(0), dest, dest_rect, clip_rect, band);
         return;

      case RSPF_SINT32:
         unloadBandTemplate(rspf_sint32(0), dest, dest_rect, clip_rect, band);
         return;
         
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         unloadBandTemplate(rspf_float32(0), dest, dest_rect, clip_rect, band);
         return;
         
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         unloadBandTemplate(rspf_float64(0), dest, dest_rect, clip_rect, band);
         return;
         
      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
                       rspfErrorCodes::RSPF_ERROR,
                       "rspfImageData::unloadBand\n\
File %s line %d\nUnsupported scalar type for method!",
                       __FILE__,
                       __LINE__);      
         return;
   }
}

void rspfImageData::unloadBand( void* dest,
                                 rspf_uint32 src_band,
                                 rspf_uint32 dest_band,
                                 const rspfIrect& dest_rect,
                                 rspfInterleaveType il_type,
                                 OverwriteBandRule ow_type ) const
{
   unloadBand( dest, src_band, dest_band, dest_rect, getImageRectangle(), il_type, ow_type );
}

void rspfImageData::unloadBand( void* dest,
                                 rspf_uint32 src_band,
                                 rspf_uint32 dest_band,
                                 const rspfIrect& dest_rect,
                                 const rspfIrect& clip_rect,
                                 rspfInterleaveType il_type,
                                 OverwriteBandRule ow_type ) const
{
   static const char  MODULE[] = "rspfImageData::unloadBand";

   if ( il_type == RSPF_BSQ )
   {
      unloadBandToBsq( dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " NOTICE:"
         << "\nUnsupported interleave type:  " << il_type << "  Returning..."
         << std::endl;
   }
}

void rspfImageData::unloadBandToBsq( void* dest,
                                      rspf_uint32 src_band,
                                      rspf_uint32 dest_band,
                                      const rspfIrect& dest_rect,
                                      const rspfIrect& clip_rect,
                                      OverwriteBandRule ow_type ) const
{
   switch (getScalarType())
   {
      case RSPF_UINT8:
         unloadBandToBsqTemplate(rspf_uint8(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_SINT8:
         unloadBandToBsqTemplate(rspf_sint8(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_UINT16:
      case RSPF_USHORT11:
         unloadBandToBsqTemplate(rspf_uint16(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_SINT16:
         unloadBandToBsqTemplate(rspf_sint16(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_UINT32:
         unloadBandToBsqTemplate(rspf_uint32(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_SINT32:
         unloadBandToBsqTemplate(rspf_sint32(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
         unloadBandToBsqTemplate(rspf_float32(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
         unloadBandToBsqTemplate(rspf_float64(0), dest, src_band, dest_band, dest_rect, clip_rect, ow_type );
         return;

      case RSPF_SCALAR_UNKNOWN:
      default:
         rspfSetError(getClassName(),
            rspfErrorCodes::RSPF_ERROR,
            "rspfImageData::unloadBandToBsq\n\
            File %s line %d\nUnsupported scalar type for method!",
            __FILE__,
            __LINE__);      
         return;
   }
}

template <class T>
void rspfImageData::unloadBandTemplate(T, // dummy template variable
                                        void* dest,
                                        const rspfIrect& dest_rect,
                                        const rspfIrect& clip_rect,
                                        rspf_uint32 band) const
{
   static const char  MODULE[] = "rspfImageData::unloadBand";

   // Check the pointers.
   if (!dest)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nNULL pointer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }
   
   rspfIrect img_rect = getImageRectangle();
   
   // Clip the clip_rect to the tile rect.
   rspfIrect output_clip_rect = clip_rect.clipToRect(img_rect);

   // Clip it again to the destination rect.
   output_clip_rect = dest_rect.clipToRect(output_clip_rect);
   
   // Check the output clip rect for intersection.
   if (!output_clip_rect.intersects(img_rect))
   {
      return;
   }
   if ( !(output_clip_rect.intersects(dest_rect)) )
   {
      return;
   }

   // Check for valid band.
   if (!isValidBand(band))
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " NOTICE:"
         << "\nInvalid band:  " << band << "  Returning..."
         << std::endl;
      return;
   }
   
   rspf_uint32 buf_width = dest_rect.lr().x - dest_rect.ul().x + 1;

   // Get a pointer to the source band buffer.
   const T* s = reinterpret_cast<const T*>(getBuf(band));

   // Get the offset for each source band.
   rspf_uint32 src_offset = (output_clip_rect.ul().y - img_rect.ul().y) *
      getWidth() + output_clip_rect.ul().x - img_rect.ul().x;
   
   // Move the pointers to the start.
   T* d = reinterpret_cast<T*>(dest);

   rspf_uint32 dest_offset = (output_clip_rect.ul().y - dest_rect.ul().y) *
      buf_width + output_clip_rect.ul().x - dest_rect.ul().x;

   d += dest_offset;
   s += src_offset;

   for (rspf_int32 line=output_clip_rect.ul().y;
        line<=output_clip_rect.lr().y; ++line)
   {
      rspf_uint32 i=0;
      for (rspf_int32 samp=clip_rect.ul().x;
           samp<=output_clip_rect.lr().x; ++samp)
      {
         d[i] = s[i];
         ++i;
      }
      d += buf_width;
      s += getWidth();
   }
}

template <class T> void
rspfImageData::unloadTileToBipTemplate(T, // dummy template variable
                                        void* dest,
                                        const rspfIrect& dest_rect,
                                        const rspfIrect& clip_rect) const
{
   static const char  MODULE[] = "rspfImageData::unloadTileToBip";
   
   // Check the pointer.
   if (!dest)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nNULL pointer passed to method!  Returning..." << std::endl;
      return;
   }
   
   bool  dataIsNull = false;
   if (getDataObjectStatus() == RSPF_NULL)
   {
      dataIsNull = true;
   }
   
   rspfIrect img_rect = getImageRectangle();
   
   // Clip the clip_rect to the tile rect.
   rspfIrect output_clip_rect = clip_rect.clipToRect(img_rect);

   // Clip it again to the destination rect.
   output_clip_rect = dest_rect.clipToRect(output_clip_rect);
   
   // Check the output clip rect for intersection.
   if (output_clip_rect.hasNans())
   {
      return;
   }
   if ( !(output_clip_rect.intersects(dest_rect)) )
   {
      return;
   }
   
   rspf_int32 num_bands = getNumberOfBands();
   
   rspf_int32 buf_width = dest_rect.width() * num_bands;

   if(!dataIsNull)
   {
      // Get the number of bands and grab a pointers to each one.
      const T** s = new const T*[num_bands];
      
      rspf_int32 band;
      rspf_int32 s_width = getWidth();
      
      for (band=0; band<num_bands; band++)
      {
         s[band] = reinterpret_cast<const T*>(getBuf(band));
      }
      
      // Move the pointers to the start.
      T* d = reinterpret_cast<T*>(dest);      
      
      d += (output_clip_rect.ul().y - dest_rect.ul().y) * buf_width +
           (output_clip_rect.ul().x - dest_rect.ul().x) * num_bands;
      
      // Get the offset for each source band.
      rspf_int32 src_offset = (output_clip_rect.ul().y - img_rect.ul().y) *
         s_width + (output_clip_rect.ul().x - img_rect.ul().x);
      
      rspf_int32 output_clip_width  = output_clip_rect.width();
      rspf_int32 output_clip_height = output_clip_rect.height();
      
      for (band=0; band<(rspf_int32)getNumberOfBands(); band++)
      {
         s[band] += src_offset;
      }
      
      rspf_int32 j;
      for (rspf_int32 line=0; line<output_clip_height; ++line)
      {
         j = 0;
         for (rspf_int32 samp=0; samp<output_clip_width; ++samp, j+=num_bands)
         {
            for (band=0; band<num_bands; ++band)
            {
               d[j+band] = s[band][samp];
            }
         }
         
         // increment to next line...
         d += buf_width;
         for (band=0; band<num_bands; ++band)
         {
            s[band] += s_width;
         }
      
      }
      delete [] s;
   }
   else
   {
      rspf_int32 band = 0;
      T* nulls = new T[num_bands];
      for(band = 0; band < num_bands; ++band)
      {
         nulls[band] = static_cast<T>(m_nullPixelValue[band]);
      }
      // Move the pointers to the start.
      T* d = reinterpret_cast<T*>(dest);
      
      d += (output_clip_rect.ul().y - dest_rect.ul().y) * buf_width +
           (output_clip_rect.ul().x - dest_rect.ul().x) * num_bands;

      for (rspf_int32 line=output_clip_rect.ul().y;
           line<=output_clip_rect.lr().y; ++line)
      {
         rspf_int32 i=0;
         rspf_int32 j=0;
         for (rspf_int32 samp=output_clip_rect.ul().x;
              samp<=output_clip_rect.lr().x; ++samp)
         {
            for (band=0; band<num_bands; ++band)
            {
               d[j+band] = nulls[band];
            }
            ++i;  // increment to next pixel...
            j+= num_bands;
         }
         
         // increment to next line...
         d += buf_width;
      }
      delete [] nulls;
      nulls = 0;
   }
}

template <class T> void
rspfImageData::unloadTileToBipAlphaTemplate(T, // dummy template variable
                                             void* dest,
                                             const rspfIrect& dest_rect,
                                             const rspfIrect& clip_rect) const
{
   static const char  M[] = "rspfImageData::unloadTileToBipAlphaTemplate";

   // Sanity checks:

   // Are we null:
   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << M << " ERROR:"
         << "\nAttempting to copy null tile!  Returning..." << std::endl;
      return;
   }
   
   // Check the pointer.
   if (!dest)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << M << " ERROR:"
         << "\nNULL pointer passed to method!  Returning..." << std::endl;
      return;
   }

   if ( !hasAlpha() )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << M << " ERROR:"
         << "\nTile has no alpha!  Returning..." << std::endl;
      return; 
   }
   
   rspfIrect img_rect = getImageRectangle();
   
   // Clip the clip_rect to the tile rect.
   rspfIrect output_clip_rect = clip_rect.clipToRect(img_rect);

   // Clip it again to the destination rect.
   output_clip_rect = dest_rect.clipToRect(output_clip_rect);
   
   // Check the output clip rect for intersection.
   if (output_clip_rect.hasNans())
   {
      return;
   }
   if ( !(output_clip_rect.intersects(dest_rect)) )
   {
      return;
   }
   
   const rspf_int32 NUM_DATA_BANDS = getNumberOfBands();
   const rspf_int32 BANDS = NUM_DATA_BANDS + 1; // Has data plus alpha.
   const rspf_float64 MAX_PIX = static_cast<T>(getMaxPix(0));
   const rspf_float64 ALPHA_MAX_PIX = 255.00;
   const rspf_int32 S_WIDTH = getWidth();
   const rspf_int32 D_WIDTH = dest_rect.width() * BANDS;
   const rspf_int32 OUTPUT_CLIP_WIDTH  = output_clip_rect.width();
   const rspf_int32 OUTPUT_CLIP_HEIGHT = output_clip_rect.height();

   bool uint8Flag = ( getScalarType() == RSPF_UINT8 );
   
   // Get the offset for each source band relative to start of clip rect.
   const rspf_int32 SRC_OFFSET = (output_clip_rect.ul().y - img_rect.ul().y) *
      S_WIDTH + (output_clip_rect.ul().x - img_rect.ul().x);
   
   // Get pointers to each one band of image data.
   std::vector<const T*> s(NUM_DATA_BANDS);
   rspf_int32 band;
   for (band = 0; band < NUM_DATA_BANDS; ++band)
   {
      s[band] = reinterpret_cast<const T*>(getBuf(band));

      // Move to start of clip rect.
      s[band] += SRC_OFFSET;
   }

   // Get pointer to the alpha channel:
   const rspf_uint8* a = getAlphaBuf();

   // Move to start of clip rect.
   a += SRC_OFFSET;

   // Pointer to the destination buffer.
   T* d = reinterpret_cast<T*>(dest); 

   // Move to start of clip rect.
   d += (output_clip_rect.ul().y - dest_rect.ul().y) * D_WIDTH +
      (output_clip_rect.ul().x - dest_rect.ul().x) * BANDS;
   

   // Loop to copy data:
   for (rspf_int32 line = 0; line < OUTPUT_CLIP_HEIGHT; ++line)
   {
      rspf_int32 j = 0;
      rspf_int32 alphaIdx = 0;
      for (rspf_int32 samp = 0; samp < OUTPUT_CLIP_WIDTH; ++samp, j +=BANDS)
      {
         // Copy the pixels:
         for (band = 0; band < NUM_DATA_BANDS; ++band)
         {
            d[j+band] = s[band][samp];
         }
         
         // Copy alpha channel converting to scalar type.
         if ( uint8Flag )
         {
            d[j+NUM_DATA_BANDS] = a[alphaIdx++];
         }
         else
         {
            d[j+NUM_DATA_BANDS] =
               static_cast<T>( (a[alphaIdx++]/ALPHA_MAX_PIX) * MAX_PIX );
         }
      }
      
      // Increment pointers to next line...
      d += D_WIDTH;
      for (band = 0; band < NUM_DATA_BANDS; ++band)
      {
         s[band] += S_WIDTH;
      }
      a += S_WIDTH;
   }
}

template <class T> void
rspfImageData::unloadTileToBilTemplate(T,  // dummy template arg...
                                        void* dest,
                                        const rspfIrect& dest_rect,
                                        const rspfIrect& clip_rect) const
{
  static const char  MODULE[] = "rspfImageData::unloadTileToBil";

   // Check the pointers.
   if (!dest)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nNULL pointer passed to method!  Returning..." << std::endl;
      return;
   }
   bool dataIsNull = false;
   if (getDataObjectStatus() == RSPF_NULL)
   {
      dataIsNull = true;
   }

   rspfIrect img_rect = getImageRectangle();
   
   // Clip the clip_rect to the tile rect.
   rspfIrect output_clip_rect = clip_rect.clipToRect(img_rect);

   // Clip it again to the destination rect.
   output_clip_rect = dest_rect.clipToRect(output_clip_rect);
   
   // Check the output clip rect for intersection.
   if (output_clip_rect.hasNans())
   {
      return;
   }
   if ( !(output_clip_rect.intersects(dest_rect)) )
   {
      return;
   }

   rspf_uint32 num_bands = getNumberOfBands();

   rspf_uint32 buf_width = dest_rect.width();

   if(!dataIsNull)
   {
      // Get the number of bands and grab a pointers to each one.
      const T** s = new const T*[num_bands];
      
      rspf_uint32 band;
      
      for (band=0; band<num_bands; band++)
      {
         s[band] = reinterpret_cast<const T*>(getBuf(band));
      }
      
      // Get the offset for each source band.
      rspf_uint32 src_offset = (output_clip_rect.ul().y - img_rect.ul().y) *
         getWidth() + (output_clip_rect.ul().x - img_rect.ul().x);
      
      // Move the pointers to the start.
      T* d = reinterpret_cast<T*>(dest);

      d += (output_clip_rect.ul().y - dest_rect.ul().y) *
         buf_width * num_bands +
         (output_clip_rect.ul().x-dest_rect.ul().x);
      
      for (band=0; band<num_bands; ++band)
      {
         s[band] += src_offset;
      }
      
      for (rspf_int32 line=output_clip_rect.ul().y;
           line<=output_clip_rect.lr().y; ++line)
      {
         for (band=0; band<num_bands; ++band)
         {
            rspf_uint32 i=0;
            for (rspf_int32 samp=output_clip_rect.ul().x;
                 samp<=output_clip_rect.lr().x; ++samp)
            {
               d[i] = s[band][i];
               ++i;
            }
            d       += buf_width;
            s[band] += getWidth();
         }
      }
      // Free up memory allocated for pointers.
      delete [] s;
   }
   else
   {
      rspf_uint32 band = 0;
      T* nulls = new T[num_bands];
      for(band = 0; band < num_bands; ++band)
      {
         nulls[band] = static_cast<T>(m_nullPixelValue[band]);
      }
      
      // Move the pointers to the start.
      T* d = reinterpret_cast<T*>(dest);

      d += (output_clip_rect.ul().y - dest_rect.ul().y) *
           buf_width * num_bands + (output_clip_rect.ul().x - dest_rect.ul().x);
      
      for (rspf_int32 line=output_clip_rect.ul().y;
           line<=output_clip_rect.lr().y; ++line)
      {
         for (band=0; band<num_bands; ++band)
         {
            rspf_uint32 i=0;
            for (rspf_int32 samp=output_clip_rect.ul().x;
                 samp<=output_clip_rect.lr().x; ++samp)
            {
               d[i] = nulls[band];
               ++i;
            }
            d += buf_width;
         }
      }
      delete [] nulls;
   }
}

template <class T> void rspfImageData::nullTileAlphaTemplate(
   T,
   const rspf_uint8* src,
   const rspfIrect& src_rect,
   const rspfIrect& clip_rect,
   bool multiplyAlphaFlag)
{
   static const char  MODULE[] = "rspfImageData::nullTileAlphaTemplate";
   
   // Check the pointer.
   if (!src)
   {
      // Set the error...
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "%s File %s line %d\nNULL pointer passed to method!",
                    MODULE,
                    __FILE__,
                    __LINE__);
      return;
   }
   
   const rspfIrect img_rect = getImageRectangle();
   
   // Check for intersect.
   if ( ! img_rect.intersects(src_rect) )
   {
      return; // Nothing to do here.
   }

   // Check the clip rect.
   if (!clip_rect.completely_within(img_rect))
   {
      return;
   }
   
   // Check the status and allocate memory if needed.
   if (getDataObjectStatus() == RSPF_NULL) initialize();
   
   // Get the width of the buffers.
   rspf_uint32 num_bands = getNumberOfBands();
   rspf_uint32 s_width = src_rect.width();
   rspf_uint32 d_width = getWidth();
   rspf_uint32 band;
   rspf_uint32 destinationOffset = (clip_rect.ul().y - img_rect.ul().y) * d_width +
                              (clip_rect.ul().x - img_rect.ul().x);
   rspf_uint32 destinationIndex = destinationOffset;
   rspf_uint32 sourceOffset = (clip_rect.ul().y - src_rect.ul().y) * s_width +
                         (clip_rect.ul().x - src_rect.ul().x);
   rspf_uint32 sourceIndex = sourceOffset;
   rspf_uint32 clipHeight = clip_rect.height();
   rspf_uint32 clipWidth = clip_rect.width();

   if(!multiplyAlphaFlag)
   {
      // Copy the data.
      for (band=0; band<num_bands; band++)
      {
         T* destinationBand = static_cast<T*>(getBuf(band));
         destinationIndex = destinationOffset;
         sourceIndex = sourceOffset;
         T nullPix = (T)getNullPix(band);
         T minPix = (T)getMinPix(band);
         for (rspf_uint32 line = 0; line < clipHeight; ++line)
         {
            for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
            {
               if(src[sourceIndex+sample] == 0)
               {
                  destinationBand[destinationIndex + sample] = nullPix;
               }
               else if(destinationBand[destinationIndex + sample] == nullPix)
               {
                  destinationBand[destinationIndex + sample] = minPix;
               }
            }
            sourceIndex += s_width;
            destinationIndex += d_width;
         }
      }
   }
   else
   {
      rspf_float64 normalizer = 1.0/255.0;
      // Copy the data.
      for (band=0; band<num_bands; band++)
      {
         T* destinationBand = static_cast<T*>(getBuf(band));
         destinationIndex = destinationOffset;
         sourceIndex = sourceOffset;
         T nullPix = (T)getNullPix(band);
         T minPix = (T)getMinPix(band);
         for (rspf_uint32 line = 0; line < clipHeight; ++line)
         {
            for (rspf_uint32 sample = 0; sample < clipWidth; ++sample)
            {
               if(src[sourceIndex+sample] != 255)
               {
                  if(src[sourceIndex+sample] != 0)
                  {
                     destinationBand[destinationIndex + sample] = (T)(destinationBand[destinationIndex + sample]*
                                                                      (src[sourceIndex+sample]*normalizer));
                     if(destinationBand[destinationIndex + sample] == nullPix)
                     {
                        destinationBand[destinationIndex + sample] = minPix;
                     }
                  }
                  else
                  {
                     destinationBand[destinationIndex + sample] = nullPix;
                  }
               }
               else if(destinationBand[destinationIndex + sample] == nullPix)
               {
                  destinationBand[destinationIndex + sample] = minPix;
               }
            }
            sourceIndex += s_width;
            destinationIndex += d_width;
         }
      }
      
   }
}

template <class T> void
rspfImageData::unloadTileToBsqTemplate(T,  // dummy template arg...
                                        void* dest,
                                        const rspfIrect& dest_rect,
                                        const rspfIrect& clip_rect) const
{
   static const char  MODULE[] = "rspfImageData::unloadTileToBsq";
   
   // Check the pointers.
   if (!dest)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nNULL pointer passed to method!  Returning..." << std::endl;
      return;
   }

   bool dataIsNull = false;
   if (getDataObjectStatus() == RSPF_NULL)
   {
      dataIsNull = true;
   }

   rspfIrect img_rect = getImageRectangle();
   
   // Clip the clip_rect to the tile rect.
   rspfIrect output_clip_rect = clip_rect.clipToRect(img_rect);

   // Clip it again to the destination rect.
   output_clip_rect = dest_rect.clipToRect(output_clip_rect);
   
   // Check the output clip rect for intersection.
   if (output_clip_rect.hasNans())
   {
      return;
   }
   if ( !(output_clip_rect.intersects(dest_rect)) )
   {
      return;
   }

   rspf_uint32 num_bands = getNumberOfBands();
   rspf_uint32 band      = 0;
   if(!dataIsNull)
   {
      rspf_uint32 d_width       = dest_rect.lr().x - dest_rect.ul().x + 1;
      rspf_uint32 d_band_offset = d_width * (dest_rect.lr().y-dest_rect.ul().y+1);
      rspf_uint32 s_width       = getWidth();
      rspf_uint32 s_offset      = (output_clip_rect.ul().y - img_rect.ul().y) *
                                   s_width + (output_clip_rect.ul().x - img_rect.ul().x);

      T* d        = static_cast<T*>(dest);
      const T** s = new const T*[num_bands];

      // Grab a pointers to each one.
      for (band=0; band<num_bands; ++band)
      {
         s[band] = reinterpret_cast<const T*>(getBuf(band));

         // Move to first valid pixel.
         s[band] += s_offset;
      }

      // Move to first valid pixel.
      d += (output_clip_rect.ul().y - dest_rect.ul().y) * d_width +
           (output_clip_rect.ul().x - dest_rect.ul().x);

      for (band=0; band<num_bands; ++band)
      {
         rspf_uint32 d_buf_offset = 0;

         for (rspf_int32 line=output_clip_rect.ul().y;
              line<=output_clip_rect.lr().y; ++line)
         {
            rspf_int32 i=0;
            for (rspf_int32 samp=output_clip_rect.ul().x;
                 samp<=output_clip_rect.lr().x; ++samp)
            {
               d[d_buf_offset+i] = s[band][i];
               ++i;
            }

            d_buf_offset += d_width;
            s[band]      += s_width;
         }
         d += d_band_offset;
      }
      
      // Free up memory allocated for pointers.
      delete [] s;
   }
   else
   {
      rspf_uint32 d_width       = dest_rect.lr().x - dest_rect.ul().x + 1;
      rspf_uint32 d_band_offset = d_width * (dest_rect.lr().y-dest_rect.ul().y+1);
      
      rspf_uint8* d = static_cast<rspf_uint8*>(dest);
            
      // Move to first valid pixel.
      d += (output_clip_rect.ul().y - dest_rect.ul().y) * d_width +
           (output_clip_rect.ul().x - dest_rect.ul().x);
      
      for (band=0; band<num_bands; ++band)
      {
         rspf_uint8 np = static_cast<rspf_uint8>(m_nullPixelValue[band]);
         rspf_uint32 d_buf_offset = 0;
         
         for (rspf_int32 line=output_clip_rect.ul().y;
              line<=output_clip_rect.lr().y; ++line)
         {
            rspf_int32 i=0;
            for (rspf_int32 samp=output_clip_rect.ul().x;
                 samp<=output_clip_rect.lr().x; ++samp)
            {
               d[d_buf_offset+i] = np;
               ++i;
            }
            
            d_buf_offset += d_width;
         }
         d += d_band_offset;
      }
   }
}

template <class T> void
rspfImageData::unloadBandToBsqTemplate(T,  // dummy template arg...
                                        void* dest,
                                        rspf_uint32 src_band,
                                        rspf_uint32 dest_band,
                                        const rspfIrect& dest_rect,
                                        const rspfIrect& clip_rect,
                                        OverwriteBandRule ow_type) const
{
   static const char  MODULE[] = "rspfImageData::unloadBandToBsq";

   // Check the pointers.
   if (!dest)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nNULL pointer passed to method!  Returning..." << std::endl;
      return;
   }

   bool dataIsNull = false;
   if (getDataObjectStatus() == RSPF_NULL)
   {
      dataIsNull = true;
   }

   rspfIrect img_rect = getImageRectangle();

   // Clip the clip_rect to the tile rect.
   rspfIrect output_clip_rect = clip_rect.clipToRect(img_rect);

   // Clip it again to the destination rect.
   output_clip_rect = dest_rect.clipToRect(output_clip_rect);

   // Check the output clip rect for intersection.
   if (output_clip_rect.hasNans())
   {
      return;
   }
   if ( !(output_clip_rect.intersects(dest_rect)) )
   {
      return;
   }

   rspf_uint32 num_bands = getNumberOfBands();
   rspf_uint32 band      = 0;
   if(!dataIsNull)
   {
      rspf_uint32 d_width       = dest_rect.lr().x - dest_rect.ul().x + 1;
      rspf_uint32 d_band_offset = d_width * (dest_rect.lr().y-dest_rect.ul().y+1);
      rspf_uint32 s_width  = getWidth();
      rspf_uint32 s_offset = (output_clip_rect.ul().y - img_rect.ul().y) *
         s_width + (output_clip_rect.ul().x - img_rect.ul().x);

      T* d        = static_cast<T*>(dest);
      const T** s = new const T*[num_bands];

      // Grab a pointers to each one.
      for (band=0; band<num_bands; ++band)
      {
         s[band] = reinterpret_cast<const T*>(getBuf(band));

         // Move to first valid pixel.
         s[band] += s_offset;
      }

      // Move to first valid pixel.
      d += (output_clip_rect.ul().y - dest_rect.ul().y) * d_width +
           (output_clip_rect.ul().x - dest_rect.ul().x);

      rspf_uint32 d_dest_band_offset = dest_band * d_band_offset;
      rspf_uint32 d_buf_offset = 0;

      for (rspf_int32 line=output_clip_rect.ul().y;
         line<=output_clip_rect.lr().y; ++line)
      {
         rspf_int32 i=0;
         for (rspf_int32 samp=output_clip_rect.ul().x;
            samp<=output_clip_rect.lr().x; ++samp)
         {
            rspf_uint32 d_pixel_offset = d_buf_offset+i;
            rspf_uint32 d_dest_band_pixel_offset = d_pixel_offset + d_dest_band_offset;

            switch( ow_type )
            {
               case COLOR_DISCREPANCY:
               case COLOR_DISCREPANCY_OF_ANY_FROM_DEST:
               {
                  T d_dest_band = d[d_dest_band_pixel_offset];

                  for ( band=0; band<num_bands && band!=dest_band; ++band )
                  {
                     T d_other_band = d[d_pixel_offset + (band * d_band_offset)];

                     // test for color discrepancy
                     if ( d_other_band != d_dest_band )
                     {
                        d[d_dest_band_pixel_offset] = s[src_band][i];
                        break;
                     }
                  }
               }
               break;

               case COLOR_DISCREPANCY_OF_ALL_FROM_DEST:
               {
                  T d_dest_band = d[d_dest_band_pixel_offset];

                  bool bFoundSameValue = false;
                  for ( band=0; band<num_bands && band!=dest_band; ++band )
                  {
                     T d_other_band = d[d_pixel_offset + (band * d_band_offset)];

                     // test for color sameness
                     if ( d_other_band == d_dest_band )
                     {
                        bFoundSameValue = true;
                        break;
                     }
                  }
                  if ( bFoundSameValue == false )
                  {
                     d[d_dest_band_pixel_offset] = s[src_band][i];
                  }
               }
               break;

               case COLOR_EQUALITY_OF_ANY_TO_SRC:
               {
                  T d_src_band = s[src_band][i];

                  for ( band=0; band<num_bands && band!=dest_band; ++band )
                  {
                     T d_other_band = d[d_pixel_offset + (band * d_band_offset)];

                     // test for color discrepancy
                     if ( d_other_band == d_src_band )
                     {
                        d[d_dest_band_pixel_offset] = s[src_band][i];
                        break;
                     }
                  }
               }
               break;

               case COLOR_EQUALITY_OF_ALL_TO_SRC:
               {
                  T d_src_band = s[src_band][i];

                  bool bFoundDifferentValue = false;
                  for ( band=0; band<num_bands && band!=dest_band; ++band )
                  {
                     T d_other_band = d[d_pixel_offset + (band * d_band_offset)];

                     // test for color discrepancy
                     if ( d_other_band != d_src_band )
                     {
                        bFoundDifferentValue = true;
                        break;
                     }
                  }
                  if ( bFoundDifferentValue == false )
                  {
                     d[d_dest_band_pixel_offset] = s[src_band][i];
                  }
               }
               break;

               case NULL_RULE:
               default:
               {
                  d[d_dest_band_pixel_offset] = s[src_band][i];
               }
               break;
            }

            ++i;
         }

         d_buf_offset += d_width;
         s[src_band]  += s_width;
      }

      // Free up memory allocated for pointers.
      delete [] s;
   }
   else
   {
      rspf_uint32 d_width       = dest_rect.lr().x - dest_rect.ul().x + 1;
      rspf_uint32 d_band_offset = d_width * (dest_rect.lr().y-dest_rect.ul().y+1);

      rspf_uint8* d = static_cast<rspf_uint8*>(dest);

      // Move to first valid pixel.
      d += (output_clip_rect.ul().y - dest_rect.ul().y) * d_width +
         (output_clip_rect.ul().x - dest_rect.ul().x);

      for (band=0; band<num_bands; ++band)
      {
         rspf_uint8 np = static_cast<rspf_uint8>(m_nullPixelValue[band]);
         rspf_uint32 d_buf_offset = 0;

         for (rspf_int32 line=output_clip_rect.ul().y;
            line<=output_clip_rect.lr().y; ++line)
         {
            rspf_int32 i=0;
            for (rspf_int32 samp=output_clip_rect.ul().x;
               samp<=output_clip_rect.lr().x; ++samp)
            {
               d[d_buf_offset+i] = np;
               ++i;
            }

            d_buf_offset += d_width;
         }
         d += d_band_offset;
      }
   }
}

rspfObject* rspfImageData::dup() const
{
   return new rspfImageData(*this);
}

void rspfImageData::copyTileToNormalizedBuffer(rspf_float64* buf)const
{
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileToNormalizedBuffer ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileToNormalizedBuffer ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }
   
   switch(getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         memmove(buf, getBuf(), getDataSizeInBytes());
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         rspf_uint32 upperBound = getWidth()*getHeight()*getNumberOfBands();
         const rspf_float32* inputBuf = static_cast<const rspf_float32*>(getBuf());
         for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            buf[offset] = inputBuf[offset];
         }
         break;
      }
      case RSPF_FLOAT32:
      {
         copyTileToNormalizedBuffer((rspf_float32)0, buf);
         break;
      }
      case RSPF_FLOAT64:
      {
         copyTileToNormalizedBuffer((rspf_float64)0, buf);
         break;
      }
      case RSPF_UINT8:
      {
         copyTileToNormalizedBuffer((rspf_uint8)0, buf);
         break;
      
      }
      case RSPF_SINT8:
      {
         copyTileToNormalizedBuffer((rspf_sint8)0, buf);
         break;
      
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyTileToNormalizedBuffer((rspf_uint16)0, buf);
         break;
      }
      case RSPF_SINT16:
      {
         copyTileToNormalizedBuffer((rspf_sint16)0, buf);
         break;
      }
      case RSPF_UINT32:
      {
         copyTileToNormalizedBuffer((rspf_uint32)0, buf);
         break;
      }
      case RSPF_SINT32:
      {
         copyTileToNormalizedBuffer((rspf_sint32)0, buf);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "NOTICE:  copyTileToNormalizedBuffer not implemented yet"
            << std::endl;
      }
      
   }  // End of "switch(getScalarType())"
}

template <class T>
void rspfImageData::copyTileToNormalizedBuffer(T /* dummyTemplate */,
                                                rspf_float64* buf) const
{
   const rspf_uint32 SIZE  = getSizePerBand();
   const rspf_uint32 BANDS = getNumberOfBands();
   
   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const rspf_float64 MIN_PIX = getMinPix(band);
      const rspf_float64 MAX_PIX = getMaxPix(band);
      const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
      const rspf_float64 NP      = getNullPix(band);

      const T* s = (T*)getBuf(band);  // source
      rspf_float64* d = (rspf_float64*)(buf + (band*SIZE));  // destination
      
      for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
      {
         rspf_float64 p = s[offset];
         if(p != NP)
         {
            if( p == MIN_PIX)
            {
               d[offset] = RSPF_DEFAULT_MIN_PIX_NORM_DOUBLE;
            }
            else
            {
               d[offset] = (p-MIN_PIX)/RANGE;
            }
         }
         else
         {
            d[offset] = 0.0;
         }
      }
   }   
}

template <class T>
void rspfImageData::copyTileToNormalizedBuffer(T /* dummyTemplate */,
                                                rspf_float32* buf) const
{
   const rspf_uint32 SIZE  = getSizePerBand();
   const rspf_uint32 BANDS = getNumberOfBands();
   
   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const rspf_float64 MIN_PIX = getMinPix(band);
      const rspf_float64 MAX_PIX = getMaxPix(band);
      const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
      const rspf_float64 NP      = getNullPix(band);
      
      const T* s = (T*)getBuf(band);  // source
      rspf_float32* d = (rspf_float32*)(buf + (band*SIZE));  // destination
      
      for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
      {
         rspf_float64 p = s[offset];
         if(p != NP)
         {
            if( p == MIN_PIX)
            {
               d[offset] = RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
            }
            else
            {
               d[offset] = (p-MIN_PIX)/RANGE;
            }
         }
         else
         {
            d[offset] = 0.0;
         }
      }
   }   
}

template <class T>
void rspfImageData::copyTileToNormalizedBuffer(T /* dummyTemplate */,
                                                rspf_uint32 band,
                                                rspf_float64* buf) const
{
   const rspf_uint32  SIZE    = getSizePerBand();
   const rspf_float64 MIN_PIX = getMinPix(band);
   const rspf_float64 MAX_PIX = getMaxPix(band);
   const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
   const rspf_float64 NP      = getNullPix(band);
   
   const T* s = (T*)getBuf(band);  // source
   rspf_float64* d = (rspf_float64*)(buf);  // destination
   
   for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
   {
         rspf_float64 p = s[offset];
         if(p != NP)
         {
            if( p == MIN_PIX)
            {
               d[offset] = RSPF_DEFAULT_MIN_PIX_NORM_DOUBLE;
            }
            else
            {
               d[offset] = (p-MIN_PIX)/RANGE;
            }
         }
         else
         {
            d[offset] = 0.0;
         }
   }   
}

template <class T>
void rspfImageData::copyTileToNormalizedBuffer(T /* dummyTemplate */,
                                                rspf_uint32 band,
                                                rspf_float32* buf) const
{
   const rspf_uint32  SIZE    = getSizePerBand();
   const rspf_float64 MIN_PIX = getMinPix(band);
   const rspf_float64 MAX_PIX = getMaxPix(band);
   const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
   const rspf_float64 NP      = getNullPix(band);
   
   const T* s = (T*)getBuf(band);  // source
   rspf_float32* d     = (rspf_float32*)(buf);  // destination
   
   for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
   {
      rspf_float64 p = s[offset];
      if(p != NP)
      {
         if( p == MIN_PIX)
         {
            d[offset] = RSPF_DEFAULT_MIN_PIX_NORM_FLOAT;
         }
         else
         {
            d[offset] = (p-MIN_PIX)/RANGE;
         }
      }
      else
      {
         d[offset] = 0.0;
      }
   }   
}

template <class T>
void rspfImageData::copyNormalizedBufferToTile(T /* dummyTemplate */,
                                                rspf_float64* buf)
{
   const rspf_uint32 SIZE  = getSizePerBand();
   const rspf_uint32 BANDS = getNumberOfBands();
   
   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const rspf_float64 MIN_PIX = getMinPix(band);
      const rspf_float64 MAX_PIX = getMaxPix(band);
      const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
      const T NP                  = (T)getNullPix(band);
      
      rspf_float64* s = buf + (band*SIZE); // source
      T* d   = (T*)getBuf(band); // destination
      
      for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
      {
         const rspf_float64 P = s[offset];
         if(P != 0.0)
         {
            d[offset] = (T)(MIN_PIX + RANGE*P);
         }
         else
         {
            d[offset] = NP;
         }
      }
   }
}

template <class T>
void rspfImageData::copyNormalizedBufferToTile(T /* dummyTemplate */,
                                                rspf_float32* buf)
{
   const rspf_uint32 SIZE  = getSizePerBand();
   const rspf_uint32 BANDS = getNumberOfBands();
   
   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      const rspf_float64 MIN_PIX = getMinPix(band);
      const rspf_float64 MAX_PIX = getMaxPix(band);
      const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
      const T             NP      = (T)getNullPix(band);
      
      rspf_float32* s = buf + (band*SIZE); // source
      T* d   = (T*)getBuf(band); // destination
      
      for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
      {
         const rspf_float64 P = s[offset];
         if(P != 0.0)
         {
	   rspf_float64 test = MIN_PIX + RANGE*P;
	   if(test > MAX_PIX) test = MAX_PIX;
	   d[offset] = (T)test;
         }
         else
         {
            d[offset] = NP;
         }
      }
   }
}

template <class T>
void rspfImageData::copyNormalizedBufferToTile(T /* dummyTemplate */,
                                                rspf_uint32 band,
                                                rspf_float64* buf)
{
   const rspf_uint32  SIZE    = getSizePerBand();
   const rspf_float64 MIN_PIX = getMinPix(band);
   const rspf_float64 MAX_PIX = getMaxPix(band);
   const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
   const T             NP      = (T)getNullPix(band);
   
   rspf_float64* s = buf; // source
   T* d   = (T*)getBuf(band); // destination
   
   for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
   {
      const rspf_float64 P = s[offset];
      if(P != 0.0)
      {
	   rspf_float64 test = MIN_PIX + RANGE*P;
	   if(test > MAX_PIX) test = MAX_PIX;
	   d[offset] = (T)test;
      }
      else
      {
         d[offset] = NP;
      }
   }
}

template <class T>
void rspfImageData::copyNormalizedBufferToTile(T /* dummyTemplate */,
                                                rspf_uint32 band,
                                                rspf_float32* buf)
{
   const rspf_uint32 SIZE     = getSizePerBand();
   const rspf_float64 MIN_PIX = getMinPix(band);
   const rspf_float64 MAX_PIX = getMaxPix(band);
   const rspf_float64 RANGE   = (MAX_PIX-MIN_PIX);
   const T NP                  = (T)getNullPix(band);
   
   rspf_float32* s = buf; // source
   T* d   = (T*)getBuf(band); // destination
   
   for(rspf_uint32 offset = 0; offset < SIZE; ++offset)
   {
      const rspf_float64 P = s[offset];
      if(P != 0.0)
      {
	   rspf_float64 test = MIN_PIX + RANGE*P;
	   if(test > MAX_PIX) test = MAX_PIX;
	   d[offset] = (T)test;
      }
      else
      {
         d[offset] = NP;
      }
   }
}

void rspfImageData::copyTileBandToNormalizedBuffer(rspf_uint32 band,
                                                    rspf_float64* buf) const
{
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileBandToNormalizedBuffer ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileBandToNormalizedBuffer ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }
   
   if(!getBuf(band)) return;

   switch(getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         memmove(buf, getBuf(band), getDataSizeInBytes());
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         rspf_uint32 upperBound = getWidth()*getHeight();
         const rspf_float32* inputBuf = static_cast<const rspf_float32*>(getBuf(band));
         for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            buf[offset] = inputBuf[offset];
         }
         break;
      }
      case RSPF_FLOAT32:
      {
         copyTileToNormalizedBuffer((rspf_float32)0, band, buf);
         break;
      }
      case RSPF_FLOAT64:
      {
         copyTileToNormalizedBuffer((rspf_float64)0, band, buf);
         break;
      }
      case RSPF_UINT8:
      {
         copyTileToNormalizedBuffer((rspf_uint8)0, band, buf);
         break;
      
      }
      case RSPF_SINT8:
      {
         copyTileToNormalizedBuffer((rspf_sint8)0, band, buf);
         break;
      
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyTileToNormalizedBuffer((rspf_uint16)0, band, buf);
         break;
      }
      case RSPF_SINT16:
      {
         copyTileToNormalizedBuffer((rspf_sint16)0, band, buf);
         break;
      }
      case RSPF_UINT32:
      {
         copyTileToNormalizedBuffer((rspf_uint32)0, band, buf);
         break;
      }
      case RSPF_SINT32:
      {
         copyTileToNormalizedBuffer((rspf_sint32)0, band, buf);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::copyTileBandToNormalizedBuffer ERROR:"
            << " Unknown scalar type!"
            << std::endl;
      }
      
   }  // End of "switch(getScalarType())"
   
}

void rspfImageData::copyTileToNormalizedBuffer(rspf_float32* buf)const
{
   
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileToNormalizedBuffer ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileToNormalizedBuffer ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }

   switch(getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         rspf_uint32 upperBound = getWidth()*getHeight()*getNumberOfBands();
         const rspf_float32* inputBuf = static_cast<const rspf_float32*>(getBuf());
         for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            buf[offset] = inputBuf[offset];
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         memmove(buf, getBuf(), getDataSizeInBytes());
         break;
      }
      case RSPF_FLOAT32:
      {
         copyTileToNormalizedBuffer((rspf_float32)0, buf);
         break;
      }
      case RSPF_FLOAT64:
      {
         copyTileToNormalizedBuffer((rspf_float64)0, buf);
         break;
      }
      case RSPF_UINT8:
      {
         copyTileToNormalizedBuffer((rspf_uint8)0, buf);
         break;
      
      }
      case RSPF_SINT8:
      {
         copyTileToNormalizedBuffer((rspf_sint8)0, buf);
         break;
      
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyTileToNormalizedBuffer((rspf_uint16)0, buf);
         break;
      }
      case RSPF_SINT16:
      {
         copyTileToNormalizedBuffer((rspf_sint16)0, buf);
         break;
      }
      case RSPF_UINT32:
      {
         copyTileToNormalizedBuffer((rspf_uint32)0, buf);
         break;
      }
      case RSPF_SINT32:
      {
         copyTileToNormalizedBuffer((rspf_sint32)0, buf);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "NOTICE:  copyTileToNormalizedBuffer not implemented yet"
            << std::endl;
      }
      
   }  // End of "switch(getScalarType())"
}

void rspfImageData::copyTileBandToNormalizedBuffer(rspf_uint32 band,
                                                    rspf_float32* buf)const
{
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileBandToNormalizedBuffer ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyTileBandToNormalizedBuffer ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }

   switch(getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         rspf_uint32 upperBound = getWidth()*getHeight();
         const rspf_float32* inputBuf = static_cast<const rspf_float32*>(getBuf(band));
         for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            buf[offset] = inputBuf[offset];
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         memmove(buf, getBuf(band), getSizePerBandInBytes());
         break;
      }
      case RSPF_FLOAT32:
      {
         copyTileToNormalizedBuffer((rspf_float32)0, band, buf);
         break;
      }
      case RSPF_FLOAT64:
      {
         copyTileToNormalizedBuffer((rspf_float64)0, band, buf);
         break;
      }
      case RSPF_UINT8:
      {
         copyTileToNormalizedBuffer((rspf_uint8)0, band, buf);
         break;
      
      }
      case RSPF_SINT8:
      {
         copyTileToNormalizedBuffer((rspf_sint8)0, band, buf);
         break;
      
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyTileToNormalizedBuffer((rspf_uint16)0, band, buf);
         break;
      }
      case RSPF_SINT16:
      {
         copyTileToNormalizedBuffer((rspf_sint16)0, band, buf);
         break;
      }
      case RSPF_UINT32:
      {
         copyTileToNormalizedBuffer((rspf_uint32)0, band, buf);
         break;
      }
      case RSPF_SINT32:
      {
         copyTileToNormalizedBuffer((rspf_sint32)0, band, buf);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::copyTileBandToNormalizedBuffer ERROR:"
            << " Unknown scalar type"
            << std::endl;
      }

   }  // End of "switch(getScalarType())"
}


void rspfImageData::copyNormalizedBufferToTile(rspf_float64* buf)
{
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }
   
   switch(getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         memmove(getBuf(), buf, getDataSizeInBytes());
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         rspf_uint32 upperBound = getWidth()*getHeight()*getNumberOfBands();
         rspf_float32* inputBuf = static_cast<rspf_float32*>(getBuf());
         for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            inputBuf[offset] = static_cast<rspf_float32>(buf[offset]);
         }
         break;
      }
      case RSPF_FLOAT32:
      {
         copyNormalizedBufferToTile((rspf_float32)0, buf);
         break;
      }
      case RSPF_FLOAT64:
      {
         copyNormalizedBufferToTile((rspf_float64)0, buf);
         break;
      }
      case RSPF_UINT8:
      {
         copyNormalizedBufferToTile((rspf_uint8)0, buf);
         break;
      
      }
      case RSPF_SINT8:
      {
         copyNormalizedBufferToTile((rspf_sint8)0, buf);
         break;
      
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyNormalizedBufferToTile((rspf_uint16)0, buf);
         break;
      }
      case RSPF_SINT16:
      {
         copyNormalizedBufferToTile((rspf_sint16)0, buf);
         break;
      }
      case RSPF_UINT32:
      {
         copyNormalizedBufferToTile((rspf_uint32)0, buf);
         break;
      }
      case RSPF_SINT32:
      {
         copyNormalizedBufferToTile((rspf_sint32)0, buf);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::copyNormalizedBufferToTile\n"
            << "Unknown scalar type!" << std::endl;
      }
      
   } // End of "switch(getScalarType())".
}

void rspfImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                rspf_float64* buf)
{
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }

   if(band < getNumberOfBands())
   {
      switch(getScalarType())
      {
         case RSPF_NORMALIZED_DOUBLE:
         {
            memmove(getBuf(band), buf, getSizePerBandInBytes());
            break;
         }
         case RSPF_NORMALIZED_FLOAT:
         {
            rspf_uint32 upperBound = getWidth()*getHeight();
            rspf_float32* inputBuf = static_cast<rspf_float32*>(getBuf(band));
            for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
            {
               inputBuf[offset] = static_cast<rspf_float32>(buf[offset]);
            }
            break;
         }
         case RSPF_FLOAT32:
         {
            copyNormalizedBufferToTile((rspf_float32)0, band, buf);
            break;
         }
         case RSPF_FLOAT64:
         {
            copyNormalizedBufferToTile((rspf_float64)0, band, buf);
            break;
         }
         case RSPF_UINT8:
         {
            copyNormalizedBufferToTile((rspf_uint8)0, band, buf);
            break;
         
         }
         case RSPF_SINT8:
         {
            copyNormalizedBufferToTile((rspf_sint8)0, band, buf);
            break;
         
         }
         case RSPF_USHORT11:
         case RSPF_UINT16:
         {
            copyNormalizedBufferToTile((rspf_uint16)0, band, buf);
            break;
         }
         case RSPF_SINT16:
         {
            copyNormalizedBufferToTile((rspf_sint16)0, band, buf);
            break;
         }
         case RSPF_UINT32:
         {
            copyNormalizedBufferToTile((rspf_uint32)0, band, buf);
            break;
         }
         case RSPF_SINT32:
         {
            copyNormalizedBufferToTile((rspf_sint32)0, band, buf);
            break;
         }
         case RSPF_SCALAR_UNKNOWN:
         default:
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfImageData::copyNormalizedBufferToTile\n"
               << "Unknown scalar type." << std::endl;
         }
         
      } // End of "switch(getScalarType())".
   }
}

void rspfImageData::copyNormalizedBufferToTile(rspf_float32* buf)
{
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }

   switch(getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         rspf_uint32 upperBound = getWidth()*getHeight()*getNumberOfBands();
         rspf_float64* inputBuf = static_cast<rspf_float64*>(getBuf());
         for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            inputBuf[offset] = buf[offset];
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         memmove(getBuf(), buf, getDataSizeInBytes());
         break;
      }
      case RSPF_FLOAT32:
      {
         copyNormalizedBufferToTile((rspf_float32)0, buf);
         break;
      }
      case RSPF_FLOAT64:
      {
         copyNormalizedBufferToTile((rspf_float64)0, buf);
         break;
      }
      case RSPF_UINT8:
      {
         copyNormalizedBufferToTile((rspf_uint8)0, buf);
         break;
         
      }
      case RSPF_SINT8:
      {
         copyNormalizedBufferToTile((rspf_sint8)0, buf);
         break;
         
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyNormalizedBufferToTile((rspf_uint16)0, buf);
         break;
      }
      case RSPF_SINT16:
      {
         copyNormalizedBufferToTile((rspf_sint16)0, buf);
         break;
      }
      case RSPF_UINT32:
      {
         copyNormalizedBufferToTile((rspf_uint32)0, buf);
         break;
      }
      case RSPF_SINT32:
      {
         copyNormalizedBufferToTile((rspf_sint32)0, buf);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageDatacopyNormalizedBufferToTile\n"
            << "Unknown scalar type!" << std::endl;
      }
      
   }  // End of "switch(getScalarType())".
}

void rspfImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                rspf_float32* buf)
{
   if (!buf)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nNull buffer passed to method!  Returning..." << std::endl;
      return;
   }

   if (getDataObjectStatus() == RSPF_NULL)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfImageData::copyNormalizedBufferToTile ERROR:"
         << "\nThis object is null! Returning..." << std::endl;
      return;
   }

   switch(getScalarType())
   {
      case RSPF_NORMALIZED_DOUBLE:
      {
         rspf_uint32 upperBound = getWidth()*getHeight();
         rspf_float64* inputBuf = static_cast<rspf_float64*>(getBuf(band));
         for(rspf_uint32 offset = 0; offset < upperBound; ++offset)
         {
            inputBuf[offset] = buf[offset];
         }
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      {
         memmove(getBuf(band), buf, getSizePerBandInBytes());
         break;
      }
      case RSPF_FLOAT32:
      {
         copyNormalizedBufferToTile((rspf_float32)0, band, buf);
         break;
      }
      case RSPF_FLOAT64:
      {
         copyNormalizedBufferToTile((rspf_float64)0, band, buf);
         break;
      }
      case RSPF_UINT8:
      {
         copyNormalizedBufferToTile((rspf_uint8)0, band, buf);
         break;
         
      }
      case RSPF_SINT8:
      {
         copyNormalizedBufferToTile((rspf_sint8)0, band, buf);
         break;
         
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyNormalizedBufferToTile((rspf_uint16)0, band, buf);
         break;
      }
      case RSPF_SINT16:
      {
         copyNormalizedBufferToTile((rspf_sint16)0, band, buf);
         break;
      }
      case RSPF_UINT32:
      {
         copyNormalizedBufferToTile((rspf_uint32)0, band, buf);
         break;
      }
      case RSPF_SINT32:
      {
         copyNormalizedBufferToTile((rspf_sint32)0, band, buf);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::copyNormalizedBufferToTile\n"
            << "Unknown scalar type!" << std::endl;
      }

   }  // End of "switch(getScalarType())".
}

rspf_float64 rspfImageData::getMinNormalizedPix() const
{
   if (m_scalarType == RSPF_NORMALIZED_DOUBLE ||
       m_scalarType == RSPF_NORMALIZED_FLOAT)
   {
      return m_minPixelValue[0];  // Already normalized.
   }
   return (1.0 / (m_maxPixelValue[0] - m_minPixelValue[0] + 1.0));
}

std::ostream& rspfImageData::print(std::ostream& out) const
{
   out << getClassName() << "::print:"
       << "\nm_origin:  " << m_origin << "\n";
   if(m_nullPixelValue.size())
   {
      out << "Null values: ";
      
      copy(m_nullPixelValue.begin(),
           m_nullPixelValue.begin() + getNumberOfBands(),
           std::ostream_iterator<rspf_float64>(out, " "));
      out << "\n";
   }
   if(m_minPixelValue.size())
   {
      out << "Min values:  ";
      
      copy(m_minPixelValue.begin(),
           m_minPixelValue.begin() + getNumberOfBands(),
           std::ostream_iterator<rspf_float64>(out, " "));
      out << "\n";
   }
   if(m_maxPixelValue.size())
   {
      out << "Max values:  ";
      
      copy(m_maxPixelValue.begin(),
           m_maxPixelValue.begin() + getNumberOfBands(),
           std::ostream_iterator<rspf_float64>(out, " "));
      out << "\n";
   }
   out << "width:            " << getWidth()
       << "\nheight:           " << getHeight()
       << "\nimage rectangle:  " << getImageRectangle()
       << "\nindexed:          " << m_indexedFlag
       << std::endl;

   return rspfRectilinearDataObject::print(out);
}

void rspfImageData::stretchMinMax()
{
   if ( (getDataObjectStatus() != RSPF_NULL) &&
        (getDataObjectStatus() != RSPF_EMPTY) )
   {
      switch (getScalarType())
      {
         case RSPF_UINT8:
         {
            stretchMinMax(rspf_uint8(0));
            return;
         }  
         case RSPF_SINT8:
         {
            stretchMinMax(rspf_sint8(0));
            return;
         }  
         case RSPF_UINT16:
         case RSPF_USHORT11:
         {
            stretchMinMax(rspf_uint16(0));
            return;
         }  
         case RSPF_SINT16:
         {
            stretchMinMax(rspf_sint16(0));
            return;
         }  
         case RSPF_UINT32:
         {
            stretchMinMax(rspf_uint32(0));
            return;
         }
         case RSPF_SINT32:
         {
            stretchMinMax(rspf_sint32(0));
            return;
         }  
         case RSPF_FLOAT32:
         case RSPF_NORMALIZED_FLOAT:
         {
            stretchMinMax(rspf_float32(0.0));
            return;
         }  
         case RSPF_NORMALIZED_DOUBLE:
         case RSPF_FLOAT64:
         {
            stretchMinMax(rspf_float64(0.0));
            return;
         }  
         case RSPF_SCALAR_UNKNOWN:
         default:
         {
            setDataObjectStatus(RSPF_STATUS_UNKNOWN);
            rspfSetError(getClassName(),
                          rspfErrorCodes::RSPF_ERROR,
                          "rspfImageData::stretchMinMax File %s line %d\n\
Invalid scalar type:  %d",
                          __FILE__,
                          __LINE__,
                          getScalarType());
            break;
         }
      }
   }
}

template <class T> void rspfImageData::stretchMinMax(T /* dummyTemplate */)
{
   const rspf_uint32 BANDS  = getNumberOfBands();
   const rspf_uint32 SPB    = getSizePerBand();

   // scalar min
   const rspf_float64 S_MIN = rspf::defaultMin(getScalarType());

   // scalar max
   const rspf_float64 S_MAX = rspf::defaultMax(getScalarType());

   // scalar null
   const rspf_float64 S_NUL = rspf::defaultNull(getScalarType());

   // scalar range
   const rspf_float64 S_RNG = S_MAX-S_MIN+1.0;

   for(rspf_uint32 band = 0; band < BANDS; ++band)
   {
      T* s = static_cast<T*>(getBuf(band));

      if (s)
      {
         const rspf_float64 T_NUL = m_nullPixelValue[band]; // tile null
         const rspf_float64 T_MIN = m_minPixelValue[band];  // tile min
         const rspf_float64 T_MAX = m_maxPixelValue[band];  // tile max
         const rspf_float64 T_RNG = (T_MAX-T_MIN)+1;           // tile range
         const rspf_float64 SPP = S_RNG / T_RNG; // stretch per pixel
         
         for(rspf_uint32 i = 0; i < SPB; ++i)
         {
            rspf_float64 p = s[i];
            if ( p == T_NUL )
            {
               p = S_NUL;
            }
            else if (p <= T_MIN)
            {
               p = S_MIN;
            }
            else if (p >= T_MAX)
            {
               p = S_MAX;
            }
            else
            {
               // Stretch...
               p = (p - T_MIN + 1.0) * SPP + S_MIN - 1.0;
            }
            s[i] = rspf::round<T>(p);
         }

         // Set the min, max, null:
         m_minPixelValue[band]  = S_MIN;
         m_maxPixelValue[band]  = S_MAX;
         m_nullPixelValue[band] = S_NUL;
         
      }
   } 
}

void rspfImageData::computeAlphaChannel()
{
   if ( getDataObjectStatus() != RSPF_NULL )
   {
      switch (getScalarType())
      {
         case RSPF_UINT8:
         {
            computeAlphaChannel(rspf_uint8(0));
            return;
         }  
         case RSPF_SINT8:
         {
            computeAlphaChannel(rspf_sint8(0));
            return;
         }  
         case RSPF_UINT16:
         case RSPF_USHORT11:
         {
            computeAlphaChannel(rspf_uint16(0));
            return;
         }  
         case RSPF_SINT16:
         {
            computeAlphaChannel(rspf_sint16(0));
            return;
         }  
         case RSPF_UINT32:
         {
            computeAlphaChannel(rspf_uint32(0));
            return;
         }
         case RSPF_SINT32:
         {
            computeAlphaChannel(rspf_sint32(0));
            return;
         }  
         case RSPF_FLOAT32:
         case RSPF_NORMALIZED_FLOAT:
         {
            computeAlphaChannel(rspf_float32(0.0));
            return;
         }  
         case RSPF_NORMALIZED_DOUBLE:
         case RSPF_FLOAT64:
         {
            computeAlphaChannel(rspf_float64(0.0));
            return;
         }  
         case RSPF_SCALAR_UNKNOWN:
         default:
         {
            setDataObjectStatus(RSPF_STATUS_UNKNOWN);
            rspfSetError(getClassName(),
                          rspfErrorCodes::RSPF_ERROR,
                          "rspfImageData::computeAlphaChannel File %s line %d\n\
Invalid scalar type:  %d",
                          __FILE__,
                          __LINE__,
                          getScalarType());
            break;
         }
      }
   }
}

template <class T> void rspfImageData::computeAlphaChannel(T /* dummyTemplate */)
{
   const rspf_uint32 SPB = getSizePerBand();
   const rspf_uint8  ANP = 0;   // Alpha Null Pixel
   const rspf_uint8  AVP = 255; // Alpha Valid Pixel

   // Allocate the alpha channel if not already.
   if (m_alpha.size() != SPB)
   {
      m_alpha.resize(SPB);
   }

   if (getDataObjectStatus() == RSPF_FULL)
   {
      memset( static_cast<void*>(&m_alpha.front()),
              static_cast<int>(AVP),
              static_cast<int>(SPB) );
   }
   else if (getDataObjectStatus() == RSPF_EMPTY)
   {
      // Start with alpha being set to 0.  
      memset( static_cast<void*>(&m_alpha.front()),
              static_cast<int>(ANP),
              static_cast<int>(SPB) );
   }
   else // Partial must check each pixel.
   {
      const rspf_uint32 BANDS = getNumberOfBands();

      std::vector<T> null_pix(BANDS);
      std::vector<const T*> buf(BANDS); // Pointer to pixel data.
      rspf_uint32 band;
      for(band = 0; band < BANDS; ++band)
      {
         buf[band] = static_cast<const T*>(getBuf(band));
         null_pix[band] = static_cast<T>(m_nullPixelValue[band]);
      }
   
      for(rspf_uint32 i = 0; i < SPB; ++i)
      {
         //---
         // Start with alpha being set to 0.  If any band has a valid pixel set
         // to 255 and break from band loop.
         //---
         m_alpha[i] = ANP;
         for(band = 0; band < BANDS; ++band)
         {
            if (buf[band][i] != null_pix[band])
            {
               m_alpha[i] = AVP;
               break;
            }
         }
      }
   }
   
}  //  End: template <class T> void rspfImageData::computeAlphaChannel

rspf_uint32 rspfImageData::getWidth()const
{
   return m_spatialExtents[0];
}

rspf_uint32 rspfImageData::getHeight()const
{
   return m_spatialExtents[1];
}

void rspfImageData::getWidthHeight(rspf_uint32& w, rspf_uint32& h)
{
   w = m_spatialExtents[0];
   h = m_spatialExtents[1];
}

void rspfImageData::setWidth(rspf_uint32 width)
{
   m_spatialExtents[0] = width;
}

void rspfImageData::setHeight(rspf_uint32 height)
{
   m_spatialExtents[1] = height;
}

void rspfImageData::setWidthHeight(rspf_uint32 w, rspf_uint32 h)
{
   m_spatialExtents[0] = w;
   m_spatialExtents[1] = h;
}

void rspfImageData::setOrigin(const rspfIpt& origin)
{
   m_origin = origin;
}

rspf_uint32 rspfImageData::getDataSizeInBytes()const
{
   return getSizeInBytes();
}

void rspfImageData::copyLine(const void* src,
                              rspf_int32 lineNumber,
                              rspf_int32 lineStartSample,
                              rspf_int32 lineStopSample,
                              rspfInterleaveType lineInterleave)
{
   switch(m_scalarType)
   {
      case RSPF_UINT8:
      {
         copyLineTemplate((rspf_uint8)0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
      }
      case RSPF_SINT8:
      {
         copyLineTemplate((rspf_sint8)0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
         
      }
      case RSPF_USHORT11:
      case RSPF_UINT16:
      {
         copyLineTemplate((rspf_uint16)0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
      }
      case RSPF_SINT16:
      {
         copyLineTemplate((rspf_sint16)0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
      }
      case RSPF_UINT32:
      {
         copyLineTemplate((rspf_uint32)0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
      }
      case RSPF_SINT32:
      {
         copyLineTemplate((rspf_sint32)0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
      }
      case RSPF_NORMALIZED_FLOAT:
      case RSPF_FLOAT32:
      {
         copyLineTemplate((rspf_float32)0.0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
      }
      case RSPF_NORMALIZED_DOUBLE:
      case RSPF_FLOAT64:
      {
         copyLineTemplate((rspf_float64)0.0, src, lineNumber, lineStartSample,
                          lineStopSample, lineInterleave);
         break;
      }
      case RSPF_SCALAR_UNKNOWN:
      default:
      {
         // Shouldn't hit this.
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfImageData::copyLine Unsupported scalar type!"
            << std::endl;
         break;
      }
   }
   
} // End: void rspfImageData::copyLine

template <class T>
void rspfImageData::copyLineTemplate(T /* dummyTemplate */,
                                      const void* src,
                                      rspf_int32 lineNumber,
                                      rspf_int32 lineStartSample,
                                      rspf_int32 lineStopSample,
                                      rspfInterleaveType lineInterleave)
{
   if (src)
   {
      const rspfIrect RECT = getImageRectangle();

      // Check for intersect:
      if ( ( lineNumber       >= RECT.ul().y)     &&
           ( lineNumber       <= RECT.lr().y)     &&
           ( lineStartSample  <  lineStopSample)  &&
           ( lineStartSample  <= RECT.lr().x)     &&
           ( lineStopSample   >= RECT.ul().x) )
      {
         const rspf_int32 BANDS = static_cast<rspf_int32>(m_numberOfDataComponents);
         const rspf_int32 START_SAMP =
            (lineStartSample > RECT.ul().x)?lineStartSample:RECT.ul().x;
         const rspf_int32 STOP_SAMP  =
            (lineStopSample  < RECT.lr().x)?lineStopSample:RECT.lr().x;
         const rspf_int32 SAMPS = STOP_SAMP - START_SAMP + 1;

         std::vector<T*> d(BANDS);

         rspf_int32 band;
         for (band = 0; band < BANDS; ++band)
         {
            d[band] = static_cast<T*>(getBuf(band));

            // Position at start sample.
            d[band] +=  (lineNumber - RECT.ul().y) * RECT.width() + (START_SAMP - RECT.ul().x);
         }
         
         if (lineInterleave == RSPF_BIP)
         {
            const T* S = static_cast<const T*>(src); // Source buffer:
            
            // Position at start sample.
            S += (START_SAMP - lineStartSample) * BANDS;
            
            rspf_int32 srcOffset = 0;
            for (rspf_int32 samp = 0; samp < SAMPS; ++samp)
            {
               for (band = 0; band < BANDS; ++band)
               {
                  d[band][samp] = S[srcOffset++];
               }
            }
         }
         else
         {
            const rspf_int32 W = lineStopSample - lineStartSample + 1;
            std::vector<const T*> S(BANDS);
            for (band = 0; band < BANDS; ++band)
            {
               S[band] = static_cast<const T*>(src) + (START_SAMP - lineStartSample);
               if (band)
               {
                  S[band] += band * W; // Move to line.
               }
            }

            for (band = 0; band < BANDS; ++band)
            {
               for (rspf_int32 samp = 0; samp < SAMPS; ++samp)
               {
                  d[band][samp] = S[band][samp];
               }
            }
         }
         
      } // intersect check
      
   } // if (src)
   
} // End: template <class T> void rspfImageData::copyLineTemplate

void rspfImageData::setIndexedFlag(bool flag)
{
   m_indexedFlag = flag;
}

bool rspfImageData::getIndexedFlag() const
{
   return m_indexedFlag;
}

bool rspfImageData::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   bool result = rspfRectilinearDataObject::saveState(kwl, prefix);
   rspfString null_pixels;
   rspfString min_pixels;
   rspfString max_pixels;
   rspf::toSimpleStringList(null_pixels, m_nullPixelValue);
   rspf::toSimpleStringList(min_pixels, m_minPixelValue);
   rspf::toSimpleStringList(max_pixels, m_maxPixelValue);
   
   kwl.add(prefix, "null_pixels", null_pixels, true);
   kwl.add(prefix, "min_pixels", min_pixels, true);
   kwl.add(prefix, "max_pixels", max_pixels, true);
   
   rspfString alpha;
   rspf::toSimpleStringList(alpha, m_alpha);
   kwl.add(prefix, "alpha", alpha, true);
   kwl.add(prefix, "indexed", m_indexedFlag, true);
   kwl.add(prefix, "origin", m_origin.toString(), true);
   return result;
   
}

bool rspfImageData::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool result = rspfRectilinearDataObject::loadState(kwl, prefix);
   m_spatialExtents.resize(2);
   if(result)
   {
      const char* null_pixels = kwl.find(prefix, "null_pixels");
      const char* min_pixels = kwl.find(prefix, "min_pixels");
      const char* max_pixels = kwl.find(prefix, "max_pixels");
      const char* alpha = kwl.find(prefix, "alpha");
      const char* origin = kwl.find(prefix, "origin");
      const char* indexed = kwl.find(prefix, "indexed");
      rspfString rectString = kwl.find(prefix, "rect");
      const char* numberOfBands = kwl.find(prefix, "number_bands");
      m_nullPixelValue.clear();
      m_minPixelValue.clear();
      m_maxPixelValue.clear();
      m_alpha.clear();
      if(null_pixels)
      {
         if(!rspf::toSimpleVector(m_nullPixelValue, rspfString(null_pixels)))
         {
            return false;
         }
      }
      if(min_pixels)
      {
         if(!rspf::toSimpleVector(m_minPixelValue, rspfString(min_pixels)))
         {
            return false;
         }
      }
      if(max_pixels)
      {
         if(!rspf::toSimpleVector(m_maxPixelValue, rspfString(max_pixels)))
         {
            return false;
         }
      }
      if(alpha)
      {
         if(!rspf::toSimpleVector(m_alpha, rspfString(alpha)))
         {
            return false;
         }
      }
      if(origin)
      {
         m_origin.toPoint(origin);
      }
      m_indexedFlag = false;
      if(indexed)
      {
         m_indexedFlag = rspfString(indexed).toBool();
      }
      if(!rectString.empty())
      {
        rspfIrect rect;

        if(rect.toRect(rectString))
        {
          setImageRectangle(rect);
        }
      }
      if(numberOfBands)
      {
        rspf_uint32 nBands = rspfString(numberOfBands).toUInt32();
        setNumberOfDataComponents(nBands);
        if(m_nullPixelValue.empty()||
          m_minPixelValue.empty()||
          m_maxPixelValue.empty())
          {
            initializeDefaults();
          }
      }
   }
   
   return result;
}                     
