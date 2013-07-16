//*******************************************************************
// Copyright (C) 2005 Garrett Potts. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfClosestToCenterCombiner.h 22162 2013-02-26 01:37:48Z gpotts $
#ifndef rspfClosestToCenterCombiner_HEADER
#define rspfClosestToCenterCombiner_HEADER
#include <vector>
#include <rspf/imaging/rspfImageMosaic.h>
/**
  * This implements a closest to center combiner.
  *
  * It will first grab all images overlapping a given rectangle query.  The first non-null
  * closest pixel to the center of the image is used.  This basically uses only pixels along
  * the "sweet" spot of the image.  This combiner is typically used when there is overlapping
  * sensor data.
  * 
  */
class RSPF_DLL rspfClosestToCenterCombiner : public rspfImageMosaic
{
public:
   rspfClosestToCenterCombiner();

   /**
    * Executes the rspfClosestToCenterCombiner algorithm. sets the pixels to the closest
    * Valid value.  This simulates a sweet spot cutter.
    * 
    * @param rect The region of interest.
    * @param resLevel.  For this combiner this is assumed to always be 0
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
protected:
   class rspfClosestToCenterCombinerInfo
   {
   public:
      rspfClosestToCenterCombinerInfo(rspfRefPtr<rspfImageData> tile,
                                       rspf_int32 idx)
         :theTile(tile),
         theIndex(idx)
         {
         }
      rspfRefPtr<rspfImageData> theTile;
      rspf_int32                 theIndex;
   };

   
   rspf_int32 findIdx(const std::vector<rspfClosestToCenterCombinerInfo >& normTileList,
                       const rspfIpt& pt, rspf_uint32 offset)const;

TYPE_DATA
};

#endif
