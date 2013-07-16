//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Garrett Potts (gpotts@imagelinks.com)
//
//*****************************************************************************
//  $Id: rspfPolyLine.cpp 15927 2009-11-16 17:30:08Z dburken $
//
#include <rspf/base/rspfPolyLine.h>
#include <rspf/base/rspfCommon.h>
#include <algorithm>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfLine.h>
#include <rspf/base/rspfPolygon.h>
#include <sstream>
#include <iterator>
using namespace std;

static const char* NUMBER_VERTICES_KW = "number_vertices";

rspfPolyLine::rspfPolyLine(const vector<rspfIpt>& polyLine)
   :theVertexList(polyLine.size()),
    theAttributeList(),
    theCurrentVertex(0)
{
   // Assign std::vector<rspfIpt> list to std::vector<rspfDpt> theVertexList.
   for (std::vector<rspfIpt>::size_type i = 0; i < polyLine.size(); ++i)
   {
      theVertexList[i] = polyLine[i];
   }
}

rspfPolyLine::rspfPolyLine(const vector<rspfDpt>& polyLine)
   :theCurrentVertex(0)
{
   theVertexList = polyLine;
}

//*****************************************************************************
//  CONSTRUCTOR: rspfPolyLine(int numVertices, const rspfDpt* vertex_array)
//  
//*****************************************************************************
rspfPolyLine::rspfPolyLine(int numVertices, const rspfDpt* v)
   : theCurrentVertex(0)
{
   theVertexList.insert(theVertexList.begin(),
                        v, v+numVertices);
}

//*****************************************************************************
//  COPY CONSTRUCTOR: rspfPolyLine(rspfPolyLine)
//  
//*****************************************************************************
rspfPolyLine::rspfPolyLine(const rspfPolyLine& polyLine)
   :theCurrentVertex(0)
{
   *this = polyLine;
}

rspfPolyLine::rspfPolyLine(rspfDpt v1,
                             rspfDpt v2,
                             rspfDpt v3,
                             rspfDpt v4)
   : theVertexList(4),
     theCurrentVertex(0)
{
   theVertexList[0] = v1;
   theVertexList[1] = v2;
   theVertexList[2] = v3;
   theVertexList[3] = v4;
}

rspfPolyLine::rspfPolyLine(const rspfIrect& rect)
   : theVertexList(4),
     theCurrentVertex(0)
{
   theVertexList[0] = rect.ul();
   theVertexList[1] = rect.ur();
   theVertexList[2] = rect.lr();
   theVertexList[3] = rect.ll();
}

rspfPolyLine::rspfPolyLine(const rspfDrect& rect)
   : theVertexList(4),
     theCurrentVertex(0)
{
   theVertexList[0] = rect.ul();
   theVertexList[1] = rect.ur();
   theVertexList[2] = rect.lr();
   theVertexList[3] = rect.ll();
}

rspfPolyLine::rspfPolyLine(const rspfPolygon& polygon)
   :theVertexList(polygon.getNumberOfVertices()+1),
    theCurrentVertex(0)
{
   rspf_uint32 n = polygon.getNumberOfVertices();
   
   if(n)
   {
      for(rspf_uint32 i = 0; i < n; ++i)
      {
         theVertexList[i] = polygon[i];
      }

      theVertexList[n] = polygon[n-1];
   }
   else
   {
      theVertexList.clear();
   }
}

//*****************************************************************************
//  DESTRUCTOR: ~rspfPolyLine
//  
//*****************************************************************************
rspfPolyLine::~rspfPolyLine()
{
}

void rspfPolyLine::roundToIntegerBounds(bool compress)
{
  int i = 0;
  for(i = 0; i < (int)theVertexList.size(); ++i)
    {
      theVertexList[i] = rspfIpt(theVertexList[i]);
    }
  if(compress&&theVertexList.size())
    {
      vector<rspfDpt> polyLine;
      
      polyLine.push_back(theVertexList[0]);
      rspfDpt testPt = theVertexList[0];
      for(i=1; i < (int)theVertexList.size(); ++i)
	{
	  if(testPt!=theVertexList[i])
	    {
	      testPt = theVertexList[i];
	      polyLine.push_back(testPt);
	    }
	}
      theVertexList    = polyLine;
      theCurrentVertex = 0;
    }
}

bool rspfPolyLine::hasNans()const
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

