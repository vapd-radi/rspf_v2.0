//*****************************************************************************
// FILE: rspfPolygon.h
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Garrett Potts
//
//*****************************************************************************
//  $Id: rspfGeoPolygon.cpp 17815 2010-08-03 13:23:14Z dburken $

#include <ostream>
#include <sstream>
#include <algorithm>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>

static const char* NUMBER_VERTICES_KW = "number_vertices";

std::ostream& operator <<(std::ostream& out, const rspfGeoPolygon& poly)
{
   if(poly.size())
   {
      if(poly.size() >1)
      {
         for(rspf_uint32 i = 0; i <  poly.size()-1; ++i)
         {
            out << "P" << i << ": " << poly[i] << std::endl;
         }
         out << "P"  << (poly.size()-1)
             << ": " << poly[poly.size()-1] << std::endl;
      }
      else
      {
         out << "P0: " << poly[0] << std::endl;
      }
   }

   return out;
}

bool rspfGeoPolygon::hasNans()const
{
   int upper = (int)theVertexList.size();
   int i = 0;

   for(i = 0; i < upper; ++i)
   {
      if(theVertexList[i].hasNans())
      {
         return true;
      }
   }

   return false;
}

bool rspfGeoPolygon::vertex(int index, rspfGpt& v) const 
{
   if((index >= (int)theVertexList.size()) ||
      (index < 0))
   {
      return false;
   }

   v = theVertexList[index];
   theCurrentVertex = index;

   return true;
}

bool rspfGeoPolygon::nextVertex(rspfDpt& v) const 
{
   ++theCurrentVertex;
   if(theCurrentVertex >= (rspf_int32)theVertexList.size())
   {
      return false;
   }
   v = theVertexList[theCurrentVertex];
   
   return true;
}

void rspfGeoPolygon::stretchOut(rspfGeoPolygon& newPolygon,
                                 double displacement)
{
   newPolygon.resize(size());
   if(size() >= 3)
   {
      const rspfDatum* datum = theVertexList[0].datum();
      checkOrdering();
      double signMult = 1.0;
      if(theOrderingType == RSPF_COUNTERCLOCKWISE_ORDER)
      {
         signMult = -1.0;
      }
      
      rspfDpt prev, current, next;

      rspf_uint32 prevI;
      rspf_uint32 currentI;
      rspf_uint32 nextI;
      rspf_uint32 i = 0;
      rspf_uint32 upper = size();

      bool equalEndsFlag = false;
      if(theVertexList[0] == theVertexList[theVertexList.size()-1])
      {
         equalEndsFlag = true;
         prevI    = 0;
         currentI = 1;
         nextI    = 2;
         i = 1;
         --upper;
      }
      else
      {
         equalEndsFlag = false;
         prevI    = size()-1;
         currentI = 0;
         nextI    = 1;
      }
      for(; i < upper;++i)
      {
         prev    = theVertexList[prevI];
         current = theVertexList[currentI];
         next    = theVertexList[nextI];

         rspfDpt averageNormal;
         
         rspfDpt diffPrev = current - prev;
         rspfDpt diffNext = next - current;

         diffPrev = diffPrev*(1.0/diffPrev.length());
         diffNext = diffNext*(1.0/diffNext.length());

         rspfDpt diffPrevNormal(-diffPrev.y,
                                 diffPrev.x);
         rspfDpt diffNextNormal(-diffNext.y,
                                 diffNext.x);
         
         averageNormal     = (diffPrevNormal + diffNextNormal);
         averageNormal     = averageNormal*(signMult*(1.0/averageNormal.length()));
         rspfDpt newPoint = rspfDpt( theVertexList[i].lond(),
                                       theVertexList[i].latd()) +
                             averageNormal*displacement;
         newPolygon[i].latd(newPoint.lat);
         newPolygon[i].lond(newPoint.lon);
         newPolygon[i].height(theVertexList[i].height());
         newPolygon[i].datum(datum);
         
         ++prevI;
         ++currentI;
         ++nextI;

         prevI%=size();
         nextI%=size();
      }
      if(equalEndsFlag)
      {
         
         prev    = theVertexList[theVertexList.size()-2];
         current = theVertexList[0];
         next    = theVertexList[1];
         
         rspfDpt averageNormal;
         
         rspfDpt diffPrev = current - prev;
         rspfDpt diffNext = next - current;

         diffPrev = diffPrev*(1.0/diffPrev.length());
         diffNext = diffNext*(1.0/diffNext.length());

         rspfDpt diffPrevNormal(-diffPrev.y,
                                 diffPrev.x);
         rspfDpt diffNextNormal(-diffNext.y,
                                 diffNext.x);
         
         averageNormal     = (diffPrevNormal + diffNextNormal);
         averageNormal     = averageNormal*(signMult*(1.0/averageNormal.length()));
         rspfDpt newPoint = rspfDpt( theVertexList[i].lond(),
                                       theVertexList[i].latd()) +
                             averageNormal*displacement;
         newPolygon[0].latd(newPoint.lat);
         newPolygon[0].lond(newPoint.lon);
         newPolygon[0].height(theVertexList[i].height());
         newPolygon[0].datum(datum);
         
         newPolygon[(int)theVertexList.size()-1] = newPolygon[0];
      }
   }
}


