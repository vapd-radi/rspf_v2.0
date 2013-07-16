#include <iostream>
#include <iomanip>
#include <rspf/base/rspfTieGpt.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTieFeature.h>
#pragma warning(push)
#pragma warning(disable : 4482)

inline rspfTieFeature::rspfTieFeature(const rspfGFeature& gptFeature, const rspfDFeature& dptFeature, const rspf_float64& aScore, const rspfString& astrId/*=""*/)
: rspfReferenced(),
score(aScore),
strId(astrId)
{
	m_groundFeature = gptFeature;
	m_imageFeature = dptFeature;
	updateTieFeatureType();
}

inline rspfTieFeature::rspfTieFeature(const rspfTieFeature& tieFeature) :
rspfReferenced(tieFeature),
score(tieFeature.score),
strId(tieFeature.strId)
{
	if (this != &tieFeature)
	{
		rspfReferenced::operator=(tieFeature);
		m_groundFeature = tieFeature.m_groundFeature;
		m_imageFeature = tieFeature.m_imageFeature;
		updateTieFeatureType();
	}
}

inline void rspfTieFeature::setGroundFeature(rspfGFeature gptFeature)
{
	m_groundFeature = gptFeature;
	updateTieFeatureType();
}
inline const rspfGFeature& rspfTieFeature::getGroundFeature()const
{
	return m_groundFeature;
}
inline rspfGFeature& rspfTieFeature::refGroundFeature()
{
	return m_groundFeature;
}
inline void rspfTieFeature::setImageFeature(rspfDFeature dptFeature)
{
	m_imageFeature = dptFeature;
	updateTieFeatureType();
}
inline const rspfDFeature& rspfTieFeature::getImageFeature()const
{
	return m_imageFeature;
}
inline rspfDFeature& rspfTieFeature::refImageFeature()
{
	return m_imageFeature;
}

void rspfTieFeature::updateTieFeatureType()
{
	if(rspfDFeature::rspfDPointType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGPointType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTiePointPoint;
	}
	else if(rspfDFeature::rspfDPointType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGLineType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTiePointLine;
	}
	else if(rspfDFeature::rspfDPointType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGAreaType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTiePointArea;
	}
	else if(rspfDFeature::rspfDLineType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGPointType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTieLinePoint;
	}
	else if(rspfDFeature::rspfDLineType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGLineType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTieLineLine;
	}
	else if(rspfDFeature::rspfDLineType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGAreaType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTieLineArea;
	}
	else if(rspfDFeature::rspfDAreaType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGPointType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTieAreaPoint;
	}
	else if(rspfDFeature::rspfDAreaType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGLineType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTieAreaLine;
	}
	else if(rspfDFeature::rspfDAreaType == m_imageFeature.m_featureType 
		&& rspfGFeature::rspfGAreaType == m_groundFeature.m_featureType)
	{
		m_tieFeatureType = rspfTieAreaArea;
	}
	else
	{
		m_tieFeatureType = rspfTieUnknown;
	}
}
#pragma warning(pop)