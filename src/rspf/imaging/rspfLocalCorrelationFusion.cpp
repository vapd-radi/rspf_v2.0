//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LICENSE:  See top level LICENSE.txt
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfLocalCorrelationFusion.cpp 15833 2009-10-29 01:41:53Z eshirschorn $
#include <rspf/imaging/rspfLocalCorrelationFusion.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/base/rspf2dLinearRegression.h>
#include <rspf/base/rspfNormRgbVector.h>
#include <rspf/base/rspfHsiVector.h>
#include <rspf/imaging/rspfImageDataFactory.h>

RTTI_DEF2(rspfLocalCorrelationFusion, "rspfLocalCorrelationFusion", rspfFusionCombiner, rspfAdjustableParameterInterface);


static const rspf_uint32 REGRESSION_COEFFICIENT_ATTENUATOR_OFFSET = 0;
static const rspf_uint32 REGRESSION_COEFFICIENT_CLAMP_OFFSET = 1;
static const rspf_uint32 HIGH_PASS_GAIN_OFFSET = 2;
static const rspf_uint32 PAN_BLURR_WIDTH_OFFSET = 3;
static const rspf_uint32  NUMBER_OF_ADJUSTABLE_PARAMETERS = 4;

rspfLocalCorrelationFusion::rspfLocalCorrelationFusion()
   :theBlurrKernelWidth(1.5),
    theHighPassKernelWidth(3),
    theRegressionWidth(5)
{
   theLowPassFilter  = new rspfImageGaussianFilter;
   theHighPassFilter = new rspfConvolutionSource;

   setFilters();

   initAdjustableParameters();
}

rspfLocalCorrelationFusion::~rspfLocalCorrelationFusion()
{
}

rspfRefPtr<rspfImageData> rspfLocalCorrelationFusion::getTile(const rspfIrect& rect,
                                                                 rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return rspfRefPtr<rspfImageData>();
   }

   if (!theIntensityConnection)
   {
      return theInputConnection->getTile(rect, resLevel);
   }

   rspf_int32 regressionW2 = (rspf_int32)ceil(theRegressionWidth/2.0);
   
   rspfIrect expandedRegressionRect(rect.ul().x - regressionW2,
                                     rect.ul().y - regressionW2,
                                     rect.lr().x + regressionW2,
                                     rect.lr().y + regressionW2);
   
   if(!theNormLowPassTile.valid())
   {
      theNormLowPassTile = new rspfImageData(this,
                                              RSPF_NORMALIZED_FLOAT,
                                              1,
                                              expandedRegressionRect.width(),
                                              expandedRegressionRect.height());
      theNormHighPassTile = new rspfImageData(this,
                                              RSPF_NORMALIZED_FLOAT,
                                               1,
                                               expandedRegressionRect.width(),
                                               expandedRegressionRect.height());
      theNormLowPassTile->initialize();
      theNormHighPassTile->initialize();
      theNormLowPassTile->makeBlank();
      theNormHighPassTile->makeBlank();
   }

   theNormLowPassTile->setImageRectangle(expandedRegressionRect);
   theNormHighPassTile->setImageRectangle(expandedRegressionRect);
   theNormLowPassTile->makeBlank();
   theNormHighPassTile->makeBlank();

   if(!theLowPassFilter->getInput() && getInput())
   {
      initialize();
   }

   rspfRefPtr<rspfImageData> lowTile  = theLowPassFilter->getTile(expandedRegressionRect, resLevel);
   rspfRefPtr<rspfImageData> highTile = theHighPassFilter->getTile(expandedRegressionRect, resLevel);
