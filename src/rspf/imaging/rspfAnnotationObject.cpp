//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationObject.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/imaging/rspfAnnotationObject.h>

RTTI_DEF1(rspfAnnotationObject, "rspfAnnotationObject", rspfObject)

rspf_uint32 rspfAnnotationObject::theIdCounter = 0;

rspfAnnotationObject::~rspfAnnotationObject()
{}

void rspfAnnotationObject::restIdCounter()
{
  theIdCounter = 0;
}

rspf_uint32 rspfAnnotationObject::getNextId()
{
  rspf_uint32 id = theIdCounter;
  ++theIdCounter;
  return id;
}

rspfAnnotationObject::rspfAnnotationObject(rspf_uint8 r,
                                             rspf_uint8 g,
                                             rspf_uint8 b,
                                             rspf_uint8 thickness)
   :theRed(r),
    theGreen(g),
    theBlue(b),
    theThickness(thickness)
{
  theId = getNextId();
}

void rspfAnnotationObject::setColor(rspf_uint8 r,
                                     rspf_uint8 g,
                                     rspf_uint8 b)
{
   theRed    = r;
   theGreen = g;
   theBlue   = b;
}

void rspfAnnotationObject::setThickness(rspf_uint8 thickness)
{
   theThickness = thickness;
}

void rspfAnnotationObject::getColor(rspf_uint8 &r,
                                     rspf_uint8 &g,
                                     rspf_uint8 &b)const
{
   r = theRed;
   g = theGreen;
   b = theBlue;
}

rspf_uint8 rspfAnnotationObject::getThickness()const
{
   return theThickness;
}

rspf_uint32 rspfAnnotationObject::getId()const
{
  return theId;
}

rspf_uint32 rspfAnnotationObject::setId()
{
  theId = getNextId();
  return getId();
}

bool rspfAnnotationObject::saveState(rspfKeywordlist& kwl,
                                      const char* prefix) const
{
   kwl.add(prefix, "red",       theRed);
   kwl.add(prefix, "green",     theGreen);
   kwl.add(prefix, "blue",      theBlue);
   kwl.add(prefix, "thickness", theThickness);
   
   return rspfObject::saveState(kwl, prefix);
}

bool rspfAnnotationObject::loadState(const rspfKeywordlist& kwl,
                                      const char* prefix)
{
   const char* red       = kwl.find(prefix, "red");
   const char* green     = kwl.find(prefix, "green");
   const char* blue      = kwl.find(prefix, "blue");
   const char* thickness = kwl.find(prefix, "thickness");

   if(red)
   {
      theRed = (rspf_uint8)rspfString(red).toLong();
   }
   if(green)
   {
      theGreen = (rspf_uint8)rspfString(green).toLong();
   }
   if(blue)
   {
      theBlue = (rspf_uint8)rspfString(blue).toLong();
   }
   if(thickness)
   {
      theThickness = (rspf_uint8)rspfString(thickness).toLong();
   }
   
   return rspfObject::loadState(kwl, prefix);
}

bool rspfAnnotationObject::isPointWithin(const rspfDpt& /* imagePoint */) const
{
   return false;
}

rspfString rspfAnnotationObject::getName()const
{
   return theName;
}

void rspfAnnotationObject::setName(const rspfString& name)
{
   theName = name;
}

rspfDrect rspfAnnotationObject::getBoundingRect()const
{
   rspfDrect rect;
   getBoundingRect(rect);
   return rect;
}

void rspfAnnotationObject::applyScale(const rspfDpt& scale)
{
   applyScale(scale.x,
              scale.y);
}

std::ostream& rspfAnnotationObject::print(ostream& out) const
{
   
   out << setw(15)<<setiosflags(ios::left)<<"Red:" << (long)theRed   << endl
       <<  setw(15)<<setiosflags(ios::left)<<"Green:" << (long)theGreen << endl
       <<  setw(15)<<setiosflags(ios::left)<<"Blue" << (long)theBlue << endl
       <<  setw(15)<<setiosflags(ios::left)<<"Thickness:"<<theThickness
       << "color_red:   " << (long)theRed       << endl
       << "color_green: " << (long)theGreen     << endl
       << "color_blue:  " << (long)theBlue      << endl
       << "thickness:   " << theThickness << endl;
   return out;
}
