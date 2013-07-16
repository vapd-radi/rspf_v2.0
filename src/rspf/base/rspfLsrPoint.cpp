//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
//   Contains declaration of a 3D point object in the Earth-centered, earth
//   fixed (ECEF) coordinate system.
//
// SOFTWARE HISTORY:
//>
//   08Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Log$
//  Revision 1.4  2006/11/28 21:09:36  gpotts
//  Fixing copyrights
//
//  Revision 1.3  2006/06/13 19:10:52  dburken
//  Global include path change.
//
//  Revision 1.2  2003/05/02 15:34:17  dburken
//  Fixed build errors on AIX VisualAge compiler.  <dburken@imagelinks.com>
//
//  Revision 1.1  2001/08/13 21:29:18  okramer
//  Initial delivery of ECEF and LSR suite. (okramer@imagelinks.com)
//

#include <rspf/base/rspfLsrPoint.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfLsrVector.h>
#include <rspf/base/rspfMatrix3x3.h>

//*****************************************************************************
//  CONSTRUCTOR: rspfLsrPoint(rspfLsrPoint, space)
//  
//*****************************************************************************
rspfLsrPoint::rspfLsrPoint(const rspfLsrPoint& convert_this,
                             const rspfLsrSpace& new_space)
   : theLsrSpace(new_space)
{
   initialize(rspfEcefPoint(convert_this));
}

//*****************************************************************************
//  CONSTRUCTOR: rspfLsrPoint(const rspfLsrPoint&  convert_this)
//  
//*****************************************************************************
rspfLsrPoint::rspfLsrPoint(const rspfGpt&  gpt,
                             const rspfLsrSpace& space)
   : theLsrSpace(space)
{
   initialize(rspfEcefPoint(gpt));
}

//*****************************************************************************
//  CONSTRUCTOR: rspfLsrPoint(const rspfLsrPoint&  convert_this)
//  
//*****************************************************************************
rspfLsrPoint::rspfLsrPoint(const rspfEcefPoint& convert_this,
                             const rspfLsrSpace& space)
   : theLsrSpace(space)
{
   initialize(convert_this);
}

//*****************************************************************************
//  PROTECTED METHOD: rspfLsrPoint::initialize(rspfEcefPoint)
//  
//  Convenience method used by several constructors for initializing theData
//  given an ECEF point. Assumes theLsrSpace has been previously initialized.
//  
//*****************************************************************************
void rspfLsrPoint::initialize(const rspfEcefPoint& ecef_point)
{
   if(ecef_point.hasNans())
   {
      makeNan();
   }
   else
   {
      //
      // Translate to new space given the space's offset origin:
      //
      rspfColumnVector3d xlated ((ecef_point - theLsrSpace.origin()).data());
      
      //
      // Rotate by the inverse (transpose) of the LSR-to-ECEF rot matrix:
      //
      theData = theLsrSpace.ecefToLsrRotMatrix() * xlated;
   }
}

//*****************************************************************************
//   OPERATOR: rspfLsrPoint::operator-(rspfLsrPoint)
//  
//*****************************************************************************
rspfLsrVector rspfLsrPoint::operator-(const rspfLsrPoint& p) const
{
   if(hasNans()||p.hasNans()||(theLsrSpace != p.lsrSpace()))
   {
      theLsrSpace.lsrSpaceErrorMessage();
      return rspfLsrVector(rspf::nan(), rspf::nan(), rspf::nan(), theLsrSpace);
      
   }
   return rspfLsrVector(theData-p.theData, theLsrSpace);
}

//*****************************************************************************
//  OPERATOR: rspfLsrPoint::operator+(rspfLsrVector)
//  
//*****************************************************************************
rspfLsrPoint rspfLsrPoint::operator+(const rspfLsrVector& v) const
{
   if(hasNans()||v.hasNans()||(theLsrSpace != v.lsrSpace()))
   {
      rspfLsrSpace::lsrSpaceErrorMessage();
      return rspfLsrPoint(rspf::nan(), rspf::nan(), rspf::nan(), theLsrSpace);
      
   }
   return rspfLsrPoint(theData + v.data(), theLsrSpace);
}

