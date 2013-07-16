//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// DESCRIPTION:
//   Contains implementation of class rspfGeoid
//
// SOFTWARE HISTORY:
//>
//   17Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************

#include <rspf/base/rspfGeoid.h>

RTTI_DEF2(rspfGeoid, "rspfGeoid", rspfObject, rspfErrorStatusInterface)
RTTI_DEF1(rspfIdentityGeoid, "rspfIdentityGeoid", rspfGeoid)

rspfGeoid::rspfGeoid()
   :
      rspfObject(),
      rspfErrorStatusInterface()
{}

rspfGeoid::~rspfGeoid()
{}