void rspfPolyLine::getIntegerBounds(rspf_int32& minX,
                                     rspf_int32& minY,
                                     rspf_int32& maxX,
                                     rspf_int32& maxY)const
{
   rspf_int32 npoly = (rspf_int32)theVertexList.size();
   int i = 0;
   
   if(npoly)
   {
      minX = (rspf_int32)floor(theVertexList[0].x);
      maxX = (rspf_int32)ceil(theVertexList[0].x);
      minY = (rspf_int32)floor(theVertexList[0].y);
      maxY = (rspf_int32)ceil(theVertexList[0].y);
      
      for(i =1; i < npoly; ++i)
      {
         minX = std::min((rspf_int32)floor(theVertexList[i].x),
                         (rspf_int32)minX);
         maxX = std::max((rspf_int32)ceil(theVertexList[i].x),
                         (rspf_int32)maxX);
         minY = std::min((rspf_int32)floor(theVertexList[i].y),
                         (rspf_int32)minY);
         maxY = std::max((rspf_int32)ceil(theVertexList[i].y),
                         (rspf_int32)maxY);
      }
   }
   else
   {
      minX = RSPF_INT_NAN;
      minY = RSPF_INT_NAN;
      maxX = RSPF_INT_NAN;
      maxY = RSPF_INT_NAN;
   }
}

void rspfPolyLine::getBounds(double& minX,
                              double& minY,
                              double& maxX,
                              double& maxY)const
{
   rspf_int32 npoly = (rspf_int32)theVertexList.size();
   
   if(npoly)
   {
      int i = 0;
      minX = theVertexList[0].x;
      maxX = theVertexList[0].x;
      minY = theVertexList[0].y;
      maxY = theVertexList[0].y;
      
      for(i =1; i < npoly; ++i)
      {
         minX = std::min(theVertexList[i].x, minX);
         maxX = std::max(theVertexList[i].x, maxX);
         minY = std::min(theVertexList[i].y, minY);
         maxY = std::max(theVertexList[i].y, maxY);
      }
   }
   else
   {
      minX = rspf::nan();
      minY = rspf::nan();
      maxX = rspf::nan();
      maxY = rspf::nan();
   }
}

bool rspfPolyLine::isWithin(const rspfDrect& rect)const
{
   if(theVertexList.size() == 1)
   {
      return rect.pointWithin(theVertexList[0]);
   }
   else if(theVertexList.size() > 1)
   {
      for(rspf_uint32 i = 0; i < (theVertexList.size() - 1); ++i)
      {
         rspfDpt p1 = theVertexList[i];
         rspfDpt p2 = theVertexList[i+1];
         
         if(rect.clip(p1, p2))
         {
            return true;
         }
      }
   }

   return false;
}


bool rspfPolyLine::clipToRect(vector<rspfPolyLine>& result,
                              const rspfDrect& rect)const
{
   result.clear();
   
   if(theVertexList.size() <1) return false;

   rspfPolyLine currentPoly;

   if(theVertexList.size() == 1)
   {
      rect.pointWithin(theVertexList[0]);
      currentPoly.addPoint(theVertexList[0]);
      result.push_back(currentPoly);
   }
   else
   {
      rspfDpt pt1 = theVertexList[0];
      rspfDpt pt2 = theVertexList[1];
      rspf_uint32 i = 1;
      
      while(i < theVertexList.size())
      {
         
         bool p1Inside = rect.pointWithin(pt1);
         bool p2Inside = rect.pointWithin(pt2);

         if(p1Inside&&p2Inside) // both inside so save the first
         {
            currentPoly.addPoint(pt1);
            pt1 = pt2;            
         }
         // going from inside to outside
         else if(p1Inside&&
                 !p2Inside)
         {
            currentPoly.addPoint(pt1);
            rspfDpt save = pt2;
            if(rect.clip(pt1, pt2))
            {
               currentPoly.addPoint(pt2);
               result.push_back(currentPoly);

               currentPoly.clear();
            }
            pt2  = save;
            pt1  = save;
         }// going outside to the inside
         else if(!p1Inside&&
                 p2Inside)
         {            
            if(rect.clip(pt1, pt2))
            {
               currentPoly.addPoint(pt1);
            }
            pt1 = pt2;
           
         }
         else // both outside must do a clip to see if crosses rect
         {
            rspfDpt p1 = pt1;
            rspfDpt p2 = pt2;

            if(rect.clip(p1, p2))
            {
               currentPoly.addPoint(p1);
               currentPoly.addPoint(p2);
            }
            pt1 = pt2;
         }
         ++i;
         
         if(i < theVertexList.size())
         {
            pt2 = theVertexList[i];
         }            
      }
      if(rect.pointWithin(pt2))
      {
         currentPoly.addPoint(pt2);
      }
   }

   if(currentPoly.getNumberOfVertices() > 0)
   {
      result.push_back(currentPoly);
   }
   
   return (result.size()>0);
}   

bool rspfPolyLine::isPointWithin(const rspfDpt& point) const
{
   if(theVertexList.size() == 1)
   {
      return (point == theVertexList[0]);
   }
   else
   {
      for(rspf_uint32 i = 1; i < theVertexList.size(); ++i)
      {
         if(rspfLine(theVertexList[i-1],
                      theVertexList[i]).isPointWithin(point))
         {
            return true;
         }
      }
   }
   
   return false;
}

bool rspfPolyLine::vertex(int index, rspfDpt& tbd_vertex) const 
{
   if((index >= (int)theVertexList.size()) ||
      (index < 0))
   {
      return false;
   }

   tbd_vertex = theVertexList[index];
   theCurrentVertex = index;

   return true;
}

