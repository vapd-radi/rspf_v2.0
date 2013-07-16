//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Oscar Kramer (okramer@imagelinks.com)
//
// DESCRIPTION: Contains declaration of class rspfPolygon.
//   This class provides utilities associated with N-vertex, convex
//   (i.e., only two intersections for any line passing through the polygon).
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfPolyLine.h 15833 2009-10-29 01:41:53Z eshirschorn $

#ifndef rspfPolyLine_HEADER
#define rspfPolyLine_HEADER
#include <iostream>
#include <vector>
using namespace std;

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfString.h>
class rspfLine;
class rspfPolygon;

/*!****************************************************************************
 *
 * CLASS:  rspfPolyLine
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfPolyLine
{
public:
   rspfPolyLine()
      :theCurrentVertex(0)
      {}
   rspfPolyLine(const vector<rspfIpt>& polygon);
   rspfPolyLine(const vector<rspfDpt>& polygon);
   rspfPolyLine(int numVertices, const rspfDpt* vertex_array);
   
   rspfPolyLine(const rspfPolyLine& copy_this);
   rspfPolyLine(const rspfPolygon& polygon);
   
   /*!
    * CONSTRUCTOR: Provided for convenience. Does not imply the polygon is
    * limited to four vertices:
    */
   rspfPolyLine(rspfDpt v1,
                rspfDpt v2,
                rspfDpt v3,
                rspfDpt v4);
   
   rspfPolyLine(const rspfIrect& rect);
   rspfPolyLine(const rspfDrect& rect);

   ~rspfPolyLine();

   rspfDpt& operator[](int index)
      {
         return theVertexList[index];
      }
   const rspfDpt& operator[](int index)const
      {
         return theVertexList[index];
      }
   
   rspf_uint32 getNumberOfVertices()const
      {
         return (rspf_uint32)theVertexList.size();
      }
   
   void getIntegerBounds(rspf_int32& minX,
                         rspf_int32& minY,
                         rspf_int32& maxX,
                         rspf_int32& maxY)const;
   void getBounds(double& minX,
                  double& minY,
                  double& maxX,
                  double& maxY)const;
   void getBoundingRect(rspfDrect& rect)const
      {
         rspf_int32 minX;
         rspf_int32 minY;
         rspf_int32 maxX;
         rspf_int32 maxY;
         getIntegerBounds(minX, minY, maxX, maxY);
         rect = rspfDrect(minX, minY, maxX, maxY);
         
      }
   rspfDrect getBoundingRect()const
      {
         rspfDrect result;

         getBoundingRect(result);

         return result;
      }
   
   void roundToIntegerBounds(bool compress=true);
   void clear()
      {
         theVertexList.clear();
      }
   void addPoint(const rspfDpt& pt)
      {
         theVertexList.push_back(pt);
      }
   void addPoint(double x, double y)
      {
         theVertexList.push_back(rspfDpt(x, y));
      }
   void addAttribute( const rspfString& attribute )
     {
       theAttributeList.push_back( attribute );
     }

   rspfDpt midPoint()const;
   
   /*!
    * will sequence through the polygon and check to see if any values are NAN
    */
   bool hasNans()const;
  
  rspf_uint32 size()
  {
    return getNumberOfVertices();
  }
  void resize(rspf_uint32 newSize)
  {
    theVertexList.resize(newSize);
  }
   const vector<rspfDpt>& getVertexList()const
      {
         return theVertexList;
      }

   vector<rspfDpt>& getVertexList()
      {
         return theVertexList;
      }
   vector<rspfString>& getAttributeList()
      {
         return theAttributeList;
      }
      
   bool clipToRect(vector<rspfPolyLine>& result,
                   const rspfDrect& rect)const;

   /*!
    * Will clip this poly line list to the past in rect and
    * will return true if any part of this object is visible within
    * the rectangle;
    */
   bool isWithin(const rspfDrect& rect)const;
   
   
   /*!
    * METHOD: pointWithin(rspfDpt)
    * Returns TRUE if point is inside polygon.
    */
   bool pointWithin(const rspfDpt& point) const
      {
         return isPointWithin(point);
      }
   bool isPointWithin(const rspfDpt& point) const;

   /*!
    * METHOD: vertex(index)
    * Returns the rspfDpt vertex given the index. Returns false if no vertex
    * defined.
    */
   bool vertex(int index, rspfDpt& tbd_vertex) const;

   /*!
    * METHOD: nextVertex()
    * Assigns the rspfDpt tbd_vertex following the current vertex. The current
    * vertex is initialized with a call to vertex(int), or after the last
    * vertex is reached. Returns false if no vertex defined. Intended to be
    * when cycling through all vertices.
    */
   bool nextVertex(rspfDpt& tbd_vertex) const;

   void reverseOrder();
   /*!
    * OPERATORS: (Some are inlined at bottom) 
    */
   const rspfPolyLine& operator= (const rspfPolyLine& copy_this);
   const rspfPolyLine& operator= (const vector<rspfDpt>& vertexList);
   const rspfPolyLine& operator= (const vector<rspfIpt>& vertexList);
   const rspfPolyLine& operator= (const rspfIrect& rect);
   const rspfPolyLine& operator= (const rspfPolygon& polygon);
   const rspfPolyLine& operator= (const rspfDrect& rect);
   bool                operator==(const rspfPolyLine& compare_this) const;
   bool                operator!=(const rspfPolyLine& compare_this) const;

   const rspfPolyLine& operator *=(const rspfDpt& scale);
   const rspfPolyLine& operator *=(double scale)
      {
         return ((*this)*=rspfDpt(scale, scale));
      }
   
   rspfPolyLine operator *(const rspfDpt& scale)const;
   rspfPolyLine operator *(double scale)const
      {
         return ((*this)*rspfDpt(scale, scale));
      }

   /*!
    * METHOD: print()
    */
   void print(ostream& os) const;
   friend ostream& operator<<(ostream&, const rspfPolyLine&);


   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);
protected:
   vector<rspfDpt> theVertexList;
   vector<rspfString> theAttributeList;
   
   mutable rspf_int32 theCurrentVertex;
};

inline bool rspfPolyLine::operator!=(const rspfPolyLine& compare_this) const
{
   return !(*this == compare_this);
}

inline ostream& operator<<(ostream& os, const rspfPolyLine& polyLine)
{
   polyLine.print(os);
   return os;
}

#endif
