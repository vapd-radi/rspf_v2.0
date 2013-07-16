#ifndef rspfPositionQualityEvaluator_HEADER
#define rspfPositionQualityEvaluator_HEADER
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfLsrVector.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfMatrix3x3.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatap.h>
#include <rspf/matrix/newmatio.h>
enum pqeProbLev_t
{
   ONE_SIGMA = 0,
   P50,
   P90,
   P95,
   NUM_PROB_LEVELS
};
struct pqeErrorEllipse
{
   double   theSemiMinorAxis;
   double   theSemiMajorAxis;
   double   theAzimAngle;
   rspfGpt theCenter;
};
struct pqeRPCModel
{
   double theLineScale;
   double theSampScale;
   double theLatScale;
   double theLonScale;
   double theHgtScale;
   double theLineOffset;
   double theSampOffset;
   double theLatOffset;
   double theLonOffset;
   double theHgtOffset;
   double theLineNumCoef[20];
   double theLineDenCoef[20];
   double theSampNumCoef[20];
   double theSampDenCoef[20];
   char   theType; // A or B
};
typedef std::vector<rspfDpt> pqeImageErrorEllipse_t;
class RSPF_DLL rspfPositionQualityEvaluator
{
public:
   /** @brief default constructor */
   rspfPositionQualityEvaluator();
   
   /** @brief covariance matrix-based constructor
    *
    * @param pt     Current ECF ground estimate.
    * @param covMat 3X3 ECF covariance matrix.
    */
   rspfPositionQualityEvaluator(
      const rspfEcefPoint& pt,
      const NEWMAT::Matrix& covMat);
   
   /** @brief LOS error/geometry-based constructor
    *
    * @param pt           Current ECF ground estimate.
    * @param errBiasLOS   0.68p LOS bias component.
    * @param errRandLOS   0.68p LOS random component.
    * @param elevAngleLOS target elevation angle.
    * @param azimAngleLOS target azimuth angle.
    * @param surfN        surface normal unit vector (defaults to unit Z).
    * @param surfCovMat   surface ENU 3X3 covariance matrix (defaults to zero).
    */
   rspfPositionQualityEvaluator(
      const rspfEcefPoint&      pt,
      const rspf_float64&       errBiasLOS,
      const rspf_float64&       errRandLOS,
      const rspf_float64&       elevAngleLOS,
      const rspf_float64&       azimAngleLOS,
      const rspfColumnVector3d& surfN = rspfColumnVector3d(0,0,1),
      const NEWMAT::Matrix&      surfCovMat = rspfMatrix3x3::createZero());
   
   /** @brief LOS error/coefficient-based constructor
    *
    * @param pt         Current ECF ground estimate.
    * @param errBiasLOS 0.68p LOS bias component.
    * @param errRandLOS 0.68p LOS random component.
    * @param rpc        RPC coefficients.
    * @param surfN      surface normal unit vector (defaults to unit Z).
    * @param surfCovMat surface ENU 3X3 covariance matrix (defaults to zero).
    */
   rspfPositionQualityEvaluator(
      const rspfEcefPoint&      pt,
      const rspf_float64&       errBiasLOS,
      const rspf_float64&       errRandLOS,
      const pqeRPCModel&         rpc,
      const rspfColumnVector3d& surfN = rspfColumnVector3d(0,0,1),
      const NEWMAT::Matrix&      surfCovMat = rspfMatrix3x3::createZero());
   /** @brief virtual destructor */
   ~rspfPositionQualityEvaluator();
   
   /**
    * @brief Add contributing covariance matrix
    *
    * @param covMat 3X3 covariance matrix.
    *
    * @return true on success, false on error.
    */
   bool addContributingCovariance(
      NEWMAT::Matrix& covMat);
   
   /**
    * @brief Add contributing CE/LE
    *
    * @param cCE contributing 90% circular error (m).
    * @param cLE contributing 90% linear error (m).
    *
    * @return true on success, false on error.
    */
   bool addContributingCE_LE(
      const rspf_float64& cCE,
      const rspf_float64& cLE);
   
