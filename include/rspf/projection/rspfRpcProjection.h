#ifndef rspfRpcProjection_HEADER
#define rspfRpcProjection_HEADER
#include <rspf/projection/rspfOptimizableProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfAdjustableParameterInterface.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/matrix/newmat.h>
/*!****************************************************************************
 *
 * CLASS:  rspfRpcProjection
 *
 *****************************************************************************/
class RSPF_DLL rspfRpcProjection : public rspfProjection,
                                     public rspfOptimizableProjection,
                                     public rspfAdjustableParameterInterface
{
public:
   enum PolynomialType
   {
      A='A',  // corresponds to "RPC00A"
      B='B'   // corresponds to "RPC00B"
   };
   
   /*!
    * CONSTRUCTORS:
    */
   rspfRpcProjection();
   rspfRpcProjection(const rspfRpcProjection& copy_this);
   ~rspfRpcProjection();
   rspfRpcProjection& operator=(const rspfRpcProjection& source);
   void initAdjustableParameters();
   virtual void adjustableParametersChanged();
   
   virtual rspfObject* getBaseObject();
   virtual const rspfObject* getBaseObject()const;
   
   void setAttributes(rspf_float64 theSampleOffset,
                      rspf_float64 theLineOffset,
                      rspf_float64 theSampleScale,
                      rspf_float64 theLineScale,
                      rspf_float64 theLatOffset,
                      rspf_float64 theLonOffset,
                      rspf_float64 theHeightOffset,
                      rspf_float64 theLatScale,
                      rspf_float64 theLonScale,
                      rspf_float64 theHeightScale,
                      const std::vector<double>& xNumeratorCoeffs,
                      const std::vector<double>& xDenominatorCoeffs,
                      const std::vector<double>& yNumeratorCoeffs,
                      const std::vector<double>& yDenominatorCoeffs,
                      PolynomialType polyType = B);
                      
   /*!
    * METHOD: worldToLineSample()
    * Overrides base class implementation. Directly computes line-sample from
    * the polynomials.
    */
   virtual void  worldToLineSample(const rspfGpt& world_point,
                                   rspfDpt&       image_point) const;
   /*!
    * METHOD: print()
    * Extends base-class implementation. Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   /*!
    * METHODS:  saveState, loadState
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files. Returns true if successful.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual void  lineSampleToWorld(const rspfDpt& image_point,
                                   rspfGpt&       world_point) const;
   virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
                                        const double&   heightEllipsoid,
                                        rspfGpt&       worldPoint) const;
   /*!
    * METHOD: dup()
    * Returns pointer to a new instance, copy of this.
    */
   virtual rspfObject* dup() const { return new rspfRpcProjection(*this); }
   virtual rspfGpt origin()const;
   virtual rspfDpt getMetersPerPixel() const;
   virtual bool operator==(const rspfProjection& projection) const;
   
   /*
    * optimizable interface
    */
    virtual bool setupOptimizer(const rspfString& setup);
    inline virtual bool useForward()const {return true;}
    virtual rspf_uint32 degreesOfFreedom()const;
    virtual double optimizeFit(const rspfTieGptSet& tieSet, double* targetVariance=NULL);
   /*!
    * METHOD: getForwardDeriv()
    * gives forward() partial derivative regarding parameter parmIdx (>=0)
    * default implementation is centered finite difference
    * -should be reimplemented with formal derivative in child class
    */
   virtual rspfDpt getForwardDeriv(int parmIdx, const rspfGpt& gpos, double hdelta=1e-11);
   /*!
    * METHOD: getInverseDeriv()
    * gives inverse() partial derivative regarding parameter parmIdx (>=0)
    * default implementation is centered finite difference
    * -should be reimplemented with formal derivative in child class
    */
   virtual rspfGpt getInverseDeriv(int parmIdx, const rspfDpt& ipos, double hdelta=1e-11);
   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return true.
    */
   virtual bool isAffectedByElevation() const { return true; }
protected:
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
   
   /*!
    * METHOD: buildNormalEquation
    * builds linearized system  (LMS equivalent)
    * A*dp = projResidue
    * 
    * A: symetric matrix = tJ*J
    * dp: system parameter shift that we want to estimate
    * projResidue = tJ * residue
    *
    * t: transposition operator
    * J = jacobian of transform relative to parameters p, transform can be forward() or inverse()
    * jacobian is obtained via finite differences
    * residue can be image (2D) or ground residue(3D)
    *
    * TODO: use image/ground points covariance matrices
    */
   void buildNormalEquation(const rspfTieGptSet& tieSet,
                                      NEWMAT::SymmetricMatrix& A,
                                      NEWMAT::ColumnVector& residue,
                                      NEWMAT::ColumnVector& projResidue,
                                      double pstep_scale);
   /*!
    * METHOD: getResidue()
    * returns ground opr image residue
    */
   NEWMAT::ColumnVector getResidue(const rspfTieGptSet& tieSet);
   NEWMAT::ColumnVector solveLeastSquares(NEWMAT::SymmetricMatrix& A,  NEWMAT::ColumnVector& r)const;
   /*!
    * stable invert stolen from rspfRpcSolver
    */
   NEWMAT::Matrix invert(const NEWMAT::Matrix& m)const;
   PolynomialType thePolyType;
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
   double theIntrackOffset;
   double theCrtrackOffset;
   double theIntrackScale;
   double theCrtrackScale;
   double theYawSkew;  // = sin(theYawOffset)
   double theCosMapRot;
   double theSinMapRot;
   
   TYPE_DATA
};
#endif
