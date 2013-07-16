//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Class declaration of rspfU16ImageData.  Specialized image data object for
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
// $Id: rspfU11ImageData.h 16052 2009-12-08 22:20:40Z dburken $

#ifndef rspfU11ImageData_HEADER
#define rspfU11ImageData_HEADER

#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfNormalizedU11RemapTable.h>

class RSPFDLLEXPORT rspfU11ImageData : public rspfImageData
{
public:
   rspfU11ImageData(rspfSource* source,
                     rspf_uint32 bands = 1);

   rspfU11ImageData(rspfSource* source,
                     rspf_uint32 bands,
                     rspf_uint32 width,
                     rspf_uint32 height);

   rspfU11ImageData(const rspfU11ImageData &rhs);
   

   virtual rspfObject* dup()const;

   /*!
    * will fill the entire band with
    * the value.
    */
   void fill(rspf_uint32 band, double value);

   bool isNull(rspf_uint32 offset)const;
   void setNull(rspf_uint32 offset);

   virtual rspfDataObjectStatus validate() const;

   /*!
    * will go to the band and offset and compute the
    * normalized float and return it back to the
    * caller through the result argument.
    */
   virtual void getNormalizedFloat(rspf_uint32 offset,
                                   rspf_uint32 bandNumber,
                                   float& result)const;

   /*!
    * This will assign to this object a normalized
    * value by unnormalizing to its native type.
    */
   virtual void setNormalizedFloat(rspf_uint32 offset,
                                   rspf_uint32 bandNumber,
                                   float input);
   
   /*!
    * Will use the memory that you pass in to normalize
    * this data object.
    */
   virtual void convertToNormalizedFloat(rspfImageData* result)const;
   
   /*!
    * Will use the memory that you pass in to normalize
    * this data object.
    */
   virtual void convertToNormalizedDouble(rspfImageData* result)const;

   /*!
    * Will take the normalized input and convert it
    * to this tile's data type.  Example:  if this
    * tile is of type UCHAR and its input is of type
    * NORALIZED_FLOAT it will unnormalize the data by
    * doing:
    *
    * minPix + normalizedInput*(maxPix-minPix)
    *
    * on a per band basis.
    */
   virtual void unnormalizeInput(rspfImageData* normalizedInput);

   /*!
    * This will compute the average value for the band.
    */
   virtual double computeAverageBandValue(rspf_uint32 bandNumber = 0);

   /*!
    * This will call the compute average band value and then
    * use that in the calculation of:
    
    * It will then do a SUM[(Mean - Actual)^2]/width*height.
    *
    * This is the average variance from the passed in
    * mean.  Basically think of the mean as a completely
    * grey image and we would like to see how this
    * image varies from the passed in mean.
    */
   virtual double computeMeanSquaredError(double meanValue,
                                          rspf_uint32 bandNumber = 0);

   virtual void setValue(long x, long y, double color);

   /*!
    * Copies entire tile to buf passed in.  Data put in buf is normalized.
    * The "buf" passed to method is assumed to be at least as big as:
    * "getSize() * sizeof(double)"
    */
   virtual void copyTileToNormalizedBuffer(double* buf) const;
   
   /*!
    * Copies buf passed in to tile.  Data is unnormalized to the tile's
    * scalar type.
    * The "buf" passed to method is assumed to be at least as big as the tiles:
    * "getSize() * sizeof(double)"
    */
   virtual void copyNormalizedBufferToTile(double* buf);

   /*!
    * Will copy this tiles specified band number to the normalized buffer.
    * if the band is out of range then nothing is done and returns.
    */
   virtual void copyTileToNormalizedBuffer(rspf_uint32 band,
                                           double* buf) const;
   virtual void copyTileToNormalizedBuffer(rspf_uint32 band,
                                           float* buf)const;

      /*!
    * Copies entire tile to buf passed in.  Data put in buf is normalized.
    * The "buf" passed to method is assumed to be at least as big as:
    * "getSize() * sizeof(double)"
    */
   virtual void copyTileToNormalizedBuffer(float* buf) const;
   
   /*!
    * Copies buf passed in to tile.  Data is unnormalized to the tile's
    * scalar type.
    * The "buf" passed to method is assumed to be at least as big as the tiles:
    * "getSize() * sizeof(double)"
    */
   virtual void copyNormalizedBufferToTile(float* buf);

   /*!
    * Will copy the normalized buffer to this tiles
    * specified band.  If band is out of range then nothing
    * is done and returns.
    */
   virtual void copyNormalizedBufferToTile(rspf_uint32 band,
                                           double* buf);
   virtual void copyNormalizedBufferToTile(rspf_uint32 band,
                                           float* buf);

   /** @return The first non-null index of the normalized remap table. */
   virtual rspf_float64 getMinNormalizedPix() const;

protected:
   virtual ~rspfU11ImageData();
   rspfU11ImageData();

private:

   static const rspfNormalizedU11RemapTable m_remapTable;
   
TYPE_DATA
};

inline rspf_float64 rspfU11ImageData::getMinNormalizedPix() const
{
   return m_remapTable[1];
}

#endif
