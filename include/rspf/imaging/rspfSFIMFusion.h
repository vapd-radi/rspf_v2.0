//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LICENSE:  See top level LICENSE.txt
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfSFIMFusion.h 19827 2011-07-15 19:47:30Z gpotts $
#ifndef rspfSFIMFusion_HEADER
#define rspfSFIMFusion_HEADER
#include <rspf/imaging/rspfFusionCombiner.h>
#include <rspf/imaging/rspfConvolutionSource.h>
#include <rspf/imaging/rspfImageGaussianFilter.h>
#include <rspf/base/rspfAdjustableParameterInterface.h>

/**
 * This class imlements the fusion algorithm from the paper:
 *
 * "Smoothing Filter-based Intesity Modulation: a spectral preserve
 *  image fusion technique for improving spatial details"
 *
 *  Pulished in INT. J. Remote Sensing, 2000, Vol. 21 NO. 18, 3461-3472
 *
 *  By J. G. LIU
 *
 *
 * Auther: Garrett Potts
 * LICENSE: LGPL
 */
class RSPF_DLL rspfSFIMFusion : public rspfFusionCombiner,
                                  public rspfAdjustableParameterInterface
{
public:
   rspfSFIMFusion();
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
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
protected:
   virtual ~rspfSFIMFusion();
   void setFilters();
   bool computeRegression(rspf_float64& slopeResult,
                          const rspfIpt& origin,
                          rspfRefPtr<rspfImageData> panData,
                          rspfRefPtr<rspfImageData> colorData,
                          rspf_uint32 colorBandIdx);
   
   rspf_float64 theLowPassKernelWidth;
   rspf_uint32 theHighPassKernelWidth;
   // These are low and high pass filters for the single pan band
   //
   rspfRefPtr<rspfImageGaussianFilter> theLowPassFilter;
   rspfRefPtr<rspfConvolutionSource>   theHighPassFilter;
   rspfRefPtr<rspfImageData>           theNormLowPassTile;
   rspfRefPtr<rspfImageData>           theNormHighPassTile;
   rspfRefPtr<rspfImageData>           theNormColorData;

   NEWMAT::Matrix                        theHighPassMatrix;
   bool                                  theAutoAdjustScales;

TYPE_DATA   
};

#endif
