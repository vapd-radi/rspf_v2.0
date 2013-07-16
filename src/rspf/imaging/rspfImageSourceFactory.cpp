//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: This is the image source factory.
//              It will try to create image filters
//              first then it will try the image writers
//              and image handlers (image readers)
//
//*************************************************************************
// $Id: rspfImageSourceFactory.cpp 19005 2011-03-03 15:56:58Z gpotts $

#include <rspf/imaging/rspfImageSourceFactory.h>
#include <rspf/imaging/rspfConvolutionFilter1D.h>
#include <rspf/imaging/rspfImageGaussianFilter.h>
#include <rspf/imaging/rspfImageRenderer.h>
#include <rspf/imaging/rspfCacheTileSource.h>
#include <rspf/imaging/rspfFeatherMosaic.h>
#include <rspf/imaging/rspfHistogramRemapper.h>
#include <rspf/imaging/rspfNullPixelFlip.h>
#include <rspf/imaging/rspfImageMosaic.h>
#include <rspf/imaging/rspfClosestToCenterCombiner.h>
#include <rspf/imaging/rspfBlendMosaic.h>
#include <rspf/imaging/rspfMaxMosaic.h>
#include <rspf/imaging/rspfOrthoImageMosaic.h>
#include <rspf/imaging/rspfColorNormalizedFusion.h>
#include <rspf/imaging/rspfLocalCorrelationFusion.h>
#include <rspf/imaging/rspfSFIMFusion.h>
#include <rspf/imaging/rspfTopographicCorrectionFilter.h>
#include <rspf/imaging/rspfBandSelector.h>
#include <rspf/imaging/rspfNBandToIndexFilter.h>
#include <rspf/imaging/rspfRgbToGreyFilter.h>
#include <rspf/imaging/rspfRgbToJpegYCbCrSource.h>
#include <rspf/imaging/rspfRgbToHsvSource.h>
#include <rspf/imaging/rspfRgbToHsiSource.h>
#include <rspf/imaging/rspfHsiToRgbSource.h>
#include <rspf/imaging/rspfHsvToRgbSource.h>
#include <rspf/imaging/rspfHsiRemapper.h>
#include <rspf/imaging/rspfJpegYCbCrToRgbSource.h>
#include <rspf/imaging/rspfPolyCutter.h>
#include <rspf/imaging/rspfGeoPolyCutter.h>
#include <rspf/imaging/rspfHistogramEqualization.h>
#include <rspf/imaging/rspfHistogramMatchFilter.h>
#include <rspf/imaging/rspfHistogramThreshholdFilter.h>
#include <rspf/imaging/rspfConvolutionSource.h>
#include <rspf/imaging/rspfSICDToDetectedImage.h>
#include <rspf/imaging/rspfBrightnessContrastSource.h>
#include <rspf/imaging/rspfBrightnessMatch.h>
#include <rspf/imaging/rspfScalarRemapper.h>
#include <rspf/imaging/rspfBandMergeSource.h>
#include <rspf/imaging/rspfIndexToRgbLutFilter.h>
#include <rspf/imaging/rspfBumpShadeTileSource.h>
#include <rspf/imaging/rspfSubImageTileSource.h>
#include <rspf/imaging/rspfRLevelFilter.h>
#include <rspf/imaging/rspfBandClipFilter.h>
#include <rspf/imaging/rspfValueAssignImageSourceFilter.h>
#include <rspf/imaging/rspfImageSharpenFilter.h>
#include <rspf/imaging/rspf3x3ConvolutionFilter.h>
#include <rspf/imaging/rspfCastTileSourceFilter.h>
#include <rspf/imaging/rspfEquationCombiner.h>
#include <rspf/imaging/rspfImageGaussianFilter.h>
#include <rspf/imaging/rspfMaskFilter.h>
#include <rspf/imaging/rspfPixelFlipper.h>
#include <rspf/imaging/rspfElevImageSource.h>
#include <rspf/imaging/rspfLandsatTopoCorrectionFilter.h>
#include <rspf/imaging/rspfGeoAnnotationSource.h>
#include <rspf/imaging/rspfMapCompositionSource.h>
#include <rspf/imaging/rspfScaleFilter.h>
#include <rspf/imaging/rspfEastingNorthingCutter.h>
#include <rspf/imaging/rspfEdgeFilter.h>
#include <rspf/imaging/rspfFftFilter.h>
#include <rspf/imaging/rspfWatermarkFilter.h>
#include <rspf/imaging/rspfTrimFilter.h>
#include <rspf/imaging/rspfTwoColorView.h>
#include <rspf/imaging/rspfMemoryImageSource.h>

