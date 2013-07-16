//*****************************************************************************
// FILE: rspfEcefPoint.cpp
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
//  $Id: rspfEcefPoint.cpp 11399 2007-07-26 13:39:54Z dburken $

#include <ostream>

#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfGeocent.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfEcefVector.h>

//*****************************************************************************
//  CONSTRUCTOR: rspfEcefPoint(const rspfGpt&  convert_this)
//  
//*****************************************************************************
rspfEcefPoint::rspfEcefPoint(const rspfGpt&  gpt)
{
   if(!gpt.isHgtNan())
   {
      gpt.datum()->ellipsoid()->latLonHeightToXYZ(gpt.latd(),
                                                  gpt.lond(),
                                                  gpt.height(),
                                                  theData[0],
                                                  theData[1],
                                                  theData[2]);
   }
   else
   {
      gpt.datum()->ellipsoid()->latLonHeightToXYZ(gpt.latd(),
                                                  gpt.lond(),
                                                  0.0,
                                                  theData[0],
                                                  theData[1],
                                                  theData[2]);
   }
}

rspfEcefPoint::rspfEcefPoint(const rspfDpt3d& pt)
   :theData(pt.x, pt.y, pt.z)
{
}

//*****************************************************************************
//  METHOD: rspfEcefPoint::operator-(rspfEcefPoint)
//*****************************************************************************
rspfEcefVector rspfEcefPoint::operator-(const rspfEcefPoint& p) const
{
   return rspfEcefVector(theData - p.theData);
}

//*****************************************************************************
//  METHOD: rspfEcefPoint::operator+(rspfEcefVector)
//*****************************************************************************
rspfEcefPoint rspfEcefPoint::operator+(const rspfEcefVector& v) const
{
   return rspfEcefPoint(theData + v.data());
}

//*****************************************************************************
//  METHOD: rspfEcefPoint::operator-(rspfEcefVector)
//*****************************************************************************
rspfEcefPoint rspfEcefPoint::operator-(const rspfEcefVector& v) const
{
   return rspfEcefPoint(theData - v.data());
}

rspfString rspfEcefPoint::toString(rspf_uint32 precision) const
{
   return theData.toString(precision);
}

void rspfEcefPoint::toPoint(const std::string& s)
{
   theData.toPoint(s);
}

void rspfEcefPoint::print(std::ostream& os) const
{
   if(isNan())
   {
      os << "(rspfEcefPoint) " << "nan nan nan";
   }
   
   os << "(rspfEcefPoint) " << theData;
}

std::ostream& operator<<(std::ostream& os , const rspfEcefPoint& instance)
{
   instance.print(os); return os;
}
