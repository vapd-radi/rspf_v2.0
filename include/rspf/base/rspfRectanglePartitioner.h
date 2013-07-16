//----------------------------------------------------------------------------
// Copyright (C) 2004 David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// Utility class to partition up a rectangle.
//
// $Id: rspfRectanglePartitioner.h 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------
#ifndef rspfRectanglePartitioner_HEADER
#define rspfRectanglePartitioner_HEADER

#include <rspf/base/rspfConstants.h>
#include <vector>

class rspfIrect;

/**
 * class rspfRectanglePartitioner
 * Utility class to partition up a rectangle.
 */
class RSPFDLLEXPORT rspfRectanglePartitioner
{
public:

   /** default construtor */
   rspfRectanglePartitioner();

   /** destructor */
   ~rspfRectanglePartitioner();

   /**
    * Partitions the rectangle doing a binary split on the longest dimension
    * until the total size is less than or equal to the "maxSizeInBytes".
    * Initializes "result" with the resulting rectangles starting at the
    * upper left corner.
    *
    * @param inputRectangle The source rectangle usually the bounding rectangle
    * of a scene.
    *
    * @param result The vector of rspfIrects to hold the result.
    *
    * @param maxSizeInBytes Maximum size of the resulting rectangle partitions
    * in bytes.
    *
    * @param bands The number of bands in the image.
    *
    * @param bytesPerPixel.  The number of bytes per pixel for a single band.
    *
    * @param internalOverlapPixels Overlap in pixels for desired partitions.
    * (default = 0) This is added to all four sides of the rectangle so 
    *
    * @note The result rectangle passed in will be cleared for starters.
    *
    * @note Edge rectangles will be clipped to the inputRectangle.
    */
   void binaryPartition(const rspfIrect& inputRectangle,
                        std::vector<rspfIrect>& result,
                        rspf_uint64 maxSizeInBytes,
                        rspf_uint32 bands,
                        rspf_uint32 bytesPerPixel,
                        rspf_uint32 internalOverlapPixels = 0) const;

private:

   /**
    * Recursively splits the input rectangle until the size is less than
    * or equal to maxSizeInBytes.
    * 
    * @param rect Input rectangle to split.
    *
    * @param maxSizeInBytes Maximum size of the resulting rectangle partitions
    * in bytes.
    *
    * @param bands The number of bands in the image.
    *
    * @param bytesPerPixel.  The number of bytes per pixel for a single band.
    *
    * @param internalOverlapPixels Overlap in pixels for desired partitions.
    * (default = 0)
    *
    * @note Will split the longest dimension.  If square will split in the
    * lengthwise direction.
    */
   void splitUntilLessThanMax(rspfIrect& rect,
                              rspf_uint64 maxSizeInBytes,
                              rspf_uint32 bands,
                              rspf_uint32 bytesPerPixel,
                              rspf_uint32 internalOverlapPixels = 0) const;

   /**
    * Splits rectangle in half rounding up if needed.
    *
    * @param input Rectangle to split.
    *
    * @note output will be a zero base rect.
    *
    * @notes Will split the longest dimension.  If square will split in the
    * lengthwise direction.
    */
   void splitRect(rspfIrect& rect) const;

   /**
    * @param rect Rectangle to give size for in bytes.
    *
    * @param bands The number of bands in the image.
    *
    * @param bytesPerPixel.  The number of bytes per pixel for a single band.
    *
    * @param internalOverlapPixels Overlap in pixels.
    * (default = 0)
    * 
    * @return size of rectangle in bytes.
    */
   rspf_uint64 getSize(const rspfIrect& rect,
                        rspf_uint32 bands,
                        rspf_uint32 bytesPerPixel,
                        rspf_uint32 internalOverlapPixels = 0) const;

   /**
    * Convenience trace method.
    *
    * @param r Input rectangle.
    *
    * @param v Vector of resulting partitioned rectangles.
    *
    * @param maxSizeInBytes Maximum size of the resulting rectangle partitions
    * in bytes.
    *
    * @param bands The number of bands in the image.
    *
    * @param bytesPerPixel.  The number of bytes per pixel for a single band.
    */
   void trace(const rspfIrect& r,
              const std::vector<rspfIrect>& v,
              rspf_uint64 maxSizeInBytes,
              rspf_uint32 bands,
              rspf_uint32 bytesPerPixel) const;
   
};

#endif /* End of "#ifndef rspfRectanglePartitioner_HEADER" */
