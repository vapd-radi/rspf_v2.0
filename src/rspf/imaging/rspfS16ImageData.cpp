//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Class definition of rspfS16ImageData.  Specialized image data object for
// signed short data.
//
//*************************************************************************
// $Id: rspfS16ImageData.cpp 16052 2009-12-08 22:20:40Z dburken $

#include <rspf/imaging/rspfS16ImageData.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>

RTTI_DEF1(rspfS16ImageData, "rspfS16ImageData", rspfImageData)
   
const rspfNormalizedS16RemapTable rspfS16ImageData::m_remapTable;

rspfS16ImageData::rspfS16ImageData()
   :
      rspfImageData()
{
   m_scalarType = RSPF_SINT16;
}

rspfS16ImageData::rspfS16ImageData(rspfSource* source,
                                     rspf_uint32 bands)
   :
      rspfImageData(source,
                     RSPF_SINT16,
                     bands)
{
}

rspfS16ImageData::rspfS16ImageData(rspfSource* source,
                                     rspf_uint32 bands,
                                     rspf_uint32 width,
                                     rspf_uint32 height)
   :
      rspfImageData(source,
                     RSPF_SINT16,
                     bands,
                     width,
                     height)
{
}

rspfS16ImageData::rspfS16ImageData(const rspfS16ImageData &rhs)
   :
      rspfImageData(rhs)
{
}

rspfS16ImageData::~rspfS16ImageData()
{
}

rspfObject* rspfS16ImageData::dup()const
{
   return new rspfS16ImageData(*this);
}

rspfDataObjectStatus rspfS16ImageData::validate() const
{
   if (getBuf() == NULL)
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
      rspf_sint16 np = (rspf_sint16)m_nullPixelValue[band];
      const rspf_sint16* p  = getSshortBuf(band);
      for (rspf_uint32 i=0; i<BOUNDS; i++)
      {
         // check if the band is null
         if (p[i] != np) count++;         
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

void rspfS16ImageData::getNormalizedFloat(rspf_uint32 offset,
                                           rspf_uint32 bandNumber,
                                           float& result)const
{
   if( (getDataObjectStatus() != RSPF_NULL) && isValidBand(bandNumber) )
   {
      const rspf_sint16* sourceBuf = getSshortBuf(bandNumber);
      result =
         static_cast<float>(m_remapTable.normFromPix(sourceBuf[offset]));
   }
}

void rspfS16ImageData::setNormalizedFloat(rspf_uint32 offset,
                                           rspf_uint32 bandNumber,
                                           float inputValue)
{
   if( (getDataObjectStatus() != RSPF_NULL) &&  isValidBand(bandNumber) )
   {
      rspf_sint16* sourceBuf = getSshortBuf(bandNumber);
      sourceBuf[offset]
         = static_cast<rspf_sint16>(m_remapTable.pixFromNorm(inputValue));
   }
}

void
rspfS16ImageData::convertToNormalizedFloat(rspfImageData* result)const
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
      for(rspf_uint32 bandCount = 0;
          bandCount < m_numberOfDataComponents;
          ++bandCount)
      {
         const rspf_sint16* sourceBuf = getSshortBuf(bandCount);
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

void rspfS16ImageData::convertToNormalizedDouble(rspfImageData* result)const
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
      for(rspf_uint32 bandCount = 0;
          bandCount < m_numberOfDataComponents;
          ++bandCount)
      {
         const rspf_sint16* sourceBuf = getSshortBuf(bandCount);
         double* resultBuf = static_cast<double*>(result->getBuf(bandCount));
         for(rspf_uint32 counter = 0; counter <  size; ++counter)
         {
            resultBuf[counter] = m_remapTable.normFromPix(sourceBuf[counter]);
         }
      }
   }
}

void rspfS16ImageData::unnormalizeInput(rspfImageData* normalizedInput)
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
            rspf_sint16* resultBuf = getSshortBuf(bandCount);
            for(counter = 0; counter <  size; ++counter)
            {
               resultBuf[counter]
                  = static_cast<rspf_sint16>(m_remapTable.
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
            rspf_sint16* resultBuf = getSshortBuf(bandCount);
            for(counter = 0; counter <  size; ++counter)
            {
               resultBuf[counter]
                  = static_cast<rspf_sint16>(m_remapTable.
                                              pixFromNorm(sourceBuf[counter]));
            }
         }
      }
   }
}

