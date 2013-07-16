//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Description:
// 
// Contains class definition with common error codes and methods to go
// from code to string and string to code.
//
//*************************************************************************
// $Id: rspfErrorCodes.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfString.h>

const rspfErrorCode rspfErrorCodes::RSPF_OK                   = 0;
const rspfErrorCode rspfErrorCodes::RSPF_NO_ERROR             = 0;
const rspfErrorCode rspfErrorCodes::RSPF_WARNING              = -1;
const rspfErrorCode rspfErrorCodes::RSPF_ERROR                = 1;
const rspfErrorCode rspfErrorCodes::RSPF_ERROR_STD_PARALLEL_1 = 2; 
const rspfErrorCode rspfErrorCodes::RSPF_LAT_ERROR            = 3;
const rspfErrorCode rspfErrorCodes::RSPF_LON_ERROR            = 4;
const rspfErrorCode rspfErrorCodes::RSPF_NORTHING_ERROR       = 5;
const rspfErrorCode rspfErrorCodes::RSPF_ORIGIN_LAT_ERROR     = 6;
const rspfErrorCode rspfErrorCodes::RSPF_CENT_MER_ERROR       = 7;
const rspfErrorCode rspfErrorCodes::RSPF_A_ERROR              = 8;
const rspfErrorCode rspfErrorCodes::RSPF_B_ERROR              = 9;
const rspfErrorCode rspfErrorCodes::RSPF_A_LESS_B_ERROR       = 10;
const rspfErrorCode rspfErrorCodes::RSPF_FIRST_STDP_ERROR     = 11;
const rspfErrorCode rspfErrorCodes::RSPF_SEC_STDP_ERROR       = 12;
const rspfErrorCode rspfErrorCodes::RSPF_FIRST_SECOND_ERROR   = 13;
const rspfErrorCode rspfErrorCodes::RSPF_HEMISPHERE_ERROR     = 14;
const rspfErrorCode rspfErrorCodes::RSPF_EASTING_ERROR        = 15;
const rspfErrorCode rspfErrorCodes::RSPF_RADIUS_ERROR         = 16;
const rspfErrorCode rspfErrorCodes::RSPF_ORIGIN_LON_ERROR     = 17;
const rspfErrorCode rspfErrorCodes::RSPF_ORIENTATION_ERROR    = 18;
const rspfErrorCode rspfErrorCodes::RSPF_SCALE_FACTOR_ERROR   = 19;
const rspfErrorCode rspfErrorCodes::RSPF_ZONE_ERROR           = 20;
const rspfErrorCode rspfErrorCodes::RSPF_ZONE_OVERRIDE_ERROR  = 21;
const rspfErrorCode rspfErrorCodes::RSPF_INVALID_FILE_ERROR   = 22;
const rspfErrorCode rspfErrorCodes::RSPF_OPEN_FILE_ERROR      = 23;
const rspfErrorCode rspfErrorCodes::RSPF_WRITE_FILE_ERROR     = 24;
const rspfErrorCode rspfErrorCodes::RSPF_ERROR_UNKNOWN        = RSPF_INT_NAN;

rspfErrorCodes* rspfErrorCodes::theInstance = NULL;

rspfErrorCodes::rspfErrorCodes()
{
}
   
rspfErrorCodes::rspfErrorCodes(const rspfErrorCodes& /* rhs */ )
{
}

const rspfErrorCodes& rspfErrorCodes::operator=(const rspfErrorCodes&)
{
  return *this;
}

rspfErrorCodes::~rspfErrorCodes()
{
   if (theInstance)
   {
      delete theInstance;
      theInstance = NULL;
   }
}

rspfErrorCodes* rspfErrorCodes::instance()
{
   if (!theInstance)
   {
      theInstance = new rspfErrorCodes();
   }

   return theInstance;
}

