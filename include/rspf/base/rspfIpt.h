//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:  
//
// Contains class declaration for ipt.
// Used to represent an interger point containing an x and y data member.
// 
//*******************************************************************
//  $Id: rspfIpt.h 19793 2011-06-30 13:26:56Z gpotts $

#ifndef rspfIpt_HEADER
#define rspfIpt_HEADER
#include <iosfwd>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfDpt.h>

// Forward class declarations.
class rspfFpt;
class rspfDpt3d;
class rspfString;

class RSPFDLLEXPORT rspfIpt
{
public:

   rspfIpt() : x(0), y(0) {}

   rspfIpt(rspf_int32 anX, rspf_int32 aY) : x(anX), y(aY) {}
         
   rspfIpt(const rspfIpt& pt) : x(pt.x), y(pt.y) {}

   rspfIpt(const rspfDpt& pt);

   rspfIpt(const rspfFpt& pt);

   rspfIpt(const rspfDpt3d &pt);

   const rspfIpt& operator=(const rspfIpt& pt);

   const rspfIpt& operator=(const rspfDpt& pt);

   const rspfIpt& operator=(const rspfFpt& pt);

   bool operator==(const rspfIpt& pt) const
   { return ( (x == pt.x) && (y == pt.y) ); } 
   
   bool operator!=(const rspfIpt& pt) const
   { return ( (x != pt.x) || (y != pt.y) ); }
   
   void makeNan(){x = RSPF_INT_NAN; y=RSPF_INT_NAN;}
   
   bool hasNans()const
   {
      return ( (x==RSPF_INT_NAN) || (y==RSPF_INT_NAN) );
   }
   bool isNan()const
   {
      return ( (x==RSPF_INT_NAN) && (y==RSPF_INT_NAN) );
   }
   
   std::ostream& print(std::ostream& os) const;
   
   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const rspfIpt& pt);

   /**
    * @return rspfString representing point.
    *
    * Format:  ( 30, -90 )
    *            -x- -y-
    */
   rspfString toString() const;

   /**
    * Initializes this point from string.  This method opens an istream to
    * s and then calls operator>>.
    *
    * Expected format:  ( 30, -90 )
    *                     -x- -y-
    *
    * @param s String to initialize from.
    *
    * @see operator>>
    */
   void toPoint(const std::string& s);
   
   /**
    * Method to input the formatted string of the "operator<<".
    *
    * Expected format:  ( 30, -90 )
    *                     -x- -y-
    * 
    * This method starts by doing a "makeNan" on pt.  So if anything goes
    * wrong with the stream or parsing pt could be all or partially nan.
    *
    * @param is Input stream istream to formatted text.
    * @param pt osimIpt to be initialized from stream.
    * @return istream pass in.
    */
   friend RSPFDLLEXPORT std::istream& operator>>(std::istream& is,
                                                  rspfIpt& pt);
   
   bool isEqualTo(const rspfIpt& rhs, rspfCompareType compareType=RSPF_COMPARE_FULL)const;
   /*!
    * METHOD: length()
    * Returns the RSS of the components.
    */
   double length() const { return sqrt((double)x*x + (double)y*y); }
   
   //***
   // OPERATORS: +, -, +=, -=
   // Point add/subtract with other point:
   //***
   rspfIpt operator+(const rspfIpt& p) const
      { return rspfIpt(x+p.x, y+p.y); }
   rspfIpt operator-(const rspfIpt& p) const
      { return rspfIpt(x-p.x, y-p.y); }
   const rspfIpt& operator+=(const rspfIpt& p)
      { x += p.x; y += p.y; return *this; }
   const rspfIpt& operator-=(const rspfIpt& p)
      { x -= p.x; y -= p.y; return *this; }

   //***
   // OPERATORS: *, /
   // Scale point components by scalar:
   //***
   rspfDpt operator*(const double& d) const
      { return rspfDpt(d*x, d*y); }
   rspfDpt operator/(const double& d) const
      { return rspfDpt(x/d, y/d); }

   //***
   // Public data members:
   //***
   union {rspf_int32 x; rspf_int32 u; rspf_int32 samp;};
   union {rspf_int32 y; rspf_int32 v; rspf_int32 line;};

};

inline const rspfIpt& rspfIpt::operator=(const rspfIpt& pt)
{
   if (this != &pt)
   {
      x = pt.x;
      y = pt.y;
   }
   
   return *this;
}
#endif
