//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Class definition of rspfU11ImageData.  Specialized image data object for
// unsigned short data with an 11 bit depth.
//
// NOTE:  This object is optimized for unsigned short data and assumes the
//        following:  null pixel value  = 0.0
//                    min  pixel value  = 1.0
//                    max  pixel value  = 2047.0  (2^11 - 1)
//
//        If you want anything else use the less efficient rspfImageData.
//
//*************************************************************************
// $Id: rspfU11ImageData.cpp 16052 2009-12-08 22:20:40Z dburken $

#include <rspf/imaging/rspfU11ImageData.h>
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/base/rspfHistogram.h>

RTTI_DEF1(rspfU11ImageData, "rspfU11ImageData", rspfImageData)

const rspfNormalizedU11RemapTable rspfU11ImageData::m_remapTable;
   
rspfU11ImageData::rspfU11ImageData()
   :
      rspfImageData()
{
   m_scalarType = RSPF_USHORT11;
}

rspfU11ImageData::rspfU11ImageData(rspfSource* source,
                                     rspf_uint32 bands)
   :
      rspfImageData(source,
                     RSPF_USHORT11,
                     bands)
{
}

rspfU11ImageData::rspfU11ImageData(rspfSource* source,
                                     rspf_uint32 bands,
                                     rspf_uint32 width,
                                     rspf_uint32 height)
   :
      rspfImageData(source,
                     RSPF_USHORT11,
                     bands,
                     width,
                     height)
{
}

rspfU11ImageData::rspfU11ImageData(const rspfU11ImageData &rhs)
   :
      rspfImageData(rhs)
{}

rspfU11ImageData::~rspfU11ImageData()
{
}

rspfObject* rspfU11ImageData::dup()const
{
   return new rspfU11ImageData(*this);
}

rspfDataObjectStatus rspfU11ImageData::validate() const
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
      const rspf_uint16* p  = getUshortBuf(band);
      for (rspf_uint32 i=0; i<BOUNDS; ++i)
      {
         // check if the band is null
         if (p[i] != 0) ++count;         
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

void rspfU11ImageData::getNormalizedFloat(rspf_uint32 offset,
                                           rspf_uint32 bandNumber,
                                           float& result)const
{
   if( (getDataObjectStatus() != RSPF_NULL) && isValidBand(bandNumber) )
   {
      const rspf_uint16* sourceBuf = getUshortBuf(bandNumber);
      result =
         static_cast<float>(m_remapTable.normFromPix(sourceBuf[offset]));
   }
}

void rspfU11ImageData::setNormalizedFloat(rspf_uint32 offset,
                                           rspf_uint32 bandNumber,
                                           float inputValue)
{
   if( (getDataObjectStatus() != RSPF_NULL) &&  isValidBand(bandNumber) )
   {
      rspf_uint16* sourceBuf = getUshortBuf(bandNumber);
      sourceBuf[offset]
         = static_cast<rspf_uint16>(m_remapTable.pixFromNorm(inputValue));
   }
}

void
rspfU11ImageData::convertToNormalizedFloat(rspfImageData* result)const
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
         const rspf_uint16* sourceBuf = getUshortBuf(bandCount);
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

void rspfU11ImageData::convertToNormalizedDouble(rspfImageData* result)const
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
         const rspf_uint16* sourceBuf = getUshortBuf(bandCount);
         double* resultBuf = static_cast<double*>(result->getBuf(bandCount));
         for(rspf_uint32 counter = 0; counter <  size; ++counter)
         {
            resultBuf[counter] = m_remapTable.normFromPix(sourceBuf[counter]);
         }
      }
   }

}

void rspfU11ImageData::unnormalizeInput(rspfImageData* normalizedInput)
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
            rspf_uint16* resultBuf = getUshortBuf(bandCount);
            for(counter = 0; counter <  size; ++counter)
            {
               resultBuf[counter]
                  = static_cast<rspf_uint16>(m_remapTable.
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
            rspf_uint16* resultBuf = getUshortBuf(bandCount);
            for(counter = 0; counter <  size; ++counter)
            {
               resultBuf[counter]
                  = static_cast<rspf_uint16>(m_remapTable.
                                              pixFromNorm(sourceBuf[counter]));
            }
         }
      }
   }   
}

