#ifndef rspfRpcSolver_HEADER
#define rspfRpcSolver_HEADER
#include <vector>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/matrix/newmat.h>
#include <rspf/projection/rspfRpcModel.h>
#include <rspf/projection/rspfRpcXyz2RcModel.h>
#include <rspf/projection/rspfRpcRcz2XyModel.h>
#include <rspf/projection/rspfRpcProjection.h>
class rspfProjection;
class rspfImageGeometry;
class rspfNitfRegisteredTag;
/**
 * This currently only support Rational poilynomial B format.  This can be
 * found in the NITF registered commercial tag document.
 *
 * @note x=longitude, y=latitude, z=height
 * 
 * <pre>
 * Format is:
 *  coeff[ 0]       + coeff[ 1]*x     + coeff[ 2]*y     + coeff[ 3]*z     +
 *  coeff[ 4]*x*y   + coeff[ 5]*x*z   + coeff[ 6]*y*z   + coeff[ 7]*x*x   +
 *  coeff[ 8]*y*y   + coeff[ 9]*z*z   + coeff[10]*x*y*z + coeff[11]*x*x*x +
 *  coeff[12]*x*y*y + coeff[13]*x*z*z + coeff[14]*x*x*y + coeff[15]*y*y*y +
 *  coeff[16]*y*z*z + coeff[17]*x*x*z + coeff[18]*y*y*z + coeff[19]*z*z*z;
 *
 *       where coeff is one of XNum, XDen, YNum, and YDen.  So there are 80
 *       coefficients all together.
 *
 *       
 * Currently we use a linear least squares fit to solve the coefficients.
 * This is the simplest to implement.  We probably relly need a nonlinear
 * minimizer to fit the coefficients but I don't have time to experiment.
 * Levenberg Marquardt might be a solution to look into.
 *
 *
 * 
 * HOW TO USE:
 * 
 *        rspfRpcSolver solver;
 *        solver.solveCoefficients(rect,
 *                                 *proj.get());
 *                                 
 * We can also call solve coefficients with a list of ground control points.
 * First is the list of image points followed by the ground points.
 * NOTE: Thes must be equal in size.
 * 
 *        solver.solveCoefficients(imagePoints,
 *                                 groundPoints);
 *                                 
 *                                 
 * Once you call solveCoefficients you can create the projector:
 *                                 
 *        rspfRefPtr<rspfRpcProjection> rpc = solver.createRpcProjection();
 *
 * </pre>
 * 
 */ 
class RSPF_DLL rspfRpcSolver : public rspfReferenced
{
public:
   /**
    * The use elvation flag will deterimne if we force the height t be 0.
    * If the elevation is enabled then we use the height field of the control
    * points to determine the coefficients of the RPC00 polynomial.  If its
    * false then we will ignore the height by setting the height field to 0.0.
    *
    * Note:  even if the elevation is enabled all NAN heights are set to 0.0.
    */
   rspfRpcSolver(bool useElevation=false,
                  bool useHeightAboveMSLFlag=false);
   
   /**
    * This will convert any projector to an RPC model
    */
   void solveCoefficients(const rspfDrect& imageBouunds,
                          rspfProjection* imageProj,
                          rspf_uint32 xSamples=8,
                          rspf_uint32 ySamples=8,
                          bool shiftTo0Flag=true);
   
   void solveCoefficients(const rspfDrect& imageBouunds,
                          rspfImageGeometry* geom,
                          rspf_uint32 xSamples=8,
                          rspf_uint32 ySamples=8,
                          bool shiftTo0Flag=true);
   
   /**
    * takes associated image points and ground points
    * and solves the coefficents for the rational polynomial for
    * line and sample calculations from world points.
    *
    * Note: All data will be normalized between -1 and 1 for
    *       numerical robustness.
    */ 
   void solveCoefficients(const std::vector<rspfDpt>& imagePoints,
                          const std::vector<rspfGpt>& groundControlPoints,
                          const rspfDpt& imageShift = rspfDpt(0.0,0.0));
   /**
    * Creates and Rpc model from the coefficients
    */
   rspfImageGeometry* createRpcModel()const;
   rspfImageGeometry* createRpcXyz2RcModel(rspfMapProjection* proj)const;
   rspfImageGeometry* createRpcRcz2XyModel(rspfMapProjection* proj)const;
   /**
    * Create a simple rpc projection which is a dumbed down
    * rpc model.
    */
   rspfImageGeometry* createRpcProjection()const;
   /**
    * Gives access to the solved coefficients.  For the image
    * X numerator
    */
   const std::vector<double>& getImageXNumCoefficients()const;
   /**
    * Gives access to the solved coefficients.  For the image
    * X denominator
    */
   const std::vector<double>& getImageXDenCoefficients()const;
   /**
    * Gives access to the solved coefficients.  For the image
    * Y numerator
    */
   const std::vector<double>& getImageYNumCoefficients()const;
   /**
    * Gives access to the solved coefficients. For the image
    * Y denominator
    */
   const std::vector<double>& getImageYDenCoefficients()const;
   
