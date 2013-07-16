//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfDisplayInterface.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/base/rspfDisplayInterface.h>

RTTI_DEF(rspfDisplayInterface, "rspfDisplayInterface");

rspfDisplayInterface::rspfDisplayInterface()
{
}

rspfDisplayInterface::~rspfDisplayInterface()
{
}

rspfString rspfDisplayInterface::getTitle()const
{
   rspfString result;
   
   getTitle(result);
   
   return result;
}
