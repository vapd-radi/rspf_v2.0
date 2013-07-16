//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for LookUpTable. 
//*******************************************************************
//  $Id: rspfLookUpTable.h 19448 2011-04-26 15:15:07Z dburken $

#ifndef rspfLookUpTable_HEADER
#define rspfLookUpTable_HEADER

#include <vector>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeyword.h>

class rspfKeywordlist;

//*******************************************************************
// CLASS:  LookUpTable
//*******************************************************************
class RSPFDLLEXPORT rspfLookUpTable
{
public:

   enum
   {
      NOT_FOUND    = -1
   };
   
   virtual ~rspfLookUpTable();

   /*!
    *  @return
    *  Returns the entry string associated with the entry number (key) passed
    *  in. Returns empty string if entry number is not in the list.
    */
   virtual rspfString getEntryString(rspf_int32 entry_number) const;

   /*!
    *  @param table_index Index into the table
    *  Note:  This is not the key mapped to the but the index into the table.
    *  @return
    *  Returns the entry string associated with the table index passed in.
    *  Returns empty string if index is out of range.
    */
   virtual rspfString getTableIndexString(rspf_uint32 table_index) const;

   /*!
    *  Returns the entry string associated with the entry number passed in.
    *  Returns empty string if entry number is not in the list.
    */
   virtual rspfString operator[](rspf_int32 entry_number) const;

   /*!
    *  Returns the entry string associated with lookup table keyword entry
    *  in the Keywordlist passed in.
    *  Returns empty string if keyword entry is not in the Keywordlist.
    */
   virtual rspfString getEntryString(const rspfKeywordlist& kwl,
                                      const char* prefix=0) const;

   /*!
    *  Returns the entry number associated with the entry string passed in.
    *  Returns NOT_FOUND(-1) if entry string is not in the list.
    *  If case_insensitive == true(default), the test is case insensitive;
    *  else, the test will be case sensitive.
    */
   virtual rspf_int32 getEntryNumber(const char* entry_string,
                                      bool case_insensitive = true) const;

   /*!
    *  Returns the entry number associated with the lookup table keyword
    *  entry in the Keywordlist passed in.  Returns NOT_FOUND(-1) if no
    *  matching entry.
    *  If case_insensitive == true(default), the test is case insensitive;
    *  else, the test will be case sensitive.
    */
   virtual rspf_int32 getEntryNumber(const rspfKeywordlist& kwl,
                                      const char* prefix=0,
                                      bool case_insensitive = true) const;

   /*!
    *  Returns keyword for lookups from a Keywordlist.
    */
   virtual rspfKeyword getKeyword() const=0;

   virtual rspf_uint32 getTableSize() const;
  
  void dumpValues(std::ostream& out)const;
protected:

   rspfLookUpTable(rspf_int32 table_size);

   class rspfKeyValueMap
   {
   public:
      void init (rspf_int32 key, const rspfString& value) { theKey=key; theValue=value; }
      rspf_int32  theKey;
      rspfString theValue;
   };

   std::vector<rspfKeyValueMap>  theTable;
   
   rspfLookUpTable(){}
};

#endif