   double getImageXOffset()const;
   double getImageYOffset()const;
   double getLatOffset()const;
   double getLonOffset()const;
   double getHeightOffset()const;
   double getImageXScale()const;
   double getImageYScale()const;
   double getLatScale()const;
   double getLonScale()const;
   double getHeightScale()const;
   double getRmsError()const;
   /**
    * @return rspfRefPtr<rspfNitfRegisteredTag>
    *
    * @note one of the solve methods should have been called prior to this.
    */
   rspfRefPtr<rspfNitfRegisteredTag> getNitfRpcBTag() const;
   
protected:
	virtual ~rspfRpcSolver(){}
   
   virtual void solveInitialCoefficients(NEWMAT::ColumnVector& coeff,
                                         const std::vector<double>& f,
                                         const std::vector<double>& x,
                                         const std::vector<double>& y,
                                         const std::vector<double>& z)const;
                                         
   virtual void solveCoefficients(NEWMAT::ColumnVector& coeff,
	   const std::vector<double>& f,
	   const std::vector<double>& x,
	   const std::vector<double>& y,
	   const std::vector<double>& z,
	   double thumb)const;
   
   double eval(const std::vector<double>& coeff,
               double x,
               double y,
               double z)const;
   /**
    * Inverts using the SVD method
    */
   NEWMAT::Matrix invert(const NEWMAT::Matrix& m)const;
   
   void setupSystemOfEquations(NEWMAT::Matrix& equations,
                               const NEWMAT::ColumnVector& f,
                               const std::vector<double>& x,
                               const std::vector<double>& y,
                               const std::vector<double>& z)const;
   void setupWeightMatrix(NEWMAT::DiagonalMatrix& result, // holds the resulting weights
                          const NEWMAT::ColumnVector& coefficients,
                          const NEWMAT::ColumnVector& f,
                          const std::vector<double>& x,
                          const std::vector<double>& y,
                          const std::vector<double>& z)const;
   bool theUseElevationFlag;
   bool theHeightAboveMSLFlag;
   rspfDpt      theImageOffset;
   rspfGpt      theGroundOffset;
   rspfDpt      theImageScale;
   rspf_float64 theLatScale;
   rspf_float64 theLonScale;
   rspf_float64 theHeightScale;
   rspf_float64 theError;
   /**
    * there are 20 coefficients in the cubic RPC model
    */ 
   std::vector<rspf_float64> theXNumCoeffs;
   /**
    * there are 20 coefficients in the cubic RPC model
    */ 
   std::vector<rspf_float64> theXDenCoeffs;
   /**
    * there are 20 coefficients in the cubic RPC model
    */ 
   std::vector<rspf_float64> theYNumCoeffs;
   /**
    * there are 20 coefficients in the cubic RPC model
    */ 
   std::vector<rspf_float64> theYDenCoeffs;

   NEWMAT::Matrix WithoutCol(const NEWMAT::Matrix& X,  int n)const;
   NEWMAT::ColumnVector WithoutCol(const NEWMAT::ColumnVector& y,  int n)const;
   double findMax(const std::vector<double> x, int& maxPos)const;
   NEWMAT::ColumnVector setRpcCoeffs(const NEWMAT::ColumnVector& y, vector<int> removedCols, int nCoeff)const;
   double ColumnVectorMean(const NEWMAT::ColumnVector& y)const;
   bool SetColumnOfMat(NEWMAT::Matrix& X, int pos, const NEWMAT::ColumnVector& y)const;
   bool AppendColumn(NEWMAT::Matrix& X, const NEWMAT::ColumnVector& y)const;

   double LogicSign(const NEWMAT::ColumnVector& y, const NEWMAT::ColumnVector& x)const;

   vector<double> LogicSigns(const NEWMAT::ColumnVector& y, const NEWMAT::Matrix& X)const;

   NEWMAT::ColumnVector NestedEstimate(const NEWMAT::Matrix& X, const NEWMAT::ColumnVector& y, vector<int>& removedColsTotal, double thumb)const;
   void TotalIndex(vector<int>& posTable, int relPos, int& absPos)const;
   NEWMAT::ColumnVector SampleLine(const NEWMAT::Matrix& X, const NEWMAT::ColumnVector& coeff, const vector<int>& totalIndex)const;

public:

	void solveCoefficients_Rcz2Xy(const std::vector<rspfDpt>& imagePoints,
		const std::vector<rspfGpt>& groundControlPoints,
		const rspfDpt& imageShift = rspfDpt(0.0,0.0));
};
#endif
