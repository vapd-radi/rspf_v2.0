#ifndef rspfDFeature_HEADER
#define rspfDFeature_HEADER
#include <iosfwd>
#include <string>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfString.h>
#pragma warning(push)
#pragma warning(disable : 4482)
class RSPFDLLEXPORT rspfIpt;
class RSPFDLLEXPORT rspfFpt;
class RSPFDLLEXPORT rspfDpt3d;
class RSPFDLLEXPORT rspfGpt;
class RSPFDLLEXPORT rspfDpt;

bool isPointOnSegment(rspfDpt pt, rspfDpt a, rspfDpt b);
rspfDpt Distance2Segment(rspfDpt pt, rspfDpt a, rspfDpt b, rspfDpt* intersection = NULL);
double CrossMultiplication(rspfDpt pt, rspfDpt a, rspfDpt b);

#ifndef PI
#define PI 3.14159265
#endif
// Line class
class RSPFDLLEXPORT rspfDLine
{
public:
	rspfDpt getFirstPoint()const{return first;};
	rspfDpt getSecondPoint()const{return second;};
	rspfString getId()const{return strId;};
	double getRho()const{return rho;};
	double getTheta()const{return theta;};

	void setPoints(rspfDpt Dpt1, rspfDpt Dpt2);
public:
	/*!
	* Constructors
	*/
	rspfDLine(){};
	rspfDLine(rspfDpt Dpt1, rspfDpt Dpt2);
	// 点到直线的距离
	double DistanceFromPoint(rspfDpt pt)	const;
	double DistanceFromPoint2(const rspfDpt& p) const;
	rspfDpt DistanceFromSegment(const rspfDLine& l) const;
	rspfDpt DistanceAndSegment(const rspfDLine& l) const;
	void toPoint(rspfDpt *pt) const;
	void getPointDistanceDeriv(rspfDpt dpt, double* partial_x, double* partial_y, double hdelta =1e-6) const;
	void getSegmentDistanceDeriv(const rspfDLine& l, double* partial_x, double* partial_y, double hdelta =1e-6) const;
	void getDistanceDeriv(rspfDpt* partial_x, rspfDpt* partial_y) const;
protected:	
	rspfDpt first;
	rspfDpt second;
	rspfString strId;
	
/************************************************************************/
/* 
	rho = -x * sin(theta) + y * cos(theta)
*/
/************************************************************************/
	double rho;
	double theta;
	friend class rspfDFeature;
};

// Area class
class RSPFDLLEXPORT rspfDArea
{
public:
	vector<rspfDpt> m_Points;
	rspfString strId;
public:
	/*!
	* Constructors
	*/
	rspfDArea(){};
	rspfDArea(const vector<rspfDpt>& Points);
	rspfDpt DistanceFromPoint(rspfDpt pt)	const;
	// 面到面的距离, index为area中到面的距离最大的顶点序号
	rspfDpt DistanceFromArea(const rspfDArea &area, int *index = NULL) const;
	// 面到面的距离, index为area中到面的适中距离的顶点序号
	rspfDpt DistanceFromAreaMid(const rspfDArea &area, int *index = NULL) const;
	rspfDpt getPointDistanceDeriv_X(rspfDpt dpt, double  hdelta =1e-5) const;
	rspfDpt getPointDistanceDeriv_Y(rspfDpt dpt, double  hdelta =1e-5) const;
	void getPointDistanceDeriv(rspfDpt dpt, rspfDpt* partial_x, rspfDpt* partial_y, double hdelta =1e-6) const;
	int X_Intersection(rspfDpt pt, rspfDpt a, rspfDpt b, rspfDpt* intersection = NULL)const;
private:
	double Distance2Straightline(rspfDpt pt, rspfDpt a, rspfDpt b, rspfDpt* intersection = NULL)const;
};

class RSPFDLLEXPORT rspfDPoint
{
public:
	rspfDpt point;
	rspfString strId;
public:
	/*!
	* Constructors
	*/
	rspfDPoint(){point.makeNan(); strId = "NULL";};
	rspfDPoint(const rspfDpt& gpt, rspfString astrId = "NULL"){point = gpt; strId = astrId;};
};

class RSPF_DLL rspfDFeature
{
public:
	
enum rspfDFeatureType
	{
		rspfDPointType,
		rspfDLineType,
		rspfDAreaType,
		rspfDUnknown
	};
	rspfDFeatureType m_featureType;
	vector<rspfDpt> m_Points;
	rspfString strId;
   /**
    * Constructor.  The values are assumed to be in DEGREES.
    */
   rspfDFeature();
   rspfDFeature(const rspfDFeature::rspfDFeatureType& type);
   rspfDFeature(const rspfDPoint& Dpt);		//initialize as Gpt
   rspfDFeature(const rspfDLine& Line);	//initialize as Line
   rspfDFeature(const rspfDArea& Area);	//initialize as Area
   rspfDFeature(const rspfDFeature& src);
   const rspfDFeature& operator = (const rspfDFeature &aFeature);
   void makeNan(){m_featureType = rspfDUnknown;m_Points.clear();}
   bool isNan()const
   {
	   return (m_Points.size() == 0);
   }
};
#pragma warning(pop)
#endif
