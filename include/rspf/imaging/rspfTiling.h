//*******************************************************************
// Copyright (C) 2004 Garrett Potts
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: implementation for image generator
//
//*************************************************************************
// $Id: rspfTiling.h 20103 2011-09-17 16:10:42Z dburken $
#ifndef rspfTiling_HEADER
#define rspfTiling_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfRefPtr.h>

class RSPF_DLL rspfTiling : public rspfObject
{
public:

   enum rspfTilingDeltaType
   {
      rspfTilingDeltaType_TOTAL_PIXELS,
      rspfTilingDeltaType_PER_PIXEL
   };

   rspfTiling();

   bool initialize(const rspfMapProjection& proj,
                   const rspfIrect& boundingRect);

   bool next(rspfRefPtr<rspfMapProjection>& resultProjection,
             rspfIrect& resultingBounds,
             rspfString& resultingName)const;
   
   void reset();
   
   /**
    * 
    * @param tilingDistance Distance in the specified units.
    * @param unitType Currently only angular unit degrees,
    *                 and linear unit meters are supported.
    *                 If unitType is pixel unit then the
    *                 delta per unit will be set to 1.
    *                 
    */
   void setTilingDistance(const rspfDpt& tilingDistance,
                          rspfUnitType unitType);

   /**
    * This sets the pixel delta.  The delta can either be TOTAL_PIXELS or
    * UNIT_PER_PIXEL.
    *
    * If the delta type is TOTAL_PIXELS then the unit per pixel is calculated
    * from the theTilingDistance.  This is very useful when wanting to be on
    * even geographic boundaries and would like to have a fixed width and
    * height for
    * the pixels.  Flight simulators use this tiling scheme for power of 2
    * outputs.
    *
    * if the delta type is unit per pixel then the total pixels is computed
    * form the
    * theTilingDistance.  This is useful when you might want whole number
    * GSD's and whole numbered tile boundaries.
    */
   void setDelta(const rspfDpt& delta,
                 rspfTilingDeltaType deltaType);

   /**
    * This sets the tilename mask.  The key tokens that are supported are.
    *
    * %r%         Replaces with the row number of the current tile
    * %c%         Replaces with the column number of the current tile
    * %i%         Replaces with the current tile id.
    * %or%        Replaces with the origin of the row
    * %oc%        Replaces with the origin of the col
    * %SRTM%      Replaces all filenames with the SRTM file name convention
    *
    * 
    * Examples:
    *   tile%r%_%c%  assume r = 0 and c = 100 then
    *              this is replaced with tile0_100
    *   
    */
   void setTileNameMask(const rspfString& mask);
   rspfString getTileNameMask()const;
   
   bool getRowCol(rspf_int64& row,
                  rspf_int64& col,
                  rspf_int64 tileId)const;
   bool getOrigin(rspfDpt& origin,
                  rspf_int64 tileId)const;
   bool getOrigin(rspfDpt& origin,
                  rspf_int64 row,
                  rspf_int64 col)const;
   
   void setPaddingSizeInPixels(const rspfIpt& pizelOverlap);
   rspfDpt getDeltaPerPixel()const;
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual std::ostream& print(std::ostream& out) const;

protected:

   /**
    * @return The tiling distance converted to either degrees or meters.
    */
   void getConvertedTilingDistance(rspfDpt& pt) const;

   /** The size of a tile in theTilingDistanceUnitType. */
   rspfDpt             theTilingDistance;

   /** The unit type for "theDelta", and "theTilingDistance". */
   rspfUnitType        theTilingDistanceUnitType;

   /**
    * If (theDeltaType == rspfTilingDeltaType_TOTAL_PIXELS) then
    * this is the size of the tile in pixels like 1024 x 1024.
    *
    * If (theDeltaType == rspfTilingDeltaType_PER_PIXEL) then
    * this is the size of a single pixel like 1.0 meter.
    */
   rspfDpt             theDelta;

   /** @see theDelta */
   rspfTilingDeltaType theDeltaType;

   /**
    * @brief If set tiling size is controlled by the desired output size in
    * bytes.  Set by one of output_size_in_bytes, output_size_in_kilo_bytes, or
    * output_size_in_mega_bytes.  Always stored in bytes.
    *
    * @note kilo_byte = 1024 X 1024, mega_byte = 1024 X 1024 X 1024.
    *
    * @note If set this relys on theNumberOfBands and theNumberOfBytesPerPixel
    * need to alse be set correctly.
    */
   rspf_int64 theOutputSizeInBytes;

   /**
    * @brief Output number of bands.  Needed to tile by output size in bytes.
    *
    * default = 1
    *
    * @see theOutputSizeInBytes
    */
   rspf_uint32 theNumberOfBands;

   /**
    * @brief Output number of bytes per pixel.
    * Needed to tile by output size in bytes.
    *
    * default = 1
    *
    * @see theOutputSizeInBytes
    *
    * @note This bytes per single pixel, not all bands.
    */
   rspf_uint32 theNumberOfBytesPerPixelPerBand;
   
   rspfDpt             thePaddingSizeInPixels;
   mutable rspfRefPtr<rspfMapProjection>  theMapProjection;
   rspfIrect                       theImageRect;
   rspfDrect                       theTilingRect;
   mutable rspf_int64 theTileId;
   rspf_int64         theTotalHorizontalTiles;
   rspf_int64         theTotalVerticalTiles;
   rspf_int64         theTotalTiles;
   rspfString         theTileNameMask;

   /**
    * This flags the code to shift the tie point so that the edge of the pixel
    * falls on the tile boundary.  default=false
    */
   bool theEdgeToEdgeFlag; 
   
   bool validate()const;
   bool isAngularUnit(rspfUnitType unitType)const;
   bool isLinearUnit(rspfUnitType unitType)const;
   void getTileName(rspfString& resultingName,
                    rspf_int64 row,
                    rspf_int64 col,
                    rspf_int64 id)const;
   void clampGeographic(rspfDrect& rect)const;
                    
TYPE_DATA
};

#endif
