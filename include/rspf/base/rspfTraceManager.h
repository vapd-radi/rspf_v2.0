//*****************************************************************************
// Copyright (C) 2005 Garrett Potts, all rights reserved.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
// 
// DESCRIPTION:
//   Contains declaration of class rspfTraceManager.
//
//*****************************************************************************
// $Id: rspfTraceManager.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfTraceManager_HEADER
#define rspfTraceManager_HEADER

#include <rspf/base/rspfString.h>

class rspfTrace;

class RSPFDLLEXPORT rspfTraceManager
{
public:
   /**
    * @return rspfTraceManager* to instance of the rspf trace manager.
    */
   static rspfTraceManager* instance();

   /**
    * Sets "thePattern" to "pattern", then calls setTraceFlags(true).
    *
    * @param pattern Regular expression to enable trace for.
    */
   void setTracePattern(const rspfString& pattern);

   /**
    * @param traceObj rspfTrace* to add to "theTraceList".
    */
   void addTrace(rspfTrace* traceObj);

   /**
    * @param traceObj rspfTrace* to remove from "theTraceList".
    */
   void removeTrace(rspfTrace* traceObj);
   
protected:
   /** Protected default constructor. */
   rspfTraceManager();

private:
   /**
    * Loops through "theTraceList" and sets any trace object's enable flag to
    * flag if it matches regular expression of "thePattern".
    * For trace objects not matching "thePattern" no action is taken.
    */
   void setTraceFlags(bool flag);

   /** The instance of this manager. */
   static rspfTraceManager* theInstance;

   /** The regular expression pattern like "rspfDynamic|rspfGdal". */
   rspfString               thePattern;

   /** The list of trace objects. */
   std::vector<rspfTrace*>  theTraceList;
};

#endif
