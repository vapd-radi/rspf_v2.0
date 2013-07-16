//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Frank Warmerdam (warmerdam@pobox.com)
//
// Description:
//
// Contains class declaration for rspfImageWriter
//*******************************************************************
//  $Id: rspfImageDisplayWriter.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/imaging/rspfImageDisplayWriter.h>

RTTI_DEF3(rspfImageDisplayWriter, "rspfImageDisplayWriter", rspfImageWriter, rspfDisplayInterface, rspfConnectableObjectListener);

rspfImageDisplayWriter::rspfImageDisplayWriter(rspfObject* owner)
   :rspfImageWriter(owner,
                     1,
                     0,
                     true,
                     true),
    rspfConnectableObjectListener()
{
}

rspfImageDisplayWriter::rspfImageDisplayWriter(rspfObject* owner,
                                                 rspf_uint32 numberOfInputs,
                                                 rspf_uint32 numberOfOutputs,
                                                 bool inputListIsFixed,
                                                 bool outputListIsFixed)
   : rspfImageWriter(owner,
                      numberOfInputs,
                      numberOfOutputs,
                      inputListIsFixed,
                      outputListIsFixed),
     rspfConnectableObjectListener()
{
}