double rspfGeoPolygon::area()const
{
   double area = 0;
   rspf_uint32 i=0;
   rspf_uint32 j=0;
   rspf_uint32 size = (rspf_uint32)theVertexList.size();
   
   for (i=0;i<size;i++)
   {
      j = (i + 1) % size;
      area += theVertexList[i].lon * theVertexList[j].lat;
      area -= theVertexList[i].lat * theVertexList[j].lon;
   }

   area /= 2;

   return area;
}

void rspfGeoPolygon::reverseOrder()
{
   std::reverse(theVertexList.begin(), theVertexList.end());
   
   if(theOrderingType == RSPF_COUNTERCLOCKWISE_ORDER)
   {
      theOrderingType = RSPF_CLOCKWISE_ORDER;
   }
   else if(theOrderingType == RSPF_CLOCKWISE_ORDER)
   {
      theOrderingType =  RSPF_COUNTERCLOCKWISE_ORDER;
   }
   
}

void rspfGeoPolygon::checkOrdering()const
{
   if(theOrderingType == RSPF_VERTEX_ORDER_UNKNOWN)
   {
      double areaValue = area();
      if(areaValue > 0)
      {
         theOrderingType = RSPF_COUNTERCLOCKWISE_ORDER;
      }
      else if(areaValue <= 0)
      {
         theOrderingType = RSPF_CLOCKWISE_ORDER;
      }
   }
}

rspfGpt rspfGeoPolygon::computeCentroid()const
{
   if(!size())
   {
      return rspfGpt();
   }
   rspfDpt average(0.0,0.0);
   double height=0.0;
   for(rspf_uint32 i = 0; i < size(); ++i)
   {
      average += rspfDpt(theVertexList[i]);
      height  += theVertexList[i].height();
   }

   
   average.x /= size();
   average.y /= size();
   height    /= size();

   return rspfGpt(average.y, average.x, height, theVertexList[0].datum());
}

bool rspfGeoPolygon::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   int i = 0;

   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfGeoPolygon",
           true);
   kwl.add(prefix,
           NUMBER_VERTICES_KW,
           static_cast<rspf_uint32>(theVertexList.size()),
           true);
   if(theVertexList.size())
   {
      kwl.add(prefix,
              rspfKeywordNames::DATUM_KW,
              theVertexList[0].datum()->code(),
              true);
   }
   else
   {
      kwl.add(prefix,
              rspfKeywordNames::DATUM_KW,
              "WGE",
              true);
   }
   
   for(i = 0; i < (int)theVertexList.size();++i)
   {
      rspfString vert = "v"+rspfString::toString(i);
      rspfString value = (rspfString::toString(theVertexList[i].latd()) + " " +
                           rspfString::toString(theVertexList[i].lond())  + " " +
                           ( theVertexList[i].isHgtNan()?rspfString("nan"):rspfString::toString(theVertexList[i].height())));
      kwl.add(prefix,
              vert.c_str(),
              value.c_str(),
              true);
   }

   return true;
}

bool rspfGeoPolygon::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   const char* number_vertices = kwl.find(prefix, NUMBER_VERTICES_KW);
   rspfString datumStr = kwl.find(prefix, rspfKeywordNames::DATUM_KW);
   const rspfDatum* datum = rspfDatumFactoryRegistry::instance()->create(datumStr);
   
   theVertexList.clear();
   int i = 0;
   int vertexCount = rspfString(number_vertices).toLong();
   rspfString lat, lon, height;
   for(i = 0; i < vertexCount; ++i)
   {
      rspfString v = kwl.find(prefix, (rspfString("v")+rspfString::toString(i)).c_str());
      rspfString latString, lonString, heightString;
      v = v.trim();
      std::istringstream in(v);
      in>>lat>>lon>>height;
      theVertexList.push_back(rspfGpt(lat.toDouble(), lon.toDouble(), height.toDouble(), datum));
   }

   return true;
}
