#ifndef rspfSensorModel_HEADER
#define rspfSensorModel_HEADER
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfBilinearProjection.h>
#include <rspf/projection/rspfOptimizableProjection.h>
#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfCommon.h> /* for rspf::nan() */
#include <rspf/elevation/rspfElevSource.h>
#include <rspf/base/rspfAdjustableParameterInterface.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatap.h>
#include <rspf/base/rspfException.h>

#include <rspf/base/rspfTieGpt.h>
#include <rspf/base/rspfTieFeature.h>
#include <alglib/optimization.h>
#include <levmar.h>

#include <Eigen/Eigen>
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>
using namespace Eigen;
using namespace alglib;
class rspfKeywordlist;
class rspfTieGptSet;
struct lmder_functor;
class rspfSensorModel;

// loong
struct optimizeStruct
{
	rspfSensorModel* pThis;
	vector< rspfTieFeature > tieFeatureList;
};
/*!****************************************************************************
 *
 * CLASS:  rspfSensorModel
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfSensorModel : public rspfProjection,
                                        public rspfOptimizableProjection,
                                        public rspfAdjustableParameterInterface
{
public:
   enum CovMatStatus
   {
      COV_INVALID = 0,
      COV_PARTIAL = 1,
      COV_FULL    = 2
   };
   enum DeriveMode
   {
      OBS_INIT =-99,
      EVALUATE =-98,
      P_WRT_X = -1,
      P_WRT_Y = -2,
      P_WRT_Z = -3
   };
      
   static void funcErrorEquation(double *param, double *hx, int nparameter, int nequation, void *adata);
   static void jacErrorEquation(double *param, double *j, int nparameter, int nequation, void *adata);
   
   /*!
    * CONSTRUCTORS:
    */
   rspfSensorModel();
   rspfSensorModel(const rspfSensorModel& copy_this);
   rspfSensorModel(const rspfKeywordlist& geom_kwl);
   virtual rspfObject* getBaseObject();
   virtual const rspfObject* getBaseObject()const;
   
   /*!
    * ACCESS METHODS:
    */
   virtual rspfGpt origin()            const {return theRefGndPt; }
   virtual rspfDpt getMetersPerPixel() const {return rspfDpt(fabs(theGSD.x),
                                                               fabs(theGSD.y));}
   
   virtual const double& getNominalPosError() const { return theNominalPosError; }
   virtual const double& getRelativePosError() const { return theRelPosError; }
   virtual void setNominalPosError(const double& ce90) { theNominalPosError = ce90; }
   virtual void setRelativePosError(const double& ce90) { theRelPosError = ce90; }
   /*!
    * Implementation of base-class pure virtual projection methods. These
    * methods may be overriden by derived classes if those have more efficient
    * schemes. The implementations here are iterative (relatively slow). Both
    * depend on calls to the pure virtual lineSampleHeightToWorld() method.
    */
   virtual void  lineSampleToWorld(const rspfDpt& image_point,
                                   rspfGpt&       world_point) const;
   virtual void  worldToLineSample(const rspfGpt& world_point,
                                   rspfDpt&       image_point) const;
   /*!
    * METHOD: lineSampleHeightToWorld
    * This is the pure virtual that performs the actual work of projecting
    * the image point to the given elevation above MSL. 
    */
   virtual void lineSampleHeightToWorld(const rspfDpt& lineSampPt,
                                        const double&   heightAboveMSL,
                                        rspfGpt&       worldPt) const = 0;
   /*!
    * METHOD: imagingRay(image_point, &rspfEcefRay)
    * Given an image point, returns a ray originating at some arbitrarily high
    * point (ideally at the sensor position) and pointing towards the target.
    * Implemented here but should be overriden for more efficient solution.
    */
   virtual void imagingRay(const rspfDpt& image_point,
                           rspfEcefRay&   image_ray) const;
   /*!
    * METHOD: print()
    * Fulfills base-class pure virtual. Dumps contents of object to ostream.
    */
   virtual std::ostream& print(std::ostream& out) const;
   /**
    * @brief Sets the center line sampe of the image.
    *
    * @param pt Image center point (x = sample, y = line).
    */
   void setRefImgPt(const rspfDpt& pt);
   /**
    * @brief Sets the center latitude, longitude, height of the image.
    *
    * @param pt Image center point.
    */
   void setRefGndPt(const rspfGpt& pt);
   
   void setImageRect(const rspfDrect& imageRect);
   void setGroundRect(const rspfGpt& ul,
                      const rspfGpt& ur,
                      const rspfGpt& lr,
                      const rspfGpt& ll);
   /*!
    * METHOD: imageSize()
    * Returns the maximum line/sample rigorously defined by this model.
    */
   rspfDpt imageSize() const { return theImageSize; }
   void setImageSize(const rspfDpt& size){theImageSize = size;}
   /*!
    * This is from the adjustable parameter interface.  It is
    * called when a paraemter adjustment is made.
    */
   virtual void adjustableParametersChanged()
      {
         updateModel();
      }
   /*!
    * VIRTUAL METHOD: updateModel()
    * Following a change to the adjustable parameter set, this virtual
    * is called to permit instances to compute derived quantities after
    * parameter change.
    */
   virtual void updateModel() {}
   /*!
    * METHODS:  saveState, loadState
    * Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
    * KWL files.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   /*!
    * METHOD: insideImage(image_point)
    * Returns true if the image_point lies inside the image rectangle.
    */
   virtual bool insideImage(const rspfDpt& p) const
      {
/*          return( (p.u>=(0.0-FLT_EPSILON)) && */
/*                  (p.u<=(double)(theImageSize.u-(1-FLT_EPSILON))) && */
/*                  (p.v>=(0.0-FLT_EPSILON)) && */
/*                  (p.v<=(double)(theImageSize.v-(1-FLT_EPSILON))) ); */
         return theImageClipRect.pointWithin(p, 2.0);
      }
   /*!
    * STATIC METHOD: writeGeomTemplate(ostream)
    * Writes a template of keywords processed by loadState and saveState to
    * output stream.
    */
   static void writeGeomTemplate(ostream& os);
   /*!
    * OPERATORS: 
    */
   virtual bool operator==(const rspfProjection& proj) const; //inline below
    const rspfString&   getImageID()               const { return theImageID; }
    const rspfDrect&    getImageClipRect()         const { return theImageClipRect; }
   /*!
    * optimizableProjection implementation
    */
   virtual rspf_uint32 degreesOfFreedom()const;
   inline virtual bool needsInitialState()const {return true;}
   virtual double optimizeFit(const rspfTieGptSet& tieSet,
                              double* targetVariance=0);
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
   /*!
    * METHOD: getObsCovMat()
    * gives 2X2 covariance matrix of observations
    */
   virtual rspfSensorModel::CovMatStatus getObsCovMat(
      const rspfDpt& ipos, NEWMAT::SymmetricMatrix& Cov);
   /**
    * @brief Implementation of pure virtual
    * rspfProjection::isAffectedByElevation method.
    * @return true.
    */
   virtual bool isAffectedByElevation() const { return true; }
   /**
    * This method computes the ground sample distance(gsd) and sets class
    * attributes theGSD and theMeanGSD by doing a lineSampleHeightToWorld on
    * four points and calculating the distance from them.
    *
    * @return Nothing but throws rspfException on error.
    */
   void computeGsd();
   rspfMapProjection* m_proj;

   // loong
   void buildNormalEquation(const vector< rspfTieFeature >& tieLineList,
	   NEWMAT::SymmetricMatrix& A,
	   NEWMAT::ColumnVector& residue,
	   NEWMAT::ColumnVector& projResidue,
	   double pstep_scale,
	   bool useImageObs);
   virtual double optimizeFit(const vector< rspfTieFeature >& tieFeatureList, double* targetVariance = 0);
	// 2010.1.14 loong
	// build a error equation according to a gcp
	// used for block adjustment
	void buildErrorEquation ( const rspfTieGpt&, int nType, NEWMAT::Matrix &A,
							NEWMAT::Matrix &B, NEWMAT::ColumnVector &L, double pstep_scale);
   // 2010.1.14 loong
   // compute the image derivatives regarding the ground coordinate of the tie point
   rspfDpt getCoordinateForwardDeriv(int parmIdx , const rspfGpt& gpos, double hdelta=1e-11) const;

    int getNumofObservations(const vector< rspfTieFeature >& tieFeatureList, bool useImageObs = true);

   //2010.06.07 loong
   // returns ground or image line residue
