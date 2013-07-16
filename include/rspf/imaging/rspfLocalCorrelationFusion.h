//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LICENSE:  See top level LICENSE.txt
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfLocalCorrelationFusion.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfLocalCorrelationFusion_HEADER
#define rspfLocalCorrelationFusion_HEADER
#include <rspf/imaging/rspfFusionCombiner.h>
#include <rspf/imaging/rspfConvolutionSource.h>
#include <rspf/imaging/rspfImageGaussianFilter.h>
#include <rspf/base/rspfAdjustableParameterInterface.h>

/**
  * This implements a Local correlation fusion based on the paper:
  *
  *  "A Local Correlation Approach For the Fusion of Remote Sensing Data With Different
  *   Spatial Resolution In Forestry Applications"
  *   by J. Hill, C. Diemer, O. Stover, and Th. Udelhoven
  *   Published in:
  *       International Archives of Photogrammetry and Remote Sensing, Vol 32, Part 7-4-3 W6,
  *       Valladolid, Spain, 3-4 June, 1999
  *
  *   A new band is computed by the formula:
  *
  *   ColorHigh = ColorLow + RegressionSlope*(PanHigh-PanLow);
  *
  *   Adjustable paraemters have been added to clamp the regression slope and attenuate
  *   colorLowOffset value and blurring the Pan.
  *
  *   ColorHigh = ColorLow + Clamp(RegressionSlope)*Attenuator(PanHigh-PanLow);
  *
  *   The regression coefficient is coputed for each color band.
  */
class RSPF_DLL rspfLocalCorrelationFusion : public rspfFusionCombiner,
                                              public rspfAdjustableParameterInterface
{
public:
   rspfLocalCorrelationFusion();
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
   virtual void initialize();

   virtual rspfObject* getBaseObject()
   {
      return this;
   }
   virtual const rspfObject* getBaseObject()const
   {
      return this;
   }
   virtual void initAdjustableParameters();
   virtual void adjustableParametersChanged();
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
protected:
   virtual ~rspfLocalCorrelationFusion();
   void setFilters();
   bool computeRegression(rspf_float64& slopeResult,
                          const rspfIpt& origin,
                          rspfRefPtr<rspfImageData> panData,
                          rspfRefPtr<rspfImageData> colorData,
                          rspf_uint32 colorBandIdx);
   
   rspf_float64 theBlurrKernelWidth;
   rspf_uint32 theHighPassKernelWidth;
   rspf_uint32 theRegressionWidth;
   // These are low and high pass filters for the single pan band
   //
   rspfRefPtr<rspfImageGaussianFilter> theLowPassFilter;
   rspfRefPtr<rspfConvolutionSource> theHighPassFilter;
   rspfRefPtr<rspfImageData>         theNormLowPassTile;
   rspfRefPtr<rspfImageData>         theNormHighPassTile;
   rspfRefPtr<rspfImageData>         theNormColorData;

   NEWMAT::Matrix                      theHighPassMatrix;

TYPE_DATA   
};

#endif
