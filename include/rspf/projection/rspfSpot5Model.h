#ifndef rspfSpot5Model_HEADER
#define rspfSpot5Model_HEADER
#include <iostream>
using namespace std;
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfMatrix3x3.h>
class rspfSpotDimapSupportData;
class RSPFDLLEXPORT rspfSpot5Model : public rspfSensorModel
{
public:
   /*!
    * CONSTRUCTORS:
    */
   rspfSpot5Model();
   rspfSpot5Model(rspfSpotDimapSupportData* sd);
   rspfSpot5Model(const rspfFilename& init_file);
   rspfSpot5Model(const rspfKeywordlist& geom_kwl);
   rspfSpot5Model(const rspfSpot5Model& rhs);
   virtual ~rspfSpot5Model();
   enum AdjustParamIndex
   {
      ROLL_OFFSET = 0,
      PITCH_OFFSET,
      YAW_OFFSET,
      ROLL_RATE,
      PITCH_RATE,
      YAW_RATE,
      FOCAL_LEN_OFFSET,
      NUM_ADJUSTABLE_PARAMS // not an index
   };   
   /*!
    * Returns pointer to a new instance, copy of this.
    * Not implemented yet!  Returns NULL...
    */
   virtual rspfObject* dup() const;
   /*!
    * Extends base-class implementation. Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   /*!
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL) const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
   /*!
    * Writes a template of geom keywords processed by loadState and saveState
    * to output stream.
    */
   static void writeGeomTemplate(ostream& os);
   /*!
    * Given an image point and height, initializes worldPoint.
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const rspf_float64& heightEllipsoid,
                                        rspfGpt& worldPoint) const;
   
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
   inline virtual bool useForward()const {return true;/*false;*/} //!image to ground faster
   virtual bool setupOptimizer(const rspfString& init_file); //!uses file path to init model
   bool initFromMetadata(rspfSpotDimapSupportData* sd);
protected:
   /*!
    * Sets adjustables to default values.
    */
   void initAdjustableParameters();
   
   void loadGeometry(FILE*);
   void loadSupportData();
   void computeSatToOrbRotation(NEWMAT::Matrix& result, rspf_float64 t)const;
/*    virtual rspfDpt extrapolate (const rspfGpt& gp) const; */
/*    virtual rspfGpt extrapolate (const rspfDpt& ip, */
/* 				 const double& height=rspf::nan()) const; */
   rspfRefPtr<rspfSpotDimapSupportData> theSupportData;
   rspfFilename  theMetaDataFile;
   rspf_float64  theIllumAzimuth;  
   rspf_float64  theIllumElevation;
   rspf_float64  thePositionError;
   rspf_float64  theRefImagingTime;
   /** relative to full image */
   rspf_float64  theRefImagingTimeLine;
   
   rspf_float64  theLineSamplingPeriod;
   rspfDpt       theSpotSubImageOffset;
   rspf_float64  theRollOffset;      // degrees
   rspf_float64  thePitchOffset;     // degrees
   rspf_float64  theYawOffset;       // degrees
   rspf_float64  theRollRate;        // degrees/sec
   rspf_float64  thePitchRate;       // degrees/sec
   rspf_float64  theYawRate;         // degrees/sec
   rspf_float64  theFocalLenOffset;  // percent deviation from nominal
TYPE_DATA
};
#endif /* #ifndef rspfSpot5Model_HEADER */