rspfErrorCode rspfErrorCodes::getErrorCode(const rspfString& error_string) const
{
   // Upcase...
   rspfString str;
   str = str.upcase(error_string);

   if (str == "RSPF_OK")
   {
      return rspfErrorCodes::RSPF_OK;
   }
   else if (str == "RSPF_NO_ERROR")
   {
      return rspfErrorCodes::RSPF_NO_ERROR;
   }
   else if (str == "RSPF_WARNING")
   {
      return rspfErrorCodes::RSPF_WARNING;
   }
   else if (str == "RSPF_ERROR")
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   else if (str == "RSPF_ERROR_STD_PARALLEL_1")
   {
      return rspfErrorCodes::RSPF_ERROR_STD_PARALLEL_1;
   }
   else if (str == "RSPF_LAT_ERROR")
   {
      return rspfErrorCodes::RSPF_LAT_ERROR;
   }
   else if (str == "RSPF_LON_ERROR")
   {
      return rspfErrorCodes::RSPF_LON_ERROR;
   }
   else if (str == "RSPF_NORTHING_ERROR")
   {
      return rspfErrorCodes::RSPF_NORTHING_ERROR;
   }
   else if (str == "RSPF_ORIGIN_LAT_ERROR")
   {
      return rspfErrorCodes::RSPF_ORIGIN_LAT_ERROR;
   }
   else if (str == "RSPF_CENT_MER_ERROR")
   {
      return rspfErrorCodes::RSPF_CENT_MER_ERROR;
   }
   else if (str == "RSPF_A_ERROR")
   {
      return rspfErrorCodes::RSPF_B_ERROR;
   }
   else if (str == "RSPF_A_LESS_B_ERROR")
   {
      return rspfErrorCodes::RSPF_A_LESS_B_ERROR;
   }
   else if (str == "RSPF_FIRST_STDP_ERROR")
   {
      return rspfErrorCodes::RSPF_FIRST_STDP_ERROR;
   }
   else if (str == "RSPF_SEC_STDP_ERROR")
   {
      return rspfErrorCodes::RSPF_SEC_STDP_ERROR;
   }
   else if (str == "RSPF_FIRST_SECOND_ERROR")
   {
      return rspfErrorCodes::RSPF_FIRST_SECOND_ERROR;
   }
   else if (str == "RSPF_HEMISPHERE_ERROR")
   {
      return rspfErrorCodes::RSPF_HEMISPHERE_ERROR;
   }
   else if (str == "RSPF_EASTING_ERROR")
   {
      return rspfErrorCodes::RSPF_EASTING_ERROR;
   }
   else if (str == "RSPF_RADIUS_ERROR")
   {
      return rspfErrorCodes::RSPF_RADIUS_ERROR;
   }
   else if (str == "RSPF_ORIGIN_LON_ERROR")
   {
      return rspfErrorCodes::RSPF_ORIGIN_LON_ERROR;
   }
   else if (str == "RSPF_ORIENTATION_ERROR")
   {
      return rspfErrorCodes::RSPF_ORIENTATION_ERROR;
   }
   else if (str == "RSPF_SCALE_FACTOR_ERROR")
   {
      return rspfErrorCodes::RSPF_SCALE_FACTOR_ERROR;
   }
   else if (str == "RSPF_ZONE_ERROR")
   {
      return rspfErrorCodes::RSPF_ZONE_ERROR;
   }
   else if (str == "RSPF_ZONE_OVERRIDE_ERROR")
   {
      return rspfErrorCodes::RSPF_ZONE_OVERRIDE_ERROR;
   }
   else if (str == "RSPF_INVALID_FILE_ERROR")
   {
      return rspfErrorCodes::RSPF_INVALID_FILE_ERROR;
   }
   else if (str == "RSPF_OPEN_FILE_ERROR")
   {
      return rspfErrorCodes::RSPF_OPEN_FILE_ERROR;
   }
   else if (str == "RSPF_WRITE_FILE_ERROR")
   {
      return rspfErrorCodes::RSPF_WRITE_FILE_ERROR;
   }

   return RSPF_ERROR_UNKNOWN;
}

