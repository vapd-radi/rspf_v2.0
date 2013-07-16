//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//*******************************************************************
//$Id: rspfPolyArea2d.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfPolyArea2d_HEADER
#define rspfPolyArea2d_HEADER
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/kbool/bool_globals.h>
class Bool_Engine;

class RSPF_DLL rspfPolyArea2d : public rspfReferenced
{
public:
   friend RSPF_DLL std::ostream& operator <<(std::ostream& out, const rspfPolyArea2d& data);
   rspfPolyArea2d()
      :theEngine(0)
      {
         clearEngine();
      }
   rspfPolyArea2d(const vector<rspfDpt>& polygon)
      :theEngine(0)
   {
      clearEngine();
      (*this) = polygon;
   }
   rspfPolyArea2d(const vector<rspfIpt>& polygon)
         :theEngine(0)
      {
         clearEngine();
         (*this) = polygon;
      }
   rspfPolyArea2d(const vector<rspfGpt>& polygon)
      :theEngine(0)
      {
         clearEngine();
         (*this) = polygon;
      }
      
   rspfPolyArea2d(const rspfDpt& p1,
                   const rspfDpt& p2,
                   const rspfDpt& p3,
                   const rspfDpt& p4);
   rspfPolyArea2d(const rspfPolyArea2d& rhs);
      
   rspfPolyArea2d(const rspfIrect& rect)
      :theEngine(0)
      {
         (*this) = rect;
      }
   rspfPolyArea2d(const rspfDrect& rect)
      :theEngine(0)
      {
         (*this) = rect;
      }
   rspfPolyArea2d(const rspfPolygon& polygon)
      :theEngine(0)
   {
      (*this) = polygon;
   }
   ~rspfPolyArea2d();

   void clear()
   {
      clearEngine();
   }
   const rspfPolyArea2d& operator =(const rspfPolyArea2d& rhs);
   const rspfPolyArea2d& operator =(const rspfPolygon& rhs);
   const rspfPolyArea2d& operator =(const rspfIrect& rect);
   const rspfPolyArea2d& operator =(const rspfDrect& rect);
   const rspfPolyArea2d& operator =(const vector<rspfDpt>& polygon);
   const rspfPolyArea2d& operator =(const vector<rspfGpt>& polygon);
   const rspfPolyArea2d& operator =(const vector<rspfIpt>& polygon);


   
   rspfPolyArea2d operator &(const rspfPolyArea2d& rhs)const;
   rspfPolyArea2d operator -(const rspfPolyArea2d& rhs)const;
   rspfPolyArea2d operator +(const rspfPolyArea2d& rhs)const;
   rspfPolyArea2d operator ^(const rspfPolyArea2d& rhs)const;
   const rspfPolyArea2d& operator &=(const rspfPolyArea2d& rhs);
   const rspfPolyArea2d& operator -=(const rspfPolyArea2d& rhs);
   const rspfPolyArea2d& operator +=(const rspfPolyArea2d& rhs);
   const rspfPolyArea2d& operator ^=(const rspfPolyArea2d& rhs);
   

   void add(const rspfPolyArea2d& rhs);
   
   bool getVisiblePolygons(vector<rspfPolygon>& polyList)const;
   bool getPolygonHoles(vector<rspfPolygon>& polyList, bool includeFalsePolygons=false)const;
   bool isEmpty()const;
   bool isPointWithin(const rspfDpt& point, double epsilonBall=.0000001)const;
   bool isPointWithin(double x, double y, double epsilonBall=.0000001)const;
   void getBoundingRect(rspfDrect& rect);

   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);
   
protected:
   void armBoolEng();
   void clearEngine();
   void performOperation(const rspfPolyArea2d& rhs,
                         BOOL_OP operation); 
   void performOperation(rspfPolyArea2d& result,
                         const rspfPolyArea2d& rhs,
                         BOOL_OP operation)const;
   void clearPolygons();
   
   mutable Bool_Engine* theEngine;
};



#endif








#if 0


//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// RSPF is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
// You should have received a copy of the GNU General Public License
// along with this software. If not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-
// 1307, USA.
//
// See the GPL in the COPYING.GPL file for more details.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfPolyArea2d.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfPolyArea2d_HEADER
#define rspfPolyArea2d_HEADER
extern "C"
{
#include "libpolyclip/polyarea.h"
}

#include <iostream>
#include <vector>
using namespace std;

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfPolygon.h>

/*!
 * This class uses the multiclip 2-d polygon library
 * to do intersections, unions, set difference, and xor
 * operations.  It will also use the library to do quick
 * point inside and poly inside tests.
 */
