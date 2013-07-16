//----------------------------------------------------------------------------
// FILE: rspfElevSourceFactory.cc
//
// Copyright (C) 2002 ImageLinks, Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Class definition for rspfElevSourceFactory.
//
// This is the base class interface for elevation source factories.  Contains
// pure virtual methods that all elevation source factories must implement.
//
//----------------------------------------------------------------------------
// $Id: rspfElevSourceFactory.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/elevation/rspfElevSourceFactory.h>

RTTI_DEF1(rspfElevSourceFactory, "rspfElevSourceFactory" , rspfObject)

rspfElevSourceFactory::rspfElevSourceFactory()
   : theDirectory(rspfFilename::NIL)
{
}

rspfElevSourceFactory::~rspfElevSourceFactory()
{
}

rspfFilename rspfElevSourceFactory::getDirectory() const
{
   return theDirectory;
}

void rspfElevSourceFactory::setDirectory(const rspfFilename& directory)
{
   theDirectory = directory;
}
