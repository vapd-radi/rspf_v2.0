//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Description:
//
// Contains class definition for rspfNormalizedU11RemapTable.
// 
//*******************************************************************
//  $Id: rspfNormalizedU11RemapTable.cpp 15743 2009-10-17 13:00:45Z dburken $

#include <rspf/imaging/rspfNormalizedU11RemapTable.h>

rspf_float64 rspfNormalizedU11RemapTable::theTable[TABLE_ENTRIES];
bool          rspfNormalizedU11RemapTable::theTableIsInitialized = false;

rspfNormalizedU11RemapTable::rspfNormalizedU11RemapTable()
   : rspfNormalizedRemapTable()
{
   if (!theTableIsInitialized)
   {
      const rspf_int32   ENTRIES     = getEntries();
      const rspf_float64 DENOMINATOR = getNormalizer();
      
      //---
      // Initialize the remap table.
      //---
      theTable[0] = 0.0; // Index zero always for null.
      for (rspf_int32 i = 1; i < ENTRIES; ++i)
      {
         theTable[i] = static_cast<rspf_float64>(i)/DENOMINATOR;
      }

      theTableIsInitialized = true;
   }
}

rspfNormalizedU11RemapTable::~rspfNormalizedU11RemapTable()
{}