#include <rspf/imaging/rspfGridRemapSource.h>

// Not sure if we want to keep this here
#include <rspf/imaging/rspfAtbController.h>

#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfHistogramWriter.h>
#include <rspf/imaging/rspfRectangleCutFilter.h>


#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/imaging/rspfImageHistogramSource.h>

#include <rspf/imaging/rspfMultiBandHistogramTileSource.h>

#include <rspf/imaging/rspfBandAverageFilter.h>
#include <rspf/imaging/rspfImageToPlaneNormalFilter.h>

#include <rspf/imaging/rspfAtCorrGridRemapper.h>
#include <rspf/imaging/rspfAtCorrRemapper.h>


#include <rspf/base/rspfTrace.h>

RTTI_DEF1(rspfImageSourceFactory, "rspfImageSourceFactory", rspfImageSourceFactoryBase);

static rspfTrace traceDebug("rspfImageSourceFactory:debug");

rspfImageSourceFactory* rspfImageSourceFactory::theInstance=NULL;

rspfImageSourceFactory::~rspfImageSourceFactory()
{
   theInstance = NULL;
   rspfObjectFactoryRegistry::instance()->unregisterFactory(this);
}
rspfImageSourceFactory* rspfImageSourceFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfImageSourceFactory;
   }

   return theInstance;
}

