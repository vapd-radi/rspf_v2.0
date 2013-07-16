#ifndef rspfSarModel_HEADER
#define rspfSarModel_HEADER
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
class rspfHgtRef;
class RSPF_DLL rspfSarModel : public rspfSensorModel
{
public:
   enum AcquisitionMode
   {
      UNKNOWN = 0,
      SCAN    = 1,
      SPOT    = 2
   };
   enum AdjustParamIndex
   {
      X_POS = 0,
      Y_POS,
      Z_POS,
      NUM_ADJUSTABLE_PARAMS // not an index
   };
   /** @brief default constructor */
   rspfSarModel();
   /**
    * @brief Method to load or recreate the state of an rspfSarModel from
    * a keyword list.
    *
    * @param kwl    Keyword list to load from.
    * @param prefix Prefix for keywords, like "image01.".
    *
    * @return true on success, false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   /**
    * @brief Method to save the state of this object to a keyword list.
    *
    * @param kwl    Keyword list to save to.
    * @param prefix Prefix for keywords, like "image01.".
    *
    * @return true on success, false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   
   /*!
    * Writes a template of geom keywords processed by loadState and saveState
    * to output stream.
    */
   static void writeGeomTemplate(ostream& os);
                          
                          
   /*!
    * Returns pointer to a new instance, copy of this.
    */
   virtual rspfObject* dup() const { return 0; } // TBR
   /**
    * @brief print method.
    */
   virtual std::ostream& print(std::ostream& out) const;
   /**
    * @brief 
    */
   virtual void lineSampleToWorld(const rspfDpt& image_point,
                                  rspfGpt&       world_point) const;
   virtual void worldToLineSample(const rspfGpt& world_point,
                                  rspfDpt&       image_point) const;
   virtual void lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                        const double&   heightAboveEllipsoid,
                                        rspfGpt&       worldPt) const;
   /**
    * METHOD: imagingRay(image_point, &rspfEcefRay)
    *  Given an image point, returns a ray originating at the ARP position
    *  and pointing towards the target's position in the Output
    *  Plane.
    *  This DOES NOT provide the conventional definition for an imaging ray
    *  because the imaging locus for SAR is not a ray.
    *
    *  It DOES provide a radius vector for the range/Doppler circle.
    */
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   
   /**
    * @brief Compute partials of range/Doppler WRT ground point
    *
    * @param parmIdx computational mode:
    *        OBS_INIT, EVALUATE, P_WRT_X, P_WRT_X, P_WRT_X.
    * @param gpos Current ground point estimate.
    * @param h Not used.
    *
    * @return OBS_INT: n/a, EVALUATE: residuals, P_WRT_X/Y/Z: partials.
    */
   virtual rspfDpt getForwardDeriv(int parmIdx, const rspfGpt& gpos, double h);
   /**
    * rspfOptimizableProjection
    */
   inline virtual bool useForward() const
   {
      return false; //!image to ground faster
   } 
   /**
    * @brief Compute other parameters & update the model.
    */
   virtual void updateModel();
   /*!
    * METHOD: getObsCovMat()
    * gives 2X2 covariance matrix of observations
    */
   virtual rspfSensorModel::CovMatStatus getObsCovMat(
      const rspfDpt& ipos, NEWMAT::SymmetricMatrix& Cov);
   
