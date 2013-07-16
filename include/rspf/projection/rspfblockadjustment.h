#ifndef rspfBlockAdjustment_HEADER
#define rspfBlockAdjustment_HEADER

#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTieGpt.h>
#include <rspf/base/rspfTieGptSet.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/projection/rspfSensorModel.h>
/************************************************************************/
/*                          Block Adjustment                            */
/*	Author: loong
	Date: 2010.01.20
*/
/************************************************************************/


/***************************************************************************/
/*
 *                              DEFINES
*/
typedef std::pair<rspfDpt, rspfDpt> rspfDline;
typedef std::pair<rspfGpt, rspfGpt> rspfGline;

class RSPFDLLEXPORT rspfBlockTieGpt:public rspfTieGpt
{
public:
	rspfString m_ID;
	int m_nType;						// 0: unknown corresponding image points;
										// 1: known ground control points;
										// 2: known corresponding image points;
	vector< int > m_ImageIndices;		// begin with 0
	vector< rspfDpt > m_DptList;		// corresponding with m_ImageIndices
	int m_nUnknownIndex;				// -1:	unknown corresponding image points;
										// >-1:	known ground control points or known
										// corresponding image points (begin with 0);
public:
	/*!
	* Constructors
	*/
	rspfBlockTieGpt();
	rspfBlockTieGpt(rspfTieGpt tieGpt);
};

class RSPFDLLEXPORT rspfBlockAdjustment
{
public:

	enum SensorType
	{
		modelLandsat5,
		modelSpot5,
		modelLandsat7,
		modelTheos,
		modelAlos,
		modelAlosAVNIR2_1B2,
		modelAlosPRISM_1B2,
		UnknowMole
	};
	enum RobustMode
	{
		NONE	 =0,
		MEDIAN   =1,
		TURKEY   =2,
		HUBER    =3,
		HAMPEL   =4,
		Anddrews =5,
		IGG1     =6,
		FAIR     =7,
		DENMARK  =8
	};
	/*!
	* CONSTRUCTORS:
	*/
	rspfBlockAdjustment();

	virtual ~rspfBlockAdjustment();

	void addSensorModel(rspfSensorModel* sensorModel);
	void adjustment(vector< rspfBlockTieGpt > GptList, RobustMode mode = NONE);
	//void adjustment(vector< rspfTieLine > TieLineList, RobustMode mode = NONE);

public:
protected:
	vector< rspfSensorModel* > m_SensorModleList;
	vector< rspfBlockTieGpt > m_GptList;
	//vector< vector< rspfBlockTieGpt >> m_GptListByImages;
	vector< int > m_FirstParaIndices;		//begin with 0
	int m_nUnknownPoints;


	NEWMAT::DiagonalMatrix m_wgtMatrix;		// weight Matrix of control data

	int initiateGpt(vector< rspfBlockTieGpt > &GptList);
	vector< vector< rspfBlockTieGpt > > classifyGptByImages(vector< rspfBlockTieGpt > GptList);
	void buildErrorEquation(const vector< vector< rspfBlockTieGpt > >& GptListByImages,
		NEWMAT::Matrix &AA, NEWMAT::Matrix &BB, NEWMAT::ColumnVector &LL,
		double pstep_scale);
	NEWMAT::ColumnVector buildNormalEquation(const vector< vector< rspfBlockTieGpt > >& GptListByImages,
																	NEWMAT::Matrix &N11, NEWMAT::Matrix &N12, NEWMAT::Matrix &N22,
																	NEWMAT::ColumnVector &L1, NEWMAT::ColumnVector &L2, double pstep_scale);
	void solve2LeastSquares(NEWMAT::Matrix &N11, NEWMAT::Matrix &N12, NEWMAT::Matrix &N22,
							NEWMAT::ColumnVector &L1, NEWMAT::ColumnVector &L2,
							NEWMAT::ColumnVector &deltap, NEWMAT::ColumnVector &deltac);

	NEWMAT::ColumnVector solveLeastSquares(NEWMAT::SymmetricMatrix& A,  NEWMAT::ColumnVector& r)const;
	//NEWMAT::ColumnVector solveLeastSquares(NEWMAT::Matrix& A,  NEWMAT::ColumnVector& r)const;
	NEWMAT::Matrix invert(const NEWMAT::Matrix& m)const;

	void updateSensorModels(NEWMAT::ColumnVector deltap);
	void updateCoordinates(vector< vector< rspfBlockTieGpt > > &GptListByImages, NEWMAT::ColumnVector deltac);
	NEWMAT::ColumnVector getResidue(const vector< vector< rspfBlockTieGpt > >& GptListByImages);

	void saveBlockGpt(vector<rspfBlockTieGpt> gptList, rspfString outFile, rspfMapProjection* transMerc = NULL);
	void updateWeightsMatrix(const NEWMAT::ColumnVector& newresidue, RobustMode mode = NONE);
	void updateWeightsMatrixHuber(const NEWMAT::ColumnVector& newresidue, double coeff = 2.0);
//TYPE_DATA
};

#endif /* rspfBlockAdjustment_HEADER */