rspfObject* rspfImageSourceFactory::createObject(const rspfString& name)const
{

   // lets do the filters first
   if( name == STATIC_TYPE_NAME(rspfBandSelector) ||
       name == "rspfNativeBandSelectorTileSource" ) 
   {
      //---
      // For backwards compatibility check old name:
      // "rspfNativeBandSelectorTileSource"
      //---
      return new rspfBandSelector;
   }
   else if(name ==  STATIC_TYPE_NAME(rspfNullPixelFlip))
   {
      return new rspfNullPixelFlip;
   }
   else if(name == STATIC_TYPE_NAME(rspfImageRenderer))
   {
      return new rspfImageRenderer;
   }
   else if(name == STATIC_TYPE_NAME(rspfCacheTileSource))
   {
      return new rspfCacheTileSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfColorNormalizedFusion))
   {
      return new rspfColorNormalizedFusion;
   }
   else if(name == STATIC_TYPE_NAME(rspfLocalCorrelationFusion))
   {
      return new rspfLocalCorrelationFusion;
   }
   else if(name == STATIC_TYPE_NAME(rspfSFIMFusion))
   {
      return new rspfSFIMFusion;
   }
   else if(name == STATIC_TYPE_NAME(rspfBlendMosaic))
   {
      return new rspfBlendMosaic;
   }
   else if(name == STATIC_TYPE_NAME(rspfMaxMosaic))
   {
      return new rspfMaxMosaic;
   }
   else if(name == STATIC_TYPE_NAME(rspfHistogramRemapper))
   {
      return new rspfHistogramRemapper;
   }
   else if(name == STATIC_TYPE_NAME(rspfImageMosaic))
   {
      return new rspfImageMosaic;
   }
   else if(name == STATIC_TYPE_NAME(rspfClosestToCenterCombiner))
   {
      return new rspfClosestToCenterCombiner;
   }
   else if(name == STATIC_TYPE_NAME(rspfRgbToGreyFilter))
   {
      return new rspfRgbToGreyFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfNBandToIndexFilter))
   {
      return new rspfNBandToIndexFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfImageChain))
   {
      return new rspfImageChain;
   }
   else if(name == STATIC_TYPE_NAME(rspfPolyCutter))
   {
      return new rspfPolyCutter;
   }
   else if(name == STATIC_TYPE_NAME(rspfGeoPolyCutter))
   {
      return new rspfGeoPolyCutter;
   }
   else if(name == STATIC_TYPE_NAME(rspfJpegYCbCrToRgbSource))
   {
      return new rspfJpegYCbCrToRgbSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfRgbToJpegYCbCrSource))
   {
      return new rspfRgbToJpegYCbCrSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfHsiToRgbSource))
   {
      return new rspfHsiToRgbSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfRgbToHsiSource))
   {
      return new rspfRgbToHsiSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfHsvToRgbSource))
   {
      return new rspfHsvToRgbSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfRgbToHsvSource))
   {
      return new rspfRgbToHsvSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfHistogramEqualization))
   {
      return new rspfHistogramEqualization;
   }
   else if(name == STATIC_TYPE_NAME(rspfHistogramMatchFilter))
   {
      return new rspfHistogramMatchFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfHistogramThreshholdFilter))
   {
      return new rspfHistogramThreshholdFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfGeoAnnotationSource))
   {
      return new rspfGeoAnnotationSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfConvolutionSource))
   {
      return new rspfConvolutionSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfSICDToDetectedImage))
   {
	   return new rspfSICDToDetectedImage;
   }
   else if(name == STATIC_TYPE_NAME(rspfBrightnessContrastSource))
   {
      return new rspfBrightnessContrastSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfBrightnessMatch))
   {
      return new rspfBrightnessMatch;
   }
   else if(name == STATIC_TYPE_NAME(rspfScalarRemapper))
   {
      return new rspfScalarRemapper;
   }
   else if(name == STATIC_TYPE_NAME(rspfBandMergeSource))
   {
      return new rspfBandMergeSource;
   }  
   else if(name == STATIC_TYPE_NAME(rspfFeatherMosaic))
   {
      return new rspfFeatherMosaic;
   }  
   else if(name == STATIC_TYPE_NAME(rspfGridRemapSource))
   {
      return new rspfGridRemapSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfEquationCombiner))
   {
      return new rspfEquationCombiner;
   }
   else if(name == STATIC_TYPE_NAME(rspfConvolutionFilter1D))
   {
      return new rspfConvolutionFilter1D;
   }
   else if(name == STATIC_TYPE_NAME(rspfImageGaussianFilter))
   {
      return new rspfImageGaussianFilter;
   }
   // Not sure if we want to keep this here
   else if(name == STATIC_TYPE_NAME(rspfAtbController))
   {
      return new rspfAtbController;
   }  
   else if(name == STATIC_TYPE_NAME(rspfIndexToRgbLutFilter))
   {
      return new rspfIndexToRgbLutFilter;
   }
   else if(name ==  STATIC_TYPE_NAME(rspfBumpShadeTileSource))
   {
      return new rspfBumpShadeTileSource;
   }
   else if(name ==  STATIC_TYPE_NAME(rspfSubImageTileSource))
   {
      return new rspfSubImageTileSource;
   }
   else if(name ==  STATIC_TYPE_NAME(rspfRLevelFilter))
   {
      return new rspfRLevelFilter;
   }
   else if(name ==  STATIC_TYPE_NAME(rspfBandClipFilter))
   {
      return new rspfBandClipFilter;
   }
   else if(name ==  STATIC_TYPE_NAME(rspfHsiRemapper))
   {
      return new rspfHsiRemapper;
   }
   else if(name == STATIC_TYPE_NAME(rspfHistogramWriter))
   {
      return new rspfHistogramWriter;
   }
   else if(name == STATIC_TYPE_NAME(rspfImageHistogramSource))
   {
      return new rspfImageHistogramSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfRectangleCutFilter))
   {
      return new rspfRectangleCutFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfCastTileSourceFilter))
   {
      return new rspfCastTileSourceFilter;
   }
   // all the image handlers
   else if(name == STATIC_TYPE_NAME(rspfValueAssignImageSourceFilter))
   {
      return new rspfValueAssignImageSourceFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfImageSharpenFilter))
   {
      return new rspfImageSharpenFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspf3x3ConvolutionFilter))
   {
      return new rspf3x3ConvolutionFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfOrthoImageMosaic))
   {
      return new rspfOrthoImageMosaic;
   }
   else if(name == STATIC_TYPE_NAME(rspfMapCompositionSource))
   {
      return new rspfMapCompositionSource;
   }
   else if(name ==  STATIC_TYPE_NAME(rspfElevImageSource))
   {
      return new rspfElevImageSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfMaskFilter))
   {
      return new rspfMaskFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfMultiBandHistogramTileSource))
   {
      return new rspfMultiBandHistogramTileSource;
   }
   else if(name == STATIC_TYPE_NAME(rspfBandAverageFilter))
   {
      return new rspfBandAverageFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfPixelFlipper))
   {
      return new rspfPixelFlipper();
   }
   else if(name == STATIC_TYPE_NAME(rspfScaleFilter))
   {
      return new rspfScaleFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfImageToPlaneNormalFilter))
   {
      return new rspfImageToPlaneNormalFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfTopographicCorrectionFilter))
   {
      return new rspfTopographicCorrectionFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfLandsatTopoCorrectionFilter))
   {
      return new rspfLandsatTopoCorrectionFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfAtCorrRemapper))
   {
      return new rspfAtCorrRemapper();
   }
   else if(name == STATIC_TYPE_NAME(rspfAtCorrGridRemapper))
   {
      return new rspfAtCorrGridRemapper();
   }
   else if(name == STATIC_TYPE_NAME(rspfEastingNorthingCutter))
   {
      return new rspfEastingNorthingCutter();
   }
   else if(name == STATIC_TYPE_NAME(rspfEdgeFilter))
   {
      return new rspfEdgeFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfFftFilter))
   {
      return new rspfFftFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfWatermarkFilter))
   {
      return new rspfWatermarkFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfImageGaussianFilter))
   {
      return new rspfImageGaussianFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfTrimFilter))
   {
      return new rspfTrimFilter();
   }
   else if(name == STATIC_TYPE_NAME(rspfTwoColorView))
   {
      return new rspfTwoColorView;
   }
   else if(name == STATIC_TYPE_NAME(rspfImageHistogramSource))
   {
      return new rspfImageHistogramSource();
   }
   else if(name == STATIC_TYPE_NAME(rspfImageSourceFilter))
   {
      // this is just a pass through source
      return new rspfImageSourceFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfMemoryImageSource))
   {
      // this is just a pass through source
      return new rspfMemoryImageSource;
   }
   return NULL;
}