//   rspfRefPtr<rspfImageData> highTile = getNormIntensity(expandedRegressionRect, resLevel);
   rspfIpt ul = rect.ul();
   rspfIpt origin;

   // if we don't have valid low and high pass then return the input color tile
   // in its original format
   //
   if(!lowTile.valid()||!highTile.valid())
   {
      return theInputConnection->getTile(rect, resLevel);
   }

   if((lowTile->getDataObjectStatus() == RSPF_EMPTY)||
      (!lowTile->getBuf()) ||
      (highTile->getDataObjectStatus() == RSPF_EMPTY)||
      (!highTile->getBuf()))
   {
      return theInputConnection->getTile(rect, resLevel);
   }

   rspfRefPtr<rspfImageData> normColorData = getNormTile(expandedRegressionRect, resLevel);
   
   rspf_uint32 y = 0;
   rspf_uint32 x = 0;
   rspf_uint32 w = theTile->getWidth();
   rspf_uint32 h = theTile->getHeight();

   theTile->makeBlank();
   theTile->setImageRectangle(rect);
   
   if(!normColorData.valid())
   {
      return 0;
//      return theTile;
   }

   if((normColorData->getDataObjectStatus() == RSPF_EMPTY)||
      !normColorData->getBuf())
   {
      return theTile;
   }
   rspfRefPtr<rspfImageData> normColorOutputData = (rspfImageData*)normColorData->dup();
   normColorOutputData->setImageRectangle(rect);
   normColorOutputData->loadTile(normColorData.get());
   
   rspf_float64 slopeResult = 0.0;
   rspf_uint32 idx = 0;
   std::vector<rspf_float32*> bands(normColorData->getNumberOfBands());
   
   lowTile->copyTileToNormalizedBuffer((rspf_float32*)theNormLowPassTile->getBuf());
   
   highTile->copyTileToNormalizedBuffer((rspf_float32*)theNormHighPassTile->getBuf());
   theNormLowPassTile->validate();
   theNormHighPassTile->validate();
   rspfRefPtr<rspfImageData> lowPan = (rspfImageData*)theNormLowPassTile->dup();
   lowPan->setImageRectangle(rect);
   lowPan->loadTile(theNormLowPassTile.get());
   rspfRefPtr<rspfImageData> highPan = (rspfImageData*)theNormHighPassTile->dup();
   highPan->setImageRectangle(rect);
   highPan->loadTile(theNormHighPassTile.get());
   
   rspf_float32* panHigh = (rspf_float32*)highPan->getBuf();
   rspf_float32* panLow  = (rspf_float32*)lowPan->getBuf();
   for(idx = 0; idx < bands.size(); ++idx)
   {
      bands[idx] = (rspf_float32*)normColorOutputData->getBuf(idx);
   }
   double panAttenuator = computeParameterOffset(REGRESSION_COEFFICIENT_ATTENUATOR_OFFSET);
   double delta = 0.0;
   rspf_uint32 bandsSize = (rspf_uint32)bands.size();
   rspf_float64 slopeClamp = computeParameterOffset(REGRESSION_COEFFICIENT_CLAMP_OFFSET);
   rspf_float64 minSlope = -slopeClamp;
   rspf_float64 maxSlope = slopeClamp;
   for(y = 0; y < h; ++y)
   {
      origin.y = ul.y + y;
      for(x = 0; x < w; ++x)
      {
         origin.x = ul.x + x;
         
//          if(computeRegression(slopeResult,
//                               origin,
//                               theNormLowPassTile,
//                               normColorData,
//                               0))
//          {
//             delta = panAttenuator*slopeResult*(*panHigh - *panLow);
            
         for(idx = 0; idx < bandsSize; ++idx)
         {
            if(*bands[idx] != 0.0) // if band is not null
            {
               if(computeRegression(slopeResult,
                                    origin,
                                    theNormLowPassTile,
                                    normColorData,
                                    idx))
               {
                  if(slopeResult < minSlope) slopeResult = minSlope;
                  if(slopeResult > maxSlope) slopeResult = maxSlope;
                  delta = panAttenuator*slopeResult*(*panHigh - *panLow);
                  rspf_float32 normMinPix = (rspf_float32)normColorOutputData->getMinPix(idx);
                  *bands[idx] += delta;
                  if(*bands[idx] > 1.0) *bands[idx] = 1.0;
                  if(*bands[idx] < normMinPix) *bands[idx] = normMinPix;
               }
            }
            ++bands[idx];
         }
         ++panHigh;
         ++panLow;
      }
   }

   theTile->copyNormalizedBufferToTile((rspf_float32*)normColorOutputData->getBuf());
   theTile->validate();
   
   return theTile;
}