protected:
   /** @brief virtual destructor */
   virtual ~rspfSarModel();
   
   
   /*!
    * Assigns initial default values to adjustable parameters and related
    * members.
    */
   void initAdjustableParameters();
   /**
    * @brief Get ARP time for SPOT mode (constant time).
    */
   virtual rspf_float64 getArpTime() const;
   /**
    * @brief Get ARP time for SCAN mode (varying time).
    *
    * @param imgPt The image coordinates.
    */
   virtual rspf_float64 getArpTime(const rspfDpt& imgPt) const;
   /**
    * @brief Get ARP position for SPOT mode (constant time).
    */
   virtual rspfEcefPoint  getArpPos() const;
   /**
    * @brief Get ARP position for SCAN mode (varying time).
    */
   virtual rspfEcefPoint  getArpPos(const rspf_float64& time) const;
   /**
    * @brief Get ARP velocity for SPOT mode (constant time).
    */
   virtual rspfEcefVector getArpVel() const;
   /**
    * @brief Get ARP velocity for SPOT mode (constant time).
    */
   virtual rspfEcefVector getArpVel(const rspf_float64& time) const;
   
   /**
    * @brief Method to compute range & Doppler.
    *
    * @param pt ECF ground point coordinates.
    * @param arpPos ECF ARP position.
    * @param arpVel ECF ARP velocity.
    * @param range  range.
    * @param arpVel Doppler.
    *
    * @return true on success, false on error.
    */
   virtual bool computeRangeDoppler(const rspfEcefPoint& pt,
                                    const rspfEcefPoint& arpPos,
                                    const rspfEcefVector& arpVel,
                                    rspf_float64& range,
                                    rspf_float64& doppler) const;
  /**
    * @brief Method to compute image coordinates from output plane coordinates.
    *
    * @param opPt  ECF output plane position of point.
    * @param imgPt image position of point.
    *
    * @return true on success, false on error.
    */
   virtual bool computeImageFromOP(const rspfEcefPoint& opPt, rspfDpt& imgPt) const;
   
   /**
    * @brief Method to compute output plane coordinates from image coordinates.
    *
    * @param imgPt image position of point.
    * @param opPt  ECF output plane position of point.
    *
    * @return true on success, false on error.
    */
   virtual bool computeOPfromImage(const rspfDpt& imgPt, rspfEcefPoint& opPt) const;
   
   /**
    * @brief Method to project output plane coordinates to surface.
    *
    * @param opPt   ECF output plane point coordinates.
    * @param range  range.
    * @param arpVel Doppler.
    * @param arpPos ECF ARP position.
    * @param arpVel ECF ARP velocity.
    * @param hgtRef Height reference defining intersection surface.
    * @param ellPt  ECF point coordinates.
    *
    * @return true on success, false on error.
    */
   virtual bool projOPtoSurface(const rspfEcefPoint& opPt,
                                const rspf_float64& range,
                                const rspf_float64& doppler,
                                const rspfEcefPoint& arpPos,
                                const rspfEcefVector& arpVel,
                                const rspfHgtRef* hgtRef,
                                rspfEcefPoint& ellPt) const;
   /**
    * @brief Method to project ellipsoid coordinates to output plane.
    *
    * @param ellPt ECF ellipsoid position.
    * @param opPt  ECF output plane position of point.
    *
    * @return true on success, false on error.
    */
   virtual bool projEllipsoidToOP(const rspfEcefPoint& ellPt,
                                  rspfEcefPoint& opPt) const;
   /**
    * Returns the acquisition mode as a string.
    */
   rspfString getAcquistionModeString() const;
   /**
    * @brief Sets the acquisition mode from string.
    *
    * @param mode The string representing mode.
    */
   void setAcquisitionMode(const rspfString& mode);
   /** acquisition mode */
   AcquisitionMode theAcquisitionMode;
   /** Output/Ground Reference Point (ORP) position */
   rspfEcefPoint theOrpPosition;
   /** sample (x)/line(y) image coordinates of ORP  */
   rspfDpt theOrpCenter;
   /** output plane normal */
   rspfEcefVector theOutputPlaneNormal;
   /** output plane x-axis */
   rspfEcefVector theOutputPlaneXaxis;
   /** output impulse response */
   rspf_float64 theOipr;
   /** pixel size */
   rspf_float64 thePixelSize;
   /** Aperture Reference/Center Point (ARP) time in seconds. */
   rspf_float64 theArpTime;
   /**
    * Aperture Reference Point (ARP) Polynomials.
    * Note: Size is derived dependent.
    */
   vector<rspf_float64> theArpXPolCoeff;
   vector<rspf_float64> theArpYPolCoeff;
   vector<rspf_float64> theArpZPolCoeff;
   /** Time Coefficients.  Note: Size is derived dependent. */
   vector<rspf_float64> theTimeCoeff;
   /**
    * Other computed parameters
    */
    
   /** Pixel spacing */
   rspf_float64 thePixelSpacing;
   
   /** Output/slant plane unit vectors */
   rspfEcefVector theOPX;
   rspfEcefVector theOPY;
   rspfEcefVector theOPZ;
   
   
   /**
    * Adjustment-related data used and set by getForwardDeriv
    */
   /** Observations for current point */
   rspf_float64   theObsRng;
   rspf_float64   theObsDop;
   rspf_float64   theObsTime;
   rspfEcefPoint  theObsPosition;
   rspfEcefPoint  theObsOP;
   rspfEcefPoint  theObsArpPos;
   rspfEcefVector theObsArpVel;
   rspfEcefVector theObsArpVel_U;
   rspf_float64   theObsArpVel_Mag;
   
   /** Partials for current point */
   rspf_float64 theParDopWRTaz;
   /** Adjustable parameters */
   rspfEcefVector theLsrOffset;      // meters
   TYPE_DATA
};
#endif /* #ifndef rspfSarModel_HEADER */