rspfObject* rspfImageSourceFactory::createObject(const rspfKeywordlist& kwl,
                                                   const char* prefix)const
{
   static const char* MODULE = "rspfImageSourceFactory::createSource";
   
   rspfString copyPrefix;
   if (prefix)
   {
      copyPrefix = prefix;
   }
   
   rspfObject* result = NULL;
   
   if(traceDebug())
   {
      CLOG << "looking up type keyword for prefix = " << copyPrefix << endl;
   }

   const char* lookup = kwl.find(copyPrefix, "type");
   if(lookup)
   {
      rspfString name = lookup;
      result           = createObject(name);
      
      if(result)
      {
         if(traceDebug())
         {
            CLOG << "found source " << result->getClassName() << " now loading state" << endl;
         }
         result->loadState(kwl, copyPrefix.c_str());
      }
      else
      {
         if(traceDebug())
         {
            CLOG << "type not found " << lookup << endl;
         }
      }
   }
   else
   {
      if(traceDebug())
      {
         CLOG << "type keyword not found" << endl;
      }
   }
   return result;
}

void rspfImageSourceFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfBandSelector));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageRenderer));
   typeList.push_back(STATIC_TYPE_NAME(rspfCacheTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfBlendMosaic));
   typeList.push_back(STATIC_TYPE_NAME(rspfMaxMosaic));   
   typeList.push_back(STATIC_TYPE_NAME(rspfNullPixelFlip));
   typeList.push_back(STATIC_TYPE_NAME(rspfColorNormalizedFusion));
   typeList.push_back(STATIC_TYPE_NAME(rspfLocalCorrelationFusion));
   typeList.push_back(STATIC_TYPE_NAME(rspfSFIMFusion));
   typeList.push_back(STATIC_TYPE_NAME(rspfHistogramRemapper));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageMosaic));
   typeList.push_back(STATIC_TYPE_NAME(rspfClosestToCenterCombiner));
   typeList.push_back(STATIC_TYPE_NAME(rspfRgbToGreyFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfNBandToIndexFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageChain));
   typeList.push_back(STATIC_TYPE_NAME(rspfPolyCutter));
   typeList.push_back(STATIC_TYPE_NAME(rspfGeoPolyCutter));
   typeList.push_back(STATIC_TYPE_NAME(rspfJpegYCbCrToRgbSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfRgbToJpegYCbCrSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfHsiToRgbSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfRgbToHsiSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfHsvToRgbSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfRgbToHsvSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfHistogramEqualization));
   typeList.push_back(STATIC_TYPE_NAME(rspfHistogramMatchFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfHistogramThreshholdFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfGeoAnnotationSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfConvolutionSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfSICDToDetectedImage));
   typeList.push_back(STATIC_TYPE_NAME(rspfBrightnessContrastSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfBrightnessMatch));
   typeList.push_back(STATIC_TYPE_NAME(rspfScalarRemapper));
   typeList.push_back(STATIC_TYPE_NAME(rspfBandMergeSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfFeatherMosaic));
   typeList.push_back(STATIC_TYPE_NAME(rspfGridRemapSource));
   // Not sure if we want to keep this here
   typeList.push_back(STATIC_TYPE_NAME(rspfAtbController));
   typeList.push_back(STATIC_TYPE_NAME(rspfIndexToRgbLutFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfBumpShadeTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfSubImageTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfRLevelFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfBandClipFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfHsiRemapper));
   typeList.push_back(STATIC_TYPE_NAME(rspfHistogramWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageHistogramSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfRectangleCutFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfElevImageSource));
   
   typeList.push_back(STATIC_TYPE_NAME(rspfValueAssignImageSourceFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageSharpenFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspf3x3ConvolutionFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfOrthoImageMosaic));
   typeList.push_back(STATIC_TYPE_NAME(rspfMapCompositionSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfCastTileSourceFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfEquationCombiner));
   typeList.push_back(STATIC_TYPE_NAME(rspfConvolutionFilter1D));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageGaussianFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfMaskFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfMultiBandHistogramTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfBandAverageFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfPixelFlipper));
   typeList.push_back(STATIC_TYPE_NAME(rspfScaleFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageToPlaneNormalFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfTopographicCorrectionFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfLandsatTopoCorrectionFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfAtCorrRemapper));
   typeList.push_back(STATIC_TYPE_NAME(rspfAtCorrGridRemapper));
   typeList.push_back(STATIC_TYPE_NAME(rspfEastingNorthingCutter));
   typeList.push_back(STATIC_TYPE_NAME(rspfEdgeFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfFftFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfWatermarkFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageGaussianFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfTrimFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfTwoColorView));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageHistogramSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageSourceFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfMemoryImageSource));
}

// Hide from use...
rspfImageSourceFactory::rspfImageSourceFactory()
   :rspfImageSourceFactoryBase()
{}

rspfImageSourceFactory::rspfImageSourceFactory(const rspfImageSourceFactory&)
   :rspfImageSourceFactoryBase()
{}

const rspfImageSourceFactory& rspfImageSourceFactory::operator=(rspfImageSourceFactory&)
{
   return *this;
}
