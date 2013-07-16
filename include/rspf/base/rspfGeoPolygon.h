//*****************************************************************************
// FILE: rspfPolygon.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Garrett Potts
//
//*****************************************************************************
//  $Id: rspfGeoPolygon.h 12760 2008-04-29 16:33:29Z dburken $
#ifndef rspfGeoPolygon_HEADER
#define rspfGeoPolygon_HEADER
#include <vector>
#include <rspf/base/rspfGpt.h>

class rspfKeywordlist;

class RSPFDLLEXPORT rspfGeoPolygon
{
public:
   friend RSPF_DLL std::ostream& operator <<(std::ostream& out, const rspfGeoPolygon& poly);
   rspfGeoPolygon():theCurrentVertex(-1),
                     theOrderingType(RSPF_VERTEX_ORDER_UNKNOWN)
      {}
   rspfGeoPolygon(const std::vector<rspfGpt>& points)
      :theVertexList(points),
       theOrderingType(RSPF_VERTEX_ORDER_UNKNOWN)
      {
         theCurrentVertex = 0;
      }
   rspfGeoPolygon(const rspfGeoPolygon& rhs)
      {
         theVertexList   = rhs.theVertexList;
         theOrderingType = rhs.theOrderingType;
	 theAttributeList = rhs.theAttributeList;
	 theHoleList = rhs.theHoleList;
         theCurrentVertex = rhs.theCurrentVertex;
      }

    void addPoint(const rspfGpt& pt)
      {
         theVertexList.push_back(pt);
      }
   void addPoint(double lat, double lon, double h=rspf::nan(), const rspfDatum* datum=0)
      {
         theVertexList.push_back(rspfGpt(lat, lon, h, datum));
      }
   void addAttribute( const rspfString& attribute )
     {
       theAttributeList.push_back( attribute );
     }
   void addHole( const rspfGeoPolygon& polygon )
     {
       theHoleList.push_back( polygon );
     }
   rspfGpt& operator[](int index)
      {
         return theVertexList[index];
      }
   const rspfGpt& operator[](int index)const
      {
         return theVertexList[index];
      }
   
   const std::vector<rspfGpt>& getVertexList()const
      {
         return theVertexList;
      }
   std::vector<rspfString>& getAttributeList()
      {
         return theAttributeList;
      }
   std::vector<rspfGeoPolygon>& getHoleList()
      {
         return theHoleList;
      }

   void clear()
      {
         theVertexList.clear();
      }
   rspf_uint32 size()const
      {
         return (rspf_uint32)theVertexList.size();
      }
   void resize(rspf_uint32 newSize)
      {
         theVertexList.resize(newSize);
         theCurrentVertex = 0;
         theOrderingType  = RSPF_VERTEX_ORDER_UNKNOWN;
      }
   const rspfGeoPolygon& operator = (const std::vector<rspfGpt>& rhs)
      {
         theVertexList = rhs;
         theCurrentVertex = 0;
         theOrderingType = RSPF_VERTEX_ORDER_UNKNOWN;

         return *this;
      }

   const rspfGeoPolygon& operator = (const rspfGeoPolygon& rhs)
      {
         if(&rhs != this)
         {
            theVertexList = rhs.theVertexList;
            theCurrentVertex = rhs.theCurrentVertex;
	    theAttributeList = rhs.theAttributeList;
	    theHoleList = rhs.theHoleList;
         }
         theOrderingType = rhs.theOrderingType;

         return *this;
      }
   void stretchOut(rspfGeoPolygon& newPolygon,
                   double displacement);
   double area()const;

   rspfGpt computeCentroid()const;
   
   bool vertex(int index, rspfGpt& v) const;
   bool nextVertex(rspfDpt& v) const;
   bool hasNans()const;
   void reverseOrder();
   
   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;
   
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);
    
   void checkOrdering()const;
   rspfVertexOrdering getOrdering()const
      {
         if(theOrderingType == RSPF_VERTEX_ORDER_UNKNOWN)
         {
            checkOrdering();
         }
         return theOrderingType;
      }
   void setOrdering(rspfVertexOrdering ordering)
      {
         theOrderingType = ordering;
      }
protected:
   std::vector<rspfGpt> theVertexList;
   std::vector<rspfString> theAttributeList;
   std::vector<rspfGeoPolygon> theHoleList;
   mutable rspf_int32 theCurrentVertex;
   
   /*!
    * This enumeration is found in rspfConstants.h
    */
   mutable rspfVertexOrdering theOrderingType;
   
};

#endif
