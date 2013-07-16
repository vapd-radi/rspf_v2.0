//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//  Class for representing points in some local space rectangular (LSR)
//  coordinate system. This coordinate system is related to the ECEF system
//  by the rspfLsrSpace member object. This class simplifies coordinate
//  conversions between LSR and ECEF, and other LSR points.
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfLsrPoint.h 22197 2013-03-12 02:00:55Z dburken $

#ifndef rspfLsrPoint_HEADER
#define rspfLsrPoint_HEADER

#include <rspf/base/rspfLsrSpace.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfNotify.h>

class rspfGpt;
class rspfLsrVector;

//*****************************************************************************
//  CLASS: rspfLsrPoint
//
//*****************************************************************************
class RSPFDLLEXPORT rspfLsrPoint
{
public:
   /*!
    * CONSTRUCTORS: 
    */
   rspfLsrPoint()
      : theData(0,0,0) {}
   
   rspfLsrPoint(const rspfLsrPoint& copy_this)
      : theData(copy_this.theData), theLsrSpace(copy_this.theLsrSpace) {}
   
   rspfLsrPoint(const rspfColumnVector3d& assign_this,
                 const rspfLsrSpace& space)
      : theData(assign_this), theLsrSpace(space) {}
   
   rspfLsrPoint(const double& x,
                 const double& y,
                 const double& z,
                 const rspfLsrSpace& space)
      : theData(x, y, z), theLsrSpace(space) {}
   
   rspfLsrPoint(const rspfLsrPoint& convert_this,
                 const rspfLsrSpace&);

   rspfLsrPoint(const rspfGpt& convert_this,
                 const rspfLsrSpace&);

   rspfLsrPoint(const rspfEcefPoint& convert_this,
                 const rspfLsrSpace&);

   /*!
    * OPERATORS:
    */
   inline const rspfLsrPoint& operator= (const rspfLsrPoint&); //inline below
   rspfLsrVector operator- (const rspfLsrPoint&)  const;
   rspfLsrPoint  operator+ (const rspfLsrVector&) const;
   inline bool    operator==(const rspfLsrPoint&)  const;//inline below
   inline bool    operator!=(const rspfLsrPoint&)  const;//inline below

   /*!
    * CASTING OPERATOR:
    * Used as: myEcefVector = rspfEcefPoint(this) -- looks like a constructor
    * but is an operation on this object. ECEF knows nothing about LSR, so
    * cannot provide an rspfEcefVector(rspfLsrPoint) constructor.
    */
   operator rspfEcefPoint() const; // inline below

   /*!
    * DATA ACCESS METHODS: 
    */
   double    x() const { return theData[0]; }
   double&   x()       { return theData[0]; }
   double    y() const { return theData[1]; }
   double&   y()       { return theData[1]; }
   double    z() const { return theData[2]; }
   double&   z()       { return theData[2]; }

   
   rspfColumnVector3d&       data()           { return theData; }
   const rspfColumnVector3d& data()     const { return theData; }

   rspfLsrSpace&             lsrSpace()       { return theLsrSpace; }
   const rspfLsrSpace&       lsrSpace() const { return theLsrSpace; }

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
   /*!
    * Debug Dump: 
    */
   inline void print(ostream& stream = rspfNotify(rspfNotifyLevel_INFO)) const;

   friend ostream& operator<< (ostream& os , const rspfLsrPoint& instance)
      { instance.print(os); return os; }

protected:
   /*!
    * METHOD: initialize(rspfEcefPoint)
    * Convenience method used by several constructors for initializing theData
    * given an ECEF point. Assumes theLsrSpace has been previously initialized.
    */
   void initialize(const rspfEcefPoint& ecef_point);
   
   rspfColumnVector3d theData;
   rspfLsrSpace       theLsrSpace;

};

//================== BEGIN DEFINITIONS FOR INLINE METHODS =====================

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrPoint::operator=(rspfLsrPoint)
//*****************************************************************************
inline const rspfLsrPoint&
rspfLsrPoint::operator=(const rspfLsrPoint& p)
{
   theData = p.theData;
   theLsrSpace = p.theLsrSpace;
   
   return *this;
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrPoint::operator==(rspfLsrPoint)
//*****************************************************************************
inline bool rspfLsrPoint::operator==(const rspfLsrPoint& p) const
{
   return ((theData == p.theData) && (theLsrSpace == p.theLsrSpace));
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrPoint::operator!=(rspfLsrPoint)
//*****************************************************************************
inline bool rspfLsrPoint::operator!=(const rspfLsrPoint& p) const
{
   return (!(*this == p));
}

//*****************************************************************************
//  INLINE OPERATOR: rspfLsrPoint::operator rspfEcefPoint()
//
//  Looks like a constructor for an rspfEcefPoint but is an operation on this
//  object. Returns the rspfEcefPoint equivalent.
//*****************************************************************************
inline rspfLsrPoint::operator rspfEcefPoint() const
{
   return rspfEcefPoint(theLsrSpace.origin().data() +
                         theLsrSpace.lsrToEcefRotMatrix()*theData);
}

//*****************************************************************************
//  INLINE METHOD: rspfLsrPoint::print(ostream)
//  
//  Dumps contents for debug purposes.
//*****************************************************************************
inline void rspfLsrPoint::print(ostream& os) const
{
   os << "(rspfLsrPoint)\n"
      << "  theData = " << theData
      << "\n  theLsrSpace = " << theLsrSpace;
}

#endif