double rspfS16ImageData::computeMeanSquaredError(double meanValue,
                                                  rspf_uint32 bandNumber)
{
   double result = -1; // invalid MSE
   rspf_uint32 index = 0;
   double delta = 0;
   rspf_uint32 validPixelCount=0;
   
   rspf_sint16* buffer = getSshortBuf(bandNumber);
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


double rspfS16ImageData::computeAverageBandValue(rspf_uint32 bandNumber)
{
   double result = 0.0;
   rspf_uint32 index = 0;
   rspf_uint32 validPixelCount=0;
   
   rspf_sint16* buffer = getSshortBuf(bandNumber);
   if(buffer)
   {
      rspf_uint32 bounds = getSizePerBand();
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

void rspfS16ImageData::setValue(rspf_int32 x, rspf_int32 y, double color)
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
      
      for(rspf_uint32 band = 0; band < m_numberOfDataComponents; band++)
      {
         rspf_sint16* buf = getSshortBuf(band)+offset;
         *buf = (rspf_sint16)color;
      }
   }
}

void rspfS16ImageData::fill(rspf_uint32 band, double value)
{
   void* s         = getBuf(band);

   if (s == NULL) return; // nothing to do...

   rspf_uint32 size_in_pixels = getSizePerBand();
   rspf_sint16* p = getSshortBuf(band);

   for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = (rspf_sint16)value;

   // Set the status to unknown since we don't know about the other bands.
   setDataObjectStatus(RSPF_STATUS_UNKNOWN);
}


bool rspfS16ImageData::isNull(rspf_uint32 offset)const
{
   for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)  
   {
      const rspf_sint16* buf = getSshortBuf(band)+offset;
      
      if((*buf) != m_nullPixelValue[band])
      {
         return false;
      }
   }

   return true;
}

void rspfS16ImageData::setNull(rspf_uint32 offset)
{
   for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)  
   {
      rspf_sint16* buf = getSshortBuf(band)+offset;
      *buf       = static_cast<rspf_sint16>(m_nullPixelValue[band]);
   }
}

void rspfS16ImageData::copyTileToNormalizedBuffer(double* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfS16ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }

   const rspf_uint32 SIZE = getSizePerBand();
   
   if(SIZE > 0)
   {   
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         const rspf_sint16* s = getSshortBuf(band);  // source
         double* d = buf + (band*SIZE);   // destination
         
         for(rspf_uint32 index = 0; index < SIZE; ++index)
         {
            d[index] = m_remapTable.
               normFromPix(static_cast<rspf_int32>(s[index]));
         }
      }
   }
}

void rspfS16ImageData::copyTileToNormalizedBuffer(rspf_uint32 band,
                                                   double* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                     rspfErrorCodes::RSPF_ERROR,
                    "rspfS16ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   if(!getBuf(band)) return;
   
   const rspf_uint32 SIZE = getSizePerBand();
   
   if(SIZE)
   {
      const rspf_float64 RANGE = (getMaxPix(band)-getMinPix(band)+1);
      
      const rspf_sint16* s = getSshortBuf(band);  // source
      double* d = buf;   // destination
      
      for(rspf_uint32 index = 0; index < SIZE; ++index)
      {
         rspf_float64 p = s[index];
         
         if (p == m_nullPixelValue[band])
         {
            d[index] = 0.0;
         }
         else if (p >= m_maxPixelValue[band])
         {
            d[index] = 1.0;
         }
         else
         {
            //---
            // Normalize...
            // Note that this will shift any negatives to positive prior
            // to dividing.
            //---
            d[index] =
               ((p-m_minPixelValue[band]+1) / RANGE);
         }
      }
   }
}

void rspfS16ImageData::copyNormalizedBufferToTile(double* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfS16ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); band++)
      {
         double* s = buf + (band*size); // source
         rspf_sint16* d = getSshortBuf(band); // destination
         
         for(rspf_uint32 index = 0; index <  size; index++)
         {
            d[index] = m_remapTable.pixFromNorm(s[index]);
         }
      }
   }   
}

void rspfS16ImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                   double* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfS11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if((size > 0)&&getBuf(band))
   {
      double* s = buf; // source
      rspf_sint16* d = getSshortBuf(band); // destination
      
      for(rspf_uint32 index = 0; index <  size; ++index)
      {
         *d = m_remapTable.pixFromNorm(*s);
         ++d;
         ++s;
      }
   }
}

void rspfS16ImageData::copyTileToNormalizedBuffer(float* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfS16ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         const rspf_sint16* s = getSshortBuf(band);  // source
         float* d = buf + (band*size);   // destination
         
         for(rspf_uint32 index = 0; index < size; ++index)
         {
            d[index] = m_remapTable.
               normFromPix(static_cast<rspf_int32>(s[index]));
         }
      }
   }
}

void rspfS16ImageData::copyTileToNormalizedBuffer(rspf_uint32 band,
                                                   float* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfS16ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   if(!getBuf(band)) return;
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      const rspf_sint16* s = getSshortBuf(band);  // source
      float* d = buf;   // destination
      
      for(rspf_uint32 index = 0; index < size; ++index)
      {
         *d = m_remapTable.normFromPix(static_cast<rspf_int32>(*s));
      }
   }
}

void rspfS16ImageData::copyNormalizedBufferToTile(float* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfS16ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         float* s = buf + (band*size); // source
         rspf_sint16* d = getSshortBuf(band); // destination
         
         for(rspf_uint32 index = 0; index <  size; ++index)
         {
            d[index] = m_remapTable.pixFromNorm(s[index]);
         }
      }
   }
}

void rspfS16ImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                   float* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfS16ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if((size > 0)&&getBuf(band))
   {
      float* s = buf; // source
      rspf_sint16* d = getSshortBuf(band); // destination
      
      for(rspf_uint32 index = 0; index <  size; ++index)
      {
         *d = m_remapTable.pixFromNorm(*s);
         ++d;
         ++s;
      }
   }
}

