//*******************************************************************
// 
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Class definition of rspfU8ImageData.  Specialized image data object for
// unsigned char data.
//
// NOTE:  This object is optimized for unsigned char data and assumes the
//        following:  null pixel value  = 0.0
//                    min  pixel value  = 1.0
//                    max  pixel value  = 255.0
//
//        If you want anything else use the less efficient rspfImageData.
//
//*************************************************************************
// $Id: rspfU8ImageData.cpp 16052 2009-12-08 22:20:40Z dburken $

#include <cstring>  // for memset function
using namespace std;

#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>

RTTI_DEF1(rspfU8ImageData, "rspfU8ImageData", rspfImageData)

const rspfNormalizedU8RemapTable rspfU8ImageData::m_remapTable;
   
rspfU8ImageData::rspfU8ImageData(rspfSource* source,
                                   rspf_uint32 bands)
   :
      rspfImageData(source,
                     RSPF_UCHAR,
                     bands)
{
}

rspfU8ImageData::rspfU8ImageData(rspfSource* source,
                                   rspf_uint32 bands,
                                   rspf_uint32 width,
                                   rspf_uint32 height)
   :
      rspfImageData(source,
                     RSPF_UCHAR,
                     bands,
                     width,
                     height)
{
}

rspfU8ImageData::~rspfU8ImageData()
{
}

rspfObject* rspfU8ImageData::dup()const
{
   return new rspfU8ImageData(*this);
}

void rspfU8ImageData::fill(double value)
{
   rspfImageData::fill(value);
}

rspfDataObjectStatus rspfU8ImageData::validate() const
{
   if (m_dataBuffer.size() == 0)
   {
      setDataObjectStatus(RSPF_NULL);
      return RSPF_NULL;
   }
   
   rspf_uint32 count = 0;
   const rspf_uint32 SIZE = getSize();
   const rspf_uint32 BOUNDS = getSizePerBand();
   const rspf_uint32 NUMBER_OF_BANDS = getNumberOfBands();
   
   for(rspf_uint32 band = 0; band < NUMBER_OF_BANDS; ++band)
   {
      const rspf_uint8* p  = getUcharBuf(band);
      rspf_uint8 np = (rspf_uint8)getNullPix(band);
      for (rspf_uint32 i=0; i<BOUNDS; ++i)
      {
         // check if the band is null
         if (p[i] != np) ++count;         
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

void rspfU8ImageData::getNormalizedFloat(rspf_uint32 offset,
                                          rspf_uint32 bandNumber,
                                          float& result)const
{
   // make sure that the types and width and height are good.
   if( (getDataObjectStatus() == RSPF_NULL) && isValidBand(bandNumber) )
   {
      return;
   }
   
   const rspf_uint8* sourceBuf = getUcharBuf(bandNumber);
   result = static_cast<float>(m_remapTable.normFromPix(sourceBuf[offset]));
}

void rspfU8ImageData::setNormalizedFloat(rspf_uint32 offset,
                                          rspf_uint32 bandNumber,
                                          float inputValue)
{
   // make sure that the types and width and height are
   // good.
   if((getDataObjectStatus() == RSPF_NULL)&&
      (bandNumber < getNumberOfBands()))
   {
      return;
   }

   rspf_uint8* sourceBuf = getUcharBuf(bandNumber);
   sourceBuf[offset]
      = static_cast<rspf_uint8>(m_remapTable.pixFromNorm(inputValue));
}

void rspfU8ImageData::convertToNormalizedFloat(rspfImageData* result)const
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

   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 bandCount = 0; bandCount < m_numberOfDataComponents; ++bandCount)
      {
         const rspf_uint8* sourceBuf = getUcharBuf(bandCount);
         float* resultBuf = static_cast<float*>(result->getBuf(bandCount));
         for(rspf_uint32 counter = 0; counter <  size; ++counter)
         {
            resultBuf[counter]
               = static_cast<float>(m_remapTable.
                                    normFromPix(sourceBuf[counter]));
         }
      }
   }
}

void rspfU8ImageData::convertToNormalizedDouble(rspfImageData* result)const
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

   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 bandCount = 0; bandCount < m_numberOfDataComponents; ++bandCount)
      {
         const rspf_uint8* sourceBuf = getUcharBuf(bandCount);
         double* resultBuf = static_cast<double*>(result->getBuf(bandCount));
         for(rspf_uint32 counter = 0; counter <  size; ++counter)
         {
            resultBuf[counter] = m_remapTable.normFromPix(sourceBuf[counter]);
         }
      }
   }
}

