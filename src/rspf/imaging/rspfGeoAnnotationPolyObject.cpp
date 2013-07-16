//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationPolyObject.cpp 19734 2011-06-06 23:45:36Z dburken $

#include <sstream>

#include <rspf/imaging/rspfGeoAnnotationPolyObject.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfTrace.h>

static const rspfTrace
traceDebug(rspfString("rspfGeoAnnotationPolyObject:debug"));

RTTI_DEF1(rspfGeoAnnotationPolyObject,
          "rspfGeoAnnotationPolyObject",
          rspfGeoAnnotationObject)

rspfGeoAnnotationPolyObject::rspfGeoAnnotationPolyObject(bool enableFill,
                                                           rspf_uint8 r,
                                                           rspf_uint8 g,
                                                           rspf_uint8 b,
                                                           rspf_uint8 thickness)
   :rspfGeoAnnotationObject(r, g, b, thickness),
    thePolygon(),
    theProjectedPolyObject(0),
    m_PolyType(RSPF_POLY_EXTERIOR_RING)
{
   theProjectedPolyObject = new rspfAnnotationPolyObject(enableFill,
                                                          r,
                                                          g,
                                                          b,
                                                          thickness);
}
   
rspfGeoAnnotationPolyObject::rspfGeoAnnotationPolyObject(
   const std::vector<rspfGpt>& groundPts,
   bool enableFill,
   rspf_uint8 r,
   rspf_uint8 g,
   rspf_uint8 b,
   rspf_uint8 thickness)
   :rspfGeoAnnotationObject(r, g, b, thickness),
    thePolygon(),
    theProjectedPolyObject(0),
    m_PolyType(RSPF_POLY_EXTERIOR_RING)
{
   
   thePolygon = groundPts;
   
   // we will initialize the projected polygon's size
   // so we don't have to reset it every time we do a
   // projection
   //
   std::vector<rspfDpt> projectedPoints(thePolygon.size());
   
   theProjectedPolyObject = new rspfAnnotationPolyObject(projectedPoints,
                                                          enableFill,
                                                          r,
                                                          g,
                                                          b,
                                                          thickness);
}

rspfGeoAnnotationPolyObject::rspfGeoAnnotationPolyObject(
   const rspfGeoAnnotationPolyObject& rhs)
   :rspfGeoAnnotationObject(rhs),
    thePolygon(rhs.thePolygon),
    theProjectedPolyObject(rhs.theProjectedPolyObject.valid()?(rspfAnnotationPolyObject*)rhs.theProjectedPolyObject->dup():(rspfAnnotationPolyObject*)0),
    m_PolyType(rhs.m_PolyType)
{
}

rspfGeoAnnotationPolyObject::~rspfGeoAnnotationPolyObject()
{
   theProjectedPolyObject = 0;
}

rspfObject* rspfGeoAnnotationPolyObject::dup()const
{
   return new rspfGeoAnnotationPolyObject(*this);
}

void rspfGeoAnnotationPolyObject::applyScale(double x,
                                              double y)
{
   for(int i = 0; i < (int)thePolygon.size(); ++i)
   {
      thePolygon[i].lond(thePolygon[i].lond()*x);
      thePolygon[i].latd(thePolygon[i].latd()*y);
   }
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->applyScale(x, y);
   }
      
}

void rspfGeoAnnotationPolyObject::transform(rspfImageGeometry* projection)
{
   // make sure it's not null
   if(!projection)
   {
      return;
   }
   rspfPolygon& poly = theProjectedPolyObject->getPolygon();
   const std::vector<rspfGpt>::size_type BOUNDS = thePolygon.size();
   
   for(std::vector<rspfGpt>::size_type index=0; index < BOUNDS; ++index)
   {
      projection->worldToLocal(thePolygon[(int)index], poly[(int)index]);
   }

   // update the bounding rect
   //
   theProjectedPolyObject->computeBoundingRect();
}

std::ostream& rspfGeoAnnotationPolyObject::print(std::ostream& out)const
{
   out << "number_of_points:  " << thePolygon.size() << std::endl;
   if(thePolygon.size() > 0)
   {
      for(long index =0; index < (long)(thePolygon.size()-1); ++index)
      {
         out << thePolygon[index] << std::endl;
      }
      out << thePolygon[thePolygon.size()-1] << std::endl;
   }
   out << "Projected Polygon" << std::endl;
   theProjectedPolyObject->print(out);
   return out;
}

