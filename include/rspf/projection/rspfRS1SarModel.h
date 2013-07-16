//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Oscar Kramer
//
// Description:
//
// Sensor Model for Radarsat1 SAR sensor.
// 
//*******************************************************************
//  $Id:$

#ifndef rspfRS1SarModel_HEADER
#define rspfRS1SarModel_HEADER

#include <rspf/base/rspfLsrPoint.h>
#include <rspf/base/rspfLsrVector.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfDblGrid.h>
#include <rspf/projection/rspfSarModel.h>
#include <rspf/base/rspfLagrangeInterpolator.h>
#include <rspf/support_data/rspfCeosData.h>

class NEWMAT::Matrix;

//*****************************************************************************
// CLASS:  rspfRS1SarModel
//*****************************************************************************
class rspfRS1SarModel : public rspfSensorModel
{
public:
   rspfRS1SarModel();
   rspfRS1SarModel(const rspfFilename& imageDir);

   virtual ~rspfRS1SarModel();
   
   enum ImagingMode
   {
      UNKNOWN_MODE = 0,
      SCN,     // ScanSAR Narrow Beam
      SCW,     // ScanSAR Wide Beam
      SGC,     // SAR Georeferenced Coarse Resolution
      SGF,     // SAR Georeferenced Fine Resolution
      SGX,     // SAR Georeferenced Extra Fine Resolution
      SLC,     // Single Look Complex
      SPG,     // SAR Precision Geocoded
      SSG,     // SAR Systematically Geocoded
      RAW,
      ERS
   };

   enum DirectionFlag
   {
      UNKNOWN_DIRECTION = 0,
      ASCENDING,
      DESCENDING
   };
   
   //! Fulfills rspfObject base-class pure virtuals. Saves modeling info to KWL.
   //! Returns true if successful.
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=NULL) const;
   
   //! Fulfills rspfObject base-class pure virtuals. Reads modeling info from KWL.
   //! Returns true if successful.
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=NULL);

   //! Establishes geographic 3D point given image line, sample and ellipsoid height.
   virtual void lineSampleHeightToWorld(const rspfDpt& imagePt, 
                                        const double& heightAboveEllipsoid, 
                                        rspfGpt& worldPt) const;

   //! Given an image point, returns a ray originating at some arbitrarily high
   //! point (in this model at the sensor position) and pointing towards the target.
   virtual void imagingRay(const rspfDpt& image_point, rspfEcefRay& image_ray) const;

   inline virtual bool useForward() const { return false; } //!image to ground faster 

   //!  Returns pointer to a new instance, copy of this.
   virtual rspfObject* dup() const { return 0; } // TBR

protected:
   void setImagingMode(char* modeStr);
   void initFromCeos(const rspfFilename& dataDir);
   void initAdjParms();
   void establishEphemeris();
   void eciToEcfXform(const double& julianDay, NEWMAT::Matrix& xform) const;
   void establishOrpInterp();
   void establishOrpGrid();
   void establishVehicleSpace();
   void interpolatedScanORP(const rspfDpt& orp, rspfEcefPoint& orp_ecf) const;
   void deallocateMemory();
   
   rspfRefPtr<rspfCeosData> theCeosData;
   rspfRefPtr<rspfLagrangeInterpolator> theArpPosInterp;   // in ECF
   rspfRefPtr<rspfLagrangeInterpolator> theArpVelInterp;   // in ECF
   rspfRefPtr<rspfLagrangeInterpolator> theLocalOrpInterp; // in ECF
   ImagingMode     theImagingMode;
   double          theIllumAzimuth;
   double          theIllumElevation;
   rspfEcefPoint  theORP;
   double          theRefHeight;
   double          theGHA;           // Greenwich Hour Angle of first eph. pt.
   double          theEphFirstSampTime; // in seconds from start of day
   double          theSinOrientation;
   double          theCosOrientation;
   double          theSinSkew;
   rspfLsrSpace   theVehicleSpace;
   rspfEcefVector thePosCorrection;
   int             theFirstLineDay;   // julian day
   double          theFirstLineTime;  // seconds
   double          theTimePerLine;    // seconds
   DirectionFlag   theDirectionFlag;
   double          theSrGrCoeff[6];
   rspfDpt        thePixelSpacing; 
   //***
   // Additional data members used for scan-mode imagery:
   //***
   rspfDblGrid    theLatGrid;
   rspfDblGrid    theLonGrid;
   
   //***
   // Adjustable Parameters:
   //***
   double          theInTrackOffset; // meters
   double          theCrTrackOffset; // meters
   double          theRadialOffset;  // meters
   double          theLineScale;
   double          theSkew;
   double          theOrientation; // degrees
   
   //***
   // Adjustable model parameters array indexes:
   //***
   enum ADJUSTABLE_PARAM_INDEXES
   {
      INTRACK_OFFSET,
      CRTRACK_OFFSET,
      RADIAL_OFFSET,
      LINE_SCALE,
      SKEW,
      ORIENTATION,
      NUM_ADJUSTABLE_PARAMS  // not a parameter
   };

};


#endif

