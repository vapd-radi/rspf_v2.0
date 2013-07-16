//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
// $Id: rspfIntensityAdjustmentFilter.cpp 13330 2008-07-28 18:04:40Z dburken $
#include <rspf/imaging/rspfIntensityAdjustmentFilter.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfHsvVector.h>
#include <rspf/base/rspfRgbVector.h>

RTTI_DEF1(rspfIntensityAdjustmentFilter, "rspfIntensityAdjustmentFilter", rspfImageSourceFilter);

rspfIntensityAdjustmentFilter::rspfIntensityAdjustmentFilter()
   :rspfImageSourceFilter(),
    theMeanIntensityTarget(rspf::nan()),
    theNormTile(NULL),
    theTile(NULL),
    theBlankTile(NULL)
{
   theGridBounds.makeNan();
}

rspfIntensityAdjustmentFilter::~rspfIntensityAdjustmentFilter()
{
}

rspfRefPtr<rspfImageData> rspfIntensityAdjustmentFilter::getTile(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   if (!theTile.valid())
   {
      allocate(); // first time through.
   }
   
   if(theBlankTile.valid())
   {
      theBlankTile->setImageRectangle(rect);
   }
   if(!theInputConnection)
   {
      return theBlankTile;
   }
   rspfRefPtr<rspfImageData> data =
      theInputConnection->getTile(rect, resLevel);
   
   if(!isSourceEnabled())
   {
      return data;
   }
   theTile->setImageRectangle(rect);
   theNormTile->setImageRectangle(rect);
   loadNormTile(data);

   if(rspf::isnan(theMeanIntensityTarget))
   {
      theMeanIntensityTarget = theMeanIntensityGrid.meanValue();
   }

   loadNormTile(data);
   
   if((theNormTile->getDataObjectStatus()!=RSPF_NULL)&&
      (theNormTile->getDataObjectStatus()!=RSPF_EMPTY))
   {
      float* buf[3];
      if(data->getNumberOfBands() == 3)
      {
         buf[0] = (float*)theNormTile->getBuf(0);
         buf[1] = (float*)theNormTile->getBuf(1);
         buf[2] = (float*)theNormTile->getBuf(2);
      }
      else
      {
         buf[0] = (float*)theNormTile->getBuf(0);
         buf[1] = (float*)theNormTile->getBuf(0);
         buf[2] = (float*)theNormTile->getBuf(0);
         
      }
      rspf_int32 y = 0;
      rspf_int32 x = 0;
      rspf_int32 upperY = theNormTile->getHeight();
      rspf_int32 upperX = theNormTile->getWidth();
      rspfIpt origin = rect.ul();
      
      for(y = 0; y < upperY; ++y)
      {
         for(x = 0; x < upperX; ++x)
         {
            rspfIpt pt = rspfIpt(origin.x + x,
                                   origin.y + y) - theGridBounds.ul();

            if((buf[0] != 0) &&
               (buf[1] != 0) &&
               (buf[2] != 0))
            {
               
               rspfRgbVector rgb((rspf_uint8)(*(buf[0])*255.0),
                                  (rspf_uint8)(*(buf[1])*255.0),
                                  (rspf_uint8)(*(buf[2])*255.0));
               rspfHsvVector hsv(rgb);
               
               hsv.setV(matchTargetMean(hsv.getV(),
                                        theMeanIntensityGrid(pt.x, pt.y),
                                        theMeanIntensityTarget,
                                        theNormTile->getMinPix(0),
                                        theNormTile->getMaxPix(0)));
               rspfRgbVector result(hsv);
               
               *buf[0] = result.getR()/255.0;
               *buf[1] = result.getG()/255.0;
               *buf[2] = result.getB()/255.0;
            }
            
            ++buf[0];
            ++buf[1];
            ++buf[2];
         }
      }
   }
   theTile->copyNormalizedBufferToTile((float*)theNormTile->getBuf());
   theTile->validate();
   
   return theTile;
}

void rspfIntensityAdjustmentFilter::initialize()
{
   rspfImageSourceFilter::initialize();

   theNormTile = NULL;
   theBlankTile = NULL;
   
   if(theInputConnection)
   {
      theGridBounds = theInputConnection->getBoundingRect();
   }
   else
   {
      theGridBounds.makeNan();
   }
}

