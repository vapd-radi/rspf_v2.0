//*****************************************************************************
// FILE: rspfPolygon.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class rspfPolygon.
//   This class provides utilities associated with N-vertex, convex
//   (i.e., only two intersections for any line passing through the polygon).
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfPolygon.h 19180 2011-03-22 17:36:33Z oscarkramer $

#ifndef rspfPolygon_HEADER
#define rspfPolygon_HEADER
#include <iostream>
#include <vector>
using namespace std;

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>

class rspfLine;

/******************************************************************************
 *
 * CLASS:  rspfPolygon
 *
 *****************************************************************************/
class RSPFDLLEXPORT rspfPolygon
{
public:
   rspfPolygon();
   rspfPolygon(const vector<rspfIpt>& polygon);
   rspfPolygon(const vector<rspfDpt>& polygon);
   rspfPolygon(int numVertices, const rspfDpt* vertex_array);

   rspfPolygon(const rspfPolygon& copy_this);
   
   /**
    * CONSTRUCTOR: Provided for convenience. Does not imply the polygon is
    * limited to four vertices:
    */
   rspfPolygon(rspfDpt v1,
                rspfDpt v2,
                rspfDpt v3,
                rspfDpt v4);
   
   rspfPolygon(const rspfIrect& rect);
   rspfPolygon(const rspfDrect& rect);

   ~rspfPolygon();

   rspfDpt& operator[](int index);
   
   const rspfDpt& operator[](int index)const;

   rspf_uint32 getVertexCount()const;
   
   rspf_uint32 getNumberOfVertices()const;

   //! Returns polygon area. Negative indicates CW ordering of vertices (in right-handed coordinates)
   double area()const;

   void getIntegerBounds(rspf_int32& minX,
                         rspf_int32& minY,
                         rspf_int32& maxX,
                         rspf_int32& maxY)const;
   void getFloatBounds(rspf_float64& minX,
                       rspf_float64& minY,
                       rspf_float64& maxX,
                       rspf_float64& maxY)const;

   void getBoundingRect(rspfIrect& rect)const;
   void getBoundingRect(rspfDrect& rect)const;

   /**
    * Initializes minRect with the minimum area rect (not-necessarily
    * aligned with axes) that bounds this polygon.
    *
    * @param minRect Polygon to initialize with the minimum rect.
    */
   void getMinimumBoundingRect(rspfPolygon& minRect) const;
   
   void roundToIntegerBounds(bool compress=false);

   void clear();

   void addPoint(const rspfDpt& pt);
   void addPoint(double x, double y);

   rspfDpt midPoint()const;
   
   /**
    * will sequence through the polygon and check to see if any values are NAN
    */
   bool hasNans()const;

   const vector<rspfDpt>& getVertexList()const;
      
   /**
    * Uses the rspfPolyArea2d class for the intersection
    */
   bool clipToRect(vector<rspfPolygon>& result,
                   const rspfDrect& rect)const;

   
   /**
    * METHOD: clipLineSegment(p1, p2)
    * Implements Cyrus-Beck clipping algorithm as described in:
    * http://www.daimi.au.dk/~mbl/cgcourse/wiki/cyrus-beck_line-clipping_.html
    * Clips the line segment defined by thw two endpoints provided. The
    * endpoints are modified as needed to represent the clipped line. Returnes
    * true if intersection present.
    */
   bool clipLineSegment(rspfDpt& p1, rspfDpt& p2) const;

   /**
    * METHOD: pointWithin(rspfDpt)
    * Returns TRUE if point is inside polygon.
    */
   bool pointWithin(const rspfDpt& point) const;

   bool isPointWithin(const rspfDpt& point) const;

   /**
   * METHOD: isRectWithin()
   * Returns true if all the corner points of the given rect fit within.
   */
   bool isRectWithin(const rspfIrect &rect) const;

   /**
   * METHOD: isPolyWithin()
   * Returns true if all the vertices of the given polygon fit within.
   */
   bool isPolyWithin(const rspfPolygon &poly) const;

   /**
    * METHOD: vertex(index)
    * Returns the rspfDpt vertex given the index. Returns false if no vertex
    * defined.
    */
   bool vertex(int index, rspfDpt& tbd_vertex) const;

   /**
    * METHOD: nextVertex()
    * Assigns the rspfDpt tbd_vertex following the current vertex. The current
    * vertex is initialized with a call to vertex(int), or after the last
    * vertex is reached. Returns false if no vertex defined. Intended to be
    * when cycling through all vertices.
    */
   bool nextVertex(rspfDpt& tbd_vertex) const;

   void reverseOrder();
   /**
    * OPERATORS: (Some are inlined at bottom) 
    */
   const rspfPolygon& operator= (const rspfPolygon& copy_this);
   const rspfPolygon& operator= (const vector<rspfDpt>& vertexList);
   const rspfPolygon& operator= (const vector<rspfIpt>& vertexList);
   const rspfPolygon& operator= (const rspfIrect& rect);
   const rspfPolygon& operator= (const rspfDrect& rect);
   bool                operator==(const rspfPolygon& compare_this) const;
   bool                operator!=(const rspfPolygon& compare_this) const;

   const rspfPolygon& operator *=(const rspfDpt& scale);
   const rspfPolygon& operator *=(double scale);
   
   rspfPolygon operator *(const rspfDpt& scale)const;
   rspfPolygon operator *(double scale)const;
   
   void resize(rspf_uint32 newSize);

   /**
    * METHOD: print()
    */
   void print(ostream& os) const;
   friend ostream& operator<<(ostream&, const rspfPolygon&);


   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);

   rspfVertexOrdering checkOrdering()const;

   rspfVertexOrdering getOrdering()const;

protected:
   /**
   * METHOD: getCentroid()
   * Assigns the rspfDpt centroid the polygon.
   * Warning: centroid is not guaranteed to be inside the polygon!
    */
   void getCentroid(rspfDpt &centroid) const;

   /**
   * METHOD: shrink()
   * Shrinks the current polygon by inset, return true if success.
    */
   bool shrink(rspfPolygon &dest, double inset) const;
   
   /**
   * METHOD: remove()
   * Removes the vertex from the polygon.

    */
   void removeVertex(int vertex);
   /**
   * METHOD: removeSmallestContributingVertex()
   * Removes the vertex that contributes the smallest area to the polygon.
    */
   void removeSmallestContributingVertex();

   void intersectEdge(rspfDpt& result,
                      const rspfLine& segment,
                      const rspfDrect& rect,
                      int edge);
   
   bool isInsideEdge(const rspfDpt& pt,
                     const rspfDrect& rect,
                     int edge)const;

   /**
   * Assigns destPt the point that fits a circle of given radius inside the polygon vertex.
   * Warning: destPt is not guaranteed to be inside the polygon!
   * (you may not be able to fit a circle of the given radius inside the polygon)
   */
   void fitCircleInsideVertex(rspfDpt &destPt, unsigned int vertex, double radius) const;

   mutable rspfVertexOrdering theOrderingType;
   vector<rspfDpt> theVertexList;
   mutable rspf_int32 theCurrentVertex;
};

#endif /* End of "#ifndef rspfPolygon_HEADER" */
