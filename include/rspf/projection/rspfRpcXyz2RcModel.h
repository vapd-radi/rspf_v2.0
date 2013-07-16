#ifndef rspfRpcXyz2RcModel_HEADER
#define rspfRpcXyz2RcModel_HEADER
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDblGrid.h>
#include <rspf/base/rspfFilename.h>

#include <alglib/optimization.h>
using namespace alglib;
/*!****************************************************************************
*
* CLASS:  rspfRpcXyz2RcModel
*
*****************************************************************************/
class RSPFDLLEXPORT rspfRpcXyz2RcModel : public rspfSensorModel
{
public:
	enum PolynomialType
	{
		A='A',  // corresponds to "RPC00A"
		B='B'   // corresponds to "RPC00B"
	};
	/** @brief RPC model structure used for access function */
	struct rpcModelStruct
	{
		double lineScale;
		double sampScale;
		double latScale;
		double lonScale;
		double hgtScale;
		double lineOffset;
		double sampOffset;
		double latOffset;
		double lonOffset;
		double hgtOffset;
		double lineNumCoef[20];
		double lineDenCoef[20];
		double sampNumCoef[20];
		double sampDenCoef[20];
		char   type;
	};
	/** @brief default constructor */
	rspfRpcXyz2RcModel();
	/** @brief copy construtor */
	rspfRpcXyz2RcModel(const rspfRpcXyz2RcModel& copy_this);
	/** @brief virtual destructor */
	virtual ~rspfRpcXyz2RcModel();
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
		PolynomialType polyType = B,
		bool computeGsdFlag=true);
	void setAttributes(rspfRpcXyz2RcModel::rpcModelStruct& model, bool computeGsdFlag=true);
	void setMetersPerPixel(const rspfDpt& metersPerPixel);
	/**
	* This method computes the ground sample distance(gsd) and sets class
	* attribute theGSD and theMeanGSD by doing a lineSampleHeightToWorld on
	* three points and calculating the distance from them.
	*
	* @return Nothing but throws rspfException on error.
	*/
	void computeGsd();
	/**
	* @brief Sets data member theBiasError, theRandError.
	*
	* @param biasError Error - Bias 68% non time - varying error estimate
	* assumes correlated images.  Units = meters.
	*
	* @param randError Error - Random 68% time - varying error estimate
	* assumes uncorrelated images. Units = meters.
	*
	* @param initNominalPostionErrorFlag If true the base data member
	* theNominalPosError will be initialized with:
	* sqrt(theBiasError*theBiasError +theRandError*theRandError)
	*/
	void setPositionError(const rspf_float64& biasError,
		const rspf_float64& randomError,
		bool initNominalPostionErrorFlag);
	/**
	* @brief worldToLineSample()
	* Overrides base class implementation. Directly computes line-sample from
	* the polynomials.
	*/
	virtual void  worldToLineSample(const rspfGpt& world_point,
		rspfDpt&       image_point) const;
	/**
	* @brief print()
	* Extends base-class implementation. Dumps contents of object to ostream.
	*/
	virtual std::ostream& print(std::ostream& out) const;
	/**
	* @brief saveState
	* Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
	* KWL files. Returns true if successful.
	*/
	virtual bool saveState(rspfKeywordlist& kwl,
		const char* prefix=0) const;

	/**
	* @brief loadState
	* Fulfills rspfObject base-class pure virtuals. Loads and saves geometry
	* KWL files. Returns true if successful.
	*/
	virtual bool loadState(const rspfKeywordlist& kwl,
		const char* prefix=0);
	virtual void  lineSampleToWorld(const rspfDpt& image_point,
		rspfGpt&       world_point) const;
	virtual void lineSampleHeightToWorld(const rspfDpt& image_point,
		const double&   heightEllipsoid,
		rspfGpt&       worldPoint) const;

	/**
	* @brief imagingRay()
	* Overrides base class pure virtual.
	*/
	virtual void imagingRay(const rspfDpt& image_point,
		rspfEcefRay&   image_ray) const;
	/**
	* @brief STATIC METHOD: writeGeomTemplate(ostream)
	* Writes a template of geom keywords processed by loadState and saveState
	* to output stream.
	*/
	static void writeGeomTemplate(ostream& os);
	virtual void updateModel();
	virtual void initAdjustableParameters();
	/**
	* @brief dup()
	* Returns pointer to a new instance, copy of this.
	*/
	virtual rspfObject* dup() const;
	inline virtual bool useForward()const {return false;}
	/** @brief uses file path to init model */
	virtual bool setupOptimizer(const rspfString& init_file);
	/**
	* @brief Compute partials of samp/line WRT ground point
	*
	* @param parmIdx computational mode:
	*        OBS_INIT, EVALUATE, P_WRT_X, P_WRT_X, P_WRT_X.
	*
	* @param gpos Current ground point estimate.
	*
	* @param h Not used.
	*
	* @return OBS_INT: n/a, EVALUATE: residuals, P_WRT_X/Y/Z: partials.
	*/
	virtual rspfDpt getForwardDeriv(int parmIdx,
		const rspfGpt& gpos,
		double h);
	/**
	* @brief Returns Error - Bias.
	* @return Error - Bias
	* @note See NITF field "ERR_BIAS" from RPC00x tag where x = A or B.
	*/
	double getBiasError() const;
	/**
	* @brief Returns Error - Random.
	* @return Error - Random
	* @note See NITF field "ERR_RAND" from RPC00x tag where x = A or B.
	*/
	double getRandError() const;
	/**
	* @brief Returns RPC parameter set in structure.
	* @param rpcModelStruct structure to initialize.
	*/
	void getRpcParameters(rspfRpcXyz2RcModel::rpcModelStruct& model) const;


	// loong 2012.2
	vector<rspfDpt> getModelForwardDeriv(const rspfGpt& pos, double h);
	void updateRpcModelParams(NEWMAT::ColumnVector deltap);

	int getModelParamNum()const {return 80;};

	enum rpcOptimizeType
	{
		allParam,		//所有参数
		imageAffine,	//像方仿射变换
		unknown,		//未知
	};
	rpcOptimizeType m_optimizeType;	
	void saveRpcModelStruct(fstream &fs)const;
	static void rpcForward(int mode, int n, const NEWMAT::ColumnVector& x, double& fx, NEWMAT::ColumnVector& g, int& result, const NEWMAT::ColumnVector& paramList);
	static void init_rpcForward(int n, NEWMAT::ColumnVector& x, const NEWMAT::ColumnVector& paramList);
	//static PolynomialType m_Static_thePolyType;
	//static double m_Static_nLat;	//标准化后的纬度
	//static double m_Static_nLon;	//标准化后的经度
	//static double m_Static_nHgt;	//标准化后的高程
	//static double m_Static_U;		//标准化后的Line
	//static double m_Static_V;		//标准化后的Sample

	//static double m_Static_theLineNumCoef[20];
	//static double m_Static_theLineDenCoef[20];
	//static double m_Static_theSampNumCoef[20];
	//static double m_Static_theSampDenCoef[20];


protected:
	enum AdjustParamIndex
	{
		INTRACK_OFFSET = 0,
		CRTRACK_OFFSET,
		INTRACK_SCALE,
		CRTRACK_SCALE,
		MAP_ROTATION,
		NUM_ADJUSTABLE_PARAMS // not an index
	};
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

	double theIntrackOffset;
	double theCrtrackOffset;
	double theIntrackScale;
	double theCrtrackScale;
	double theCosMapRot;
	double theSinMapRot;
	/** error */
	double theBiasError; // meters
	double theRandError; // meters
	double theLineNumCoef[20];
	double theLineDenCoef[20];
	double theSampNumCoef[20];
	double theSampDenCoef[20];

	TYPE_DATA
};
#endif
