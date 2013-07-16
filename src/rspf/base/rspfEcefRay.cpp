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
//   10Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Log$
//  Revision 1.4  2006/11/29 02:01:07  gpotts
//  pdated license
//
//  Revision 1.3  2006/06/13 19:10:51  dburken
//  Global include path change.
//
//  Revision 1.2  2002/05/31 12:51:14  okramer
//  Added rspfEcefRay::intersectAboveEarthEllipsoid()
//
//  Revision 1.1  2001/08/14 15:01:45  okramer
//  Initial delivery (okramer@imagelinks.com)
//

#include <ostream>

#include <rspf/base/rspfEcefRay.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>

//***
// Define Trace flags for use within this file:
//***
#include <rspf/base/rspfTrace.h>
static rspfTrace traceExec  ("rspfEcefRay:exec");
static rspfTrace traceDebug ("rspfEcefRay:debug");

//*****************************************************************************
//  CONSTRUCTOR: rspfEcefRay(rspfEcefPoint, rspfEcefPoint)
//  
//  Constructs a ray originating at "from" and pointing in direction of "to"
//  
//*****************************************************************************
rspfEcefRay::rspfEcefRay(const rspfEcefPoint& from,
                           const rspfEcefPoint& to)
   : theOrigin(from)
{
   rspfEcefVector ecef (to - from);
   ecef.normalize();
   theDirection = ecef;
}
   

//*****************************************************************************
//  CONSTRUCTOR: rspfEcefRay(rspfGpt, rspfGpt)
//  
//  Constructs a ray originating at "from" and pointing in direction of "to"
//  
//*****************************************************************************
rspfEcefRay::rspfEcefRay(const rspfGpt& from,
                           const rspfGpt& to)
   : theOrigin(from)
{
   rspfEcefVector ecef (rspfEcefPoint(to) - rspfEcefPoint(from));
   ecef.normalize();
   theDirection = ecef;
}
   
//*****************************************************************************
//  METHOD: rspfEcefRay::intersectAboveEarthEllipsoid
//  
//  Intersects the ray with the given elevation above the earth ellipsoid.
//  
//*****************************************************************************
rspfEcefPoint rspfEcefRay::intersectAboveEarthEllipsoid
   (const double&     argHeight,
    const rspfDatum* argDatum) const
{
   const rspfDatum* datum = argDatum;
   if (!datum)
      datum = rspfDatumFactory::instance()->wgs84();

   //***
   // Pass the call on to ellipsoid where intersection math is implemented:
   //***
   rspfEcefPoint solution;
   bool intersected = datum->ellipsoid()->nearestIntersection
                      (*this, argHeight, solution);

   if (!intersected)
      solution.makeNan();

   return solution;
}

std::ostream& rspfEcefRay::print(std::ostream& os) const
{
   os << "(rspfEcefRay)\n"
      << "   theOrigin    = " << theOrigin
      << "\n   theDirection = " << theDirection << std::endl;
   return os;
}

std::ostream& operator<<(std::ostream& os ,
                         const rspfEcefRay& instance)
{
   return instance.print(os);
}


