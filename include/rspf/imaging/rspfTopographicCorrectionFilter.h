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
// $Id: rspfTopographicCorrectionFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfTopographicCorrectionFilter_HEADER
#define rspfTopographicCorrectionFilter_HEADER
#include <rspf/imaging/rspfImageCombiner.h>
#include <rspf/matrix/newmat.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfPropertyEvent.h>
#include <rspf/base/rspf2dLinearRegression.h>

class rspfImageData;

class rspfImageSourceConnection;
class rspfImageToPlaneNormalFilter;
/*!
 *
 * class rspfTopographicCorrectionFilter
 *
 * This clas is used to execute terrain correction.  I t requires 2 inputs
 * where the first input is a multi band data source to correct and the
 * second is elevation normals used in light incident calculations.
 *
 * A sample chain might look like:
 *
 *  landsatHandler ---> Cache ---> renderer ---> cache ---------------------------------------
 *                                                                                            |--> Topo correction filter ---> Output
 *  ortho mosaic of elevation data----> imageToPlaneNormals --> Cache ---> Renderer -->Cache --
 *
 * the rspfImageToPlaneNormalsFilter is used to take a single band elevation source and compute normals at each post.  I suggest that you
 * set the renderer to use a higher order convolution than just nearest neighbor, maybe Cubic.  This way you have smoother interpolation
 * as you change zoom levels.
 *
 *
 * Correction Types:
 *
 *   Cosine correction:     In order for this to run it does not need any additional informat but the multi band source and
 *                          an normal calculation input.
 *
 *   Cosine-C correction:   In order for this to run it will need theC array populated for each band.  It defaults
 *                          to 0 forcing a default Cosine correction to take place.
 *
 *   Minnaert correction:   In order for this to run it will need theK array populated for each band.  The defaults should work pretty good
 *  
 */
class RSPFDLLEXPORT rspfTopographicCorrectionFilter : public rspfImageCombiner
{
public:
   enum rspfTopoCorrectionType
   {
      TOPO_CORRECTION_COSINE    = 0,
      TOPO_CORRECTION_COSINE_C  = 1,
      TOPO_CORRECTION_MINNAERT  = 2
   };
   rspfTopographicCorrectionFilter();

   rspfTopographicCorrectionFilter(rspfImageSource* colorSource,
                                    rspfImageSource* elevSource);
   
   rspfRefPtr<rspfImageData> getTile(const  rspfIrect& tileRect,
                                       rspf_uint32 resLevel=0);
   
   virtual rspf_uint32 getNumberOfOutputBands() const;

   virtual rspfScalarType getOutputScalarType() const;

   virtual double getNullPixelValue(rspf_uint32 band=0)const;

   virtual double getMinPixelValue(rspf_uint32 band=0)const;

   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;

   virtual void getDecimationFactor(rspf_uint32 resLevel,
                                    rspfDpt& result) const;
   
   virtual void getDecimationFactors(vector<rspfDpt>& decimations) const;

   virtual rspf_uint32 getNumberOfDecimationLevels()const;

   virtual double getAzimuthAngle()const;

   virtual double getElevationAngle()const;

   virtual void setAzimuthAngle(double angle);

   virtual void setElevationAngle(double angle);

   virtual void initialize();

   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   bool canConnectMyInputTo(rspf_int32 inputIndex,
                            const rspfConnectableObject* object)const;
      
   virtual void connectInputEvent(rspfConnectionEvent& event);

   virtual void disconnectInputEvent(rspfConnectionEvent& event);

   virtual void propertyEvent(rspfPropertyEvent& event);

   virtual void refreshEvent(rspfRefreshEvent& event);

   virtual rspfTopoCorrectionType getTopoCorrectionType()const;

   virtual void setTopoCorrectionType(rspfTopoCorrectionType topoType);

   const std::vector<double>& getGainValues()const;

   void setGainValues(const std::vector<double>& gainValues);
  
   const vector<double>& getBiasValues()const;

   void setBiasValues(const std::vector<double>& biasValues);

   const vector<double>& getKValues()const;

   double getK(int idx)const;

   void setK(int idx, double value);

   void setKValues(const vector<double>& kValues);

   double getC(int idx)const;

   void setC(int idx, double value);

   void setCValues(const vector<double>& cValues);
   
   void computeLightDirection();

protected:
   virtual ~rspfTopographicCorrectionFilter();
   virtual void allocate();
   
   /*!
    * The result of the illumination equation is stored in
    * here.  This is populated on each call to getTile.
    */
   rspfRefPtr<rspfImageData>   theTile;

   std::vector<rspf_uint32> theBandMapping;
   
//   rspfImageToPlaneNormalFilter* theNormalFilter;
//   rspfScalarRemapper*           theScalarRemapper;
   std::vector<double> theC;
   std::vector<double> theK;
   std::vector<double> theBias;
   std::vector<double> theGain;
   
   /*!
    * Used for the light vector computation.
    */
   double theLightSourceElevationAngle;

   /*!
    * Used for the light vector computation.
    */
   double theLightSourceAzimuthAngle;

   /*!
    * This is computed from the elevation and
    * azimuth angles of the light source.
    */
   double theLightDirection[3];

   double         theJulianDay;
   mutable bool   theCComputedFlag;
   rspfTopoCorrectionType theTopoCorrectionType;   
   double         theNdviLowTest;
   double         theNdviHighTest;
   
   virtual void executeTopographicCorrection(
      rspfRefPtr<rspfImageData>& outputData,
      rspfRefPtr<rspfImageData>& colorData,
      rspfRefPtr<rspfImageData>& normalData);

   template <class T> void executeTopographicCorrectionTemplate(
      T dummy,
      rspfRefPtr<rspfImageData>& outputData,
      rspfRefPtr<rspfImageData>& colorData,
      rspfRefPtr<rspfImageData>& normalData);

   template <class T> void executeTopographicCorrectionMinnaertTemplate(
      T dummy,
      rspfRefPtr<rspfImageData>& outputData,
      rspfRefPtr<rspfImageData>& colorData,
      rspfRefPtr<rspfImageData>& normalData);
  
//    virtual void computeC();

//    template<class T>
//    void addRegressionPointsTemplate(T, //dummy
//                                     std::vector<rspf2dLinearRegression>& regressionPoints,
//                                     std::vector<int>& cosineIBucketCount,
//                                     rspf_int32 maxCountPerBucket,
//                                     rspfImageData* colorData,
//                                     rspfImageData* normalData);
   template <class T> bool computeNdvi(T dummy,
                                       rspf_float64& result,
                                       rspf_uint32 offset,
                                       const std::vector<T*>& bands)const;

   void resizeArrays(rspf_uint32 newSize);

   bool setCCmputedFlag();
TYPE_DATA
};

#endif
