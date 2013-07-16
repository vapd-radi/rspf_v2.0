#ifndef rspfFcsiModel_HEADER
#define rspfFcsiModel_HEADER
#include <iostream>
using namespace std;
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/projection/rspfRadialDecentLensDistortion.h>
class RSPF_DLL rspfFcsiModel : public rspfSensorModel
{
public:
   /*!
    * CONSTRUCTORS:
    */
   rspfFcsiModel();
   rspfFcsiModel(const rspfFilename& init_file);
   rspfFcsiModel(const rspfKeywordlist& geom_kwl);
   rspfFcsiModel(const rspfFcsiModel& rhs);
   
   enum AdjustParamIndex
   {
      X_POS = 0,
      Y_POS,
      Z_POS,
      X_ROT,
      Y_ROT,
      Z_ROT,
      FOCAL_LEN,
      SCAN_SKEW,
      NUM_ADJUSTABLE_PARAMS // not an index
   };
   /*!
    * Returns pointer to a new instance, copy of this.
    */
   virtual rspfObject* dup() const { return new rspfFcsiModel(*this); }
   
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
   
   /*!
    * Overrides base class pure virtual.
    */
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
    * Rigorous inverse transform implented, overrides base-class' iterative
    * solution.
    */
   virtual void worldToLineSample(const rspfGpt& world_point,
                                  rspfDpt&       image_point) const;
      
   /*!
    * Following a change to the adjustable parameter set, this virtual
    * is called to permit instances to compute derived quantities after
    * parameter change.
    */
   virtual void updateModel();
   /*!
    * Global keywords for use by this model:
    */
   static const char* PRINCIPAL_POINT_X_KW;
   static const char* PRINCIPAL_POINT_Y_KW;
   static const char* SCAN_SCALE_MATRIX_00_KW;
   static const char* SCAN_SCALE_MATRIX_01_KW;
   static const char* SCAN_SCALE_MATRIX_10_KW;
   static const char* SCAN_SCALE_MATRIX_11_KW;
   static const char* SCAN_SKEW_ANGLE_KW;
   static const char* SCAN_ROTATION_ANGLE_KW;
   static const char* FOCAL_LENGTH_KW;
   static const char* PLATFORM_POSITION_X_KW;
   static const char* PLATFORM_POSITION_Y_KW;
   static const char* PLATFORM_POSITION_Z_KW;
   static const char* CAMERA_ORIENTATION_MATRIX_ELEM_KW;
   /*!
    * rspfOptimizableProjection
    */
   inline virtual bool useForward()const {return true;} //!ground to image faster (you don't need DEM)
protected:
   virtual ~rspfFcsiModel() { delete theOpticalDistortion; }
   /*!
    * Assigns initial default values to adjustable parameters and related
    * members.
    */
   void initAdjustableParameters();
   rspfDpt       thePrincipalPoint; // principal point location in pixels
   double         theScanScaleMatrix[4];// converts pixel to film X, Y mm
   double         theScanSkew;       // degrees
   double         theScanRotation;   // degrees left rotation of scanned image
   double         theFocalLen;     // millimeters
   rspfRadialDecentLensDistortion* theOpticalDistortion;
   
   rspfEcefPoint thePlatformPos;
   NEWMAT::Matrix theLsrToEcfRot;
   
   rspfEcefVector theEcfOffset;      // meters
   double          theXrotCorr;       // degrees
   double          theYrotCorr;       // degrees
   double          theZrotCorr;       // degrees
   double          theFocalOffset;    // millimeters
   double          theScanSkewCorr;   // degrees
   rspfEcefPoint theAdjPlatformPos;
   double         theAdjFocalLen;
   NEWMAT::Matrix theAdjLsrToEcfRot;
   NEWMAT::Matrix theAdjEcfToLsrRot;
   double         theScanXformMatrix[4]; 
   double         theInvScanXformMatrix[4];
   TYPE_DATA
};
#endif