//   NEWMAT::ColumnVector getResidue(const vector< rspfTieLine >& tieLineList, bool useImageObs = true);
   //2010.06.07 loong
   // returns ground or image feature residue
   NEWMAT::ColumnVector getResidue(const vector< rspfTieFeature >& tieFeatureList, bool useImageObs = true);

   double alglib_optimization(rspfTieGptSet tieSet);
   bool m_bHgtOptimize;
   double optimizeFit_withHeight(rspfTieGptSet& tieSet,double* targetVariance=0);
   void buildNormalEquation_withHeight(rspfTieGptSet& tieSet,
	   NEWMAT::SymmetricMatrix& A,
	   NEWMAT::ColumnVector& residue,
	   NEWMAT::ColumnVector& projResidue,
	   double pstep_scale);
   rspfDpt getForwardDeriv_Height(const rspfGpt& gpos, double hdelta=1e-11);
   double calcHeight(const rspfTieFeature& tieFeature) const;
   virtual void calcHeight_buildNormalEquation(const rspfTieFeature& tieFeature,
	   double& a,
	   NEWMAT::ColumnVector& residue,
	   double& b,
	   double pstep_scale)const;
   NEWMAT::ColumnVector calcHeight_getResidue(const rspfTieFeature& tieFeature)const;
   static double my_model(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data);
   typedef struct  
   {
	   rspfSensorModel *pModel;
	   rspfTieGptSet	*pGptSet;
   } myData;
   static void  function1_fvec(const real_1d_array &x, real_1d_array &fi, void *ptr);
   static void  function1_jac(const real_1d_array &x, real_1d_array &fi, real_2d_array &jac, void *ptr);
