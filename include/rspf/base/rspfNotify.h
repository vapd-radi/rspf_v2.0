//-------------------------------------------------------------------
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for rspfNotify
//-------------------------------------------------------------------
//  $Id: rspfNotify.h 21458 2012-08-07 20:40:35Z dburken $
#ifndef rspfNotify_HEADER
#define rspfNotify_HEADER 1

#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <iostream>
#include <fstream>

/**
  * Notification level enumeration.  This specifies the
  * level of the mesage being written.
  */
enum rspfNotifyLevel
{
   rspfNotifyLevel_ALWAYS = 0,
   rspfNotifyLevel_FATAL  = 1,
   rspfNotifyLevel_WARN   = 2,
   rspfNotifyLevel_NOTICE = 3,
   rspfNotifyLevel_INFO   = 4,
   rspfNotifyLevel_DEBUG  = 5
};

/**
 * Flags to allow the user to turn off certain notification levels
 * 
 */
enum rspfNotifyFlags
{
   rspfNotifyFlags_NONE   = 0,
   rspfNotifyFlags_FATAL  = 1,
   rspfNotifyFlags_WARN   = 2,
   rspfNotifyFlags_NOTICE = 4,
   rspfNotifyFlags_INFO   = 8,
   rspfNotifyFlags_DEBUG  = 16,
   rspfNotifyFlags_ALL    = (rspfNotifyFlags_FATAL|
                              rspfNotifyFlags_WARN|
                              rspfNotifyFlags_NOTICE|
                              rspfNotifyFlags_INFO|
                              rspfNotifyFlags_DEBUG)
};

RSPFDLLEXPORT void rspfSetDefaultNotifyHandlers();

RSPFDLLEXPORT void rspfSetNotifyStream(std::ostream* outputStream,
                                         rspfNotifyFlags whichLevelsToRedirect=rspfNotifyFlags_ALL);

RSPFDLLEXPORT std::ostream* rspfGetNotifyStream(rspfNotifyLevel whichLevel);

RSPFDLLEXPORT bool rspfIsReportingEnabled();

RSPFDLLEXPORT std::ostream& rspfNotify(rspfNotifyLevel level = rspfNotifyLevel_WARN);

/**
 *
 */
RSPFDLLEXPORT void rspfSetLogFilename(const rspfFilename& filename);

/** @brief Returns the log filename if set. */
/* RSPFDLLEXPORT const char* rspfGetLogFilename(); */
RSPFDLLEXPORT void rspfGetLogFilename(rspfFilename& logFile);


/**
 *
 */
RSPFDLLEXPORT void rspfDisableNotify(rspfNotifyFlags notifyLevel=rspfNotifyFlags_ALL);

/**
 *
 */
RSPFDLLEXPORT void rspfEnableNotify(rspfNotifyFlags notifyLevel=rspfNotifyFlags_ALL);


RSPFDLLEXPORT void rspfSetNotifyFlag(rspfNotifyFlags notifyFlags);
RSPFDLLEXPORT void rspfPushNotifyFlags();
RSPFDLLEXPORT void rspfPopNotifyFlags();
RSPFDLLEXPORT rspfNotifyFlags rspfGetNotifyFlags();


/**
 * 
 */
RSPFDLLEXPORT void  rspfSetError( const char *className,
                                    rspf_int32 error,
                                    const char *fmtString=0, ...);

/**
 * This is for general warnings and information feedback
 *
 * @param fmtString a C printf() style formatting string used (with the
 * following arguments) to prepare an error message.
 */
RSPFDLLEXPORT void  rspfSetInfo( const char *className,
                                   const char *fmtString=0, ...);

#endif /* #ifndef rspfNotify_HEADER */