   /**
    * @brief Subtract contributing covariance matrix
    *
    * @param covMat 3X3 covariance matrix.
    *
    * @return true on success, false on error.
    */
   bool subtractContributingCovariance(
      NEWMAT::Matrix& covMat);
   
   /**
    * @brief Subtract contributing CE/LE
    *
    * @param cCE contributing 90% circular error (m).
    * @param cLE contributing 90% linear error (m).
    *
    * @return true on success, false on error.
    */
   bool subtractContributingCE_LE(
      const rspf_float64& cCE,
      const rspf_float64& cLE);
   
   /**
    * @brief Covariance matrix access
    *
    * @param covMat 3X3 covariance matrix.
    *
    * @return true on success, false on error.
    */
   bool getCovMatrix(
      NEWMAT::Matrix& covMat) const;
   
   /**
    * @brief Compute circular/linear error (CE/LE).
    *
    * @param pLev Probability level.
    * @param CE   pLev% circular error (m).
    * @param LE   pLev% linear error (m).
    *
    * @return true on success, false on error.
    */
   bool computeCE_LE(
      const pqeProbLev_t   pLev,
            rspf_float64& CE,
            rspf_float64& LE) const;
   
   /**
    * @brief Extract error ellipse parameters
    *
    * @param pLev    Probability level.
    * @param ellipse pLev% error ellipse.
    *
    * @return true on success, false on error.
    */
   bool extractErrorEllipse(
      const pqeProbLev_t     pLev,
            pqeErrorEllipse& ellipse);
   
   /**
    * @brief Extract error ellipse parameters; valid only with RPC parameters
    *
    * @param pLev             Probability level.
    * @param angularIncrement Angular increment for ellipse point spacing (deg)
    * @param ellipse          pLev% error ellipse.
    * @param ellImage         pLev% image space error ellipse.
    *
    * @return true on success, false on error.
    */
   bool extractErrorEllipse(
      const pqeProbLev_t            pLev,
      const rspf_float64&          angularIncrement,
            pqeErrorEllipse&        ellipse,
            pqeImageErrorEllipse_t& ellImage);
                                  
   
   /**
    * @brief State accessor.
    */
   inline bool isValid() const { return theEvaluatorValid; }
                                  
   
   /**
    * @brief Print method.
    */
   std::ostream& print(std::ostream& out) const;
protected:
private:
   bool            theEvaluatorValid;
   rspfGpt        thePtG;
   NEWMAT::Matrix  theCovMat;  //local ENU frame
   rspfLsrSpace   theLocalFrame;
   pqeErrorEllipse theEllipse;
   NEWMAT::Matrix  theEigenvectors;
   pqeRPCModel     theRpcModel;
   
   bool decomposeMatrix();
   
   bool constructMatrix(const rspf_float64&       errBiasLOS,
                        const rspf_float64&       errRandLOS,
                        const rspf_float64&       elevAngleLOS,
                        const rspf_float64&       azimAngleLOS,
                        const rspfColumnVector3d& surfN,
                        const NEWMAT::Matrix&      surfCovMat);
   bool formCovMatrixFromCE_LE(const rspf_float64&  CE,
                               const rspf_float64&  LE,
                                     NEWMAT::Matrix& covMat) const;
   double compute90PCE() const;
   
   bool computeElevAzim(const pqeRPCModel     rpc,
                              rspf_float64&  elevAngleLOS,
                              rspf_float64&  azimAngleLOS) const;
   
   double polynomial(const double& nlat,
                     const double& nlon,
                     const double& nhgt,
                     const double* coeffs) const;
   double dPoly_dLat(const double& nlat,
                     const double& nlon,
                     const double& nhgt,
                     const double* coeffs) const;
   double dPoly_dLon(const double& nlat,
                     const double& nlon,
                     const double& nhgt,
                     const double* coeffs) const;
   double dPoly_dHgt(const double& nlat,
                     const double& nlon,
                     const double& nhgt,
                     const double* coeffs) const;
    
    
    
   rspfColumnVector3d vperp(const rspfColumnVector3d& v1,
                             const rspfColumnVector3d& v2) const;
                             
   double atan3(const rspf_float64 y, const rspf_float64 x) const;
};
#endif /* #ifndef rspfPositionQualityEvaluator_HEADER */
