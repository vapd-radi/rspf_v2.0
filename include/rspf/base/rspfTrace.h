//*****************************************************************************
// FILE: rspfTrace.h
//
// License:  See top level LICENSE.txt file.
//
// DESCRIPTION:
//   Contains declaration of class rspfTrace. Used for tracing code execution.
//   Implemented as a singluy linked list of rspfTrace objects.
//
// SOFTWARE HISTORY:
//   24Apr2001  Oscar Kramer
//              Initial coding.
//*****************************************************************************
// $Id: rspfTrace.h 11650 2007-08-24 12:02:47Z dburken $

#ifndef rspfTrace_HEADER
#define rspfTrace_HEADER

#include <rspf/base/rspfString.h>
#include <rspf/base/rspfNotify.h>

// Macro for use with trace...
#define CLOG rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " (\"" __FILE__ "\", line " << __LINE__ << ") DEBUG: "


class RSPFDLLEXPORT rspfTrace
{
public:
   rspfTrace( const rspfString& trace_name );
   ~rspfTrace();

   /**
    * @returns true if enabled false if not.
    */
   bool isEnabled() const { return theEnabledFlag; }

   /**
    * Returns "theTraceName" as an rspfString.
    */
   rspfString getTraceName() const { return theTraceName; }


   /**
    * Sets "theEnabled" flag.
    *
    * @param flag true == enabled, false == disabled.
    */
   void setTraceFlag(bool flag) { theEnabledFlag = flag; }
   
   /**
    * Operator() for rspfTrace.  Given the static instance:
    * static rspfTrace traceDebug("myTrace");
    *
    * You can do:
    * if (traceDebug())
    * {
    *     CLOG << "Your trace stuff goes here..." << endl;
    * }
    */
   bool operator()() const { return theEnabledFlag; }

private:
   rspfString                theTraceName;
   bool                       theEnabledFlag;
};

#endif
