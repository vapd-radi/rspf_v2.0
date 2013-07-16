//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfGeoPolyCutter.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfGeoPolyCutter.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <algorithm>

static const char* NUMBER_POLYGONS_KW = "number_polygons";

RTTI_DEF2(rspfGeoPolyCutter, "rspfGeoPolyCutter", rspfPolyCutter, rspfViewInterface)

rspfGeoPolyCutter::rspfGeoPolyCutter()
   : rspfPolyCutter(),
     rspfViewInterface(NULL)
{
   rspfViewInterface::theObject = this;
   theGeoPolygonList.push_back(rspfGeoPolygon());
}

rspfGeoPolyCutter::~rspfGeoPolyCutter()
{
}

bool rspfGeoPolyCutter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
    rspfString newPrefix = prefix;

    for(int i = 0; i < (int)thePolygonList.size();++i)
    {
       newPrefix = rspfString(prefix) + "geo_polygon" + rspfString::toString(i)+".";

       theGeoPolygonList[i].saveState(kwl, newPrefix.c_str());
    }
    kwl.add(prefix,
            NUMBER_POLYGONS_KW,
            static_cast<rspf_uint32>(theGeoPolygonList.size()),
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

    if(theViewProjection.valid())
    {
       rspfString viewPrefix = prefix;
       viewPrefix += "view.";

       theViewProjection->saveState(kwl, viewPrefix.c_str());
    }
  
   return rspfImageSourceFilter::saveState(kwl, prefix);;
}

bool rspfGeoPolyCutter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   rspfString copyPrefix(prefix);
   rspfString polygons =  rspfString("^(") + copyPrefix + "geo_polygon[0-9]+.)";
   vector<rspfString> keys =
      kwl.getSubstringKeyList( polygons );
   int offset = (int)(copyPrefix+"geo_polygon").size();
   
   std::vector<int> numberList(keys.size());
   for(int idx = 0; idx < (int)numberList.size();++idx)
   {
      rspfString numberStr(keys[idx].begin() + offset,
                            keys[idx].end());
      numberList[idx] = numberStr.toInt();
   }
   std::sort(numberList.begin(), numberList.end());
   
   rspfString newPrefix;
   thePolygonList.clear();
   for(int i = 0; i < (int)numberList.size();++i)
   {
      theGeoPolygonList.push_back(rspfGeoPolygon());
      newPrefix = copyPrefix+"geo_polygon"+rspfString::toString(numberList[i])+".";
      theGeoPolygonList[i].loadState(kwl, newPrefix.c_str());
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

   rspfString viewPrefix = prefix;
   viewPrefix += "view.";
   theViewProjection = new rspfImageGeometry();
   if(theViewProjection->loadState(kwl,
                                viewPrefix))
   {
      transformVertices();
   }
   return rspfImageSourceFilter::loadState(kwl, prefix);
}


void rspfGeoPolyCutter::setPolygon(const vector<rspfDpt>& polygon,
                                 rspf_uint32 index)
{
   if(theViewProjection.valid())
   {
      rspfPolyCutter::setPolygon(polygon);
      invertPolygon(index);
   }
}

void rspfGeoPolyCutter::setPolygon(const vector<rspfIpt>& polygon,
                                 rspf_uint32 index)
{
   if(theViewProjection.valid())
   {
      rspfPolyCutter::setPolygon(polygon);
      invertPolygon(index);
   }
}

void rspfGeoPolyCutter::setPolygon(const vector<rspfGpt>& polygon,
                                    rspf_uint32 i)
{
   if(i < theGeoPolygonList.size())
   {
      theGeoPolygonList[i] = polygon;

      transformVertices(i);
   }
}

void rspfGeoPolyCutter::setPolygon(const rspfGeoPolygon& polygon,
                                    rspf_uint32 i)
{
   if(i < theGeoPolygonList.size())
   {
      theGeoPolygonList[i] = polygon.getVertexList();

      transformVertices(i);
   }
}


void rspfGeoPolyCutter::addPolygon(const vector<rspfGpt>& polygon)
{
   theGeoPolygonList.push_back(polygon);
   thePolygonList.push_back(rspfPolygon());

   if(theViewProjection.valid())
   {
      transformVertices(((int)theGeoPolygonList.size())-1);
   }
}