void rspfGeoAnnotationPolyObject::draw(rspfRgbImage& anImage)const
{
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->draw(anImage);
   }
}

rspfAnnotationObject* rspfGeoAnnotationPolyObject::getNewClippedObject(
   const rspfDrect& rect)const
{
   if(theProjectedPolyObject.valid())
   {
      return theProjectedPolyObject->getNewClippedObject(rect);
   }
   
   return (rspfAnnotationObject*)0;
}

bool rspfGeoAnnotationPolyObject::intersects(const rspfDrect& rect)const
{
   if(theProjectedPolyObject.valid())
   {
      return theProjectedPolyObject->intersects(rect);
   }
   
   return false;
}

void rspfGeoAnnotationPolyObject::getBoundingRect(rspfDrect& rect)const
{
   rect.makeNan();
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->getBoundingRect(rect);
   }
}

void rspfGeoAnnotationPolyObject::computeBoundingRect()
{
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->computeBoundingRect();
   }
}

const std::vector<rspfGpt>& rspfGeoAnnotationPolyObject::getPolygon()const
{
   return thePolygon;
}

void rspfGeoAnnotationPolyObject::setPolygon(const std::vector<rspfGpt>& poly)
{
   thePolygon = poly;
   std::vector<rspfDpt> projectedPoints(thePolygon.size());
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->setPolygon(projectedPoints);
   }
   
}


void rspfGeoAnnotationPolyObject::setFillFlag(bool flag)
{
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->setFillFlag(flag);
   }
}

void rspfGeoAnnotationPolyObject::setColor(rspf_uint8 r,
                                            rspf_uint8 g,
                                            rspf_uint8 b)
{
   rspfAnnotationObject::setColor(r, g, b);
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->setColor(r, g, b);
   }
}

void rspfGeoAnnotationPolyObject::setThickness(rspf_uint8 thickness)
{
   rspfAnnotationObject::setThickness(thickness);
   if(theProjectedPolyObject.valid())
   {
      theProjectedPolyObject->setThickness(thickness);
   }
}

bool rspfGeoAnnotationPolyObject::saveState(rspfKeywordlist& kwl,
                                             const char* prefix) const
{
   rspf_uint32 vIdx = 0;
   
   for(rspf_uint32 i = 0; i < thePolygon.size(); ++i)
   {
      rspfString key = "v";
      key += rspfString::toString(vIdx);

      kwl.add(prefix, key.c_str(), thePolygon[i].toString().c_str());

      ++vIdx;
   }
   
   return rspfGeoAnnotationObject::saveState(kwl, prefix);
}

bool rspfGeoAnnotationPolyObject::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   if (!theProjectedPolyObject)
   {
      return false;
   }
   
   //---
   // Base class state must be called first to pick up colors...
   //---
   bool status =  rspfGeoAnnotationObject::loadState(kwl, prefix);

   rspf_uint32 index = 0;
   rspfString copyPrefix = prefix;
   rspf_uint32 count = kwl.getNumberOfSubstringKeys(copyPrefix+"v[0-9]*");

   rspf_uint32 numberOfMatches = 0;
   const rspf_uint32 MAX_INDEX = count + 100;
   
   while(numberOfMatches < count)
   {
      rspfString key = "v";
      key += rspfString::toString(index);

      const char* lookup = kwl.find(prefix, key.c_str());
      if (lookup)
      {
         ++numberOfMatches;
         rspfGpt gpt;

         std::istringstream is(lookup);
         is >> gpt;
         thePolygon.push_back(gpt);
      }

      ++index;
      
      if (index > MAX_INDEX) // Avoid infinite loop...
      {
         break;
      }
   }

   // Set up the annotator.
   theProjectedPolyObject->setColor(theRed, theGreen, theBlue);
   theProjectedPolyObject->setThickness(theThickness);
   
   std::vector<rspfDpt> projectedPoints(thePolygon.size());
   theProjectedPolyObject->setPolygon(projectedPoints);
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGeoAnnotationPolyObject::loadState DEBUG:" << std::endl;
      for (rspf_uint32 i=0; i<thePolygon.size(); ++i)
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "thePolygon[" << i << "]:  " << thePolygon[i] << std::endl;
      }
   }
   
   return status;
}
