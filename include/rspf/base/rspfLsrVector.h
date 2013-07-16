//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//  Class for representing vectors in some local space rectangular (LSR)
//  coordinate system. This coordinate system is related to the ECEF system
//  by the rspfLsrSpace member object. This class simplifies coordinate
//  conversions between LSR and ECEF, and other LSR vectors.
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfLsrVector.h 12790 2008-05-05 13:41:33Z dburken $

#ifndef rspfLsrVector_HEADER
#define rspfLsrVector_HEADER

#include <iosfwd>

#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfLsrPoint.h>
#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfColumnVector3d.h>

class rspfGpt;

//*****************************************************************************
//  CLASS: rspfLsrVector
//
//*****************************************************************************
class RSPFDLLEXPORT rspfLsrVector
{
public:
   /*!
    * CONSTRUCTORS: 
    */
   rspfLsrVector()
      : theData (0,0,0) {}
   
   rspfLsrVector(const rspfLsrVector& copy_this)
      : theData(copy_this.theData), theLsrSpace(copy_this.theLsrSpace) {}

   rspfLsrVector(const rspfColumnVector3d& assign_this,
                  const rspfLsrSpace& space)
      : theData(assign_this), theLsrSpace(space) {}
   
   rspfLsrVector(const double& x,
                  const double& y,
                  const double& z,
                  const rspfLsrSpace& space)
      : theData(x,y,z), theLsrSpace(space) {}
   
   rspfLsrVector(const rspfEcefVector& convert_this,
                  const rspfLsrSpace&);
   rspfLsrVector(const rspfLsrVector& convert_this,
                  const rspfLsrSpace&);

   /*!
    * OPERATORS: (all methods inlined below)
    */
   const rspfLsrVector&  operator= (const rspfLsrVector&);
   rspfLsrVector         operator- ()                      const;
   rspfLsrVector         operator+ (const rspfLsrVector&) const;
   rspfLsrVector         operator- (const rspfLsrVector&) const;
   rspfLsrPoint          operator+ (const rspfLsrPoint&)  const;
   rspfLsrVector         operator* (const double& scalar)  const;
   rspfLsrVector         operator/ (const double& scalar)  const;
   bool                   operator==(const rspfLsrVector&) const;
   bool                   operator!=(const rspfLsrVector&) const;

   /*!
    * CASTING OPERATOR:
    * Used as: myEcefVector = rspfEcefVector(this) -- looks like a constructor
    * but is an operation on this object. ECEF knows nothing about LSR, so
    * cannot provide an rspfEcefVector(rspfLsrVector) constructor.
    */
   operator rspfEcefVector() const; // inline below
   
   /*!
    * Vector-related functions: 
    */
   double                 dot(const rspfLsrVector&)       const;
   double                 angleTo(const rspfLsrVector&)   const;
   rspfLsrVector         cross(const rspfLsrVector&)     const;
   rspfLsrVector         unitVector()                     const;//inline below
   double                 magnitude()                      const;//inline below
   void                   normalize(); // inline below
   
   /*!
    * DATA ACCESS METHODS: 
    */
   double    x() const { return theData[0]; }
   double&   x()       { return theData[0]; }
   double    y() const { return theData[1]; }
   double&   y()       { return theData[1]; }
   double    z() const { return theData[2]; }
   double&   z()       { return theData[2]; }

   bool hasNans()const
   {
      return (rspf::isnan(theData[0])||
              rspf::isnan(theData[1])||
              rspf::isnan(theData[2]));
   }
   void makeNan()
   {
      theData[0] = rspf::nan();
      theData[1] = rspf::nan();
      theData[2] = rspf::nan();
   }
   rspfColumnVector3d&       data()           { return theData; }
   const rspfColumnVector3d& data()     const { return theData; }

   rspfLsrSpace&             lsrSpace()       { return theLsrSpace; }
   const rspfLsrSpace&       lsrSpace() const { return theLsrSpace; }

   /*!
    * Debug Dump: 
    */
   std::ostream& print(ostream& stream) const;

   friend std::ostream& operator<< (std::ostream& os ,
                                    const rspfLsrVector& instance);

protected:
   /*!
    * METHOD: initialize(rspfEcefVector)
    * Convenience method used by several constructors for initializing theData
    * given an ECEF vector. Assumes theLsrSpace has been previously initialized
    */
   void initialize(const rspfEcefVector& ecef_point);
   
   rspfColumnVector3d theData;
   rspfLsrSpace       theLsrSpace;

};

//================== BEGIN DEFINITIONS FOR INLINE METHODS =====================

