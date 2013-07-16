//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for rspfDpt.
// Used to represent an double point containing an x and y data member.
// 
//*******************************************************************
//  $Id: rspfDpt.h 19793 2011-06-30 13:26:56Z gpotts $
#ifndef rspfDpt_HEADER
#define rspfDpt_HEADER

#include <iosfwd>
#include <string>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfString.h>

// Forward class declarations.
class rspfIpt;
class rspfFpt;
class rspfDpt3d;
class rspfGpt;

class RSPFDLLEXPORT rspfDpt
{
public:

   rspfDpt() : x(0), y(0) {}

   rspfDpt(double anX, double aY) : x(anX), y(aY) {}
         
   rspfDpt(const rspfDpt& pt) : x(pt.x), y(pt.y) {}

   rspfDpt(const rspfFpt& pt);
   
   rspfDpt(const rspfIpt& pt);

   rspfDpt(const rspfDpt3d &pt);

   rspfDpt(const rspfGpt &pt); // assigns lat, lon only

   const rspfDpt& operator=(const rspfDpt&);

   const rspfDpt& operator=(const rspfFpt&);
   
   const rspfDpt& operator=(const rspfIpt&);

   const rspfDpt& operator=(const rspfDpt3d&);

   const rspfDpt& operator=(const rspfGpt&); // assigns lat, lon only

   bool operator==(const rspfDpt& pt) const
   { return ( rspf::almostEqual(x, pt.x) && rspf::almostEqual(y, pt.y) ); } 

   bool operator!=(const rspfDpt& pt) const
   { return !(*this == pt ); }

   void makeNan(){x = rspf::nan(); y=rspf::nan();}
   
   bool hasNans()const
   {
      return (rspf::isnan(x) || rspf::isnan(y));
   }

   bool isNan()const
   {
      return (rspf::isnan(x) && rspf::isnan(y));
   }

   /*!
    * METHOD: length()
    * Returns the RSS of the components.
    */
   double length() const { return sqrt(x*x + y*y); }
   
   //***
   // OPERATORS: +, -, +=, -=
   // Point add/subtract with other point:
   //***
   rspfDpt operator+(const rspfDpt& p) const
      { return rspfDpt(x+p.x, y+p.y); }
   rspfDpt operator-(const rspfDpt& p) const
      { return rspfDpt(x-p.x, y-p.y); }
   const rspfDpt& operator+=(const rspfDpt& p)
      { x += p.x; y += p.y; return *this; }
   const rspfDpt& operator-=(const rspfDpt& p)
      { x -= p.x; y -= p.y; return *this; }

   //***
   // OPERATORS: *, /
   // Scale point components by scalar:
   //***
   rspfDpt operator*(const double& d) const
      { return rspfDpt(d*x, d*y); }
   rspfDpt operator/(const double& d) const
      { return rspfDpt(x/d, y/d); }

   std::ostream& print(std::ostream& os, rspf_uint32 precision=15) const;
   
   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& os,
                                                  const rspfDpt& pt);

   /**
    * @param precision Output floating point precision.
    * 
    * @return rspfString representing point.
    *
    * Output format:  ( 30.00000000000000, -90.00000000000000 )
    *                   --------x--------  ---------y--------
    */
   rspfString toString(rspf_uint32 precision=15) const;

   /**
    * Initializes this point from string.  This method opens an istream to
    * s and then calls operator>>.
    *
    * Expected format:  ( 30.00000000000000, -90.00000000000000 )
    *                     --------x--------  ---------y--------
    *
    * @param s String to initialize from.
    *
    * @see operator>>
    */
   void toPoint(const std::string& s);
   
   /**
    * Method to input the formatted string of the "operator<<".
    *
    * Expected format:  ( 30.00000000000000, -90.00000000000000 )
    *                     --------x--------  ---------y--------
    * 
    * This method starts by doing a "makeNan" on pt.  So if anything goes
    * wrong with the stream or parsing pt could be all or partially nan.
    *
    * @param is Input stream istream to formatted text.
    * @param pt osimDpt to be initialized from stream.
    * @return istream pass in.
    */
   friend RSPFDLLEXPORT std::istream& operator>>(std::istream& is,
                                                  rspfDpt& pt);
   bool isEqualTo(const rspfDpt& rhs, rspfCompareType compareType=RSPF_COMPARE_FULL)const;
   //***
   // Public data members:
   //***
   union {double x; double samp; double u; double lon;};
   union {double y; double line; double v; double lat;};

};

inline const rspfDpt& rspfDpt::operator=(const rspfDpt& pt)
{
   if (this != &pt)
   {
      x = pt.x;
      y = pt.y;
   }
   
   return *this;
}

#endif /* #ifndef rspfDpt_HEADER */
