//*****************************************************************************
// FILE: rspfGridRemapEngineFactory.cc
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains implementation of class rspfGridRemapEngineFactory
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfGridRemapEngineFactory.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/imaging/rspfGridRemapEngineFactory.h>
#include <rspf/imaging/rspfHsvGridRemapEngine.h>
#include <rspf/imaging/rspfRgbGridRemapEngine.h>
#include <rspf/imaging/rspfMonoGridRemapEngine.h>

//*****************************************************************************
//  STATIC METHOD: rspfGridRemapEngineFactory::create()
//  
//*****************************************************************************
rspfGridRemapEngine* rspfGridRemapEngineFactory::create(const char* s)
{
   if (!s)
      return 0;
   
   if (strcmp(s, "rspfHsvGridRemapEngine") == 0)
      return new rspfHsvGridRemapEngine;

   else  if (strcmp(s, "rspfRgbGridRemapEngine") == 0)
      return new rspfRgbGridRemapEngine;

   else  if (strcmp(s, "rspfMonoGridRemapEngine") == 0)
      return new rspfMonoGridRemapEngine;

   return 0;
}