protected:
   virtual ~rspfSensorModel();
   
   /*!
    * METHOD: extrapolate()
    * Extrapolates solutions for points outside of the image. The second
    * version accepts a height value -- if left at the default, the elevation
    * will be looked up via theElevation object.
    */
   virtual rspfDpt extrapolate (const rspfGpt& gp) const;
   virtual rspfGpt extrapolate (const rspfDpt& ip,
				 const double& height=rspf::nan()) const;
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
   NEWMAT::ColumnVector getResidue(const rspfTieGptSet& tieSet, bool useImageObs = true);
   NEWMAT::ColumnVector solveLeastSquares(NEWMAT::SymmetricMatrix& A,  NEWMAT::ColumnVector& r)const;
   /*!
    * stable invert stolen from rspfRpcSolver
    */
   NEWMAT::Matrix invert(const NEWMAT::Matrix& m)const;
   rspfIpt       theImageSize;       // pixels
   /*!
    * Support sub-image of larger full image by maintaining offset to UL corner
    */
   rspfDpt       theSubImageOffset;  // pixels
   
   rspfString    theImageID;
   rspfString    theSensorID;
   rspfString    theSensorDATE;
   rspfDpt       theGSD;             // meters
   rspf_float64  theMeanGSD;         // meters
   rspfGpt       theRefGndPt;        // should be image center
   rspfDpt       theRefImgPt;        // should be image center
   rspfPolygon   theBoundGndPolygon;
   rspfDrect     theImageClipRect;
   rspf_float64  theRelPosError; 	// meters, relative to other models in the set
   rspf_float64  theNominalPosError; // meters
   /** Partials for current point */
   rspfDpt theParWRTx;
   rspfDpt theParWRTy;
   rspfDpt theParWRTz;
   
   /** Observations & residuals for current point */
   rspfDpt theObs;
   rspfDpt theResid;
   /**
    * Used as an initial guess for iterative solutions and a guess for points outside the support
    * bounds.
    */ 
   rspfRefPtr<rspfProjection> theSeedFunction;
   
   mutable bool theExtrapolateImageFlag;
   mutable bool theExtrapolateGroundFlag;
   
   double parameters_optimization(const vector< rspfTieFeature >& tieFeatureList, double* targetVariance/* = 0*/);
   double levmar_optimization(const vector< rspfTieFeature >& tieFeatureList, double* targetVariance/* = 0*/);
   double eigen_levmar_optimization(const vector< rspfTieFeature >& tieFeatureList, double* targetVariance = 0);
   static void  alglib_function_fvec(const real_1d_array &x, real_1d_array &fi, void *ptr);
   static void  alglib_function_jac(const real_1d_array &x, real_1d_array &fi, real_2d_array &jac, void *ptr);
   static void levmar_function_fvec(double *param, double *hx, int nparameter, int nequation, void *adata);
   static void levmar_function_jac(double *param, double *jac, int nparameter, int nequation, void *adata);

   friend struct lmder_functor;
TYPE_DATA
};
inline bool rspfSensorModel::operator==(const rspfProjection& proj) const
{
   const  rspfSensorModel* model
      = PTR_CAST(rspfSensorModel, (const rspfProjection*) &proj);
    
   if ( (!model) ||
	(theSensorID != model->theSensorID) ||
	(theImageID != model->theImageID) ||
	(theSubImageOffset != model->theSubImageOffset) )
      return false;
   return true;
}

//loong 

// Generic functor
template<typename _Scalar, int NX=Dynamic, int NY=Dynamic>
struct Functor
{
	typedef _Scalar Scalar;
	enum {
		InputsAtCompileTime = NX,
		ValuesAtCompileTime = NY
	};
	typedef Matrix<Scalar,InputsAtCompileTime,1> InputType;
	typedef Matrix<Scalar,ValuesAtCompileTime,1> ValueType;
	typedef Matrix<Scalar,ValuesAtCompileTime,InputsAtCompileTime> JacobianType;

	const int m_inputs, m_values;

	Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
	Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

	int inputs() const { return m_inputs; }
	int values() const { return m_values; }

protected:

	// you should define that in the subclass :
	//  void operator() (const InputType& x, ValueType* v, JacobianType* _j=0) const;
};

struct lmder_functor : Functor<double>
{
public:
	lmder_functor(int np, int nobs): Functor<double>(np,nobs) {}

	int df(const VectorXd &x, MatrixXd &fjac) const;
	int operator()(const VectorXd &x, VectorXd &fvec) const;
	optimizeStruct *pData;
};
#endif