void rspfGeoPolyCutter::addPolygon(const vector<rspfIpt>& polygon)
{
   if(theViewProjection.valid())
   {
      rspfPolyCutter::addPolygon(polygon);
      theGeoPolygonList.push_back(rspfGeoPolygon());
      invertPolygon((int)thePolygonList.size()-1);
   }
}

void rspfGeoPolyCutter::addPolygon(const vector<rspfDpt>& polygon)
{
   if(theViewProjection.valid())
   {
      rspfPolyCutter::addPolygon(polygon);
      theGeoPolygonList.push_back(rspfGeoPolygon());
      invertPolygon((int)thePolygonList.size()-1);
   }
}

void rspfGeoPolyCutter::addPolygon(const rspfPolygon& polygon)
{
   if(theViewProjection.valid())
   {
      rspfPolyCutter::addPolygon(polygon);
      theGeoPolygonList.push_back(rspfGeoPolygon());
      invertPolygon((int)thePolygonList.size()-1);
   }
}

void rspfGeoPolyCutter::addPolygon(const rspfGeoPolygon& polygon)
{
   rspfPolyCutter::addPolygon(rspfPolygon());
   theGeoPolygonList.push_back(polygon);
   
   if(theViewProjection.valid())
   {
      transformVertices(((int)theGeoPolygonList.size())-1);
   }
}

void rspfGeoPolyCutter::invertPolygon(int polygonNumber)
{
   if(!theViewProjection) return;
   
   
   rspfPolygon& poly      = thePolygonList[polygonNumber];
   rspfGeoPolygon& gpoly  = theGeoPolygonList[polygonNumber];
   
   gpoly.clear();
   gpoly.resize(poly.getVertexCount());
   int j = 0;
   for(j = 0; j < (int)poly.getVertexCount(); ++j)
   {
      theViewProjection->localToWorld(poly[j], gpoly[j]);
   }
}

bool rspfGeoPolyCutter::setView(rspfObject* baseObject)
{
   
   rspfProjection* proj = dynamic_cast<rspfProjection*>(baseObject);
   if(proj)
   {
      theViewProjection = new rspfImageGeometry(0, proj);
      transformVertices();
   }
   else 
   {
      theViewProjection = dynamic_cast<rspfImageGeometry*>(baseObject);
   }

   return theViewProjection.valid();
}

rspfObject* rspfGeoPolyCutter::getView()
{
   return theViewProjection.get();
}

const rspfObject* rspfGeoPolyCutter::getView()const
{
   return theViewProjection.get();
}

void rspfGeoPolyCutter::transformVertices()
{
   if(!theViewProjection) return;
   
   if(theGeoPolygonList.size())
   {
      if(theGeoPolygonList.size() != thePolygonList.size())
      {
         thePolygonList.resize(theGeoPolygonList.size());
      }
      int i = 0;
      int j = 0;
      for(i = 0; i < (int)theGeoPolygonList.size(); ++i)
      {
         rspfGeoPolygon& gpoly = theGeoPolygonList[i];
         rspfPolygon&    poly  = thePolygonList[i];
         int nvert = (int)gpoly.size();
         if((int)poly.getVertexCount() != nvert)
         {
            poly.resize(nvert);
         }
         for(j = 0; j < nvert; ++j)
         {
//	   rspfDpt error;
            theViewProjection->worldToLocal(gpoly[j], poly[j]);
//	    theViewProjection->getRoundTripError(rspfIpt(poly[j]), error);
//	    poly[j] = poly[j] + error;
            poly[j] = rspfDpt(rspf::round<int>(poly[j].x),
                               rspf::round<int>(poly[j].y));
         }
      }
      computeBoundingRect();
   }
}

void rspfGeoPolyCutter::transformVertices(int i)
{
   if(!theViewProjection) return;
   
   rspfGeoPolygon& gpoly = theGeoPolygonList[i];
   rspfPolygon&    poly  = thePolygonList[i];
   int nvert = (int)gpoly.size();
   if((int)poly.getVertexCount() != nvert)
   {
      poly.resize(nvert);
   }
   int j = 0;
   for(j = 0; j < nvert; ++j)
   {
//     rspfDpt error;
     theViewProjection->worldToLocal(gpoly[j], poly[j]);
//     theViewProjection->getRoundTripError(poly[j], error);
//     poly[j] = poly[j] + error;
     poly[j] = rspfDpt(rspf::round<int>(poly[j].x),
			rspf::round<int>(poly[j].y));
   }
   
   computeBoundingRect();
}
