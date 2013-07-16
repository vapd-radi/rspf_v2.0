//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfUnitTypeLut.cpp 10378 2007-01-26 14:27:21Z gpotts $

#include <rspf/base/rspfUnitTypeLut.h>

rspfUnitTypeLut* rspfUnitTypeLut::theInstance = NULL;

static const int TABLE_SIZE = 17;

rspfUnitTypeLut* rspfUnitTypeLut::instance()
{
   if (!theInstance)
   {
      theInstance = new rspfUnitTypeLut;
   }
   return theInstance;
}

rspfUnitTypeLut::rspfUnitTypeLut()
  :rspfLookUpTable(TABLE_SIZE)
{
   theTable[0].theKey    = RSPF_UNIT_UNKNOWN;
   theTable[0].theValue  = "unknown";
   theTable[1].theKey    = RSPF_METERS;
   theTable[1].theValue  = "meters";
   theTable[2].theKey    = RSPF_FEET;
   theTable[2].theValue  = "feet";
   theTable[3].theKey    = RSPF_US_SURVEY_FEET;
   theTable[3].theValue  = "us_survey_feet";
   theTable[4].theKey    = RSPF_DEGREES;
   theTable[4].theValue  = "degrees";
   theTable[5].theKey    = RSPF_RADIANS;
   theTable[5].theValue  = "radians";
   theTable[6].theKey    = RSPF_NAUTICAL_MILES;
   theTable[6].theValue  = "nautical_miles";
   theTable[7].theKey    = RSPF_SECONDS;
   theTable[7].theValue  = "seconds";
   theTable[8].theKey    = RSPF_MINUTES;
   theTable[8].theValue  = "minutes";
   theTable[9].theKey    = RSPF_PIXEL;
   theTable[9].theValue  = "pixel";
   theTable[10].theKey   = RSPF_MILES;
   theTable[10].theValue = "miles";
   theTable[11].theKey   = RSPF_MILLIMETERS;
   theTable[11].theValue = "millimeters";
   theTable[12].theKey   = RSPF_MICRONS;
   theTable[12].theValue = "microns";
   theTable[13].theKey   = RSPF_CENTIMETERS;
   theTable[13].theValue = "centimeters";
   theTable[14].theKey   = RSPF_YARDS;
   theTable[14].theValue = "yards";
   theTable[15].theKey   = RSPF_INCHES;
   theTable[15].theValue = "inches";
   theTable[16].theKey   = RSPF_KILOMETERS;
   theTable[16].theValue = "kilometers";
}

rspfUnitTypeLut::~rspfUnitTypeLut()
{
}
rspf_int32 rspfUnitTypeLut::getEntryNumber(const char* entry_string,
                                             bool case_insensitive) const
{
   rspf_int32 result
      = rspfLookUpTable::getEntryNumber(entry_string, case_insensitive);
   if(result < 0)
   {
      return (rspf_int32)RSPF_UNIT_UNKNOWN;
   }
   
   return result;
}

rspf_int32 rspfUnitTypeLut::getEntryNumber(const rspfKeywordlist& kwl,
                                             const char* prefix,
                                             bool case_insensitive) const
{
   rspf_int32 result =
      rspfLookUpTable::getEntryNumber(kwl, prefix, case_insensitive);
   if(result < 0)
   {
      return (rspf_int32)RSPF_UNIT_UNKNOWN;
   }
   return result;
}

rspfKeyword rspfUnitTypeLut::getKeyword() const
{
   return rspfKeyword(rspfKeywordNames::UNITS_KW, "");
}
