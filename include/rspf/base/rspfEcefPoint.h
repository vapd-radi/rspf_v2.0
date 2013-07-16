//*****************************************************************************
// FILE: rspfEcefPoint.h
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of a 3D point object in the Earth-centered, earth
//   fixed (ECEF) coordinate system.
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer (http://www.oscarkramer.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfEcefPoint.h 20043 2011-09-06 15:00:55Z oscarkramer $

#ifndef rspfEcefPoint_HEADER
#define rspfEcefPoint_HEADER
#include <iosfwd>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfString.h>
#include <rspf/matrix/newmat.h>

class rspfGpt;
class rspfEcefVector;
class rspfDpt3d;

//*****************************************************************************
//  CLASS: rspfEcefPoint
//
//*****************************************************************************
class RSPFDLLEXPORT rspfEcefPoint
{
public:
   /*!
    * CONSTRUCTORS: 
    */
   rspfEcefPoint()
      : theData(0,0,0) {}

   rspfEcefPoint(const rspfEcefPoint& copy_this)
      : theData (copy_this.theData) {}

   rspfEcefPoint(const rspfGpt& convert_this);

   rspfEcefPoint(const double& x,
                  const double& y,
                  const double& z)
      : theData(x, y, z) {}

   rspfEcefPoint(const rspfColumnVector3d& assign_this)
      : theData(assign_this) {}

   rspfEcefPoint(const NEWMAT::ColumnVector& assign_this)
      : theData(assign_this) {}

   rspfEcefPoint(const rspfDpt3d& pt);
   
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
    * OPERATORS:
    */
   rspfEcefVector       operator- (const rspfEcefPoint&)  const;
   rspfEcefPoint        operator+ (const rspfEcefVector&) const;
   rspfEcefPoint        operator- (const rspfEcefVector&) const;
   const rspfEcefPoint& operator= (const rspfEcefPoint&);        // inline
   bool                  operator==(const rspfEcefPoint&)  const; // inline
   bool                  operator!=(const rspfEcefPoint&)  const; // inline
   
   /*!
    * COMPONENT ACCESS METHODS: 
    */
   double    x() const { return theData[0]; }
   double&   x()       { return theData[0]; }
   double    y() const { return theData[1]; }
   double&   y()       { return theData[1]; }
   double    z() const { return theData[2]; }
   double&   z()       { return theData[2]; }
   double&   operator[](int idx){return theData[idx];}
   const double&   operator[](int idx)const{return theData[idx];}
   const rspfColumnVector3d& data() const { return theData; }
   rspfColumnVector3d&       data()       { return theData; }

   double getMagnitude() const
      {
         return theData.magnitude();
      }
   double magnitude()const
   {
      return theData.magnitude();
   }
   double length()const
   {
      return theData.magnitude();
   }
   double normalize()
   {
      double result = magnitude();

      if(result > 1e-15)
      {
         theData[0]/=result;
         theData[1]/=result;
         theData[2]/=result;
      }
      
      return result;
   }

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
   
   //! Converts 3D column vector to this point.
   void toPoint(const NEWMAT::ColumnVector& v)
   {
      if (v.Nrows() == 3)
      {
         theData[0] = v[0];
         theData[1] = v[1];
         theData[2] = v[2];
      }
   }

   /*!
    * Debug Dump: 
    */
   void print(std::ostream& os = rspfNotify(rspfNotifyLevel_INFO)) const;
   
   friend RSPF_DLL std::ostream& operator<<(std::ostream& os ,
                                             const rspfEcefPoint& instance);

protected:
   rspfColumnVector3d theData;
};

//================== BEGIN DEFINITIONS FOR INLINE METHODS =====================

//*****************************************************************************
//  INLINE METHOD: rspfEcefPoint::operator=(rspfEcefPoint)
//*****************************************************************************
inline const rspfEcefPoint&
rspfEcefPoint::operator=(const rspfEcefPoint& p)
{
   theData = p.theData;
   return *this;
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefPoint::operator==(rspfEcefPoint)
//*****************************************************************************
inline bool rspfEcefPoint::operator==(const rspfEcefPoint& p) const
{
   return (theData == p.theData);
}

//*****************************************************************************
//  INLINE METHOD: rspfEcefPoint::operator!=(rspfEcefPoint)
//*****************************************************************************
inline bool rspfEcefPoint::operator!=(const rspfEcefPoint& p) const
{
   return (theData != p.theData);
}

#endif
