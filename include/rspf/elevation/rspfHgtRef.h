//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Hicks
//
// Description: Height reference class.
//----------------------------------------------------------------------------
#ifndef rspfHgtRef_HEADER
#define rspfHgtRef_HEADER 1

#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatap.h>
#include <rspf/matrix/newmatio.h>
enum HeightRefType_t
{
   AT_HGT = 0,
   AT_DEM = 1
};


class RSPF_DLL rspfHgtRef
{
public:

   /**
    * @brief constructor.
    */
   rspfHgtRef(HeightRefType_t cRefType);
   rspfHgtRef(HeightRefType_t cRefType, const rspf_float64& atHgt);

   /**
    * @brief virtual destructor.
    */
   virtual ~rspfHgtRef();
   
   /**
    * @brief Method to get height reference type (ellipsoid or DEM).
    */
   HeightRefType_t getHeightRefType() const {return theCurrentHeightRefType;}
   
   /**
    * @brief Method to get height reference.
    *
    * @param pg Reference point.
    *
    * @return Height at reference point.
    */
   virtual rspf_float64 getRefHeight(const rspfGpt& pg) const;
   
   /**
    * @brief Method to get surface information string.
    *
    * @param pg   Reference point.
    * @param info rspfElevationAccuracyInfo string.
    *
    * @return true on success, false on error.
    */
  // virtual bool getSurfaceInfo
  //    (const rspfGpt& pg, rspfElevationAccuracyInfo& info) const;
   
   /**
    * @brief Method to get surface covariance matrix.
    *
    * @param pg  Reference point.
    * @param cov 3X3 ENU covariance matrix.
    *
    * @return true on success, false on error.
    */
   virtual bool getSurfaceCovMatrix
      (const rspfGpt& pg, NEWMAT::Matrix& cov) const;
   
   /**
    * @brief Method to get surface covariance matrix.
    *
    * @param refCE  Reference surface 90% CE [m]
    * @param refLE  Reference surface 90% LE [m]
    * @param cov    3X3 ENU covariance matrix.
    *
    * @return true on success, false on error.
    */
   virtual bool getSurfaceCovMatrix
      (const rspf_float64   refCE, 
       const rspf_float64   refLE,
             NEWMAT::Matrix& cov) const;
   
   /**
    * @brief Method to get surface normal covariance matrix.
    *
    * @param pg      Reference point.
    * @param surfCov 3X3 ENU surface covariance matrix.
    * @param normCov 3X3 ECF normal covariance matrix.
    *
    * @return true on success, false on error.
    */
   bool getSurfaceNormalCovMatrix
      (const rspfGpt&       pg, 
       const NEWMAT::Matrix& surfCov, 
             NEWMAT::Matrix& normCov) const;
   
   /**
    * @brief Method to get local terrain normal unit vector (slope).
    *
    * @param pg Reference point.
    *
    * @return rspfColumnVector3D.
    */
   virtual rspfColumnVector3d getLocalTerrainNormal(const rspfGpt& pg) const;


protected:

private:
   HeightRefType_t theCurrentHeightRefType;
   rspf_float64 theCurrentRefHeight;

};

#endif /* #ifndef rspfHgtRef_HEADER */