bool rspfPolyLine::nextVertex(rspfDpt& tbd_vertex) const 
{
   ++theCurrentVertex;
   if(theCurrentVertex >= (rspf_int32)theVertexList.size())
   {
      return false;
   }
   tbd_vertex = theVertexList[theCurrentVertex];
   
   return true;
}


const rspfPolyLine&  rspfPolyLine::operator=(const rspfPolygon& polygon)
{
   theCurrentVertex = 0;
   rspf_uint32 n = polygon.getNumberOfVertices();
   
   if(n)
   {
      theVertexList.resize(n+1);
      
      for(rspf_uint32 i = 0; i < n; ++i)
      {
         theVertexList[i] = polygon[i];
      }
      
      theVertexList[n] = polygon[n-1];
   }
   else
   {
      theVertexList.clear();
   }

   return *this;
}

const rspfPolyLine&  rspfPolyLine::operator=(const rspfPolyLine& polyLine)
{
   theVertexList    = polyLine.theVertexList;
   theCurrentVertex = polyLine.theCurrentVertex;
   theAttributeList = polyLine.theAttributeList;
   
   return *this;
}

const rspfPolyLine& rspfPolyLine::operator= (const vector<rspfDpt>& vertexList)
{
   theVertexList    = vertexList;
   theCurrentVertex = 0;
   
   return *this;
}

const rspfPolyLine& rspfPolyLine::operator= (const vector<rspfIpt>& vertexList)
{
   theVertexList.resize(vertexList.size());

   // Assign std::vector<rspfIpt> list to std::vector<rspfDpt> theVertexList.
   for (std::vector<rspfIpt>::size_type i = 0; i < vertexList.size(); ++i)
   {
      theVertexList[i] = vertexList[i];
   }
   
   theCurrentVertex = 0;
   
   return *this;
}

//*****************************************************************************
//  METHOD: operator==()
//  
//*****************************************************************************
bool rspfPolyLine::operator==(const rspfPolyLine& polyLine) const
{
   if( (theVertexList.size() != polyLine.theVertexList.size()))
   {
      return false;
   }
   if(!theVertexList.size() && polyLine.theVertexList.size())
   {
      return true;
   }

   return (theVertexList == polyLine.theVertexList);
}

const rspfPolyLine& rspfPolyLine::operator *=(const rspfDpt& scale)
{
   rspf_uint32 upper = (rspf_uint32)theVertexList.size();
   rspf_uint32 i = 0;
   
   for(i = 0; i < upper; ++i)
   {
      theVertexList[i].x*=scale.x;
      theVertexList[i].y*=scale.y;
   }
   
   return *this;
}

rspfPolyLine rspfPolyLine::operator *(const rspfDpt& scale)const
{
   rspfPolyLine result(*this);

   rspf_uint32 i = 0;
   rspf_uint32 upper = (rspf_uint32)theVertexList.size();
   for(i = 0; i < upper; ++i)
   {
      result.theVertexList[i].x*=scale.x;
      result.theVertexList[i].y*=scale.y;
   }

   return result;
}


void rspfPolyLine::reverseOrder()
{
   std::reverse(theVertexList.begin(), theVertexList.end());   
}

//*****************************************************************************
//  METHOD: rspfPolyLine::print(ostream)
//  
//*****************************************************************************
void rspfPolyLine::print(ostream& os) const
{
   copy(theVertexList.begin(),
        theVertexList.end(),
        ostream_iterator<rspfDpt>(os, "\n"));
}

bool rspfPolyLine::saveState(rspfKeywordlist& kwl,
                             const char* prefix)const
{
   int i = 0;
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfPolyLine",
           true);
   kwl.add(prefix,
           NUMBER_VERTICES_KW,
           (int)theVertexList.size(),
           true);
   for(i = 0; i < (int)theVertexList.size();++i)
   {
      rspfString vert = "v"+rspfString::toString(i);;
      rspfString value = (rspfString::toString(theVertexList[i].x) + " " +
                           rspfString::toString(theVertexList[i].y) );
      kwl.add(prefix,
              vert.c_str(),
              value.c_str(),
              true);
   }
   rspfString order = "";
   
   return true;
}
   
bool rspfPolyLine::loadState(const rspfKeywordlist& kwl,
                             const char* prefix)
{
   const char* number_vertices = kwl.find(prefix, NUMBER_VERTICES_KW);
   int i = 0;

   theVertexList.clear();
   int vertexCount = rspfString(number_vertices).toLong();
   rspfString x = "0.0";
   rspfString y = "0.0";
   for(i = 0; i < vertexCount; ++i)
   {
      rspfString v = kwl.find(prefix, (rspfString("v")+rspfString::toString(i)).c_str());
      v = v.trim();

      istringstream vStream(v);
      vStream >> x >> y;
      theVertexList.push_back(rspfDpt(x.toDouble(),y.toDouble()));
   }

   return true;
}
