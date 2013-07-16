//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// LGPL
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfApplanixUtmModel.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfApplanixUtmModel_HEADER
#define rspfApplanixUtmModel_HEADER
#include <rspf/projection/rspfFcsiModel.h>
#include <rspf/projection/rspfMeanRadialLensDistortion.h>
#include <rspf/base/rspfDpt3d.h>

class RSPF_DLL rspfApplanixUtmModel : public rspfSensorModel
{
public:
   rspfApplanixUtmModel();
   rspfApplanixUtmModel(const rspfApplanixUtmModel& src);
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
//   inline virtual bool useForward()const {return false;} 
   inline virtual bool useForward()const {return true;} //!ground to image faster (you don't need DEM) //TBC
   virtual bool setupOptimizer(const rspfString& init_file); //!uses file path to init model

protected:
   NEWMAT::Matrix theCompositeMatrix;
   NEWMAT::Matrix theCompositeMatrixInverse;
   double         theOmega;
   double         thePhi;
   double         theKappa;
   double         theBoreSightTx;
   double         theBoreSightTy;
   double         theBoreSightTz;
   rspfDpt       thePrincipalPoint;
   rspfDpt       thePixelSize;
   double         theFocalLength;
   rspfEcefPoint theEcefPlatformPosition;
   rspfGpt       thePlatformPosition;
   rspf_int32    theUtmZone;
   rspf_int8     theUtmHemisphere;
   rspfDpt3d     theUtmPlatformPosition;
   rspfEcefVector theShiftValues;
   
   rspfRefPtr<rspfMeanRadialLensDistortion> theLensDistortion;
   rspfEcefPoint theAdjEcefPlatformPosition;

TYPE_DATA
};

#endif
