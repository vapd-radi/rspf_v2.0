//*******************************************************************
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
//   08Aug2001  Oscar Kramer (okramer@imagelinks.com)
//              Initial coding.
//<
//*****************************************************************************
//  $Log$
//  Revision 1.8  2006/11/28 21:09:36  gpotts
//  Fixing copyrights
//
//  Revision 1.7  2006/06/13 19:10:52  dburken
//  Global include path change.
//
//  Revision 1.6  2004/04/08 15:25:30  gpotts
//  *** empty log message ***
//
//  Revision 1.5  2004/04/08 14:12:39  gpotts
//  Fixe clog to rspfNotify
//
//  Revision 1.4  2002/10/21 21:51:46  gpotts
//  *** empty log message ***
//
//  Revision 1.3  2002/10/21 21:51:26  gpotts
//  *** empty log message ***
//
//  Revision 1.2  2001/12/20 22:26:41  bpayne
//  Added constructor accepting two rspfLsrPoint args
//
//  Revision 1.1  2001/08/13 21:29:18  okramer
//  Initial delivery of ECEF and LSR suite. (okramer@imagelinks.com)
//

#include <rspf/base/rspfLsrRay.h>

//*****************************************************************************
//  CONSTRUCTOR:    rspfLsrRay(rspfLsrPoint, rspfLsrVector)
//  
//  Constructs by transforming the given ray into the new space.
//  
//*****************************************************************************
rspfLsrRay::rspfLsrRay(const rspfLsrPoint&  origin,
                         const rspfLsrVector& direction)
   : theOrigin(origin),
     theDirection(direction.unitVector())
{
   if (origin.lsrSpace() != direction.lsrSpace() || hasNans())
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "FATAL -- rspfLsrRay(rspfLsrPoint,rspfLsrVector) Constructor:"
                                          << "\n   The origin and direction LSR quantities do not share the"
                                          << "\n   same LSR space. Setting to NAN. Check the data for errors." << std::endl;

      theOrigin    = rspfLsrPoint (rspf::nan(), rspf::nan(), rspf::nan(),
                                    origin.lsrSpace());
      theDirection = rspfLsrVector(rspf::nan(), rspf::nan(), rspf::nan(),
                                    direction.lsrSpace());
   }    
}

//*****************************************************************************
//  CONSTRUCTOR:    rspfLsrRay(rspfLsrPoint, rspfLsrPoint)
//  
//  Constructs by transforming the given ray into the new space.
//  
//*****************************************************************************
rspfLsrRay::rspfLsrRay(const rspfLsrPoint&  origin,
                         const rspfLsrPoint&  towards)
   : theOrigin(origin)
{
   if ((origin.lsrSpace() != towards.lsrSpace())||
       origin.hasNans()||
       towards.hasNans() )
   {
      rspfNotify(rspfNotifyLevel_FATAL) << "ERROR -- rspfLsrRay(rspfLsrPoint,rspfLsrPoint) Constructor:"
                                          << "\n   The origin and direction LSR quantities do not share the"
                                          << "\n   same LSR space. Setting to NAN. Check the data for errors." << std::endl;
      
      theOrigin    = rspfLsrPoint (rspf::nan(), rspf::nan(), rspf::nan(),
                                    origin.lsrSpace());
      theDirection = rspfLsrVector(rspf::nan(), rspf::nan(), rspf::nan(),
                                    origin.lsrSpace());
   }
   else
   {
      theDirection = towards - origin;
      theDirection.normalize();
   }
}
