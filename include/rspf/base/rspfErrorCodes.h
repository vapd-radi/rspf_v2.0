//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
// 
// Description:
// 
// Contains class declaration with common error codes and methods to go
// from code to string and string to code.
//
//*************************************************************************
// $Id: rspfErrorCodes.h 14789 2009-06-29 16:48:14Z dburken $

#ifndef rspfErrorCodes_HEADER
#define rspfErrorCodes_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>


/*!
 *  class rspfErrorCodes
 *  Contains class declaration with common error codes and methods to go
 *  from code to string and string to code.
 */
class RSPFDLLEXPORT rspfErrorCodes
{
public:

   ~rspfErrorCodes();

   /*!
    *  Returns a pointer to "theInstance".
    */
   static rspfErrorCodes* instance();

   /*!
    *  Returns the error code from a string.  Returns RSPF_ERROR_UNKNOWN if
    *  the string does not match.
    */
   rspfErrorCode getErrorCode(const rspfString& error_string) const;

   /*!
    *  Returns the string matching the error code.
    *  Returns "RSPF_ERROR_UNKNOWN" if code is not valid.
    */
   rspfString getErrorString(rspfErrorCode error_code) const;
   
   static const rspfErrorCode RSPF_OK;
   static const rspfErrorCode RSPF_NO_ERROR;
   static const rspfErrorCode RSPF_WARNING;
   static const rspfErrorCode RSPF_ERROR;
   static const rspfErrorCode RSPF_ERROR_STD_PARALLEL_1; 
   static const rspfErrorCode RSPF_LAT_ERROR;
   static const rspfErrorCode RSPF_LON_ERROR;
   static const rspfErrorCode RSPF_NORTHING_ERROR;
   static const rspfErrorCode RSPF_ORIGIN_LAT_ERROR;
   static const rspfErrorCode RSPF_CENT_MER_ERROR;
   static const rspfErrorCode RSPF_A_ERROR;
   static const rspfErrorCode RSPF_B_ERROR;
   static const rspfErrorCode RSPF_A_LESS_B_ERROR;
   static const rspfErrorCode RSPF_FIRST_STDP_ERROR;
   static const rspfErrorCode RSPF_SEC_STDP_ERROR;
   static const rspfErrorCode RSPF_FIRST_SECOND_ERROR;
   static const rspfErrorCode RSPF_HEMISPHERE_ERROR;
   static const rspfErrorCode RSPF_EASTING_ERROR;
   static const rspfErrorCode RSPF_RADIUS_ERROR;
   static const rspfErrorCode RSPF_ORIGIN_LON_ERROR;
   static const rspfErrorCode RSPF_ORIENTATION_ERROR;
   static const rspfErrorCode RSPF_SCALE_FACTOR_ERROR;
   static const rspfErrorCode RSPF_ZONE_ERROR;
   static const rspfErrorCode RSPF_ZONE_OVERRIDE_ERROR;
   static const rspfErrorCode RSPF_INVALID_FILE_ERROR;
   static const rspfErrorCode RSPF_OPEN_FILE_ERROR;
   static const rspfErrorCode RSPF_WRITE_FILE_ERROR;
   static const rspfErrorCode RSPF_ERROR_UNKNOWN;
   
protected:
   // Only allow instantiation through the "instance()" method.
   rspfErrorCodes();
   rspfErrorCodes(const rspfErrorCodes& rhs);
   const rspfErrorCodes& operator=(const rspfErrorCodes &rhs);

   static rspfErrorCodes* theInstance;
};

#endif
