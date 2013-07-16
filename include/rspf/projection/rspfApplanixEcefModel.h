//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LGPL
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfApplanixEcefModel.h 20496 2012-01-25 17:15:43Z gpotts $
#ifndef rspfApplanixEcefModel_HEADER
#define rspfApplanixEcefModel_HEADER
#include <rspf/projection/rspfFcsiModel.h>
#include <rspf/projection/rspfMeanRadialLensDistortion.h>
#include <rspf/base/rspfDpt3d.h>

class RSPF_DLL rspfApplanixEcefModel : public rspfSensorModel
{
public:
   rspfApplanixEcefModel();
   rspfApplanixEcefModel(const rspfDrect& imageRect, // center in image space
                          const rspfGpt& platformPosition,
                          double roll,
                          double pitch,
                          double heading,
                          const rspfDpt& principalPoint, // in millimeters
                          double focalLength, // in millimeters
                          const rspfDpt& pixelSize); // in millimeters
   rspfApplanixEcefModel(const rspfApplanixEcefModel& src);
   virtual rspfObject* dup()const;
   
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;

   void lineSampleToWorld(const rspfDpt& image_point,
                          rspfGpt&       gpt) const;
  
   
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   virtual void worldToLineSample(const rspfGpt& world_point,
                                  rspfDpt&       image_point) const;
   
   virtual void updateModel();

   void setPrincipalPoint(rspfDpt principalPoint);

   virtual bool insideImage(const rspfDpt& p) const
   {
      /*          return( (p.u>=(0.0-FLT_EPSILON)) && */
      /*                  (p.u<=(double)(theImageSize.u-(1-FLT_EPSILON))) && */
      /*                  (p.v>=(0.0-FLT_EPSILON)) && */
      /*                  (p.v<=(double)(theImageSize.v-(1-FLT_EPSILON))) ); */
      // if it's close to the edge we will consider it inside the image
      //
      return theImageClipRect.pointWithin(p, theImageClipRect.width());
   }


   void setRollPitchHeading(double roll,
                            double pitch,
                            double heading);
   
   void setPixelSize(const rspfDpt& pixelSize);
   void setImageRect(const rspfDrect& rect);
   void setFocalLength(double focalLength);
   void setPlatformPosition(const rspfGpt& gpt);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual void initAdjustableParameters();
   
   /*!
    * rspfOptimizableProjection
    */
//   inline virtual bool useForward()const {return true;} //!ground to image faster (you don't need DEM)
   inline virtual bool useForward()const {return false;} //!ground to image faster (you don't need DEM)
   virtual bool setupOptimizer(const rspfString& init_file); //!uses file path to init model

protected:
   
   NEWMAT::Matrix theCompositeMatrix;
   NEWMAT::Matrix theCompositeMatrixInverse;
   double         theRoll;
   double         thePitch;
   double         theHeading;
   rspfDpt       thePrincipalPoint;
   rspfDpt       thePixelSize;
   double         theFocalLength;
   rspfEcefPoint theEcefPlatformPosition;
   rspfRefPtr<rspfMeanRadialLensDistortion> theLensDistortion;


   rspfEcefPoint theAdjEcefPlatformPosition;
   
TYPE_DATA
};

#endif
