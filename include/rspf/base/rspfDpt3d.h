//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:  
//
// Contains class declaration for dpt3d
// Used to represent a 3d double point containing an x, y and z data member.
//*******************************************************************
//  $Id: rspfDpt3d.h 16347 2010-01-13 23:12:09Z dburken $

#ifndef rspfDpt3d_HEADER
#define rspfDpt3d_HEADER

#include <cmath>
#include <iosfwd>
#include <string>

#include <rspf/base/rspfCommon.h> /* for rspf::isnan */
#include <rspf/base/rspfColumnVector3d.h>

class rspfIpt;
class rspfDpt;

class RSPFDLLEXPORT rspfDpt3d
{
public:
   friend RSPFDLLEXPORT std::ostream & operator <<(std::ostream &out,
                                                    const rspfDpt3d &rhs);
   
   rspfDpt3d(const double &aX=0, const double &aY=0, const double &aZ=0)
      :x(aX), y(aY), z(aZ) {}

   rspfDpt3d(const rspfDpt &aPt);
   rspfDpt3d(const rspfIpt &aPt);
   
   rspfDpt3d(const rspfColumnVector3d &pt)
      : x(pt[0]), y(pt[1]), z(pt[2]) {}
   
   bool operator ==(const rspfDpt3d &rhs) const
      {
         return ( (x == rhs.x) &&
                  (y == rhs.y) &&
                  (z == rhs.z));
      }
   bool operator !=(const rspfDpt3d &rhs) const
   {
      return ( (x != rhs.x) ||
               (y != rhs.y) ||
               (z != rhs.z) );
   }
   
   void makeNan(){x = rspf::nan(); y=rspf::nan(); z=rspf::nan();}

   bool hasNans()const
   {
      return (rspf::isnan(x) || rspf::isnan(y) || rspf::isnan(z));
   }
   /*!
    * METHOD: length()
    * Returns the RSS of the components.
    */
   double length() const { return std::sqrt(x*x + y*y + z*z); }
   double length2() const { return x*x + y*y + z*z; }
   
   //***
   // OPERATORS: +, -, +=, -=
   // Point add/subtract with other point:
   //***
   rspfDpt3d operator+(const rspfDpt3d& p) const
      { return rspfDpt3d(x+p.x, y+p.y, z+p.z); }
   rspfDpt3d operator-(const rspfDpt3d& p) const
      { return rspfDpt3d(x-p.x, y-p.y, z-p.z); }
   const rspfDpt3d& operator+=(const rspfDpt3d& p)
      { x += p.x; y += p.y; z += p.z; return *this; }
   const rspfDpt3d& operator-=(const rspfDpt3d& p)
      { x -= p.x; y -= p.y; z -= p.z; return *this; }

   //***
   // OPERATORS: *, /
   // Scale point components by scalar:
   //***
   rspfDpt3d operator*(const double& d) const
      { return rspfDpt3d(d*x, d*y, d*z); }
   rspfDpt3d operator/(const double& d) const
      { return rspfDpt3d(x/d, y/d, z/d); }
  void operator /=(double value)
      {
         x /= value;
         y /= value;
         z /= value;
      }
   void operator *=(double value) 
      {
         x *= value;
         y *= value;
         z *= value;
      }
   double operator *(const rspfDpt3d& src)const
   {
      return (x*src.x + y*src.y + z*src.z);
   }
   inline const rspfDpt3d operator ^ (const rspfDpt3d& rhs) const
   {
      return rspfDpt3d(y*rhs.z-z*rhs.y,
                        z*rhs.x-x*rhs.z ,
                        x*rhs.y-y*rhs.x);
   }

   /**
    * @brief To string method.
    * 
    * @param precision Output floating point precision.
    * 
    * @return std::string representing point.
    *
    * Output format:
    * ( 0.0000000,  0.0000000,  0.00000000 )
    *   -----x----  -----y----  ------z----
    */
   std::string toString(rspf_uint32 precision=15) const;
   
   /**
    * @brief Initializes this point from string.
    *
    * Expected format:
    * 
    * ( 0.0000000,  0.0000000,  0.00000000 )
    *   -----x----  -----y----  ------z----
    *
    * @param s String to initialize from.
    */
   void toPoint(const std::string& s); 
   
   double x;
   double y;
   double z;
};

#endif
