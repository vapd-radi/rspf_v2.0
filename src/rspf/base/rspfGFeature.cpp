#include <iostream>
#include <sstream>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfDms.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfGeocent.h>
#include <rspf/base/rspfGFeature.h>
#pragma warning(push)
#pragma warning(disable : 4482)


rspfGFeature::rspfGFeature(const rspfGPoint& Gpt)
{
	m_featureType = rspfGFeatureType::rspfGPointType;
	m_Points.clear();
	strId = Gpt.strId;
	m_Points.push_back(Gpt.point);
}

rspfGFeature::rspfGFeature(const rspfGLine& Line)
{
	m_featureType = rspfGFeatureType::rspfGLineType;
	m_Points.clear();
	m_Points.push_back(Line.first);
	m_Points.push_back(Line.second);
}

rspfGFeature::rspfGFeature(const rspfGArea& Area)
{
	m_featureType = rspfGFeatureType::rspfGAreaType;
	m_Points = Area.m_Points;
}

rspfGFeature::rspfGFeature(const rspfGFeature& src)
: strId(src.strId),
m_Points(src.m_Points),
m_featureType(src.m_featureType)
{
}
const rspfGFeature& rspfGFeature::operator = (const rspfGFeature &aFeature)
{
	strId = aFeature.strId;
	m_featureType = aFeature.m_featureType;
	m_Points = aFeature.m_Points;
	return *this;
}
#pragma warning(pop)