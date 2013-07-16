//*****************************************************************************
// FILE: rspfTrace.cc
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// DESCRIPTION:
//   Contains implementation of class rspfTrace
//
// SOFTWARE HISTORY:
//>
//   24Apr2001  Oscar Kramer
//              Initial coding.
//<
//*****************************************************************************
// $Id: rspfTrace.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfTraceManager.h>
#include <rspf/base/rspfCommon.h>
//*****************************************************************************
//  CONSTRUCTOR: rspfTrace
//  
//*****************************************************************************
rspfTrace::rspfTrace(const rspfString& trace_name)
   :
      theTraceName   (trace_name),
      theEnabledFlag (false)
{
   rspfTraceManager::instance()->addTrace(this);
}

rspfTrace::~rspfTrace()
{
   rspfTraceManager::instance()->removeTrace(this);
}

