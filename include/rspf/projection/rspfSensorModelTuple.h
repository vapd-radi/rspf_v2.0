#ifndef rspfSensorModelTuple_HEADER
#define rspfSensorModelTuple_HEADER
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/elevation/rspfHgtRef.h>
#include <rspf/matrix/newmat.h>
class rspfEcefPoint;
typedef vector<rspfDpt> DptSet_t;
/**
 * Container class to hold computed rpc model inputs to the
 * rspfPositionQualityEvaluator constructor.  These are stored for retrieval
 * purposes only and will only be initialized if the underlying sensor model
 * is an rpc.
 */
class RSPF_DLL rspfRpcPqeInputs
{
public:
   rspfRpcPqeInputs();
   ~rspfRpcPqeInputs();
   rspf_float64       theRpcElevationAngle; // decimal degrees
   rspf_float64       theRpcAzimuthAngle;   // decimal degrees
   rspf_float64       theRpcBiasError;
   rspf_float64       theRpcRandError;
   rspfColumnVector3d theSurfaceNormalVector;
   NEWMAT::Matrix      theSurfaceCovMatrix;
};
class RSPF_DLL rspfSensorModelTuple
{
public:
   enum DeriveMode
   {
      OBS_INIT =-99,
      EVALUATE =-98,
      P_WRT_X = -1,
      P_WRT_Y = -2,
      P_WRT_Z = -3
   };
   enum IntersectStatus
   {
      OP_SUCCESS      = 0,
      ERROR_PROP_FAIL = 1,
      OP_FAIL         = 2
   };
   /** @brief default constructor */
   rspfSensorModelTuple();
   /** @brief virtual destructor */
   ~rspfSensorModelTuple();
   /**
    * @brief Method to add an image to the tuple.
    */
   void addImage(rspfSensorModel* image);
   
   /**
    * @brief print method.
    */
   std::ostream& print(std::ostream& out) const;
   
   /**
    * @brief Multi-image intersection method.
    *
    * @param obs     Vector of image point observations.
    * @param pt      Intersected ECF position of point.
    * @param covMat  3X3 ECF position covariance matrix [m].
    *
    * @return true on success, false on error.
    */
   rspfSensorModelTuple::IntersectStatus intersect(
      const DptSet_t         obs,
      rspfEcefPoint&  pt,
      NEWMAT::Matrix&  covMat) const;
   
   /**
    * @brief Single-image/DEM intersection method.
    *
    * @param img     Image set index of current image.
    * @param obs     Image point observations.
    * @param pt      Intersected ECF position of point.
    * @param covMat  3X3 ECF position covariance matrix [m].
    *
    * @return true on success, false on error.
    */
   rspfSensorModelTuple::IntersectStatus intersect(
      const rspf_int32&     img,
      const rspfDpt&        obs,
      rspfEcefPoint&  pt,
      NEWMAT::Matrix&  covMat);
   
   /**
    * @brief Single-image/height intersection method.
    *
    * @param img                  Image set index of current image.
    * @param obs                  Image point observations.
    * @param heightAboveEllipsoid Desired intersection height [m].
    * @param pt                   Intersected ECF position of point.
    * @param covMat               3X3 ECF position covariance matrix [m].
    *
    * @return true on success, false on error.
    *
    * @NOTE:  This method's "const" qualifier was removed as it stores rpc
    * inputs to the pqe constructor for report purposes.
    */
   rspfSensorModelTuple::IntersectStatus intersect(
      const rspf_int32&     img,
      const rspfDpt&        obs,
      const rspf_float64&   heightAboveEllipsoid,
      rspfEcefPoint&  pt,
      NEWMAT::Matrix&  covMat);
            
   /**
    * @brief Set intersection surface accuracy method.
    *
    * @param surfCE90 90% CE [m].
    * @param surfLE90 90% LE [m].
    *
    * @return true on success, false on exception.
    * Entry of negative value(s) indicates "no DEM" error prop for RPC
    */
   bool setIntersectionSurfaceAccuracy(const rspf_float64& surfCE90,
                                       const rspf_float64& surfLE90);
   /** @param obj Object to initialize with rpc pqe inputs. */
   void getRpcPqeInputs(rspfRpcPqeInputs& obj) const;
private:
   std::vector<rspfSensorModel*> theImages;
   rspf_int32    theNumImages;
   
   rspf_float64  theSurfCE90;
   rspf_float64  theSurfLE90;
   bool           theSurfAccSet;
   bool           theSurfAccRepresentsNoDEM;
   /**
    * Rpc model only, container to capture pqe inputs for report purposes only.
    */
   rspfRpcPqeInputs theRpcPqeInputs;
   
   /**
    * @brief Compute single image intersection covariance matrix.
    *
    * @param img      Image set index of current image.
    * @param obs      Image point observations.
    * @param ptG      Current ground estimate.
    * @param cRefType Current height reference type.
    * @param covMat   3X3 ECF position covariance matrix.
    *
    * @return true on success, false on error.
    */
   bool computeSingleInterCov(const rspf_int32& img,
                              const rspfDpt&    obs,
                              const rspfGpt&    ptG,
                              HeightRefType_t    cRefType,
                              NEWMAT::Matrix&    covMat);
      
   
   /**
    * @brief Get observation equation components.
    *
    * @param img   Image set index of current image.
    * @param iter  Current iteration.
    * @param obs   Observations.
    * @param ptEst Current ground estimate.
    * @param resid Observation residuals.
    * @param B     Matrix of partials of observations WRT X,Y,Z.
    * @param W     Weight matrix of observations.
    *
    * @param img Image set index of current image.
    */
   bool getGroundObsEqComponents(const rspf_int32 img,
                                 const rspf_int32 iter,
                                 const rspfDpt& obs,
                                 const rspfGpt& ptEst,
                                 rspfDpt& resid,
                                 NEWMAT::Matrix& B,
                                 NEWMAT::SymmetricMatrix& W) const;
   NEWMAT::Matrix invert(const NEWMAT::Matrix& m) const;
};
#endif /* #ifndef rspfSensorModelTuple_HEADER */
