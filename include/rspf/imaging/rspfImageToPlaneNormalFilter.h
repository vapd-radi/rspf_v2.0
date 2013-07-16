//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageToPlaneNormalFilter.h 20078 2011-09-09 12:25:50Z gpotts $
#ifndef rspfImageToPlaneNormalFilter_HEADER
#define rspfImageToPlaneNormalFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPFDLLEXPORT rspfImageToPlaneNormalFilter : public rspfImageSourceFilter
{
public:
   rspfImageToPlaneNormalFilter();
   rspfImageToPlaneNormalFilter(rspfImageSource* inputSource);

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   virtual rspfScalarType getOutputScalarType() const;
   virtual rspf_uint32    getNumberOfOutputBands() const;
   
   void setXScale(const double& scale);
   void setYScale(const double& scale);
   
   double getXScale()const;
   double getYScale()const;

   void setTrackScaleFlag(bool flag);
   bool getTrackScaleFlag()const;
   
   void   setSmoothnessFactor(double value);
   double getSmoothnessFactor()const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix);
   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix)const;
   virtual void initialize();
   /* ------------------- PROPERTY INTERFACE -------------------- */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   /* ------------------ PROPERTY INTERFACE END ------------------- */
   
protected:
   rspfRefPtr<rspfImageData> theTile;
   rspfRefPtr<rspfImageData> theBlankTile;
   rspfIrect      theInputBounds;
   bool            theTrackScaleFlag;
   double          theXScale;
   double          theYScale;
   double          theSmoothnessFactor;
   
   void initializeTile();
   virtual void computeNormals(rspfRefPtr<rspfImageData>& inputTile,
                               rspfRefPtr<rspfImageData>& outputTile);

   template <class T>
   void computeNormalsTemplate(T inputScalarTypeDummy,
                               rspfRefPtr<rspfImageData>& inputTile,
                               rspfRefPtr<rspfImageData>& outputTile);
TYPE_DATA
};

#endif