void rspfLocalCorrelationFusion::initialize()
{
   rspfFusionCombiner::initialize();
   if(!theIntensityConnection)
   {
      theLowPassFilter->disconnect();
      theHighPassFilter->disconnect();
   }
   else
   {
      theLowPassFilter->connectMyInputTo(0, PTR_CAST(rspfConnectableObject,
                                                     theIntensityConnection->getObject()));
      theHighPassFilter->connectMyInputTo(0, PTR_CAST(rspfConnectableObject,
                                                      theIntensityConnection->getObject()));
      setFilters();
      theLowPassFilter->initialize();
      theHighPassFilter->initialize();
   }
}

void rspfLocalCorrelationFusion::setFilters()
{
   theLowPassFilter->setGaussStd(theBlurrKernelWidth);
   
   theHighPassMatrix = NEWMAT::Matrix(theHighPassKernelWidth, theHighPassKernelWidth);
   theHighPassMatrix = 0;
   theHighPassMatrix[theHighPassKernelWidth>>1][theHighPassKernelWidth>>1] = 1;
   
   // adjust the gain for the high pass filter
   //
   NEWMAT::Matrix high =  theHighPassMatrix;

   rspf_float64 kernelW2 = theHighPassKernelWidth*theHighPassKernelWidth;
   double gain = computeParameterOffset(HIGH_PASS_GAIN_OFFSET)*(kernelW2);
   double multiplier = gain/(kernelW2);
   high = -multiplier;
   rspf_int32 cx = theHighPassKernelWidth>>1;
   rspf_int32 cy = theHighPassKernelWidth>>1;

   if(gain > FLT_EPSILON)
   {
      high[cy][cx] = multiplier* ( (kernelW2-1)+kernelW2/gain);
   }
   else
   {
      high = 0.0;
      high[cy][cx] = 1.0;
   }

   theHighPassFilter->setConvolution(high);
}

bool rspfLocalCorrelationFusion::computeRegression(rspf_float64& slopeResult,
                                                    const rspfIpt& origin,
                                                    const rspfRefPtr<rspfImageData> panData,
                                                    const rspfRefPtr<rspfImageData> colorData,
                                                    rspf_uint32 colorBandIdx)
{
   bool result = false;
   slopeResult = 0.0;
   if(panData.valid()&&colorData.valid())
   {
      rspf_int32 regW2 = (rspf_int32)(theRegressionWidth>>1);
      rspfIrect rect = panData->getImageRectangle();
      const rspf_float32* colorDataPtr  = (const rspf_float32*)colorData->getBuf(colorBandIdx);
      const rspf_float32* colorDataPtr2 = (const rspf_float32*)colorData->getBuf(colorBandIdx);
      
//      const rspf_float32* colorDataPtrBands[3]={0};
      
//       if(colorData->getNumberOfBands() <2)
//       {
//          colorDataPtrBands[0] = (rspf_float32*)colorData->getBuf(0);
//       }
//       else if(colorData->getNumberOfBands() <3)
//       {
//          colorDataPtrBands[0] = (rspf_float32*)colorData->getBuf(0);
//          colorDataPtrBands[1] = (rspf_float32*)colorData->getBuf(1);
//       }
//       else
//       {
//          colorDataPtrBands[0] = (rspf_float32*)colorData->getBuf(0);
//          colorDataPtrBands[1] = (rspf_float32*)colorData->getBuf(1);
//          colorDataPtrBands[2] = (rspf_float32*)colorData->getBuf(2);
//       }
      const rspf_float32* panDataPtr  = (const rspf_float32*)panData->getBuf();
      const rspf_float32* panDataPtr2 = (const rspf_float32*)panData->getBuf();
//      if(!panDataPtr || !colorDataPtrBands[0]) return result;
      if(!panDataPtr || !colorDataPtr) return result;
      
      rspf_float64 panNp   = panData->getNullPix(0);
      rspf_float64 colorNp = colorData->getNullPix(0);
      rspf_int32 w = (rspf_int32)rect.width();
      rspf_int32 offset = (rspf_int32)( ((origin.y-rect.ul().y) - regW2)*w +
                                          ((origin.x-rect.ul().x) - regW2));

      rspf_uint32 y = 0;
      rspf_uint32 x = 0;
      rspf_uint32 idx = 0;
      rspf2dLinearRegression regression;
      rspf_uint32 offsetColorBand;

      for(y = 0; y < theRegressionWidth; ++y)
      {
         offsetColorBand = (offset + y*w);
         panDataPtr   = panDataPtr2   + offsetColorBand;
         colorDataPtr = colorDataPtr2 + offsetColorBand;
         for(x = 0; x < theRegressionWidth; ++x)
         {
//             if((*panDataPtr != panNp)&&
//                (colorDataPtrBands[0][offsetColorBand] != colorNp)&&
//                (colorDataPtrBands[1][offsetColorBand] != colorNp)&&
//                (colorDataPtrBands[2][offsetColorBand] != colorNp))
//             {
            if((*panDataPtr != panNp)&&
               (*colorDataPtr != colorNp))
            {
//                rspfNormRgbVector rgbV(colorDataPtrBands[0][offsetColorBand],
//                                        colorDataPtrBands[1][offsetColorBand],
//                                        colorDataPtrBands[2][offsetColorBand]);
//                rspfHsiVector hsi(rgbV);
               
               regression.addPoint(rspfDpt(*panDataPtr, *colorDataPtr));
//               regression.addPoint(rspfDpt(*panDataPtr, hsi.getI()));
               ++idx;
            }
            ++panDataPtr;
//            ++offsetColorBand;
               ++colorDataPtr;
         }
      }
      if(idx > 2)
      {
         double slope, intercept;
         result = true;
         regression.solve();
         regression.getEquation(slope,
                                intercept);
         
         slopeResult = slope;
      }
   }

   return result;
}

