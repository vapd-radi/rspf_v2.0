//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Color normalized fusion
//
//*************************************************************************
// $Id: rspfColorNormalizedFusion.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfColorNormalizedFusion.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/imaging/rspfImageData.h>

RTTI_DEF1(rspfColorNormalizedFusion,
          "rspfColorNormalizedFusion",
          rspfFusionCombiner);

rspfColorNormalizedFusion::rspfColorNormalizedFusion()
   : rspfFusionCombiner()
{
}

rspfColorNormalizedFusion::rspfColorNormalizedFusion(rspfObject* owner)
   : rspfFusionCombiner(owner)
{
}

rspfColorNormalizedFusion::~rspfColorNormalizedFusion()
{
}

rspfRefPtr<rspfImageData> rspfColorNormalizedFusion::getTile(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> inputTile = getNormTile(rect, resLevel);

   if(!inputTile.valid())
   {
      return NULL;
   }
   if(!theInputConnection||!theIntensityConnection)
   {
      return NULL;
   }
   
   if((inputTile->getDataObjectStatus() == RSPF_NULL)||
      (inputTile->getDataObjectStatus() == RSPF_EMPTY))
   {
      return NULL;
   }

   if(theTile.valid())
   {
      theTile->setImageRectangleAndBands(rect, inputTile->getNumberOfBands());
   }

   
   rspf_float32* redBuff = (rspf_float32*)inputTile->getBuf(0);
   rspf_float32* grnBuff = (rspf_float32*)inputTile->getBuf(1);
   rspf_float32* bluBuff = (rspf_float32*)inputTile->getBuf(2);

   if(!redBuff||!grnBuff||!bluBuff)
   {
      return 0;
   }
   rspfRefPtr<rspfImageData> inputIntensity = getNormIntensity(rect, resLevel);

   if((!inputIntensity.valid()) ||
      (!inputIntensity->getBuf()) ||
      (inputIntensity->getDataObjectStatus() == RSPF_EMPTY))
   {
      return 0;
   }
   
   rspf_float32* mono_buff = (rspf_float32*)inputIntensity->getBuf(0);

   // Since NULL_PIX_VALUE is only used for Pix8 comparisons cast it now.
   const float NULL_PIX_VALUE = (rspf_float32)inputIntensity->getNullPix(0);
   const float MIN_PIX_VALUE = (rspf_float32)inputIntensity->getMinPix(0);
   const float MAX_PIX_VALUE = (rspf_float32)inputIntensity->getMaxPix(0);

   float  rgb_sum;
   float  r_wt; // Weight of red to rgb_sum.
   float  g_wt; // Weight of green to rgb_sum.
   float  b_wt; // Weight of blue to rgb_sum.
   float  iVal;
   float  redVal;
   float  greenVal;
   float  blueVal;

   int size = theTile->getWidth()*theTile->getHeight();
   
   for (int i = 0; i < size;  i++)
   {
      //***
      // If no intensity source, or, no rgb source make output pixels null.
      //***
      if ( (mono_buff[i] == NULL_PIX_VALUE) ||
           (redBuff[i]   == NULL_PIX_VALUE &&
            grnBuff[i]   == NULL_PIX_VALUE &&
            bluBuff[i]   == NULL_PIX_VALUE) )
           
      {
         redBuff[i] = NULL_PIX_VALUE;
         grnBuff[i] = NULL_PIX_VALUE;
         bluBuff[i] = NULL_PIX_VALUE;
      }
      else
      {
         redVal    = redBuff[i];
         greenVal  = grnBuff[i];
         blueVal   = bluBuff[i];
         rgb_sum   = redVal + greenVal + blueVal + 3;
	 r_wt      = 3 * (redVal + 1)   / rgb_sum;
	 g_wt      = 3 * (greenVal + 1) / rgb_sum;
	 b_wt      = 3 * (blueVal + 1)  / rgb_sum;
         iVal      = mono_buff[i] + 1;
	 
	 redVal    = r_wt * iVal - 1;
         greenVal  = g_wt * iVal - 1;
	 blueVal   = b_wt * iVal - 1;

         // Clip to max pixel value of radiometry.
         if (redVal   > MAX_PIX_VALUE) redVal   = MAX_PIX_VALUE;
         if (greenVal > MAX_PIX_VALUE) greenVal = MAX_PIX_VALUE;
         if (blueVal  > MAX_PIX_VALUE) blueVal  = MAX_PIX_VALUE;

         // Assign chip value, clamp to min pixel value of radiometry if zero.
         redBuff[i] = (float)(redVal>0.0 ? redVal : MIN_PIX_VALUE);
         grnBuff[i] = (float)(greenVal>0.0 ? greenVal : MIN_PIX_VALUE);
         bluBuff[i] = (float)(blueVal>0.0 ? blueVal : MIN_PIX_VALUE);
      }
   } // End of loop through pixels in chip.
   theTile->copyNormalizedBufferToTile((float*)inputTile->getBuf());
   theTile->validate();

   return theTile;
}
