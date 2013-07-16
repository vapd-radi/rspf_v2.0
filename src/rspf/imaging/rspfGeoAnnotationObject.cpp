//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationObject.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/imaging/rspfGeoAnnotationObject.h>

RTTI_DEF1(rspfGeoAnnotationObject,
          "rspfGeoAnnotationObject",
          rspfAnnotationObject)

rspfGeoAnnotationObject::rspfGeoAnnotationObject(unsigned char r,
                                                   unsigned char g,
                                                   unsigned char b,
                                                   long thickness)
   :rspfAnnotationObject(r, g, b, thickness)
{
}

rspfGeoAnnotationObject::rspfGeoAnnotationObject(
   const rspfGeoAnnotationObject& rhs)
   :
   rspfAnnotationObject(rhs)
{
}

rspfGeoAnnotationObject::~rspfGeoAnnotationObject()
{
}

bool rspfGeoAnnotationObject::saveState(rspfKeywordlist& kwl,
                                         const char* prefix) const
{
   return rspfAnnotationObject::saveState(kwl, prefix);
}

bool rspfGeoAnnotationObject::loadState(const rspfKeywordlist& kwl,
                                         const char* prefix)
{
   return rspfAnnotationObject::loadState(kwl, prefix);
}
