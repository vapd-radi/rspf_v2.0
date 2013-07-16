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
// $Id: rspfClosestToCenterCombiner.cpp 11955 2007-10-31 16:10:22Z gpotts $
#include <rspf/imaging/rspfClosestToCenterCombiner.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF1(rspfClosestToCenterCombiner, "rspfClosestToCenterCombiner", rspfImageMosaic);

rspfClosestToCenterCombiner::rspfClosestToCenterCombiner()
   :rspfImageMosaic()
{
}

rspfRefPtr<rspfImageData> rspfClosestToCenterCombiner::getTile(const rspfIrect& rect,
                                                                  rspf_uint32 resLevel)
{
   rspf_uint32 layerIdx = 0;
   if(!isSourceEnabled())
   {
      return rspfImageMosaic::getTile(rect, resLevel);
   }
   if(!theTile.valid())
   {
      allocate();
      if(!theTile.valid())
      {
         return 0;
      }
   }
   theTile->setImageRectangle(rect);
   theTile->makeBlank();
   std::vector<rspfClosestToCenterCombinerInfo > normTileList;
   rspfRefPtr<rspfImageData> currentTile = getNextNormTile(layerIdx, 0, rect);
   while(currentTile.valid())
   {
      normTileList.push_back(rspfClosestToCenterCombinerInfo((rspfImageData*)currentTile->dup(),
                                                              layerIdx));
      currentTile = getNextNormTile(layerIdx, rect, resLevel);
   }

   
   if(normTileList.size() == 1)
   {
      theTile->copyNormalizedBufferToTile((rspf_float32*)normTileList[0].theTile->getBuf());
   }
   else if(normTileList.size() > 1)
   {
      rspfRefPtr<rspfImageData> copyTile    = rspfImageDataFactory::instance()->create(0,
                                                                                          RSPF_NORMALIZED_FLOAT);
      copyTile->setImageRectangleAndBands(rect,
                                          getNumberOfOutputBands());
      copyTile->initialize();
                                                                                          
      rspf_int32 idx   = 0;
      rspf_uint32 w     = rect.width();
      rspf_uint32 h     = rect.height();
      rspf_uint32 idxW  = 0;
      rspf_uint32 idxH  = 0;
      rspfIpt origin    = rect.ul();
      rspfIpt ulPt      = rect.ul();
      rspf_uint32 band  = 0;
      rspf_uint32 bands = copyTile->getNumberOfBands();
      rspf_uint32 srcBandIdx = 0;
      std::vector<rspf_float32*> bandList(bands);

      for(band = 0; band < bands; ++band)
      {
         bandList[band] = (rspf_float32*)copyTile->getBuf(band);
      }
      rspf_uint32 offset   = 0;
      origin.y = ulPt.y;
      for(idxH = 0; idxH < h; ++idxH)
      {
         origin.x = ulPt.x;
         for(idxW = 0; idxW < w;++idxW)
         {
            idx = findIdx(normTileList, origin, offset);

            if(idx >=0)
            {
               for(band = 0; band < bands; ++band)
               {
                  srcBandIdx = rspf::min(normTileList[idx].theTile->getNumberOfBands(), band);
                  
                  bandList[band][offset] = *(((rspf_float32*)normTileList[idx].theTile->getBuf(srcBandIdx))+offset);
               }
            }
            ++offset;
            ++origin.x;
         }
         ++origin.y;
      }
      theTile->copyNormalizedBufferToTile((rspf_float32*)copyTile->getBuf());
   }

   theTile->validate();
   
   return theTile;

}

rspf_int32 rspfClosestToCenterCombiner::findIdx(const std::vector<rspfClosestToCenterCombinerInfo >& normTileList,
                                                  const rspfIpt& pt, rspf_uint32 offset)const
{
   rspf_float32 distance = 1000000;
   rspf_int32 returnIdx = -1;
   rspf_float32 tempDistance;
   rspf_int32 idx = 0;
   rspf_int32 maxIdx = (rspf_int32)normTileList.size();
   rspfIpt midPt;
   rspfIrect rect;
   for(idx = 0; idx < maxIdx; ++ idx)
   {
      rect = theFullResBounds[ normTileList[idx].theIndex ];
      midPt = rect.midPoint();
      
      tempDistance = (pt-midPt).length();
      if(tempDistance < distance)
      {
         if(normTileList[idx].theTile->getDataObjectStatus() == RSPF_FULL)
         {
            distance = tempDistance;
            returnIdx = idx;
         }
         else if(!normTileList[idx].theTile->isNull(offset))
         {
            distance = tempDistance;
            returnIdx = idx;
         }
      }
   }

   return returnIdx;
}
