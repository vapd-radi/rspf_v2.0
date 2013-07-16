#ifndef rspfLandSatModel_HEADER
#define rspfLandSatModel_HEADER
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <iostream>
class rspfFfL7;
class rspfString;
class rspfMapProjection;
class RSPFDLLEXPORT rspfLandSatModel : public rspfSensorModel
{
public:
   /*!
    * CONSTRUCTORS:
    */
   rspfLandSatModel();
   rspfLandSatModel(const rspfFfL7& head);
   rspfLandSatModel(const rspfFilename& init_file);
   rspfLandSatModel(const rspfKeywordlist& geom_kwl);
   rspfLandSatModel(const rspfLandSatModel& rhs);
   virtual rspfObject* dup()const;
   enum ProjectionType
   {
      UNKNOWN_PROJECTION = 0,
      UTM_MAP,
      UTM_ORBIT,
      SOM_MAP,
	  SOM_ORBIT,
	  TM_MAP,
	  TM_ORBIT
   };
   
   enum AdjustParamIndex
   {
      INTRACK_OFFSET = 0,
      CRTRACK_OFFSET,
      LINE_GSD_CORR,
      SAMP_GSD_CORR,
      ROLL_OFFSET,
      YAW_OFFSET,
      YAW_RATE,
      MAP_ROTATION,
      NUM_ADJUSTABLE_PARAMS // not an index
   };
   
   
   /*!
    * Extends base-class implementation. Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   
   /*!
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   /*!
    * Writes a template of geom keywords processed by loadState and saveState
    * to output stream.
    */
   static void writeGeomTemplate(ostream& os);
   
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   
   /*!
    * Given an image point, returns a ray originating at some arbitrarily high
    * point (ideally at the sensor position) and pointing towards the target.
    */
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   /*!
    * Following a change to the adjustable parameter set, this virtual
    * is called to permit instances to compute derived quantities after
    * parameter change.
    */
   virtual void updateModel();
   /*!
    * rspfOptimizableProjection
    */
   inline virtual bool useForward()const {return false;} //!image to ground faster
   virtual bool setupOptimizer(const rspfString& init_file); //!uses file path to init model
	 double theProjectionParams[15];
	 char    theEllipsoid[19];

	 // loong20100928
	 double getCenterLongitude() const;
	 int getPathNumber() const;
	 int getRowNumber() const;
protected:
   virtual ~rspfLandSatModel();
   /*!
    * Initializes the model given a fast format header.
    */
   void initFromHeader(const rspfFfL7& head);
   
   virtual void initAdjustableParameters();
   void initMapProjection();
   double           theIllumAzimuth;  
   double           theIllumElevation;
   double           theOrbitAltitude;
   double           theOrbitInclination;
   int              theMapZone;
   rspfDpt         theMapOffset;
   int              theWrsPathNumber;
   int              theWrsRowNumber;    
   double           theMeridianalAngle;   
   double           thePositionError;
   double			m_scale;
   double			m_centerLongitude;
   double			m_theMapoffsetX;
   double		    m_theMapoffsetY;
   double			semi_major,semi_minor;
   ProjectionType       theProjectionType;
   rspfRefPtr<rspfMapProjection>  theMapProjection;
   double           theMapAzimAngle; 
   double           theMapAzimCos;    
   double           theMapAzimSin;
   double           theMap2IcRotAngle;
   double           theMap2IcRotCos;
   double           theMap2IcRotSin;
   
   double           theIntrackOffset;
   double           theCrtrackOffset;
   double           theLineGsdCorr;   
   double           theSampGsdCorr;   
   double           theRollOffset;
   double           theYawOffset; 
   double           theYawRate; 
   double           theMapRotation; 
   NEWMAT::Matrix   theRollRotMat;
   
   TYPE_DATA
};
#endif
