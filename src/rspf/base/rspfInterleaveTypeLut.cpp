//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:
//
// Contains class definition for InterleaveTypeLUT.
//*******************************************************************
//  $Id: rspfInterleaveTypeLut.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <rspf/base/rspfInterleaveTypeLut.h>

static const int TABLE_SIZE = 4;

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfInterleaveTypeLut::rspfInterleaveTypeLut()
   :
      rspfLookUpTable(TABLE_SIZE)
{
   //***
   // Complete initialization of data member "theTable".
   // Note:  Interleave types defined in constants.h file.
   //***
   theTable[0].theKey   = RSPF_BIL;
   theTable[0].theValue = "bil";
   theTable[1].theKey   = RSPF_BIP;
   theTable[1].theValue = "bip";
   theTable[2].theKey   = RSPF_BSQ;
   theTable[2].theValue = "bsq";
   theTable[3].theKey   = RSPF_BSQ_MULTI_FILE;
   theTable[3].theValue = "bsq_multi_file";
}
