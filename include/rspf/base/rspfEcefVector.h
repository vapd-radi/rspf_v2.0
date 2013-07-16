//*****************************************************************************
// FILE: rspfEcefVector.h
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of a 3D vector object in the Earth-centered, earth
//   fixed (ECEF) coordinate system.
//
//   NOTE: There is no associated rspfEcefVector.cc file. All methods are
//         inlined here
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfEcefVector.h 20043 2011-09-06 15:00:55Z oscarkramer $

#ifndef rspfEcefVector_HEADER
#define rspfEcefVector_HEADER

#include <cmath>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfString.h>
#include <rspf/matrix/newmat.h>

class rspfGpt;

//*****************************************************************************
//  CLASS: rspfEcefVector
//
//*****************************************************************************
class RSPFDLLEXPORT rspfEcefVector
{
public:
   /*!
    * CONSTRUCTORS: 
    */
   rspfEcefVector()
      : theData(0,0,0) {}
      
   rspfEcefVector(const rspfEcefVector& copy_this)
      : theData (copy_this.theData) {}

   rspfEcefVector(const NEWMAT::ColumnVector& assign_this)
      : theData (assign_this) {}

   rspfEcefVector(const rspfEcefPoint& from,
                   const rspfEcefPoint& to)
      : theData (to.data() - from.data()) {}
   
   rspfEcefVector(const rspfGpt& from,
                   const rspfGpt& to)
      : theData ((rspfEcefPoint(to) - rspfEcefPoint(from)).data()) {}

   rspfEcefVector(const double& x,
                   const double& y,
                   const double& z)
      : theData(x, y, z) {}

   rspfEcefVector(const rspfColumnVector3d& assign_this)
      : theData(assign_this) {}


   void makeNan()
   {
      theData[0] = rspf::nan();
      theData[1] = rspf::nan();
      theData[2] = rspf::nan();
   }
   
   bool hasNans()const
   {
      return ( rspf::isnan(theData[0]) ||
               rspf::isnan(theData[1]) ||
               rspf::isnan(theData[2]) );
   }
   
   bool isNan()const
   {
      return ( rspf::isnan(theData[0]) &&
               rspf::isnan(theData[1]) &&
               rspf::isnan(theData[2]) );
   }

   /*!
    * OPERATORS: (all inlined below)
    */
   inline rspfEcefVector        operator- () const; 
   inline rspfEcefVector        operator+ (const rspfEcefVector&) const;
   inline rspfEcefVector        operator- (const rspfEcefVector&) const;
   inline rspfEcefPoint         operator+ (const rspfEcefPoint&)  const;
   inline rspfEcefVector        operator* (const double&)          const;
   inline rspfEcefVector        operator/ (const double&)          const;
   inline bool                   operator==(const rspfEcefVector&) const;
   inline bool                   operator!=(const rspfEcefVector&) const;
   inline const rspfEcefVector& operator= (const rspfEcefVector&);

   /*!
    * Vector-related functions:  (all inlined below)
    */
   inline double          dot    (const rspfEcefVector&) const;
   inline double          angleTo(const rspfEcefVector&) const; // degrees
   inline rspfEcefVector cross  (const rspfEcefVector&) const;
   inline rspfEcefVector unitVector()                    const;
   inline double          magnitude()                     const; // meters
   inline double          norm2()                         const; // squared meters
   inline double length() const;
   inline double          normalize();
   
   /*!
    * COMPONENT ACCESS METHODS: 
    */
   double    x() const { return theData[0]; }
   double&   x()       { return theData[0]; }
   double    y() const { return theData[1]; }
   double&   y()       { return theData[1]; }
   double    z() const { return theData[2]; }
   double&   z()       { return theData[2]; }
   double& operator [](int idx){return theData[idx];}
   const double& operator [](int idx)const{return theData[idx];}

   const rspfColumnVector3d& data() const { return theData; }
   rspfColumnVector3d&       data()       { return theData; }

   /**
    * @brief To string method.
    * 
    * @param precision Output floating point precision.
    * 
    * @return rspfString representing point.
    *
    * Output format:
    * ( 0.0000000,  0.0000000,  0.00000000 )
    *   -----x----  -----y----  ------z----
    */
   rspfString toString(rspf_uint32 precision=15) const;

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

