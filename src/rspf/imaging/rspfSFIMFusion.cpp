//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LICENSE:  See top level LICENSE.txt
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfSFIMFusion.cpp 20061 2011-09-07 16:46:16Z gpotts $
#include <rspf/imaging/rspfSFIMFusion.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspf2dLinearRegression.h>
#include <rspf/base/rspfNormRgbVector.h>
#include <rspf/base/rspfHsiVector.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfBooleanProperty.h>

RTTI_DEF2(rspfSFIMFusion, "rspfSFIMFusion", rspfFusionCombiner, rspfAdjustableParameterInterface);


static const rspf_uint32 HIGH_PASS_GAIN_OFFSET = 0;
static const rspf_uint32 LOW_PASS_WIDTH_OFFSET = 1;
static const rspf_uint32  NUMBER_OF_ADJUSTABLE_PARAMETERS = 2;

rspfSFIMFusion::rspfSFIMFusion()
   :theLowPassKernelWidth(1.5),
    theHighPassKernelWidth(3)
{
   theAutoAdjustScales = true;
   theLowPassFilter  = new rspfImageGaussianFilter;
   theHighPassFilter = new rspfConvolutionSource;

   setFilters();

   initAdjustableParameters();
}

rspfSFIMFusion::~rspfSFIMFusion()
{
}

