//*****************************************************************************
// FILE: rspfEcefVector.cpp
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
//  $Id: rspfEcefVector.cpp 9531 2006-09-11 11:18:27Z dburken $

#include <sstream>
#include <rspf/base/rspfEcefVector.h>

rspfString rspfEcefVector::toString(rspf_uint32 precision) const
{
   return theData.toString(precision);
}

void rspfEcefVector::toPoint(const std::string& s)
{
   theData.toPoint(s);
}
