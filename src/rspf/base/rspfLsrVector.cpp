//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
//
// DESCRIPTION:
//   Contains implementation of class 
//
// SOFTWARE HISTORY:
//>
//   ddmmm2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Id: rspfLsrVector.cpp 12790 2008-05-05 13:41:33Z dburken $

#include <rspf/base/rspfLsrVector.h>
#include <rspf/base/rspfEcefVector.h>
#include <rspf/base/rspfCommon.h>

//*****************************************************************************
//  CONSTRUCTOR: rspfLsrVector(rspfEcefVector, rspfLsrSpace)
//  
//  Establishes an LSR vector given the ECEF equivalent and the LSR space.
//  
//*****************************************************************************
rspfLsrVector::rspfLsrVector(const rspfEcefVector& convert_this,
                               const rspfLsrSpace&   new_space)
   : theLsrSpace (new_space)
{
   initialize(convert_this);
}

//*****************************************************************************
//  CONSTRUCTOR: rspfLsrVector(rspfLsrVector, rspfLsrSpace)
//  
//  Establishes an LSR vector in a new space given another LSR vector in a
//  different space.
//  
//*****************************************************************************
rspfLsrVector::rspfLsrVector(const rspfLsrVector& convert_this,
                               const rspfLsrSpace&  new_space)
   : theLsrSpace (new_space)
{
   initialize(rspfEcefVector(convert_this));
}

//*****************************************************************************
//  METHOD: rspfLsrVector::dot(rspfLsrVector)
//  
//  Computes the scalar product.
//  
//*****************************************************************************
double rspfLsrVector::dot(const rspfLsrVector& v) const
{
   if(hasNans()||v.hasNans()||(theLsrSpace != v.theLsrSpace))
   {
      rspfLsrSpace::lsrSpaceErrorMessage();
      return rspf::nan();
   }
   return theData.dot(v.data());
}

//*****************************************************************************
//  METHOD: rspfLsrVector::angleTo(rspfLsrVector)
//  
//  Returns the angle subtended (in DEGREES) between this and arg vector
//
//*****************************************************************************
double rspfLsrVector::angleTo(const rspfLsrVector& v) const
{
   if(hasNans()||v.hasNans()||(theLsrSpace != v.theLsrSpace))
   {
      rspfLsrSpace::lsrSpaceErrorMessage();
      return rspf::nan();
   }
   double mag_product = theData.magnitude() * v.theData.magnitude();
   return rspf::acosd(theData.dot(v.theData)/mag_product);
}

//*****************************************************************************
//  METHOD: rspfLsrVector::cross(rspfLsrVector)
//  
//  Computes the cross product.
//  
//*****************************************************************************
rspfLsrVector rspfLsrVector::cross(const rspfLsrVector& v) const
{
   if(hasNans()||v.hasNans()||(theLsrSpace != v.theLsrSpace))
   {
      rspfLsrSpace::lsrSpaceErrorMessage();
      return rspfLsrVector(rspf::nan(), rspf::nan(), rspf::nan(),
                            theLsrSpace);
   }
   return rspfLsrVector(theData.cross(v.data()), theLsrSpace);
}

std::ostream& rspfLsrVector::print(std::ostream& os) const
{
   os << "(rspfLsrVector)\n"
      << "  theData = " << theData
      << "\n  theLsrSpace = " << theLsrSpace;
   return os;
}

std::ostream& operator<< (std::ostream& os ,
                          const rspfLsrVector& instance)
{
   return instance.print(os);
}