//*****************************************************************************
// INLINE OPERATOR: rspfLsrVector::operator=(rspfLsrVector)
//*****************************************************************************
inline const rspfLsrVector& rspfLsrVector::operator=(const rspfLsrVector& v)
{
   theData = v.theData;
   theLsrSpace = v.theLsrSpace;

   return *this;
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator-() (negate)
//*****************************************************************************
inline rspfLsrVector rspfLsrVector::operator-() const
{
   return rspfLsrVector(-theData, theLsrSpace);
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator+(rspfLsrVector)
//*****************************************************************************
inline rspfLsrVector rspfLsrVector::operator+(const rspfLsrVector& v) const
{
   if ((theLsrSpace != v.theLsrSpace)||hasNans()||v.hasNans())
   {
      theLsrSpace.lsrSpaceErrorMessage();
      return rspfLsrVector(rspf::nan(), rspf::nan(), rspf::nan(), theLsrSpace);
   }
   return rspfLsrVector(theData + v.theData, theLsrSpace);

}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator-(rspfLsrVector)
//*****************************************************************************
inline rspfLsrVector rspfLsrVector::operator-(const rspfLsrVector& v) const
{
   if ((theLsrSpace != v.theLsrSpace)||hasNans()||v.hasNans())
   {
      theLsrSpace.lsrSpaceErrorMessage();
      return rspfLsrVector(rspf::nan(), rspf::nan(), rspf::nan(), theLsrSpace);
   }
   return rspfLsrVector(theData - v.data(), theLsrSpace);
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator+(rspfLsrPoint)
//*****************************************************************************
inline rspfLsrPoint rspfLsrVector::operator+(const rspfLsrPoint& p) const
{
   if ((theLsrSpace != p.lsrSpace())||hasNans()||p.hasNans())
   {
      theLsrSpace.lsrSpaceErrorMessage();
      return rspfLsrPoint(rspf::nan(), rspf::nan(), rspf::nan(), theLsrSpace);
   }
   return rspfLsrPoint(theData + p.data(), theLsrSpace);
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator*(double scalar)
//*****************************************************************************
inline rspfLsrVector rspfLsrVector::operator*(const double& scalar) const
{
   return rspfLsrVector(theData*scalar, theLsrSpace);
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator/(double scalar)
//*****************************************************************************
inline rspfLsrVector rspfLsrVector::operator/(const double& scalar) const
{
   return rspfLsrVector(theData/scalar, theLsrSpace);
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator==(rspfLsrVector)
//*****************************************************************************
inline bool rspfLsrVector::operator==(const rspfLsrVector& v) const
{
   return ((theData == v.theData) && (theLsrSpace == v.theLsrSpace));
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrVector::operator!=(rspfLsrVector)
//*****************************************************************************
inline bool rspfLsrVector::operator!=(const rspfLsrVector& v) const
{
   return (!(*this == v));
}

//*****************************************************************************
//  INLINE OPERATOR:  rspfEcefVector()
//
//  Looks like a constructor for an rspfEcefVector but is an operation on this
//  object. Returns the rspfEcefVector equivalent.
//
//*****************************************************************************
inline rspfLsrVector::operator rspfEcefVector() const
{   
   return rspfEcefVector(theLsrSpace.lsrToEcefRotMatrix()*theData);
}   

//*****************************************************************************
//  INLINE METHOD:  rspfLsrVector::unitVector()
//  Returns a unit vector parallel to this.
//*****************************************************************************
inline rspfLsrVector rspfLsrVector::unitVector() const
{
   if(hasNans()) return rspfLsrVector(rspf::nan(), rspf::nan(), rspf::nan(), theLsrSpace);

   return rspfLsrVector(theData/theData.magnitude(), theLsrSpace);
}

//*****************************************************************************
//  INLINE METHOD: rspfLsrVector::magnitude()
//*****************************************************************************
inline double rspfLsrVector::magnitude() const
{
   if(hasNans()) return rspf::nan();
   return theData.magnitude();
}

//*****************************************************************************
//  INLINE METHOD: rspfLsrVector::normalize()
//  Normalizes this vector.
//*****************************************************************************
inline void rspfLsrVector::normalize()
{
   theData /= theData.magnitude();
}

//*****************************************************************************
//  PROTECTED INLINE METHOD: rspfLsrPoint::initialize(rspfEcefPoint)
//  
//  Convenience method used by several constructors for initializing theData
//  given an ECEF point. Assumes theLsrSpace has been previously initialized.
//  
//*****************************************************************************
inline void rspfLsrVector::initialize(const rspfEcefVector& ecef_vector)
{
   theData = theLsrSpace.ecefToLsrRotMatrix() * ecef_vector.data();
}

#endif


