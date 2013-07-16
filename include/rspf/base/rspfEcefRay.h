//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//  Class for representing a ray in the earth-centered, earth-fixed (ECEF)
//  coordinate system. A ray is defined as having an origin point and a
//  unit direction vector radiating from the origin.
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfEcefRay.h 12769 2008-04-30 17:46:18Z dburken $

#ifndef rspfEcefRay_HEADER
#define rspfEcefRay_HEADER

#include <iosfwd>

#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfNotify.h>

class rspfGpt;
class rspfLsrRay;

//*****************************************************************************
//  CLASS: rspfEcefRay
//
//*****************************************************************************
class RSPFDLLEXPORT rspfEcefRay
{
public:
   /*!
    * CONSTRUCTORS: 
    */
   rspfEcefRay() {};
     
   rspfEcefRay(const rspfEcefRay& copy_this)
      : theOrigin(copy_this.theOrigin), theDirection(copy_this.theDirection) {}
   
   rspfEcefRay(const rspfEcefPoint&  origin,
                const rspfEcefVector& direction)
      : theOrigin(origin), theDirection(direction.unitVector()) {}
   
   rspfEcefRay(const rspfEcefPoint& from,
                const rspfEcefPoint& to);
   
   rspfEcefRay(const rspfGpt& from,
                const rspfGpt& to);

   bool isNan()const
      {
         return theOrigin.isNan()&&theDirection.isNan();
      }
   bool hasNans()const
      {
         return theOrigin.isNan()||theDirection.isNan();
      }
   void makeNan()
      {
         theOrigin.makeNan();
         theDirection.makeNan();
      }
   /*!
    * OPERATORS:
    */
   const rspfEcefRay& operator= (const rspfEcefRay& r); // inline below
   bool                operator==(const rspfEcefRay& r) const; // inline below
   bool                operator!=(const rspfEcefRay& r) const; // inline below
   
   /*!
    * DATA ACCESS METHODS:
    */
   const rspfEcefPoint&  origin()    const     { return theOrigin; }
   const rspfEcefVector& direction() const     { return theDirection; }
   void  setOrigin(const rspfEcefPoint& orig)  { theOrigin = orig; }
   void  setDirection(const rspfEcefVector& d) { theDirection=d.unitVector();}

   /*!
    * Extends the ray by distance t (meters) from the origin to the ECEF
    * point returned.
    */
   rspfEcefPoint extend(const double& t) const; // inline below

   /*!
    * This method computes a ray with the same origin but a new direction
    * corresponding to a reflection from some surface defined by its normal
    * vector (assumed to be a unit vector):
    */
   rspfEcefRay reflectRay(const rspfEcefVector& normal) const;// inline below

   /*!
    * Intersects the ray with the given elevation above the earth ellipsoid.
    */
   rspfEcefPoint intersectAboveEarthEllipsoid
      (const double& heightAboveEllipsoid,
       const rspfDatum* aDatum = rspfDatumFactory::instance()->wgs84()) const;
                                       
   /*!
    * Debug Dump
    */
   std::ostream& print(
      std::ostream& os = rspfNotify(rspfNotifyLevel_INFO))const;
   
   friend std::ostream& operator<<(std::ostream& os ,
                                   const rspfEcefRay& instance);

private:
   rspfEcefPoint  theOrigin;
   rspfEcefVector theDirection;
}; 

//================== BEGIN DEFINITIONS FOR INLINE METHODS =====================

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::operator=(rspfEcefRay)
//*****************************************************************************
inline const rspfEcefRay& rspfEcefRay::operator=(const rspfEcefRay& r)
{
   theOrigin = r.theOrigin;
   theDirection = r.theDirection;
   return *this;
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::operator==(rspfEcefRay)
//*****************************************************************************
inline bool rspfEcefRay::operator==(const rspfEcefRay& r) const
{
   return ((theOrigin == r.theOrigin) && (theDirection == r.theDirection));
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::operator!=(rspfEcefRay)
//*****************************************************************************
inline bool rspfEcefRay::operator!=(const rspfEcefRay& r) const 
{
   return !(*this == r);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::extend(double t)
//  
//   Extends the ray by distance t (meters) from the origin to the ECEF
//   point returned.
//*****************************************************************************
inline rspfEcefPoint rspfEcefRay::extend(const double& t) const
{
   return (theOrigin + theDirection*t);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::reflectRay(normal)
//  
//   This method computes a ray with the same origin but a new direction
//   corresponding to a reflection from some surface defined by its normal
//   vector:
//*****************************************************************************
inline rspfEcefRay
rspfEcefRay::reflectRay(const rspfEcefVector& normal) const
{
   rspfEcefVector new_dir(theDirection - normal*2.0*normal.dot(theDirection));
   return rspfEcefRay(theOrigin, new_dir);
}

   
#endif