void rspfIntensityAdjustmentFilter::allocate()
{
   theBlankTile = rspfImageDataFactory::instance()->create(this, this);
   theTile = (rspfImageData*)theBlankTile->dup();
   theTile->initialize();
}

void rspfIntensityAdjustmentFilter::createAndPopulateGrid(const rspfIpt& spacing,
                                                           double targetMean)
{
   if(theInputConnection)
   {
      rspf_uint32 x = 0;
      rspf_uint32 y = 0;

      theGridBounds        = theInputConnection->getBoundingRect();

      rspfDrect rect(0,
                      0,
                      theGridBounds.width()-1,
                      theGridBounds.height()-1);
                      
      theMeanIntensityGrid = rspfDblGrid(rect,
                                          rspfDpt(spacing),
                                          0);

      for(y = 0; y <= theGridBounds.height(); y+=spacing.y)
      {
         for(x = 0; x <= theGridBounds.width(); x+=spacing.x)
         {
            rspfIpt ul(x - 16,
                        y - 16);
            
            rspfIrect sampleRect(ul.x,
                                  ul.y,
                                  ul.x + 31,
                                  ul.y + 31);
            rspfRefPtr<rspfImageData> data = theInputConnection->getTile(sampleRect);
            double mean = computeMeanIntensity(data);
            theMeanIntensityGrid.setNearestNode(rspfDpt(x, y), mean);
         }
      }
//      theMeanIntensityGrid.interpolateNullValuedNodes();
   }
   theMeanIntensityTarget = targetMean;
}

double rspfIntensityAdjustmentFilter::computeMeanIntensity(
   rspfRefPtr<rspfImageData>& data)
{
   double result  = 0;
   double divisor = 0;

   if(data.valid() &&
      (data->getDataObjectStatus()!=RSPF_NULL)&&
      (data->getDataObjectStatus()!=RSPF_EMPTY))
   {
      loadNormTile(data);

      int i = 0;
      int upper = data->getWidth()*data->getHeight();
      float* buf[3];
      if(data->getNumberOfBands() == 3)
      {
         buf[0] = (float*)theNormTile->getBuf(0);
         buf[1] = (float*)theNormTile->getBuf(1);
         buf[2] = (float*)theNormTile->getBuf(2);
      }
      else
      {
         buf[0] = (float*)theNormTile->getBuf();
         buf[1] = (float*)theNormTile->getBuf();
         buf[2] = (float*)theNormTile->getBuf();
      }
      for(i = 0; i < upper; ++i)
      {
         rspfRgbVector rgb((rspf_uint8)(*(buf[0])*255.0),
                            (rspf_uint8)(*(buf[1])*255.0),
                            (rspf_uint8)(*(buf[2])*255.0));
         rspfHsvVector hsv(rgb);
         if(hsv.getV() > 0.0)
         {
            result += hsv.getV();
            divisor += 1.0;
         }
         ++buf[0];
         ++buf[1];
         ++buf[2];
      }
   }

   if(divisor > 0.0)
   {
      result /= divisor;
   }
   return result;
}

void rspfIntensityAdjustmentFilter::loadNormTile(rspfRefPtr<rspfImageData>& data)
{
   if(!theNormTile)
   {
      theNormTile = rspfImageDataFactory::instance()->create(this,
                                                              RSPF_NORMALIZED_FLOAT,
                                                              data->getNumberOfBands(),
                                                              data->getWidth(),
                                                              data->getHeight());
      theNormTile->initialize();
   }
   else
   {
      theNormTile->setImageRectangle(data->getImageRectangle());
   }
   data->copyTileToNormalizedBuffer((float*)theNormTile->getBuf());
   theNormTile->validate();
}

double rspfIntensityAdjustmentFilter::matchTargetMean(double inputValue,
                                                       double meanValue,
                                                       double targetMean,
                                                       double minValue,
                                                       double maxValue)
{
    // max change
   const double delta = targetMean - meanValue;
   double weight=0.0;
   
   // weight max change dependend on proximity to end points
   if (inputValue <= meanValue)
   {
      weight = fabs((inputValue - minValue) / (meanValue - minValue));
   }
   else
   {
      weight = fabs((maxValue - inputValue) / (maxValue - meanValue));
   }

   return  (inputValue + (delta * weight));
}
