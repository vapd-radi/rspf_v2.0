#ifndef rspfRpcModel_HEADER
#define rspfRpcModel_HEADER
#include <rspf/projection/rspfSensorModel.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDblGrid.h>
#include <rspf/base/rspfFilename.h>
/*!****************************************************************************
 *
 * CLASS:  rspfRpcModel
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfRpcModel : public rspfSensorModel
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
   rspfRpcModel();
   /** @brief copy construtor */
   rspfRpcModel(const rspfRpcModel& copy_this);
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
   void setMetersPerPixel(const rspfDpt& metersPerPixel);
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
   inline virtual bool useForward()const {return true;}
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
   void getRpcParameters(rspfRpcModel::rpcModelStruct& model) const;



   int getModelParamNum()const {return 80;};

   void setAttributes(rspfRpcModel::rpcModelStruct& model, bool computeGsdFlag=true);

   enum rpcOptimizeType
   {
	   OptNone,					//�޸���
	   OptImageTranslation,		//ƽ�Ʊ任
	   OptImageTrans_scale,		//ƽ�Ƽ�����
	   OptImageAffine,			//�񷽷���任
	   NUM_OPTIMIZE_TYPE,
   };
   void saveRpcModelStruct(fstream &fs)const;
   void saveRpcModelStruct(rspfRpcModel::rpcModelStruct& rpcStruct)const;
   static void rpcForward(int mode, int n, const NEWMAT::ColumnVector& x, double& fx, NEWMAT::ColumnVector& g, int& result, const NEWMAT::ColumnVector& paramList);
   static void init_rpcForward(int n, NEWMAT::ColumnVector& x, const NEWMAT::ColumnVector& paramList);
   static void update_model(int, int, NEWMAT::ColumnVector) {};
   void lineSampleHeightToWorld2(const rspfDpt& image_point,
	   const double&   heightEllipsoid,
	   rspfGpt&       worldPoint)const;
   
   rpcOptimizeType m_modelOptimizeType;
	virtual rspf_uint32 getNumberOfAdjustableParameters()const;
   virtual double optimizeFit(const rspfTieGptSet& tieSet,
	   double* targetVariance=0);
   //void updateRpcs(const real_1d_array &x);
   //void deupdateRpcs(const real_1d_array &x);
   //void saveRpcs(real_1d_array &x);
   //void loadRpcs(const real_1d_array &x);   
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
   /** @brief virtual destructor */
   virtual ~rspfRpcModel();
   
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