// rspfTDpt
// storage class for tie point (no height, no projection information)
// based on rspfDpt
// 
#ifndef rspfTDpt_HEADER
#define rspfTDpt_HEADER

#include <iostream>

#include <rspf/base/rspfDpt.h>

// Forward class declarations.
class rspfIpt;
class rspfFpt;
class rspfDpt3d;
class rspfGpt;

class RSPFDLLEXPORT rspfTDpt : public rspfDpt
{
public:

   inline rspfTDpt() : rspfDpt(), tie(), score(0.0) {}

   inline rspfTDpt(const rspfDpt& aPt, const rspfDpt& aTie, const rspf_float64& aScore) : rspfDpt(aPt), tie(aTie), score(aScore) {}
         
   inline rspfTDpt(const rspfTDpt& tpt) : rspfDpt(tpt), tie(tpt.tie), score(tpt.score) {}

   const rspfTDpt& operator=(const rspfTDpt&);

   inline void            setMasterPoint(const rspfDpt& mPt) { rspfDpt::operator=(mPt); }
   inline const rspfDpt& getMasterPoint()const               { return *this; }
   inline       rspfDpt& refMasterPoint()                    { return *this; }

   inline void            setSlavePoint(const rspfDpt& sPt) { tie=sPt; }
   inline const rspfDpt& getSlavePoint()const               { return tie; }
   inline       rspfDpt& refSlavePoint()                    { return tie; }

   void makeNan() 
   {
      rspfDpt::makeNan();
      tie.makeNan();
      score=rspf::nan();
   }
   
   bool hasNans()const
   {
      return (rspfDpt::hasNans() || tie.hasNans() || (rspf::isnan(score)));
   }
   
   bool isNan()const
   {
      return (rspfDpt::isNan() && tie.isNan() && (rspf::isnan(score)));
   }

   std::ostream& print(std::ostream& os) const;
   std::ostream& printTab(std::ostream& os) const;

   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const rspfTDpt& pt);

   /**
    * Method to input the formatted string of the "operator<<".
    *
    * Expected format:  ( ( rspfDpt ), ( rspfDpt ), 0.50000000000000 )
    *                     --*this---- , ----tie-----, ---score--------
    * 
    */
   friend RSPFDLLEXPORT std::istream& operator>>(std::istream& is,
                                                  rspfTDpt& pt);
   
   //***
   // Public data members:
   //***
   rspfDpt      tie;
   rspf_float64 score;
};

inline const rspfTDpt& rspfTDpt::operator=(const rspfTDpt& pt)
{
   if (this != &pt)
   {
      rspfDpt::operator=(pt);
      tie   = pt.tie;
      score = pt.score;
   }
   
   return *this;
}

#endif /* #ifndef rspfTDpt_HEADER */
