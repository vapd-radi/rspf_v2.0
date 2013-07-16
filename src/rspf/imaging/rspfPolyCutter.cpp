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
// $Id: rspfPolyCutter.cpp 21518 2012-08-22 21:15:56Z dburken $
#include <rspf/imaging/rspfPolyCutter.h>
#include <rspf/base/rspfPolyArea2d.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfActiveEdgeTable.h>
static const char* NUMBER_POLYGONS_KW = "number_polygons";

RTTI_DEF1(rspfPolyCutter, "rspfPolyCutter", rspfImageSourceFilter)

rspfPolyCutter::rspfPolyCutter()
   : rspfImageSourceFilter(),
     theTile(NULL),
     theCutType(RSPF_POLY_NULL_OUTSIDE),
     m_boundingOverwrite(false)
{
   thePolygonList.push_back(rspfPolygon());
   theBoundingRect.makeNan();
}

// rspfPolyCutter::rspfPolyCutter(rspfImageSource* inputSource,
//                                  rspfPolyArea2d* polygon)
rspfPolyCutter::rspfPolyCutter(rspfImageSource* inputSource,
                                 const rspfPolygon& polygon)
   : rspfImageSourceFilter(inputSource),
     theTile(NULL),
     theCutType(RSPF_POLY_NULL_INSIDE),
     m_boundingOverwrite(false)
{
   thePolygonList.push_back(polygon);
   computeBoundingRect();
   initialize();
}

rspfPolyCutter::~rspfPolyCutter()
{
}

rspfRefPtr<rspfImageData> rspfPolyCutter::getTile(const rspfIrect& tileRect,
                                                     rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      return theTile;
   }
   rspfRefPtr<rspfImageData> input = theInputConnection->getTile(tileRect,
                                                                   resLevel);
   
   if(!isSourceEnabled() || theBoundingRect.hasNans() || !input.valid())
   {
      return input;
   }
   
   if((input->getDataObjectStatus() == RSPF_EMPTY) ||
      (input->getDataObjectStatus() == RSPF_NULL))
   {
      return input;
   }

   if(!theTile.valid())
   {
      allocate(); // First time through...
   }

   if(!theTile.valid())
   {
      return input;
   }
   
   theTile->setImageRectangle(tileRect);
   
   theTile->loadTile(input.get());
   theTile->setDataObjectStatus(input->getDataObjectStatus());
   vector<rspfPolygon>* polyList = &thePolygonList;
   vector<rspfPolygon>  scaledPoly;
   
   rspfIrect boundingRect = getBoundingRect(resLevel);
   if(resLevel)
   {
      rspfDpt decimation;
      getDecimationFactor(resLevel, decimation);

      if(!decimation.hasNans())
      {
         for(int polyIndex = 0;
             polyIndex < (int)thePolygonList.size();
             ++polyIndex)
         {
            scaledPoly.push_back(thePolygonList[polyIndex]*decimation);
         }
         polyList = &scaledPoly;
      }
   }
   
   if(polyList->size()&&
      theTile->getDataObjectStatus()!=RSPF_NULL)
   {
//       rspfActiveEdgeTable aet;
      
      
      if(theCutType == RSPF_POLY_NULL_OUTSIDE)
      {
         if(boundingRect.intersects(tileRect))
         {
            theTile->makeBlank();
            theHelper.setImageData(theTile.get());
            for(int polyIndex = 0; polyIndex < (int)polyList->size(); ++polyIndex)
            {
               theHelper.copyInputToThis(input->getBuf(),
                                      (*polyList)[polyIndex]);
            }
            theTile->validate();
         }
         else
         {
            theTile->makeBlank();
         }
      }
      else if(theCutType == RSPF_POLY_NULL_INSIDE)
      {
         if(boundingRect.intersects(tileRect))
         {
            theHelper.setImageData(theTile.get());
            for(int polyIndex = 0;
                polyIndex < (int)polyList->size();
                ++polyIndex)
            {
               theHelper.fill(theTile->getNullPix(),
                           (*polyList)[polyIndex]);
            }
         }
         theTile->validate();
      }
      
   }
   return theTile;
}

rspfIrect rspfPolyCutter::getBoundingRect(rspf_uint32 resLevel)const
{
   rspfIrect result;

   result.makeNan();
   if(!theInputConnection)
   {
      return result;
   }
   if(!isSourceEnabled())
   {
      return rspfImageSourceFilter::getBoundingRect(resLevel);
   }
   
   result = theInputConnection->getBoundingRect(resLevel);

   if(isSourceEnabled()&&(!theBoundingRect.hasNans()))
   {
      if(theCutType == RSPF_POLY_NULL_OUTSIDE)
      {
         rspfDpt decimation;
         getDecimationFactor(resLevel, decimation);
         if(decimation.hasNans())
         {
            result =  theBoundingRect;
         }
         else
         {
            result = theBoundingRect*decimation;
         }
      }
   }
   else if(isSourceEnabled())
   {
      return theBoundingRect;
   }
   
   return result;
}