rspfString rspfErrorCodes::getErrorString(rspfErrorCode error_code) const
{
   if(error_code == RSPF_OK)
   {
      // No difference between RSPF_NO_ERROR and RSPF_OK.
      return rspfString("RSPF_OK");
   }
   if(error_code == RSPF_WARNING)
   {
      return rspfString("RSPF_WARNING");
   }
   if(error_code == RSPF_ERROR)
   {
      return rspfString("RSPF_ERROR");
   }
   if(error_code == RSPF_ERROR_STD_PARALLEL_1)
   {
      return rspfString("RSPF_ERROR_STD_PARALLEL_1");
   }
   if(error_code == RSPF_LAT_ERROR)
   {
      return rspfString("RSPF_LAT_ERROR");
   }
   if(error_code == RSPF_LON_ERROR)
   {
      return rspfString("RSPF_LON_ERROR");
   }
   if(error_code == RSPF_NORTHING_ERROR)
   {
      return rspfString("RSPF_NORTHING_ERROR");
   }
   if(error_code == RSPF_ORIGIN_LAT_ERROR)
   {
      return rspfString("RSPF_ORIGIN_LAT_ERROR");
   }
   if(error_code == RSPF_CENT_MER_ERROR)
   {
      return rspfString("RSPF_CENT_MER_ERROR");
   }
   if(error_code == RSPF_A_ERROR)
   {
      return rspfString("RSPF_A_ERROR");
   }
   if(error_code == RSPF_B_ERROR)
   {
      return rspfString("RSPF_B_ERROR");
   }
   if(error_code == RSPF_A_LESS_B_ERROR)
   {
      return rspfString("RSPF_A_LESS_B_ERROR");
   }
   if(error_code == RSPF_FIRST_STDP_ERROR)
   {
      return rspfString("RSPF_FIRST_STDP_ERROR");
   }
   if(error_code == RSPF_SEC_STDP_ERROR)
   {
      return rspfString("RSPF_SEC_STDP_ERROR");
   }
   if(error_code == RSPF_FIRST_SECOND_ERROR)
   {
      return rspfString("RSPF_FIRST_SECOND_ERROR");
   }
   if(error_code == RSPF_HEMISPHERE_ERROR)
   {
      return rspfString("RSPF_HEMISPHERE_ERROR");
   }
   if(error_code == RSPF_EASTING_ERROR)
   {
      return rspfString("RSPF_EASTING_ERROR");
   }
   if(error_code == RSPF_RADIUS_ERROR)
   {
      return rspfString("RSPF_RADIUS_ERROR");
   }
   if(error_code == RSPF_ORIGIN_LON_ERROR)
   {
      return rspfString("RSPF_ORIGIN_LON_ERROR");
   }
   if(error_code == RSPF_ORIENTATION_ERROR)
   {
      return rspfString("RSPF_ORIENTATION_ERROR");
   }
   if(error_code == RSPF_SCALE_FACTOR_ERROR)
   {
      return rspfString("RSPF_SCALE_FACTOR_ERROR");
   }
   if(error_code == RSPF_ZONE_ERROR)
   {
      return rspfString("RSPF_ZONE_ERROR");
   }
   if(error_code == RSPF_ZONE_OVERRIDE_ERROR)
   {
      return rspfString("RSPF_ZONE_OVERRIDE_ERROR");
   }
   if(error_code == RSPF_INVALID_FILE_ERROR)
   {
      return rspfString("RSPF_INVALID_FILE_ERROR");
   }
   if(error_code == RSPF_OPEN_FILE_ERROR)
   {
      return rspfString("RSPF_OPEN_FILE_ERROR");
   }
   if(error_code == RSPF_WRITE_FILE_ERROR)
   {
      return rspfString("RSPF_WRITE_FILE_ERROR");
   }

   return rspfString("RSPF_ERROR_UNKNOWN");
}