rspfRefPtr<rspfImageData> rspfSFIMFusion::getTile(const rspfIrect& rect,
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

   
   
   if(!theNormLowPassTile.valid())
   {
      theNormLowPassTile = new rspfImageData(this,
                                              RSPF_NORMALIZED_FLOAT,
                                              1,
                                              rect.width(),
                                              rect.height());
      theNormHighPassTile = new rspfImageData(this,
                                              RSPF_NORMALIZED_FLOAT,
                                               1,
                                               rect.width(),
                                               rect.height());
      theNormLowPassTile->initialize();
      theNormHighPassTile->initialize();
      theNormLowPassTile->makeBlank();
      theNormHighPassTile->makeBlank();
   }

   theNormLowPassTile->setImageRectangle(rect);
   theNormHighPassTile->setImageRectangle(rect);
   theNormLowPassTile->makeBlank();
   theNormHighPassTile->makeBlank();

   if(!theLowPassFilter->getInput() && getInput())
   {
      initialize();
   }

   rspfRefPtr<rspfImageData> lowTile  = theLowPassFilter->getTile(rect, resLevel);
   rspfRefPtr<rspfImageData> highTile = theHighPassFilter->getTile(rect, resLevel);
//   rspfRefPtr<rspfImageData> highTile = getNormIntensity(rect, resLevel);

   // if we don't have valid low and high pass then return the input color tile
   // in its original format
   //
   if(!lowTile.valid()||!highTile.valid())
   {
//       return theInputConnection->getTile(rect, resLevel);
      return 0;
   }

   if((lowTile->getDataObjectStatus() == RSPF_EMPTY)||
      (!lowTile->getBuf()) ||
      (highTile->getDataObjectStatus() == RSPF_EMPTY)||
      (!highTile->getBuf()))
   {
//      return theInputConnection->getTile(rect, resLevel);
      return 0;
   }

   rspfRefPtr<rspfImageData> normColorData = getNormTile(rect, resLevel);
   
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
   
   // rspf_float64 slopeResult = 0.0;
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
   // double delta = 0.0;
   rspf_uint32 bandsSize = (rspf_uint32)bands.size();
   double normMinPix = 0.0;
   for(y = 0; y < h; ++y)
   {
      for(x = 0; x < w; ++x)
      {
         for(idx = 0; idx < bandsSize; ++idx)
         {
            if((*bands[idx] != 0.0)&&
               (*panLow > FLT_EPSILON) ) // if band is not null and not divide by 0
            {
               normMinPix = (rspf_float32)normColorOutputData->getMinPix(idx);
               *bands[idx] = ((*bands[idx])*(*panHigh))/
                  (*panLow);
               if(*bands[idx] > 1.0) *bands[idx] = 1.0;
               if(*bands[idx] < normMinPix) *bands[idx] = normMinPix;
            }
            // let's comment out the nulling and we will instead just pass the color on
            //
//            else
//            {
//               *bands[idx] = 0.0;
//            }
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

void rspfSFIMFusion::initialize()
{
   rspfFusionCombiner::initialize();
   if(!theIntensityConnection)
   {
      theLowPassFilter->disconnectAllInputs();
      theHighPassFilter->disconnectAllInputs();
   }
   else
   {
      theLowPassFilter->connectMyInputTo(0, PTR_CAST(rspfConnectableObject,
                                                     theIntensityConnection->getObject()));
      theHighPassFilter->connectMyInputTo(0, PTR_CAST(rspfConnectableObject,
                                                      theIntensityConnection->getObject()));
      adjustableParametersChanged();
      setFilters();
      theLowPassFilter->initialize();
      theHighPassFilter->initialize();
      if(theAutoAdjustScales)
      {
         if(theInputConnection && theIntensityConnection)
         {
            rspfTypeNameVisitor visitor("rspfImageRenderer", true);
            
            theInputConnection->accept(visitor);
            rspfRefPtr<rspfConnectableObject> inputColor = visitor.getObjectAs<rspfConnectableObject>();
            visitor.reset();
            theIntensityConnection->accept(visitor);
            rspfRefPtr<rspfConnectableObject> inputPan = visitor.getObjectAs<rspfConnectableObject>();
            
            
            if(inputColor.valid()&&inputPan.valid())
            {
               rspfImageSource* inputColorSource = dynamic_cast<rspfImageSource*> (inputColor->getInput());
               rspfImageSource* inputPanSource = dynamic_cast<rspfImageSource*> (inputPan->getInput());
               
               if(inputColorSource&&inputPanSource)
               {
                  rspfRefPtr<rspfImageGeometry> colorGeom     = inputColorSource->getImageGeometry();
                  rspfRefPtr<rspfImageGeometry> intensityGeom = inputPanSource->getImageGeometry();
                  if(colorGeom.valid()&&intensityGeom.valid())
                  {
                     rspfDpt gsdIntensity = intensityGeom->getMetersPerPixel();
                     rspfDpt gsdColor     = colorGeom->getMetersPerPixel();
                     if(!gsdColor.hasNans()&&!gsdIntensity.hasNans())
                     {
                        double scaleChange = gsdColor.length()/gsdIntensity.length();
                        if(scaleChange < 1.0) scaleChange = 1.0;
                        setParameterOffset(LOW_PASS_WIDTH_OFFSET,
                                           scaleChange);
                     }
                  }
               }
            }
         }
      }
   }
}

void rspfSFIMFusion::setFilters()
{
   theLowPassFilter->setGaussStd(theLowPassKernelWidth);
   
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

void rspfSFIMFusion::initAdjustableParameters()
{
   resizeAdjustableParameterArray(NUMBER_OF_ADJUSTABLE_PARAMETERS);

   setAdjustableParameter(HIGH_PASS_GAIN_OFFSET,
                          -1.0);
   setParameterDescription(HIGH_PASS_GAIN_OFFSET,
                           "High pass gain");
   setParameterSigma(HIGH_PASS_GAIN_OFFSET,
                     1);
   setParameterCenter(HIGH_PASS_GAIN_OFFSET,
                      1.0);

   setAdjustableParameter(LOW_PASS_WIDTH_OFFSET,
                          -1);
   setParameterDescription(LOW_PASS_WIDTH_OFFSET,
                           "Low pass kernel width");
   setParameterSigma(LOW_PASS_WIDTH_OFFSET,
                     40);
   setParameterCenter(LOW_PASS_WIDTH_OFFSET,
                      40.5);
   

   setParameterOffset(LOW_PASS_WIDTH_OFFSET,
                      1.5);
}

void rspfSFIMFusion::adjustableParametersChanged()
{
//   std::cout << "Parameter offset = " << computeParameterOffset(2) << std::endl;
   theLowPassKernelWidth = (rspf_uint32)(rspf::round<int>(computeParameterOffset(LOW_PASS_WIDTH_OFFSET)));
}

void rspfSFIMFusion::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfString name = property->getName();
   if(name=="low_pass_kernel_width")
   {
      setParameterOffset(LOW_PASS_WIDTH_OFFSET,
                         property->valueToString().toDouble(),
                         true);
   }
   else if(name=="high_pass_gain")
   {
      setParameterOffset(HIGH_PASS_GAIN_OFFSET,
                         property->valueToString().toDouble(),
                         true);
   }
   else if(name=="auto_adjust_scales")
   {
      theAutoAdjustScales =  property->valueToString().toBool();
   }
   else
   {
      rspfFusionCombiner::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfSFIMFusion::getProperty(const rspfString& name)const
{
   if(name == "low_pass_kernel_width")
   {
      rspfNumericProperty* prop =  new rspfNumericProperty(name, 
                                      rspfString::toString(computeParameterOffset(LOW_PASS_WIDTH_OFFSET)),
                                      getParameterCenter(LOW_PASS_WIDTH_OFFSET)-getParameterSigma(LOW_PASS_WIDTH_OFFSET),
                                      getParameterCenter(LOW_PASS_WIDTH_OFFSET)+getParameterSigma(LOW_PASS_WIDTH_OFFSET));
      prop->setCacheRefreshBit();
      return prop;
   }
   else if(name == "high_pass_gain")
   {
      rspfNumericProperty* prop =  new rspfNumericProperty(name, 
                                      rspfString::toString(computeParameterOffset(HIGH_PASS_GAIN_OFFSET)),
                                      getParameterCenter(HIGH_PASS_GAIN_OFFSET)-getParameterSigma(HIGH_PASS_GAIN_OFFSET),
                                      getParameterCenter(HIGH_PASS_GAIN_OFFSET)+getParameterSigma(HIGH_PASS_GAIN_OFFSET));
      prop->setCacheRefreshBit();
      return prop;
   }
   else if(name=="auto_adjust_scales")
   {
      rspfBooleanProperty* prop = new rspfBooleanProperty(name,theAutoAdjustScales);
      prop->setCacheRefreshBit();
      return prop;
   }
   
   return rspfFusionCombiner::getProperty(name);
}

void rspfSFIMFusion::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfFusionCombiner::getPropertyNames(propertyNames);
   propertyNames.push_back("low_pass_kernel_width");
   propertyNames.push_back("high_pass_gain");
   propertyNames.push_back("auto_adjust_scales");
}

bool rspfSFIMFusion::saveState(rspfKeywordlist& kwl,
                                            const char* prefix) const
{
   rspfFusionCombiner::saveState(kwl, prefix);
   saveAdjustments(kwl, prefix);
   kwl.add(prefix,
           "auto_adjust_scales",
           theAutoAdjustScales,
           true);
   
   return true;
}

bool rspfSFIMFusion::loadState(const rspfKeywordlist& kwl,
                                            const char* prefix)
{
   rspfFusionCombiner::loadState(kwl, prefix);
   loadAdjustments(kwl, prefix);
   adjustableParametersChanged();
   rspfString autoAdjustScales = kwl.find(prefix, "auto_adjust_scales");
                                    
   if(!autoAdjustScales.empty())
   {
      theAutoAdjustScales = autoAdjustScales.toBool();
   }
   return true;
}
