#ifndef rspfGFeature_HEADER
#define rspfGFeature_HEADER
#include <iosfwd>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfString.h>
#pragma warning(push)
#pragma warning(disable : 4482)
class RSPFDLLEXPORT rspfDatum;
class RSPFDLLEXPORT rspfEcefPoint;

class RSPFDLLEXPORT rspfGpt;
// Area calss
class RSPFDLLEXPORT rspfGArea
{
public:
	vector<rspfGpt> m_Points;
	rspfString strId;
public:
	/*!
	* Constructors
	*/
	rspfGArea(){strId = "NULL";};
	rspfGArea(vector<rspfGpt> Points, rspfString astrId = "NULL"){m_Points = Points; strId = astrId;};
	bool inArea(rspfGpt gpt);
};

class RSPFDLLEXPORT rspfGLine
{
public:
	rspfGpt first;
	rspfGpt second;
	rspfString strId;
public:
	/*!
	* Constructors
	*/
	rspfGLine(){strId = "NULL";};
	rspfGLine(rspfGpt Gpt1, rspfGpt Gpt2, rspfString astrId = "NULL"){first = Gpt1;second = Gpt2; strId = astrId;};
};

class RSPFDLLEXPORT rspfGPoint
{
public:
	rspfGpt point;
	rspfString strId;
public:
	/*!
	* Constructors
	*/
	rspfGPoint(){point.makeNan(); strId = "NULL";};
	rspfGPoint(const rspfGpt& gpt, rspfString astrId = "NULL"){point = gpt; strId = astrId;};
};

class RSPF_DLL rspfGFeature
{
public:

enum rspfGFeatureType
	{
		rspfGPointType,
		rspfGLineType,
		rspfGAreaType,
		rspfGUnknown
	};
	rspfGFeatureType m_featureType;
	vector<rspfGpt> m_Points;
	rspfString strId;
	rspfGFeature(){m_featureType = rspfGUnknown;};
	rspfGFeature(rspfGFeatureType type){m_featureType = type;};
	rspfGFeature(const rspfGPoint& Gpt);		//initialize as Gpt
	rspfGFeature(const rspfGLine& Line);	//initialize as Line
	rspfGFeature(const rspfGArea& Area);	//initialize as Area
	rspfGFeature(const rspfGFeature& src);
	const rspfGFeature& operator = (const rspfGFeature &aFeature);
	void makeNan(){m_featureType = rspfGUnknown;m_Points.clear();}
	bool isNan()const
	{
	   return (m_Points.size() == 0);
	}
};
#pragma warning(pop)
#endif
