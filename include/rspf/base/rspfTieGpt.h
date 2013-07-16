#ifndef rspfTieGpt_HEADER
#define rspfTieGpt_HEADER

#include <iostream>
#include <rspf/base/rspfXmlNode.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfReferenced.h>

class rspfDpt;

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
class RSPFDLLEXPORT rspfTieGpt :  
                              public rspfReferenced,
                              public rspfGpt
{
public:

   inline rspfTieGpt() : 
      rspfReferenced(),
      rspfGpt(),
      tie(),
	  score(0.0),
	  GcpNumberID("")
      {}

   inline rspfTieGpt(const rspfGpt& aPt, const rspfDpt& aTie, const rspf_float64& aScore, const rspfString& aGcpNumberID="") : 
      rspfReferenced(),
      rspfGpt(aPt),
      tie(aTie),
	  score(aScore),
	  GcpNumberID(aGcpNumberID)
      {}
         
   inline rspfTieGpt(const rspfTieGpt& tpt) : 
      rspfReferenced(tpt),
      rspfGpt(tpt),
      tie(tpt.tie),
	  score(tpt.score),
	  GcpNumberID(tpt.GcpNumberID)
      {}
   inline ~rspfTieGpt() {}

   const rspfTieGpt& operator=(const rspfTieGpt&);

   inline void            setGroundPoint(const rspfGpt& mPt) { rspfGpt::operator=(mPt); }
   inline const rspfGpt& getGroundPoint()const               { return *this; }
   inline       rspfGpt& refGroundPoint()                    { return *this; }

   inline void            setImagePoint(const rspfDpt& sPt) { tie=sPt; }
   inline const rspfDpt& getImagePoint()const               { return tie; }
   inline       rspfDpt& refImagePoint()                    { return tie; }

   inline void            setScore(const rspf_float64& s) { score=s; }
   inline const rspf_float64& getScore()const             { return score; }
   inline       rspf_float64& refScore()                  { return score; }
   inline const rspfString& getGcpNumberID()const             { return GcpNumberID; }

   void makeNan() 
      {
       rspfGpt::makeNan();
       tie.makeNan();
       score=rspf::nan();
      }
   
   bool hasNans()const
   {
      return (rspfGpt::hasNans() || tie.hasNans() || (rspf::isnan(score)));
   }
   
   bool isNan()const
   {
      return (rspfGpt::isNan() && tie.isNan() && (rspf::isnan(score)));
   }
   /**
    * text serialization
    */
   std::ostream& print(std::ostream& os) const;
   std::ostream& printTab(std::ostream& os) const;

   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const rspfTieGpt& pt);
   /**
    * Method to input the formatted string of the "operator<<".
    *
    * Expected format:  ( ( rspfGpt ), ( rspfDpt ), 0.50000000000000 )
    *                     --*this---- , ----tie-----, ---score--------
    * 
    */
   friend RSPFDLLEXPORT std::istream& operator>>(std::istream& is,
                                                  rspfTieGpt& pt);
   /**
    * GML feauture (XML) serialization
    */
   rspfRefPtr<rspfXmlNode> exportAsGmlNode(rspfString aGmlVersion="2.1.2")const;
   bool importFromGmlNode(rspfRefPtr<rspfXmlNode> aGmlNode, rspfString aGmlVersion="2.1.2");
   
   /**
    * Public data members
    */
   rspfDpt      tie;
   rspf_float64 score;
   rspfString GcpNumberID;
};

inline const rspfTieGpt& rspfTieGpt::operator=(const rspfTieGpt& pt)
{
   if (this != &pt)
   {
      rspfReferenced::operator=(pt);
      rspfGpt::operator=(pt);
      tie   = pt.tie;
      score = pt.score;
	  GcpNumberID=pt.GcpNumberID;
   }
   
   return *this;
}

#endif /* #ifndef rspfTieGpt_HEADER */