void rspfLocalCorrelationFusion::initAdjustableParameters()
{
   resizeAdjustableParameterArray(NUMBER_OF_ADJUSTABLE_PARAMETERS);

   setAdjustableParameter(REGRESSION_COEFFICIENT_ATTENUATOR_OFFSET,
                          0.0);
   setParameterDescription(REGRESSION_COEFFICIENT_ATTENUATOR_OFFSET,
                           "Attenuator");
   setParameterSigma(REGRESSION_COEFFICIENT_ATTENUATOR_OFFSET,
                     2.0);
   setParameterCenter(REGRESSION_COEFFICIENT_ATTENUATOR_OFFSET,
                      1.0);

   
   setAdjustableParameter(REGRESSION_COEFFICIENT_CLAMP_OFFSET,
                          0.0);
   setParameterDescription(REGRESSION_COEFFICIENT_CLAMP_OFFSET,
                           "Regression clamp");
   setParameterSigma(REGRESSION_COEFFICIENT_CLAMP_OFFSET,
                     10.0);
   setParameterCenter(REGRESSION_COEFFICIENT_CLAMP_OFFSET,
                      10.0);
   
   
   setAdjustableParameter(HIGH_PASS_GAIN_OFFSET,
                          -1.0);
   setParameterDescription(HIGH_PASS_GAIN_OFFSET,
                           "High pass gain");
   setParameterSigma(HIGH_PASS_GAIN_OFFSET,
                     1);
   setParameterCenter(HIGH_PASS_GAIN_OFFSET,
                      1.0);

   setAdjustableParameter(PAN_BLURR_WIDTH_OFFSET,
                          -1);
   setParameterDescription(PAN_BLURR_WIDTH_OFFSET,
                           "Blurring kernel width");
   setParameterSigma(PAN_BLURR_WIDTH_OFFSET,
                     7);
   setParameterCenter(PAN_BLURR_WIDTH_OFFSET,
                      7.5);
   

   setParameterOffset(PAN_BLURR_WIDTH_OFFSET,
                      1.5);
   setParameterOffset(REGRESSION_COEFFICIENT_CLAMP_OFFSET,
                      4.0);
}

void rspfLocalCorrelationFusion::adjustableParametersChanged()
{
//   std::cout << "Parameter offset = " << computeParameterOffset(2) << std::endl;
   theBlurrKernelWidth = (rspf_uint32)(rspf::round<int>(computeParameterOffset(PAN_BLURR_WIDTH_OFFSET)));
}


bool rspfLocalCorrelationFusion::saveState(rspfKeywordlist& kwl,
                                            const char* prefix) const
{
   rspfFusionCombiner::saveState(kwl, prefix);
   saveAdjustments(kwl, prefix);

   return true;
}

bool rspfLocalCorrelationFusion::loadState(const rspfKeywordlist& kwl,
                                            const char* prefix)
{
   rspfFusionCombiner::loadState(kwl, prefix);
   loadAdjustments(kwl, prefix);
   adjustableParametersChanged();
   
   return true;
}