   //! Converts this point to a 3D column vector.
   NEWMAT::ColumnVector toVector() const
   {
      NEWMAT::ColumnVector v (3);
      v(0) = theData[0];
      v(1) = theData[1];
      v(2) = theData[2];
      return v;
   }

   /*!
    * Debug Dump: 
    */
   void print(ostream& os = rspfNotify(rspfNotifyLevel_INFO)) const
      {	 os << "(rspfEcefVector) " << theData; }

   friend ostream& operator<< (ostream& os , const rspfEcefVector& instance)
      { instance.print(os); return os; }

protected:
   rspfColumnVector3d theData;
};

//================== BEGIN DEFINITIONS FOR INLINE METHODS =====================

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator-()
//  Reverses direction of vector.
//*****************************************************************************
inline rspfEcefVector rspfEcefVector::operator-() const
{
   return rspfEcefVector(-theData);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator+(const rspfEcefVector&)
//*****************************************************************************
inline rspfEcefVector
rspfEcefVector::operator+(const rspfEcefVector& v) const
{
   return rspfEcefVector(theData + v.theData);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator-(const rspfEcefVector&)
//*****************************************************************************
inline rspfEcefVector
rspfEcefVector::operator-(const rspfEcefVector& v) const
{
   return rspfEcefVector(theData - v.theData);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator+(const rspfEcefPoint&) 
//*****************************************************************************
inline  rspfEcefPoint
rspfEcefVector::operator+(const rspfEcefPoint& p) const
{
   return rspfEcefPoint(theData + p.data());
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator*(const double&)
//*****************************************************************************
inline rspfEcefVector rspfEcefVector::operator*(const double& scalar) const
{
   return rspfEcefVector(theData*scalar);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator/(const double&)
//*****************************************************************************
inline rspfEcefVector rspfEcefVector::operator/(const double& scalar) const
{
   return rspfEcefVector(theData/scalar);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator==(const rspfEcefVector&)
//*****************************************************************************
inline bool rspfEcefVector::operator==(const rspfEcefVector& v) const
{
   return (theData == v.theData);
}
   
//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator!=(const rspfEcefVector&)
//*****************************************************************************
inline bool rspfEcefVector::operator!=(const rspfEcefVector& v) const
{
   return (theData != v.theData);
}
   
//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::operator=
//*****************************************************************************
inline const rspfEcefVector&
rspfEcefVector::operator=(const rspfEcefVector& v)
{
   theData = v.theData;
   return *this;
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::dot()
//  Computes the scalar product.
//*****************************************************************************
inline double rspfEcefVector::dot(const rspfEcefVector& v) const
{
   return theData.dot(v.theData);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::angleTo()
//  Returns the angle subtended (in DEGREES) between this and arg vector
//*****************************************************************************
inline double rspfEcefVector::angleTo(const rspfEcefVector& v) const
{
   double mag_product = theData.magnitude() * v.theData.magnitude();
   return rspf::acosd(theData.dot(v.theData)/mag_product);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::cross()
//  Computes the cross product.
//*****************************************************************************
inline rspfEcefVector rspfEcefVector::cross(const rspfEcefVector& v) const
{
   return rspfEcefVector(theData.cross(v.theData));
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::unitVector()
//  Returns a unit vector parallel to this.
//*****************************************************************************
inline rspfEcefVector rspfEcefVector::unitVector() const
{
   return rspfEcefVector(theData/theData.magnitude());
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::magnitude()
//*****************************************************************************
inline double rspfEcefVector::magnitude() const
{
   return theData.magnitude();
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::norm2()
//*****************************************************************************
inline double rspfEcefVector::norm2() const
{
   return theData.norm2();
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::length()
//*****************************************************************************
inline double rspfEcefVector::length() const
{
   return theData.magnitude();
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefVector::normalize()
//  Normalizes this vector.
//*****************************************************************************
inline double rspfEcefVector::normalize()
{
   double result = theData.magnitude();
   if(result > 1e-15)
   {
      theData /= result;
   }

   return result;
}
#endif


