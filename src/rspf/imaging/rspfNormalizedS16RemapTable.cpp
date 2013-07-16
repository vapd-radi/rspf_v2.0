//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
//
// Contains class definition for rspfNormalizedS16RemapTable.
// 
//*******************************************************************
//  $Id: rspfNormalizedS16RemapTable.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <rspf/imaging/rspfNormalizedS16RemapTable.h>

rspf_float64 rspfNormalizedS16RemapTable::theTable[TABLE_ENTRIES];
bool          rspfNormalizedS16RemapTable::theTableIsInitialized = false;

rspfNormalizedS16RemapTable::rspfNormalizedS16RemapTable()
   : rspfNormalizedRemapTable()
{
   if (!theTableIsInitialized)
   {
      const rspf_int32   ENTRIES     = getEntries();
      const rspf_float64 DENOMINATOR = getNormalizer();
      
      //---
      // Initialize the remap table.
      //
      // Specialized for elevation, make -32768 and -32767 map to 0 since
      // DTED NULL is -32767.
      //
      // NOTE: Zero will always remap back to -32768 with this hack.  This
      // could cause issues on writers that use pixFromNorm(). (drb)
      //---
      theTable[0] = 0.0; // Index zero always for null.
      theTable[1] = 0.0; // Specialized for DTED.

      for (rspf_int32 i = 2; i < ENTRIES; ++i)
      {
         theTable[i] = static_cast<rspf_float64>(i)/DENOMINATOR;
      }

      theTableIsInitialized = true;
   }
}

rspfNormalizedS16RemapTable::~rspfNormalizedS16RemapTable()
{}
