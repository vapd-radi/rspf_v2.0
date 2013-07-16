//*****************************************************************************
// FILE: rspfLsrRay.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//  Class for representing rays in some local space rectangular (LSR)
//  coordinate system. This coordinate system is related to the ECEF system
//  by the rspfLsrSpace member object. This class simplifies coordinate
//  conversions between LSR and ECEF, and other LSR spaces.
//
//  An LSR ray is defined as having an LSR origin point and an LSR unit
//  direction vector radiating from the origin.
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfLsrRay.h 11428 2007-07-27 18:44:18Z gpotts $

#ifndef rspfLsrRay_HEADER
#define rspfLsrRay_HEADER

#include <rspf/base/rspfLsrPoint.h>
#include <rspf/base/rspfLsrVector.h>
#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfNotifyContext.h>

//*****************************************************************************
//  CLASS: rspfLsrRay
//
//*****************************************************************************
class RSPFDLLEXPORT rspfLsrRay
{
public:
   /*!
    * CONSTRUCTORS: 
    */
   rspfLsrRay() {}
   
   rspfLsrRay(const rspfLsrRay& copy_this)
      : theOrigin(copy_this.theOrigin), theDirection(copy_this.theDirection) {}

   rspfLsrRay(const rspfLsrPoint&  origin,
               const rspfLsrVector& direction);

   rspfLsrRay(const rspfLsrPoint&  origin,
               const rspfLsrPoint&  towards);

   rspfLsrRay(const rspfLsrRay& convert_this,
               const rspfLsrSpace& new_space)
      : theOrigin(convert_this.theOrigin, new_space),
	theDirection(convert_this.theDirection, new_space) {}

   /*!
    * OPERATORS:
    */
   const rspfLsrRay&  operator= (const rspfLsrRay& r);       // inline below
   bool                operator==(const rspfLsrRay& r) const; // inline below
   bool                operator!=(const rspfLsrRay& r) const; // inline below

   /*!
    * DATA ACCESS METHODS:
    */
   const rspfLsrPoint&  origin()    const { return theOrigin; }
   const rspfLsrVector& direction() const { return theDirection; }
   const rspfLsrSpace&  lsrSpace()  const { return theOrigin.lsrSpace(); }

   /*!
    * CASTING OPERATOR: rspfEcefRay()
    * Looks like a constructor for an rspfEcefRay but is an operation on this
    * object. Returns the rspfEcefRay equivalent.
    */
   operator rspfEcefRay () const;  // inline below
   
   bool hasNans()const
   {
      return (theOrigin.hasNans()||theDirection.hasNans());
   }

   void makeNan()
   {
      theOrigin.makeNan();
      theDirection.makeNan();
   }
   /*!
    * METHOD: extend(t)
    * Extends the ray by distance t (meters) from the origin to the LSR
    * point returned (in same space).
    */
   rspfLsrPoint extend(const double& t) const
      {
         if(!hasNans())
         {
            return (theOrigin + theDirection*t);
         }

         rspfLsrPoint p;
         p.makeNan();
         return p;
      }

   /*!
    * Debug Dump: 
    */
   void print(ostream& stream = rspfNotify(rspfNotifyLevel_INFO)) const;  // inline below
   
   friend ostream& operator<< (ostream& os , const rspfLsrRay& instance)
      { instance.print(os); return os; }

private:
   rspfLsrPoint  theOrigin;
   rspfLsrVector theDirection;

};

//================== BEGIN DEFINITIONS FOR INLINE METHODS =====================

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::operator=(rspfEcefRay)
//*****************************************************************************
inline const rspfLsrRay& rspfLsrRay::operator=(const rspfLsrRay& r)
{
   theOrigin = r.theOrigin;
   theDirection = r.theDirection;
   return *this;
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::operator==(rspfEcefRay)
//*****************************************************************************
inline bool rspfLsrRay::operator==(const rspfLsrRay& r) const
{
   return ((theOrigin == r.theOrigin) && (theDirection == r.theDirection));
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefRay::operator!=(rspfEcefRay)
//*****************************************************************************
inline bool rspfLsrRay::operator!=(const rspfLsrRay& r) const 
{
   return !(*this == r);
}

//*****************************************************************************
//  INLINE CASTING OPERATOR: rspfEcefRay()
//  
//  Looks like a constructor for an rspfEcefRay but is an operation on this
//  object. Returns the rspfEcefRay equivalent.
//*****************************************************************************
inline rspfLsrRay::operator rspfEcefRay() const
{
   return rspfEcefRay(rspfEcefPoint(theOrigin),
                       rspfEcefVector(theDirection));
}
   
//*****************************************************************************
//  INLINE METHOD: rspfLsrRay::print(ostream)
//  Dumps contents for debug purposes.
//*****************************************************************************
inline void rspfLsrRay::print(ostream& os) const
{
   os << "(rspfLsrRay)"
      << "\n   theOrigin    = " << theOrigin
      << "\n   theDirection = " << theDirection << ends;
}
   
#endif


