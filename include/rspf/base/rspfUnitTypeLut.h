//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfUnitTypeLut.h 11959 2007-10-31 19:22:56Z gpotts $
#ifndef rspfUnitTypeLut_HEADER
#define rspfUnitTypeLut_HEADER

#include <rspf/base/rspfLookUpTable.h>
#include <rspf/base/rspfKeywordNames.h>

class RSPFDLLEXPORT rspfUnitTypeLut : public rspfLookUpTable
{
public:

   /**
    * Returns the static instance of an rspfUnitTypeLut object.
    */
   static rspfUnitTypeLut* instance();

   virtual ~rspfUnitTypeLut();

   /*!
    *  Returns the entry number associated with the entry string passed in.
    *  Returns RSPF_UNIT_UNKNOWN if entry string is not in the list.
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
   
   virtual rspfKeyword getKeyword() const;

protected:
   rspfUnitTypeLut();

private:
   static rspfUnitTypeLut* theInstance;
   
};

#endif