double rspfU11ImageData::computeMeanSquaredError(double meanValue,
                                                  rspf_uint32 bandNumber)
{
   double result = -1; // invalid MSE
   rspf_uint32 index = 0;
   double delta = 0;
   rspf_uint32 validPixelCount=0;
   
   rspf_uint16* buffer = getUshortBuf(bandNumber);
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

double rspfU11ImageData::computeAverageBandValue(rspf_uint32 bandNumber)
{
   double result = 0.0;
   rspf_uint32 index = 0;
   rspf_uint32 validPixelCount=0;
   
   rspf_uint16* buffer = getUshortBuf(bandNumber);
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

void rspfU11ImageData::setValue(long x, long y, double color)
{
   if(m_dataBuffer.size() > 0 && isWithin(x, y))
   {
      //***
      // Compute the offset into the buffer for (x,y).  This should always
      // come out positive.
      //***
      rspf_uint32 ux = static_cast<rspf_uint32>(x - m_origin.x);
      rspf_uint32 uy = static_cast<rspf_uint32>(y - m_origin.y);
      
      rspf_uint32 offset = uy * m_spatialExtents[0] + ux;
      
      for(rspf_uint32 band = 0; offset < m_dataBuffer.size() && // prevent buffer overrun
                                 band < m_numberOfDataComponents; band++)
      {
         rspf_uint16* buf = getUshortBuf(band)+offset;
         *buf = (rspf_uint16)color;
      }
   }
}

void rspfU11ImageData::fill(rspf_uint32 band, double value)
{
   void* s         = getBuf(band);

   if (s == NULL) return; // nothing to do...

   rspf_uint32 size_in_pixels = getSizePerBand();
   rspf_uint16* p = getUshortBuf(band);
   rspf_uint16 np = static_cast<rspf_uint16>(value);

   for (rspf_uint32 i=0; i<size_in_pixels; i++) p[i] = np;

   // Set the status to unknown since we don't know about the other bands.
   setDataObjectStatus(RSPF_STATUS_UNKNOWN);
}


bool rspfU11ImageData::isNull(rspf_uint32 offset)const
{
   for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)  
   {
      const rspf_uint16* buf = getUshortBuf(band)+offset;
      
      if((*buf) != m_nullPixelValue[band])
      {
         return false;
      }
   }

   return true;
}

void rspfU11ImageData::setNull(rspf_uint32 offset)
{
   rspf_uint32 bands = getNumberOfBands();
   for(rspf_uint32 band = 0; band < bands; ++band)  
   {
      rspf_uint16* buf = getUshortBuf(band)+offset;
      *buf          = (rspf_uint16)m_nullPixelValue[band];
   }
}


void rspfU11ImageData::copyTileToNormalizedBuffer(double* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); ++band)
      {
         const rspf_uint16* s = getUshortBuf(band);  // source
         double* d = buf + (band*size);   // destination

         for(rspf_uint32 index = 0; index < size; ++index)
         {
            d[index] = m_remapTable.
               normFromPix(static_cast<rspf_int32>(s[index]));
         }

      }
   }
}

void rspfU11ImageData::copyTileToNormalizedBuffer(rspf_uint32 band,
                                                   double* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   if(!getBuf(band)) return;
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      const rspf_uint16* s = getUshortBuf(band);  // source
      double* d = buf;   // destination
      
      for(rspf_uint32 index = 0; index < size; index++)
      {
         *d = m_remapTable.normFromPix(static_cast<rspf_int32>(*s));
      }
   }
}

void rspfU11ImageData::copyNormalizedBufferToTile(double* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
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
         rspf_uint16* d = getUshortBuf(band); // destination

         for(rspf_uint32 index = 0; index <  size; index++)
         {
            d[index] = m_remapTable.pixFromNorm(s[index]);
         }
      }
   }
}

void rspfU11ImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                   double* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if((size > 0)&&getBuf(band))
   {
      double* s = buf; // source
      rspf_uint16* d = getUshortBuf(band); // destination
      
      for(rspf_uint32 index = 0; index <  size; ++index)
      {
         *d = m_remapTable.pixFromNorm(*s);
         ++d;
         ++s;
      }
   }
}


void rspfU11ImageData::copyTileToNormalizedBuffer(float* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      for(rspf_uint32 band = 0; band < getNumberOfBands(); band++)
      {
         const rspf_uint16* s = getUshortBuf(band);  // source
         float* d = buf + (band*size);   // destination

         for(rspf_uint32 index = 0; index < size; index++)
         {
            d[index] = m_remapTable.
                       normFromPix(static_cast<rspf_int32>(s[index]));
         }
      }
   }
}

void rspfU11ImageData::copyTileToNormalizedBuffer(rspf_uint32 band,
                                                   float* buf) const
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   if(!getBuf(band)) return;
   
   rspf_uint32 size = getSizePerBand();
   
   if(size > 0)
   {
      const rspf_uint16* s = getUshortBuf(band);  // source
      float* d = buf;   // destination
      
      for(rspf_uint32 index = 0; index < size; ++index)
      {
         *d = m_remapTable.normFromPix(static_cast<rspf_int32>(*s));
      }
   }
}

void rspfU11ImageData::copyNormalizedBufferToTile(float* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
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
         rspf_uint16* d = getUshortBuf(band); // destination

         for(rspf_uint32 index = 0; index <  size; ++index)
         {
            d[index] = m_remapTable.pixFromNorm(s[index]);
         }
      }
   }
}


void rspfU11ImageData::copyNormalizedBufferToTile(rspf_uint32 band,
                                                   float* buf)
{
   if (!buf)
   {
      rspfSetError(getClassName(),
                    rspfErrorCodes::RSPF_ERROR,
                    "rspfU11ImageData::copyTileToNormalizedBuffer File %s line %d\nNull pointer passed to method!",
                    __FILE__,
                    __LINE__);
      return;
   }
   
   rspf_uint32 size = getSizePerBand();
   
   if((size > 0)&&getBuf(band))
   {
      float* s = buf; // source
      rspf_uint16* d = getUshortBuf(band); // destination
      
      for(rspf_uint32 index = 0; index <  size; ++index)
      {
         *d = m_remapTable.pixFromNorm(*s);
         ++d;
         ++s;
      }
   }
}

