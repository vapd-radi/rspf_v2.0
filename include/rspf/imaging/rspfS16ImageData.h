//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Class declaration of rspfS16ImageData.  Specialized image data object for
// signed short data.
//
//*************************************************************************
// $Id: rspfS16ImageData.h 16052 2009-12-08 22:20:40Z dburken $

#ifndef rspfS16ImageData_HEADER
#define rspfS16ImageData_HEADER

#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfNormalizedS16RemapTable.h>

class RSPFDLLEXPORT rspfS16ImageData : public rspfImageData
{
public:
   rspfS16ImageData(rspfSource* source,
                     rspf_uint32 bands = 1);

   rspfS16ImageData(rspfSource* source,
                    rspf_uint32 bands,
                    rspf_uint32 width,
                    rspf_uint32 height);

   rspfS16ImageData(const rspfS16ImageData &rhs);
   

   /*!
    * Perform object duplication.
    */
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
  
   virtual void setValue(rspf_int32 x, rspf_int32 y, double color);

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
   virtual void copyTileToNormalizedBuffer(rspf_uint32 band, double* buf) const;
   virtual void copyTileToNormalizedBuffer(rspf_uint32 band, float* buf)const;
   
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
   virtual ~rspfS16ImageData();
   rspfS16ImageData();

private:

   static const rspfNormalizedS16RemapTable m_remapTable;

TYPE_DATA
};

inline rspf_float64 rspfS16ImageData::getMinNormalizedPix() const
{
   return m_remapTable[1];
}

#endif
