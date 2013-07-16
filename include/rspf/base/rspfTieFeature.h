#ifndef rspfTieFeature_HEADER
#define rspfTieFeature_HEADER
#include <iostream>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfGFeature.h>
#include <rspf/base/rspfDFeature.h>
#pragma warning(push)
#pragma warning(disable : 4482)
class RSPFDLLEXPORT rspfDpt;
class RSPFDLLEXPORT rspfString;
/**
 * storage class for tie point between ground and image
 * based on rspfGpt
 * + GML feature (OGC) serialization
 *
 * NOTES
 * accuracy is not stored here (need to derive object if need a per-point accuracy) 
 * GML storage is WGS84 only, it stores above ellipsoid height (m)
 *
 * TODO : 
 * -support other datum (easy) / ground projection (big change in RSPF)
 * -unify with rspfTDpt
 */
class RSPFDLLEXPORT rspfTieFeature :  rspfReferenced
{
public:
	enum rspfTieFeatureType
	{
		rspfTiePointPoint,
		rspfTieLinePoint,
		rspfTieAreaPoint,
		rspfTiePointLine,
		rspfTieLineLine,
		rspfTieAreaLine,
		rspfTiePointArea,
		rspfTieLineArea,
		rspfTieAreaArea,
		rspfTieUnknown
	};
	rspfGFeature m_groundFeature;
	rspfDFeature m_imageFeature;
	rspfString strId;
	rspf_float64 score;
   inline rspfTieFeature() : 
      rspfReferenced(),
		  score(0.0),
	  strId("")
      {}
   inline rspfTieFeature(const rspfGFeature& gptFeature, const rspfDFeature& dptFeature, const rspf_float64& aScore, const rspfString& astrId="NULL"); 
         
   inline rspfTieFeature(const rspfTieFeature& tieFeature); 
   inline ~rspfTieFeature() {}
	inline const rspfTieFeature& operator=(const rspfTieFeature&);
   inline void					setGroundFeature(rspfGFeature gptFeature);
   inline const rspfGFeature& getGroundFeature()const;
   inline		rspfGFeature& refGroundFeature();
   inline void					setImageFeature(rspfDFeature dptFeature);
   inline const rspfDFeature& getImageFeature()const;
   inline		rspfDFeature& refImageFeature();
   inline void            setId(const rspfString& s) { strId=s; }
   inline void            setScore(const rspf_float64& s) { score=s; }
   inline const rspf_float64& getScore()const             { return score; }
   inline const rspfString& getId()const             { return strId; }
   inline       rspf_float64& refScore()                  { return score; }
   void makeNan() 
      {
       m_groundFeature.makeNan();
       m_imageFeature.makeNan();
       score=rspf::nan();
      }
   
   bool hasNans()const
   {
      return (m_groundFeature.isNan() || m_imageFeature.isNan() || (rspf::isnan(score)));
   }
   
   bool isNan()const
   {
      return (m_groundFeature.isNan() && m_imageFeature.isNan() && (rspf::isnan(score)));
   }
   rspfTieFeatureType getTieFeatureType() const{return m_tieFeatureType;};
   void updateTieFeatureType();
protected:
   rspfTieFeatureType m_tieFeatureType;
};
inline const rspfTieFeature& rspfTieFeature::operator=(const rspfTieFeature& feature)
{
   if (this != &feature)
   {
      rspfReferenced::operator=(feature);
	  m_groundFeature = feature.m_groundFeature;
	  m_imageFeature = feature.m_imageFeature;
      score = feature.score;
	  strId = feature.strId;
	  updateTieFeatureType();
   }
   return *this;
}
#pragma warning(pop)
#endif /* #ifndef rspfTieFeature_HEADER */
