//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author:  Garrett Potts 
//
//*******************************************************************
//  $Id: rspfListener.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/base/rspfListener.h>

RTTI_DEF(rspfListener, "rspfListener");

rspfListener::rspfListener()
  :theListenerEnableFlag(true)
{}

rspfListener::~rspfListener()
{
}

void rspfListener::processEvent(rspfEvent& /* event */)
{
}

void rspfListener::enableListener()
{
  theListenerEnableFlag = true;
}

void rspfListener::disableListener()
{
  theListenerEnableFlag = false;
}

void rspfListener::setListenerEnableFlag(bool flag)
{
  theListenerEnableFlag = flag;
}

bool rspfListener::isListenerEnabled()const
{
  return theListenerEnableFlag;
}

bool rspfListener::getListenerEnableFlag()const
{
  return theListenerEnableFlag;
}