void rspfU8ImageData::unnormalizeInput(rspfImageData* normalizedInput)
{
   if((normalizedInput->getScalarType() != RSPF_NORMALIZED_FLOAT) &&
      (normalizedInput->getScalarType() != RSPF_NORMALIZED_DOUBLE) )
   {
      //ERROR
      return;
   }
   
   rspf_uint32 counter = 0;
   rspf_uint32 bandCount = 0;
   rspf_uint32 size = getSizePerBand();
   rspfScalarType scalarType = normalizedInput->getScalarType();

   if(size > 0)
   {
      if(scalarType == RSPF_NORMALIZED_FLOAT)
      {
         for(bandCount = 0; bandCount < m_numberOfDataComponents; ++bandCount)
         {
            float* sourceBuf =
               static_cast<float*>(normalizedInput->getBuf(bandCount));
            rspf_uint8* resultBuf = getUcharBuf(bandCount);
            for(counter = 0; counter <  size; ++counter)
            {
               resultBuf[counter]
                  = static_cast<rspf_uint8>(m_remapTable.
                                       pixFromNorm(sourceBuf[counter]));
            }
         }
      }
      else // Double
      {
         for(bandCount = 0; bandCount < m_numberOfDataComponents; ++bandCount)
         {
            double* sourceBuf =
               static_cast<double*>(normalizedInput->getBuf(bandCount));
            rspf_uint8* resultBuf = getUcharBuf(bandCount);
            for(counter = 0; counter <  size; ++counter)
            {
               resultBuf[counter]
                  = static_cast<rspf_uint8>(m_remapTable.
                                       pixFromNorm(sourceBuf[counter]));
            }
         }
      }
   }   
}

double rspfU8ImageData::computeMeanSquaredError(double meanValue,
                                                 rspf_uint32 bandNumber)
{
   double result = -1; // invalid MSE
   rspf_uint32 index = 0;
   double delta = 0;
   rspf_uint32 validPixelCount=0;
   
   rspf_uint8* buffer = getUcharBuf(bandNumber);
   if(buffer)
   {
      rspf_uint32 bounds = getWidth()*getHeight();
      for(index = 0; index < bounds; ++index)
      {
         if(!isNull(index))
         {
            delta = *buffer - meanValue;
            result += (delta*delta);
            ++validPixelCount;
         }
         ++buffer;
      }
      if(validPixelCount > 0)
      {
         result /= validPixelCount;
      }
   }

   return result;
}

double rspfU8ImageData::computeAverageBandValue(rspf_uint32 bandNumber)
{
   double result = 0.0;
   rspf_uint32 index = 0;
   rspf_uint32 validPixelCount=0;
   
   rspf_uint8* buffer = getUcharBuf(bandNumber);
   if(buffer)
   {
      rspf_uint32 bounds = getWidth()*getHeight();
      for(index = 0; index < bounds; ++index)
      {
         if(!isNull(index))
         {
            result += *buffer;
            ++validPixelCount;
         }
         ++buffer;
      }
      if(validPixelCount > 0)
      {
         result /= validPixelCount;
      }
   }

   return result;
}

void rspfU8ImageData::setValue(rspf_int32 x, rspf_int32 y, double color)
{
   if(getBuf() != NULL && isWithin(x, y))
   {
      //***
      // Compute the offset into the buffer for (x,y).  This should always
      // come out positive.
      //***
      rspf_uint32 ux = static_cast<rspf_uint32>(x - m_origin.x);
      rspf_uint32 uy = static_cast<rspf_uint32>(y - m_origin.y);

      rspf_uint32 offset = uy * m_spatialExtents[0] + ux;
      
      for(rspf_uint32 band = 0; band < m_numberOfDataComponents; ++band)
      {
         rspf_uint8* buf = getUcharBuf(band)+offset;
         *buf = (rspf_uint8)color;
      }
   }
}

void rspfU8ImageData::fill(rspf_uint32 band, double value)
{
   void* s         = getBuf(band);

   if (s == NULL) return; // nothing to do...

   rspf_uint32 size_in_pixels = getSizePerBand();
   rspf_uint8* p = getUcharBuf(band);
   rspf_uint8 np = static_cast<rspf_uint8>(value);

   for (rspf_uint32 i=0; i<size_in_pixels; ++i) p[i] = np;

   // Set the status to unknown since we don't know about the other bands.
   setDataObjectStatus(RSPF_STATUS_UNKNOWN);
}