void rspfPolyCutter::initialize()
{
   // Capture the input connection.
   rspfImageSourceFilter::initialize();

   // Force an allocate on next getTile.
   theTile = NULL;
}

void rspfPolyCutter::allocate()
{
   theTile=NULL;
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this,
                                                          theInputConnection);
      theTile->initialize();

   }
}

bool rspfPolyCutter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   rspfString newPrefix = prefix;

   for(int i = 0; i < (int)thePolygonList.size();++i)
   {
      newPrefix = rspfString(prefix) + "polygon" + rspfString::toString(i)+".";

      thePolygonList[i].saveState(kwl, newPrefix.c_str());
   }
   kwl.add(prefix,
           NUMBER_POLYGONS_KW,
           (int)thePolygonList.size(),
           true);
   
   rspfString fillType = "null_inside";
   if(theCutType == RSPF_POLY_NULL_OUTSIDE)
   {
      fillType = "null_outside";
   }
   kwl.add(prefix,
           "cut_type",
           fillType.c_str(),
           true);   
  
   return rspfImageSourceFilter::saveState(kwl, prefix);;
}

bool rspfPolyCutter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   const char* numberPolygons = kwl.find(prefix, NUMBER_POLYGONS_KW);

   rspfString newPrefix;
   if(numberPolygons)
   {
      thePolygonList.clear();
      int npolys = rspfString(numberPolygons).toLong();
      for(int i = 0; i < npolys;++i)
      {
         thePolygonList.push_back(rspfPolygon());
         
         newPrefix = rspfString(prefix) + "polygon" + rspfString::toString(i)+".";
         
         thePolygonList[i].loadState(kwl, newPrefix.c_str());
      }
   }

   const char* lookup = kwl.find(prefix,
                                 "cut_type");
   if(lookup)
   {
      theCutType = RSPF_POLY_NULL_INSIDE;
      rspfString test = lookup;
      if(test == "null_outside")
      {
         theCutType = RSPF_POLY_NULL_OUTSIDE;
      }
   }
   else
   {
      theCutType = RSPF_POLY_NULL_OUTSIDE;
   }

   computeBoundingRect();

   
   return rspfImageSourceFilter::loadState(kwl, prefix);
}

void rspfPolyCutter::setPolygon(const vector<rspfDpt>& polygon,
                                 rspf_uint32 index)
{
   if(index < polygon.size())
   {
      thePolygonList[index] = polygon;

      computeBoundingRect();
   }

}

void rspfPolyCutter::setPolygon(const vector<rspfIpt>& polygon,
                                 rspf_uint32 index)
{
   if(polygon.size())
   {
      thePolygonList[index] = polygon;
      computeBoundingRect();
   }
}

void rspfPolyCutter::addPolygon(const vector<rspfIpt>& polygon)
{
   thePolygonList.push_back(rspfPolygon(polygon));
   computeBoundingRect();
}

void rspfPolyCutter::addPolygon(const vector<rspfDpt>& polygon)
{
   thePolygonList.push_back(rspfPolygon(polygon));
   computeBoundingRect();
}

void rspfPolyCutter::addPolygon(const rspfPolygon& polygon)
{
   thePolygonList.push_back(rspfPolygon(polygon));
   computeBoundingRect();
}


void rspfPolyCutter::setCutType(rspfPolyCutterCutType cutType)
{
   theCutType = cutType;
}

rspfPolyCutter::rspfPolyCutterCutType rspfPolyCutter::getCutType()const
{
   return theCutType;
}

void rspfPolyCutter::clear()
{
   setNumberOfPolygons(0);
}

void rspfPolyCutter::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfImageSourceFilter::setProperty(property);
}

rspfRefPtr<rspfProperty> rspfPolyCutter::getProperty(const rspfString& name)const
{

   return rspfImageSourceFilter::getProperty(name);
}

void rspfPolyCutter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSourceFilter::getPropertyNames(propertyNames);
} 

void rspfPolyCutter::computeBoundingRect()
{
   rspfIrect bounds;
   theBoundingRect.makeNan();

   for(int i = 0; i < (int)thePolygonList.size(); ++i)
   {
      thePolygonList[i].getBoundingRect(bounds);

      if(!bounds.hasNans())
      {
         if(theBoundingRect.hasNans())
         {
            theBoundingRect = bounds;
         }
         else
         {
            theBoundingRect = theBoundingRect.combine(bounds);
         }
      }
   }
}

const rspfIrect& rspfPolyCutter::getRectangle() const
{
   return theBoundingRect;
}

void rspfPolyCutter::setRectangle(const rspfIrect& rect)
{
   theBoundingRect = rect;
   m_boundingOverwrite = true;
}

void rspfPolyCutter::setNumberOfPolygons(rspf_uint32 count)
{
   if(!count)
   {
      thePolygonList.clear();
      thePolygonList.clear();
   }
   else
   {
      thePolygonList.resize(count);
   }
}

std::vector<rspfPolygon>& rspfPolyCutter::getPolygonList()
{
   return thePolygonList;
}

const std::vector<rspfPolygon>& rspfPolyCutter::getPolygonList()const
{
   return thePolygonList;
}

