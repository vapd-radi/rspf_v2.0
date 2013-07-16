//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken (dburken@imagelinks.com)
//
// Description:
//
// Contains class declaration for 
//*******************************************************************
//  $Id: rspfLookUpTable.cpp 19448 2011-04-26 15:15:07Z dburken $

#include <iostream>
using namespace std;

#include <rspf/base/rspfLookUpTable.h>
#include <rspf/base/rspfKeywordlist.h>

//*******************************************************************
// Protected Constructor:
//*******************************************************************
rspfLookUpTable::rspfLookUpTable(rspf_int32 table_size)
   :
      theTable(table_size)
{}

rspfLookUpTable::~rspfLookUpTable()
{
}

//*******************************************************************
// Public Method:
//*******************************************************************
rspfString rspfLookUpTable::getEntryString(rspf_int32 entry_number) const
{
   for (rspf_uint32 i=0; i<theTable.size(); ++i)
   {
      if (theTable[i].theKey == entry_number)
      {
         return theTable[i].theValue;
      }
   }

   return rspfString(); // Entry number not found.
}

//*******************************************************************
// Public Method:
//*******************************************************************
rspfString rspfLookUpTable::getTableIndexString(rspf_uint32 table_index) const
{
   if (table_index < theTable.size())
   {
      return theTable[table_index].theValue;
   }

   return rspfString(); // Index out of range.
}

rspfString rspfLookUpTable::operator[](rspf_int32 entry_number) const
{
   return getEntryString(entry_number);
}

//*******************************************************************
// Public Method:
//*******************************************************************
rspfString rspfLookUpTable::getEntryString(const rspfKeywordlist& kwl,
                                             const char* prefix) const
{
   rspfString s = kwl.find(prefix, getKeyword());
   return s;
}

//*******************************************************************
// Public Method:
//*******************************************************************
rspf_int32 rspfLookUpTable::getEntryNumber(const char*  entry_string,
                                             bool case_insensitive) const
{
   // Filter out trivial solution of NULL string:
   rspfString s1 = entry_string;
   if (!s1.empty())
   {
      s1 = s1.trim();
      for (rspf_uint32 i=0; i<theTable.size(); i++)
      {
         rspfString s2 = theTable[i].theValue;

         if (case_insensitive == true)
         {
            s1.downcase();
            s2.downcase();
         }

         if (s1 == s2)
         {
            return theTable[i].theKey;
         }
      }
   }

   return rspfLookUpTable::NOT_FOUND; // Entry number not found.   
}

//*******************************************************************
// Public Method:
//*******************************************************************
rspf_int32 rspfLookUpTable::getEntryNumber(const rspfKeywordlist& kwl,
                                             const char* prefix,
                                             bool case_insensitive) const
{
   const char* lookup = kwl.find(prefix, getKeyword());

   if (lookup)
   {
      return getEntryNumber(lookup, case_insensitive);
   }

   return rspfLookUpTable::NOT_FOUND; // Entry number not found.   
}

rspf_uint32 rspfLookUpTable::getTableSize() const
{
   return (rspf_uint32)theTable.size();
}

void rspfLookUpTable::dumpValues(std::ostream& out)const
{
  rspf_uint32 tableSize = getTableSize();
  rspf_uint32 i = 0;

  for(i = 0; i < tableSize; ++i)
    {
      out << theTable[0].theValue << std::endl;
    }
}
