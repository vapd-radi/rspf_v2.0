//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for fpt.
// Used to represent a two dimensional point containing data members x and y.
//*******************************************************************
//  $Id: rspfFpt.h 14789 2009-06-29 16:48:14Z dburken $

#ifndef rspfFpt_HEADER
#define rspfFpt_HEADER
#include <iosfwd>
#include <rspf/base/rspfCommon.h>

// Forward class declarations.
class rspfIpt;
class rspfDpt;


class RSPFDLLEXPORT rspfFpt
{
public:

   rspfFpt() : x(0), y(0) {}

   rspfFpt(rspf_float32 x, rspf_float32 y)
      : x(x), y(y)
      {}

   rspfFpt(rspf_float64 x, rspf_float64 y)
      : x((rspf_float64)x), y((rspf_float64)y)
      {}
         
   rspfFpt(const rspfFpt& pt) : x(pt.x), y(pt.y) {}

   rspfFpt(const rspfDpt& pt);

   rspfFpt(const rspfIpt& pt);

   const rspfFpt& operator=(const rspfFpt& pt);

   const rspfFpt& operator=(const rspfDpt&);

   const rspfFpt& operator=(const rspfIpt&);

   bool operator==(const rspfFpt& pt) const
      { return ( (x == pt.x) && (y == pt.y) ); } 

   bool operator!=(const rspfFpt& pt) const
      { return ( (x != pt.x) || (y != pt.y) ); }

   void makeNan(){x = rspf::nan(); y=rspf::nan();}
   
   bool hasNans()const
      {
         return (rspf::isnan(x) || rspf::isnan(y));
      }
   
   void print(std::ostream& os) const;
   
   friend std::ostream& operator<<(std::ostream& os, const rspfFpt& pt);

   //***
   // Public data members:
   //***
   rspf_float32 x;
   rspf_float32 y;
};

inline const rspfFpt& rspfFpt::operator=(const rspfFpt& pt)
{
   if (this != &pt)
   {
      x = pt.x;
      y = pt.y;
   }
   
   return *this;
}

#endif