class RSPF_DLL rspfPolyArea2d
{
public:
   friend RSPF_DLL ostream& operator  <<(ostream& out,
                                const rspfPolyArea2d& data);
   /*!
    * we will use the multi clip library in
    * libpolyclip directory.
    */
   rspfPolyArea2d();
   /*!
    * If the copy polygon flag is false
    * it will set its internal thePolygon to polygon
    * else it will make a copy of it.
    */
   rspfPolyArea2d(POLYAREA* polygon,
                  bool    copyPolygonFlag=false);
   
   rspfPolyArea2d(const rspfPolyArea2d& rhs);
   rspfPolyArea2d(const vector<rspfDpt>& polygon);
   rspfPolyArea2d(const vector<rspfGpt>& polygon);
   rspfPolyArea2d(rspfDpt* points,
                  long number);

   rspfPolyArea2d(const rspfDpt& p1,
                   const rspfDpt& p2,
                   const rspfDpt& p3,
                   const rspfDpt& p4);
   rspfPolyArea2d(const rspfIrect& rect)
      :thePolygon(NULL)
      {
         (*this) = rect;
      }
   rspfPolyArea2d(const rspfDrect& rect)
      :thePolygon(NULL)
      {
         (*this) = rect;
      }
   
   rspfPolyArea2d(const rspfPolygon& polygon);
   ~rspfPolyArea2d();

   /*!
    * Will scale the polygon along the x and y direction
    * by the passed in scale.  It will modify itself and
    * return a const reference to itself.
    */
   const rspfPolyArea2d& operator *=(const rspfDpt& scale);
   
   /*!
    * Will scale the polygon along the x and y direction
    * by the passed in scale.  It will return another object.
    */
   rspfPolyArea2d operator *(const rspfDpt& scale)const;

   /*!
    * Scales evenly along the x and y direction.
    */
   const rspfPolyArea2d& operator *=(double scale);

   /*!
    * Scales evenly along the x and y direction.
    */
   rspfPolyArea2d operator *(double scale)const;

   /*!
    * Assignment operator.  Allows you to assign
    * or copy one polygon to another.
    */
   const rspfPolyArea2d& operator =(const rspfPolyArea2d& rhs);

   const rspfPolyArea2d& operator =(const rspfIrect& rect);
   const rspfPolyArea2d& operator =(const rspfDrect& rect);
   
   /*!
    * Will do a set difference on the this - right
    * and return a new polygon.
    */
   rspfPolyArea2d operator -(const rspfPolyArea2d& rhs)const;

   /*!
    * This will do the same as - but will
    * update itself.
    */
   rspfPolyArea2d& operator -=(const rspfPolyArea2d& rhs);
   
   /*!
    * do a union of 2 polygons.
    */
   rspfPolyArea2d operator +(const rspfPolyArea2d& rhs)const;

   /*!
    * This will do the same as the operator
    * + but will update itself.
    */
   rspfPolyArea2d& operator +=(const rspfPolyArea2d& rhs);


   /*!
    * do an intersection of 2 polygons
    */
   rspfPolyArea2d operator &(const rspfPolyArea2d& rhs)const;

   /*!
    * This will do the same as the operator &
    * but will update itself.
    */

   rspfPolyArea2d& operator &=(const rspfPolyArea2d& rhs);

   /*!
    * Will perform the "excusive or" of the this
    * object with the right side
    */
   rspfPolyArea2d operator ^(const rspfPolyArea2d& rhs)const;

   /*!
    * This is the non const version and will update
    * itself.
    */
   rspfPolyArea2d& operator ^=(const rspfPolyArea2d& rhs);

   /*!
    * This only makes since if you know that there are no holes.
    * For example, using it after an intersection only operation by loading
    * up two filled regions and intersecting them.
    */
   bool getAllVisiblePolygons(vector<rspfPolygon>& polyList)const;
   
   bool isEmpty()const
      {
         return (thePolygon == NULL);
      }

   bool isPointWithin(const rspfDpt& point)const;
   bool isPointWithin(double x, double y)const;
   void getBoundingRect(rspfDrect& rect);

   bool saveState(rspfKeywordlist& kwl,
                  const char* prefix=0)const;
   bool loadState(const rspfKeywordlist& kwl,
                  const char* prefix=0);
protected:
   POLYAREA *thePolygon;

   void loadPolygonForCurrentContour(rspfPolygon& polygon,
                                     bool visible);
};

#endif
#endif