bool rspfU8ImageData::isNull(rspf_uint32 offset)const
{
   for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)  
   {
      const rspf_uint8* buf = getUcharBuf(band)+offset;
      
      if((*buf) != m_nullPixelValue[band])
      {
         return false;
      }
   }

   return true;
}

void rspfU8ImageData::setNull(rspf_uint32 offset)
{
   for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)  
   {
      rspf_uint8* buf = (getUcharBuf(band))+offset;
      *buf       = (rspf_uint8)m_nullPixelValue[band];
   }
}

void rspfU8ImageData::copyTileToNormalizedBuffer(double* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   rspf_uint32 size = getSizePerBand();
   if(!getBuf())
   {
      memset(buf, '\0', size);
      return;
   }
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         const rspf_uint8*  s = getUcharBuf(band);   // source
         double* d = buf + (band*size);   // destination
         if(s&&d)
         {
            for(rspf_uint32 index = 0; index < size; ++index)
            {
               d[index] = m_remapTable.
                          normFromPix(static_cast<rspf_int32>(s[index]));
            }
         }
      }
   }
}

void rspfU8ImageData::copyTileToNormalizedBuffer(float* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   rspf_uint32 size = getSizePerBand();
   if(!getBuf())
   {
      memset(buf, '\0', size);
      return;
   }
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         const rspf_uint8*  s = getUcharBuf(band);   // source
         float* d = buf + (band*size);   // destination
         if(s&&d)
         {
            for(rspf_uint32 index = 0; index < size; ++index)
            {
               d[index] = m_remapTable.
                          normFromPix(static_cast<rspf_int32>(s[index]));
            }
         }
      }
   }
}

void rspfU8ImageData::copyTileToNormalizedBuffer(rspf_uint32 band,
                                                  double* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   rspf_uint32 size = getSizePerBand();
   if(!getBuf(band))
   {
      memset(buf, '\0', size);
      return;
   }
   
   if(size > 0)
   {
      const rspf_uint8*  s = getUcharBuf(band);   // source
      double* d = buf;   // destination
      for(rspf_uint32 index = 0; index < size; ++index)
      {
         *d = m_remapTable.
              normFromPix(static_cast<rspf_int32>(*s));
         ++d;
         ++s;
      }
   }
}

void rspfU8ImageData::copyTileToNormalizedBuffer(rspf_uint32 band,
                                                  float* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   rspf_uint32 size = getSizePerBand();
   if(!getBuf(band))
   {
      memset(buf, '\0', size);
      return;
   }
   
   if(size > 0)
   {
      const rspf_uint8*  s = getUcharBuf(band);   // source
      float* d = buf;   // destination
      for(rspf_uint32 index = 0; index < size; ++index)
      {
         *d = m_remapTable.
              normFromPix(static_cast<rspf_int32>(*s));
         ++d;
         ++s;
      }
   }
}

void rspfU8ImageData::copyNormalizedBufferToTile(double* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   if(!getBuf()) return;
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         double* s = buf + (band*size);  // source
         rspf_uint8*  d = getUcharBuf(band);  // destination

         for(rspf_uint32 index = 0; index <  size; ++index)
         {
            d[index] = m_remapTable.pixFromNorm(s[index]);
         }
      }
   }
}

void rspfU8ImageData::copyNormalizedBufferToTile(float* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   if(!getBuf()) return;
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         float* s = buf + (band*size);  // source
         rspf_uint8*  d = getUcharBuf(band);  // destination

         for(rspf_uint32 index = 0; index < size; ++index)
         {
            d[index] = m_remapTable.pixFromNorm(s[index]);
         }
      }
   }
}

void rspfU8ImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                  double* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   if(!getBuf(band)) return;
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      double* s = buf;  // source
      rspf_uint8*  d = getUcharBuf(band);  // destination
      
      for(rspf_uint32 index = 0; index <  size; ++index)
      {
         d[index] = m_remapTable.pixFromNorm(s[index]);
      }
   }
}

void rspfU8ImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                  float* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU8ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   if(!getBuf(band)) return;
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      float* s = buf;  // source
      rspf_uint8*  d = getUcharBuf(band);  // destination
      
      for(rspf_uint32 index = 0; index <  size; ++index)
      {
         d[index] = m_remapTable.pixFromNorm(s[index]);
      }
   }
}

