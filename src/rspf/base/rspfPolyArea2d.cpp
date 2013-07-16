//---
// License:  See top level LICENSE.txt file.
//
// $Id: rspfPolyArea2d.cpp 17195 2010-04-23 17:32:18Z dburken $
//---
#include <rspf/base/rspfPolyArea2d.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/kbool/booleng.h>
#include <rspf/kbool/graphlst.h>
#include <rspf/kbool/_dl_itr.h>
#include <rspf/base/rspfString.h>

std::ostream& operator <<(std::ostream& out, const rspfPolyArea2d& rhs)
{
   if(rhs.isEmpty()) return out;
   if(!rhs.theEngine->StartPolygonGet()) return out;
   rspf_uint32 pointCount = 0;
   while ( rhs.theEngine->nextPolygon() )
   {
      kbEdgeType edgeType = rhs.theEngine->GetPolygonPointEdgeType();
      if(edgeType != KB_FALSE_EDGE)
      {
         if(edgeType == KB_OUTSIDE_EDGE)
         {
            std::cout << "POLYGON\n";
         }
         else
         {
            std::cout << "HOLE \n";
         }
         pointCount = 0;
         while ( rhs.theEngine->PolygonHasMorePoints() )
         {
            std::cout << "( " << rhs.theEngine->GetPolygonXPoint() << ", "
                      << rhs.theEngine->GetPolygonYPoint() << " )" << std::endl;
            ++pointCount;
         }

         if(!pointCount)
         {
            std::cout << "<EMPTY>" << std::endl;
         }
      }
   }
   
   return out;
}

rspfPolyArea2d::rspfPolyArea2d(const rspfPolyArea2d& rhs)
   :theEngine(0)
   
{
   if(rhs.theEngine)
   {
      theEngine = new Bool_Engine(*rhs.theEngine);
   }
   else
   {
      clearEngine();
   }
}
rspfPolyArea2d::rspfPolyArea2d(const rspfDpt& p1,
                                 const rspfDpt& p2,
                                 const rspfDpt& p3,
                                 const rspfDpt& p4)
   :theEngine(0)
{
   rspfPolygon temp(p1,p2,p3,p4);
   *this = temp;
}
rspfPolyArea2d::~rspfPolyArea2d()
{
   if(theEngine)
   {
      delete theEngine;
      theEngine = 0;
   }
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const rspfPolyArea2d& rhs)
{
   if(this == &rhs)
   {
      return *this;
   }
   if(rhs.theEngine)
   {
      if(theEngine)
      {
         delete theEngine;
         theEngine = new Bool_Engine(*rhs.theEngine);
      }
   }
   else
   {
      clearEngine();
   }

   return *this;
}


const rspfPolyArea2d& rspfPolyArea2d::operator =(const rspfPolygon& polygon)
{
   clearEngine();
   if(polygon.getNumberOfVertices() < 1)
   {
      return *this;
   }
   return (*this) = polygon.getVertexList();
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const rspfIrect& rect)
{
//    clearEngine();

//    theEngine->StartPolygonAdd(GROUP_A);

   std::vector<rspfIpt> polygon;
   polygon.push_back(rect.ul());
   polygon.push_back(rect.ur());
   polygon.push_back(rect.lr());
   polygon.push_back(rect.ll());

   return (*this = polygon);
//    return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const rspfDrect& rect)
{
   std::vector<rspfIpt> polygon;
   polygon.push_back(rect.ul());
   polygon.push_back(rect.ur());
   polygon.push_back(rect.lr());
   polygon.push_back(rect.ll());

   return (*this = polygon);
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const vector<rspfDpt>& polygon)
{
   clearEngine();
   if(polygon.size() <1)
   {
      return *this;
   }
   rspfPolygon polygonTemp(polygon);
   polygonTemp.checkOrdering();
   if(polygonTemp.getOrdering() == RSPF_COUNTERCLOCKWISE_ORDER)
   {
      polygonTemp.reverseOrder();
   }
   rspf_uint32 idx = 0;
   rspf_uint32 maxIdx = polygonTemp.getNumberOfVertices();

   theEngine->StartPolygonAdd(GROUP_A);
   for(idx = 0; idx < maxIdx; ++idx)
   {
      theEngine->AddPoint(polygonTemp[idx].x,
                          polygonTemp[idx].y);
   }
   theEngine->EndPolygonAdd();
   
   return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const vector<rspfGpt>& polygon)
{
   rspf_uint32 idx = 0;
   rspf_uint32 maxIdx = (rspf_uint32)polygon.size();
   clearEngine();
   
   theEngine->StartPolygonAdd(GROUP_A);
   for(idx = 0; idx < maxIdx; ++idx)
   {
      theEngine->AddPoint(polygon[idx].lond(),
                          polygon[idx].latd());
   }
   theEngine->EndPolygonAdd();
   
   return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const vector<rspfIpt>& polygon)
{
   rspfPolygon polygonTemp(polygon);
   polygonTemp.checkOrdering();
   if(polygonTemp.getOrdering() == RSPF_COUNTERCLOCKWISE_ORDER)
   {
      polygonTemp.reverseOrder();
   }
   rspf_uint32 idx = 0;
   rspf_uint32 maxIdx = polygonTemp.getNumberOfVertices();
   clearEngine();

   theEngine->StartPolygonAdd(GROUP_A);
   for(idx = 0; idx < maxIdx; ++idx)
   {
      theEngine->AddPoint(polygonTemp[idx].x,
                          polygonTemp[idx].y);
   }
   theEngine->EndPolygonAdd();

   return *this;
}


rspfPolyArea2d rspfPolyArea2d::operator &(const rspfPolyArea2d& rhs)const
{
   if(isEmpty()||rhs.isEmpty())
   {
      return rspfPolyArea2d();
   }
   rspfPolyArea2d result(*this);
   performOperation(result, rhs, BOOL_AND);
   return result;
}

rspfPolyArea2d rspfPolyArea2d::operator +(const rspfPolyArea2d& rhs)const
{
   rspfPolyArea2d result(*this);
   if(isEmpty())
   {
      return rhs;
   }
   if(rhs.isEmpty())
   {
      return result;
   }
   performOperation(result, rhs, BOOL_OR);
   return result;
}

rspfPolyArea2d rspfPolyArea2d::operator ^(const rspfPolyArea2d& rhs)const
{
   if(isEmpty()) return rhs;
   rspfPolyArea2d result(*this);
   if(rhs.isEmpty()) return result;
   performOperation(result, rhs, BOOL_EXOR);
   return result;
}

rspfPolyArea2d rspfPolyArea2d::operator -(const rspfPolyArea2d& rhs)const
{
   rspfPolyArea2d result(*this);
   if(rhs.isEmpty())
   {
      return result;
   }
   if(isEmpty())
   {
      return rspfPolyArea2d();
   }
   performOperation(result, rhs, BOOL_A_SUB_B);
   return result;
}

const rspfPolyArea2d& rspfPolyArea2d::operator &=(const rspfPolyArea2d& rhs)
{
   if(isEmpty()||rhs.isEmpty())
   {
      clearPolygons();
      return *this;
   }
   performOperation(rhs, BOOL_AND);
   return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator ^=(const rspfPolyArea2d& rhs)
{
   if(&rhs==this)
   {
      clearPolygons();
      return *this;
   }
   if(isEmpty())
   {
      *this = rhs;
   }
   if(rhs.isEmpty())
   {
      return *this;
   }
   performOperation(rhs, BOOL_EXOR);
   return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator +=(const rspfPolyArea2d& rhs)
{
   if(isEmpty())
   {
      *this = rhs;
      return *this;
   }
   if(rhs.isEmpty())
   {
      return *this;
   }
   performOperation(rhs, BOOL_OR);
   return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator -=(const rspfPolyArea2d& rhs)
{
   if(this == &rhs)
   {
      clearPolygons();
      return *this;
   }
   if(isEmpty())
   {
      return *this;
   }
   if(rhs.isEmpty())
   {
      return *this;
   }
   performOperation(rhs, BOOL_A_SUB_B);
   return *this;
}

void rspfPolyArea2d::add(const rspfPolyArea2d& rhs)
{
   if(!rhs.theEngine->StartPolygonGet())
   {
      return;
   }

   while ( rhs.theEngine->nextPolygon() )
   {
      theEngine->StartPolygonAdd(GROUP_A);
      
      while ( rhs.theEngine->PolygonHasMorePoints() )
      {
         theEngine->AddPoint(rhs.theEngine->GetPolygonXPoint(),
                             rhs.theEngine->GetPolygonYPoint());
      }
      theEngine->EndPolygonAdd();
   }
}

void rspfPolyArea2d::clearPolygons()
{
   clearEngine();
}

void rspfPolyArea2d::performOperation(rspfPolyArea2d& result,
                                       const rspfPolyArea2d& rhs,
                                       BOOL_OP operation)const
{
   rhs.theEngine->StartPolygonGet();
   while ( rhs.theEngine->nextPolygon() )
   {
      result.theEngine->StartPolygonAdd(GROUP_B);
      while ( rhs.theEngine->PolygonHasMorePoints() )
      {
         result.theEngine->AddPoint(rhs.theEngine->GetPolygonXPoint(),
                                    rhs.theEngine->GetPolygonYPoint());
      }
      result.theEngine->EndPolygonAdd();
   }
   result.theEngine->Do_Operation(operation);
}

void  rspfPolyArea2d::performOperation(const rspfPolyArea2d& rhs,
                                        BOOL_OP operation)
{
   rhs.theEngine->StartPolygonGet();
   while ( rhs.theEngine->nextPolygon() )
   {
      theEngine->StartPolygonAdd(GROUP_B);
      while ( rhs.theEngine->PolygonHasMorePoints() )
      {
         theEngine->AddPoint(rhs.theEngine->GetPolygonXPoint(),
                                    rhs.theEngine->GetPolygonYPoint());
      }
      theEngine->EndPolygonAdd();
   }
   theEngine->Do_Operation(operation);
}

void rspfPolyArea2d::clearEngine()
{
   if(theEngine)
   {
      delete theEngine;
   }
   theEngine = new Bool_Engine();
   armBoolEng();
}


void rspfPolyArea2d::armBoolEng()
{
    // set some global vals to arm the boolean engine
    double DGRID = 1000000000;  // round coordinate X or Y value in calculations to this
    double MARGE = 0.000000001;   // snap with in this range points to lines in the intersection routines
                          // should always be > DGRID  a  MARGE >= 10*DGRID is oke
                          // this is also used to remove small segments and to decide when
                          // two segments are in line.
    double CORRECTIONFACTOR = 500.0;  // correct the polygons by this number
    double CORRECTIONABER   = 1.0;    // the accuracy for the rounded shapes used in correction
    double ROUNDFACTOR      = 1.5;    // when will we round the correction shape to a circle
    double SMOOTHABER       = 10.0;   // accuracy when smoothing a polygon
    double MAXLINEMERGE     = 1000.0; // leave as is, segments of this length in smoothen
 

    // DGRID is only meant to make fractional parts of input data which 
    // are doubles, part of the integers used in vertexes within the boolean algorithm.
    // Within the algorithm all input data is multiplied with DGRID
    
    // space for extra intersection inside the boolean algorithms
    // only change this if there are problems
    int GRID = 1;

    theEngine->SetMarge( MARGE );
    theEngine->SetGrid( GRID );
    theEngine->SetDGrid( DGRID );
    theEngine->SetCorrectionFactor( CORRECTIONFACTOR );
    theEngine->SetCorrectionAber( CORRECTIONABER );
    theEngine->SetSmoothAber( SMOOTHABER );
    theEngine->SetMaxlinemerge( MAXLINEMERGE );
    theEngine->SetRoundfactor( ROUNDFACTOR );
    theEngine->SetLinkHoles(false);
    theEngine->SetOrientationEntryMode(true);
}

bool rspfPolyArea2d::getVisiblePolygons(vector<rspfPolygon>& polyList)const
{
   bool foundPolygons = false;
   if(isEmpty()) return false;
   
   theEngine->StartPolygonGet();
   while ( theEngine->nextPolygon() )
   {
      kbEdgeType edgeType = theEngine->GetPolygonPointEdgeType();
      if(edgeType == KB_OUTSIDE_EDGE)
      {
         polyList.push_back(rspfPolygon());
         rspfPolygon& polygon = polyList[polyList.size()-1];
         while ( theEngine->PolygonHasMorePoints() )
         {
            if(polygon.getNumberOfVertices())
            {
               rspfDpt pt(theEngine->GetPolygonXPoint(),
                           theEngine->GetPolygonYPoint());
               if(pt!=polygon[0])
               {
                  polygon.addPoint(pt);
                  foundPolygons = true;
               }
            }
            else
            {
               polygon.addPoint(theEngine->GetPolygonXPoint(),
                                theEngine->GetPolygonYPoint());
               foundPolygons = true;
            }
         }
      }
   }

   return foundPolygons;
}

bool rspfPolyArea2d::getPolygonHoles(vector<rspfPolygon>& polyList, bool includeFalsePolygons)const
{
   bool foundPolygons = false;
   if(isEmpty()) return false;
   
   theEngine->StartPolygonGet();
   while ( theEngine->nextPolygon() )
   {
      kbEdgeType edgeType = theEngine->GetPolygonPointEdgeType();
      if((edgeType == KB_INSIDE_EDGE)||
         ( (edgeType == KB_FALSE_EDGE)&&includeFalsePolygons))
      {
         foundPolygons = true;
         polyList.push_back(rspfPolygon());
         rspfPolygon& polygon = polyList[polyList.size()-1];
         while ( theEngine->PolygonHasMorePoints() )
         {
            polygon.addPoint(theEngine->GetPolygonXPoint(),
                             theEngine->GetPolygonYPoint());
         }
      }
   }

   return foundPolygons;
}

bool rspfPolyArea2d::isEmpty()const
{
   if(!theEngine) return true;
   if(!theEngine->graphlist()) return true;
   
   return theEngine->graphlist()->empty();
}

bool rspfPolyArea2d::isPointWithin(const rspfDpt& point, double epsilonBall)const
{
   return isPointWithin(point.x, point.y, epsilonBall);
}

bool rspfPolyArea2d::isPointWithin(double x, double y,double epsilonBall)const
{
   if(epsilonBall < theEngine->GetMarge())
   {
      epsilonBall = theEngine->GetMarge()*10;
   }
   return !((*this)&rspfDrect(x,
                               y,
                               x + epsilonBall,
                               y + epsilonBall)).isEmpty();
}

void rspfPolyArea2d::getBoundingRect(rspfDrect& rect)
{
   rect.makeNan();
   bool firstPointFlag = false;
   double minX = 0.0;
   double minY = 0.0;
   double maxX = 0.0;
   double maxY = 0.0;
   if(!isEmpty())
   {
      theEngine->StartPolygonGet();
      while ( theEngine->nextPolygon() )
      {
         kbEdgeType edgeType = theEngine->GetPolygonPointEdgeType();
         if((edgeType != KB_FALSE_EDGE)&&
            (edgeType != KB_INSIDE_EDGE))
         {
            while ( theEngine->PolygonHasMorePoints() )
            {
               double x = theEngine->GetPolygonXPoint();
               double y = theEngine->GetPolygonYPoint();

               if(!firstPointFlag)
               {
                  minX = x;
                  minY = y;
                  maxX = x;
                  maxY = y;
                  firstPointFlag = true;
               }
               else
               {
                  minX = rspf::min(x, minX);
                  maxX = rspf::max(x, maxX);
                  minY = rspf::min(y, minY);
                  maxY = rspf::max(y, maxY);
               }
            }
         }
      }
   }

   if(firstPointFlag)
   {
      rect = rspfDrect(minX, minY, maxX, maxY);
   }
}

bool rspfPolyArea2d::saveState(rspfKeywordlist& kwl,
                                   const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfPolyArea2d",
           true);
   rspf_uint32 polygonCount = 0;
   theEngine->StartPolygonGet();
   while (theEngine->nextPolygon() )
   {
      kbEdgeType edgeType = theEngine->GetPolygonPointEdgeType();
      if(edgeType != KB_FALSE_EDGE)
      {
         rspfString newPolygonPrefix = rspfString(prefix)+"polygon" + rspfString::toString(polygonCount) + ".";
         rspfString pointPrefix = newPolygonPrefix + "point";
         rspf_uint32 pointCount = 0;
         while ( theEngine->PolygonHasMorePoints() )
         {
            kwl.add(pointPrefix + rspfString::toString(pointCount),
                    rspfDpt(theEngine->GetPolygonXPoint(),
                             theEngine->GetPolygonYPoint()).toString(),
                    true);
            ++pointCount;
         }
         kwl.add(newPolygonPrefix,
                 "num_points",
                 pointCount,
                 true);
         
         ++polygonCount;
      }
   }
   
   kwl.add(prefix,
           "num_polygons",
           polygonCount,
           true);
   
   return true;
}

bool rspfPolyArea2d::loadState(const rspfKeywordlist& kwl,
                                   const char* prefix)
{
   clearEngine();

   const char* numPolygons = kwl.find("num_polygons");
   if(!numPolygons)
   {
      return false;
   }

   rspf_uint32 maxSearch      = 1000;
   rspf_uint32 numberOfPolygonsFound = 0;
   rspf_uint32 polyIdx               = 0;
   rspf_uint32 lastPolyIdx           = 0;
   rspf_uint32 maxPolygons           = rspfString(numPolygons).toUInt32();
   
   while((numberOfPolygonsFound < maxPolygons)&&
         ((polyIdx - lastPolyIdx) < maxSearch))
   {
      rspfString newPolygonPrefix = rspfString(prefix)+"polygon" + rspfString::toString(polyIdx) + ".";
      const char* num_points = kwl.find(newPolygonPrefix,
                                        "num_points");

      theEngine->StartPolygonAdd(GROUP_A);
      if(num_points)
      {
         rspf_int32 maxPoints = rspfString(num_points).toInt32();
         rspf_uint32 pointIdx = 0;
         rspf_uint32 lastPointIdx = 0;
         rspf_uint32 numberOfPointsFound = 0;
         if(maxPoints < 0) maxPoints = 0;

         while((numberOfPointsFound < (rspf_uint32)maxPoints)&&
               ((pointIdx - lastPointIdx) < maxSearch))
         {
            rspfString pointPrefix = newPolygonPrefix + "point" + rspfString::toString(pointIdx);
            const char* point = kwl.find(pointPrefix);

            if(point)
            {
               rspfDpt dpt;

               dpt.toPoint(point);
               theEngine->AddPoint(dpt.x,
                                   dpt.y);
               ++numberOfPointsFound;
               lastPointIdx = pointIdx;
            }
            ++pointIdx;
         }
         ++numberOfPolygonsFound;
         lastPolyIdx = polyIdx;
      }
      theEngine->EndPolygonAdd();
      
      ++polyIdx;
   }
         
   return true;
}


#if 0
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
// $Id: rspfPolyArea2d.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <list>
#include <sstream>
using namespace std;

#include <rspf/base/rspfPolyArea2d.h>
#include <rspf/base/rspfKeywordNames.h>

ostream& operator<<(ostream& out, const rspfPolyArea2d& data)
{
   if(!data.thePolygon)
   {
      return out;
   }
   int cnt;
   PLINE *cntr;
   POLYAREA * curpa;
   
   cnt = 0, curpa = data.thePolygon; 
   do
   {
      cnt++;
   } while ( (curpa = curpa->f) != data.thePolygon);
   
   curpa = data.thePolygon; 
   do
   {
      for ( cntr = curpa->contours, cnt = 0; cntr != NULL; cntr = cntr->next, cnt++ )
      {}
      out << cnt << endl;
      
      for ( cntr = curpa->contours; cntr != NULL; cntr = cntr->next )
      {
         VNODE *cur;
         
         out << cntr->Count << endl;
         cur = &cntr->head;
         do
         {
            out << cur->point[0] << " " <<  cur->point[1] << endl;
         } while ( ( cur = cur->next ) != &cntr->head );
      }     
   }while ( (curpa = curpa->f) != data.thePolygon);
   
   return out;
}



rspfPolyArea2d::rspfPolyArea2d()
   :thePolygon(NULL)
{  
}

rspfPolyArea2d::rspfPolyArea2d(POLYAREA* polygon,
                               bool    copyPolygonFlag)
   :thePolygon(NULL)
{
   if(copyPolygonFlag)
   {
      poly_Copy0(&thePolygon, polygon);
   }
   else
   {
      thePolygon = polygon;
   }
}

rspfPolyArea2d::rspfPolyArea2d(const rspfPolyArea2d& rhs)
   :thePolygon(NULL)
{
   poly_Copy0(&thePolygon, rhs.thePolygon);
}

rspfPolyArea2d::rspfPolyArea2d(const vector<rspfDpt>& polygon)
   :thePolygon(NULL)
{
   if(polygon.size() > 0)
   {
      PLINE  *poly = NULL;
      Vector v;
      v[0] = polygon[0].x;
      v[1] = polygon[0].y;
      v[2] = 0;
      poly = poly_NewContour(v);
      for(long index = 1; index < (long)polygon.size(); ++index)
      {

         v[0] = polygon[index].x;
         v[1] = polygon[index].y;
         v[2] = 0.0;
         
         poly_InclVertex(poly->head.prev,
                         poly_CreateNode(v));
         
      }
      poly_PreContour(poly, TRUE);
      
      if ( (poly->Flags & PLF_ORIENT) != PLF_DIR)
      {
         poly_InvContour(poly);
      }
      thePolygon = poly_Create();
      poly_InclContour(thePolygon, poly);
//       if(!poly_Valid(thePolygon))
//       {
//          if(thePolygon)
//          {
//             poly_Free(&thePolygon);
//             thePolygon = NULL;
//          }
//       }
   }   
}

rspfPolyArea2d::rspfPolyArea2d(const vector<rspfGpt>& polygon)
   :thePolygon(NULL)
{
   if(polygon.size() > 0)
   {
      PLINE  *poly = NULL;
      Vector v;
      v[0] = polygon[0].lon;
      v[1] = polygon[0].lat;
      v[2] = 0;
      poly = poly_NewContour(v);
      for(long index = 1; index < (long)polygon.size(); ++index)
      {

         v[0] = polygon[index].lon;
         v[1] = polygon[index].lat;
         v[2] = 0.0;
         
         poly_InclVertex(poly->head.prev,
                         poly_CreateNode(v));
         
      }
      poly_PreContour(poly, TRUE);
      
      if ( (poly->Flags & PLF_ORIENT) != PLF_DIR)
      {
         poly_InvContour(poly);
      }
      thePolygon = poly_Create();
      poly_InclContour(thePolygon, poly);
//       if(!poly_Valid(thePolygon))
//       {
//          if(thePolygon)
//          {
//             poly_Free(&thePolygon);
//             thePolygon = NULL;
//          }
//       }
   }   
}

rspfPolyArea2d::rspfPolyArea2d(const rspfDpt& p1,
                                 const rspfDpt& p2,
                                 const rspfDpt& p3,
                                 const rspfDpt& p4)
   :thePolygon(NULL)
{
   PLINE  *poly = NULL;
   Vector v;
   v[0] = p1.x;
   v[1] = p1.y;
   v[2] = 0;
   poly = poly_NewContour(v);
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = p2.x;
   v[1] = p2.y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = p3.x;
   v[1] = p3.y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = p4.x;
   v[1] = p4.y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   
   poly_PreContour(poly, TRUE);
   
   if ( (poly->Flags & PLF_ORIENT) != PLF_DIR)
   {
      poly_InvContour(poly);
   }
   thePolygon = poly_Create();
   poly_InclContour(thePolygon, poly);
//    if(!poly_Valid(thePolygon))
//    {
//       if(thePolygon)
//       {
//          poly_Free(&thePolygon);
//          thePolygon = NULL;
//       }
//    }   
}


rspfPolyArea2d::rspfPolyArea2d(rspfDpt* polygon,
                               long number)
   :thePolygon(NULL)
{
   if(number > 0)
   {
      PLINE  *poly = NULL;
      Vector v;
      v[0] = polygon[0].x;
      v[1] = polygon[0].y;
      v[2] = 0;
      poly = poly_NewContour(v);
      for(long index = 1; index < number; ++index)
      {

         v[0] = polygon[index].x;
         v[1] = polygon[index].y;
         v[2] = 0.0;
         
         poly_InclVertex(poly->head.prev,
                         poly_CreateNode(v));
         
      }
      poly_PreContour(poly, TRUE);
      
      if ( (poly->Flags & PLF_ORIENT) != PLF_DIR)
      {
         poly_InvContour(poly);
      }
      thePolygon = poly_Create();
      poly_InclContour(thePolygon, poly);
//       if(!poly_Valid(thePolygon))
//       {
//          poly_Free(&thePolygon);
//          thePolygon = NULL;
//       }
   }
}

rspfPolyArea2d::rspfPolyArea2d(const rspfPolygon& polygon)
   :thePolygon(NULL)
{
   if(polygon.getVertexCount() > 0)
   {
      PLINE  *poly = NULL;
      Vector v;
      v[0] = polygon[0].x;
      v[1] = polygon[0].y;
      v[2] = 0;
      poly = poly_NewContour(v);
      for(long index = 1; index < (long)polygon.getVertexCount(); ++index)
      {

         v[0] = polygon[index].x;
         v[1] = polygon[index].y;
         v[2] = 0.0;
         
         poly_InclVertex(poly->head.prev,
                         poly_CreateNode(v));
         
      }
      poly_PreContour(poly, TRUE);
      
      // make sure that its clockwise
      if ( (poly->Flags & PLF_ORIENT) != PLF_DIR)
      {
         poly_InvContour(poly);
      }
      thePolygon = poly_Create();
      poly_InclContour(thePolygon, poly);
      if(!poly_Valid(thePolygon))
      {
//         if(thePolygon)
//         {
//            poly_Free(&thePolygon);
//            thePolygon = NULL;
//         }
      }
   }   
}

rspfPolyArea2d::~rspfPolyArea2d()
{
   if(thePolygon)
   {
      poly_Free(&thePolygon);
      thePolygon = NULL;
   }
}
const rspfPolyArea2d& rspfPolyArea2d::operator *=(const rspfDpt& scale)
{
   if(!thePolygon) return *this;
   
   PLINE *cntr     = (PLINE*)NULL;
   POLYAREA *curpa = (POLYAREA*)NULL;
   
   curpa           = thePolygon;

   do{
      for ( cntr = curpa->contours; cntr != NULL; cntr = cntr->next )
      {
         VNODE *cur = &cntr->head;
         double xmin, ymin;
         double xmax, ymax;
         xmin = xmax = cur->point[0];
         ymin = ymax = cur->point[1];
         do
         {
            cur->point[0] *= scale.x;
            cur->point[1] *= scale.y;
            xmin = std::min(cur->point[0], xmin);
            xmax = std::max(cur->point[0], xmax);
            ymin = std::min(cur->point[1], ymin);
            ymax = std::max(cur->point[1], ymax);
         } while ( ( cur = cur->next ) != &cntr->head );
      }
   }
   while( (curpa = curpa->f) != thePolygon);
   
   return *this;
}

rspfPolyArea2d rspfPolyArea2d::operator *(const rspfDpt& scale)const
{
   if(!thePolygon) return *this;

   rspfPolyArea2d result(*this);
   PLINE *cntr     = (PLINE*)NULL;
   POLYAREA *curpa = (POLYAREA*)NULL;
   
   curpa           = result.thePolygon;

   do{
      for ( cntr = curpa->contours; cntr != NULL; cntr = cntr->next )
      {
         VNODE *cur = &cntr->head;
         double xmin, ymin;
         double xmax, ymax;
         xmin = xmax = cur->point[0];
         ymin = ymax = cur->point[1];
         
         do
         {
            cur->point[0] *= scale.x;
            cur->point[1] *= scale.y;
            xmin = std::min(cur->point[0], xmin);
            xmax = std::max(cur->point[0], xmax);
            ymin = std::min(cur->point[1], ymin);
            ymax = std::max(cur->point[1], ymax);
         } while ( ( cur = cur->next ) != &cntr->head );

         cntr->xmin = xmin;
         cntr->ymin = ymin;
         cntr->xmax = xmax;
         cntr->ymax = ymax;
      }
   }
   while( (curpa = curpa->f) != result.thePolygon);
   
   return result;
}

const rspfPolyArea2d& rspfPolyArea2d::operator *=(double scale)
{
   return ((*this)*=rspfDpt(scale, scale));
}

rspfPolyArea2d rspfPolyArea2d::operator *(double scale)const
{
   return ((*this)*rspfDpt(scale, scale));
}

/*!
 * Assignment operator.  Allows you to assign
 * or copy one polygon to another.
 */
const rspfPolyArea2d& rspfPolyArea2d::operator =(const rspfPolyArea2d& rhs)
{
   if(this != &rhs)
   {
      if(thePolygon)
      {
         poly_Free(&thePolygon);
         thePolygon = NULL;
      }
      if(rhs.thePolygon)
      {
         poly_Copy0(&thePolygon, rhs.thePolygon);
      }
   }

   return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const rspfIrect& rect)
{
   if(thePolygon)
   {
      poly_Free(&thePolygon);
      thePolygon = NULL;
   }
   
   PLINE  *poly = NULL;
   Vector v;
   v[0] = rect.ul().x;
   v[1] = rect.ul().y;
   v[2] = 0;
   poly = poly_NewContour(v);
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = rect.ur().x;
   v[1] = rect.ur().y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = rect.lr().x;
   v[1] = rect.lr().y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = rect.ll().x;
   v[1] = rect.ll().y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   
   poly_PreContour(poly, TRUE);
   
   if ( (poly->Flags & PLF_ORIENT) != PLF_DIR)
   {
      poly_InvContour(poly);
   }
   thePolygon = poly_Create();
   poly_InclContour(thePolygon, poly);
//    if(!poly_Valid(thePolygon))
//    {
//       if(thePolygon)
//       {
//          poly_Free(&thePolygon);
//          thePolygon = NULL;
//       }
//    }

   return *this;
}

const rspfPolyArea2d& rspfPolyArea2d::operator =(const rspfDrect& rect)
{
   if(thePolygon)
   {
      poly_Free(&thePolygon);
      thePolygon = NULL;
   }
   
   PLINE  *poly = NULL;
   Vector v;
   v[0] = rect.ul().x;
   v[1] = rect.ul().y;
   v[2] = 0;
   poly = poly_NewContour(v);
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = rect.ur().x;
   v[1] = rect.ur().y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = rect.lr().x;
   v[1] = rect.lr().y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   v[0] = rect.ll().x;
   v[1] = rect.ll().y;
   v[2] = 0;
   poly_InclVertex(poly->head.prev,
                   poly_CreateNode(v));
   
   poly_PreContour(poly, TRUE);
   
   if ( (poly->Flags & PLF_ORIENT) != PLF_DIR)
   {
      poly_InvContour(poly);
   }
   thePolygon = poly_Create();
   poly_InclContour(thePolygon, poly);
//    if(!poly_Valid(thePolygon))
//    {
//       if(thePolygon)
//       {
//          poly_Free(&thePolygon);
//          thePolygon = NULL;
//       }
//    }
   
   return *this;
}

rspfPolyArea2d rspfPolyArea2d::operator -(const rspfPolyArea2d& rhs)const
{
   if(this == &rhs)
   {
      return rspfPolyArea2d();
   }
   POLYAREA* result=NULL;

   if (poly_Boolean(this->thePolygon,
                    rhs.thePolygon,
                    &result,
                    PBO_SUB) == err_ok) 
   {
      return rspfPolyArea2d(result, false);
   }

   // if there were errors then we will just return an
   // empty polygon.
   //
   return rspfPolyArea2d();
}

rspfPolyArea2d& rspfPolyArea2d::operator -=(const rspfPolyArea2d& rhs)
{
   if(this == &rhs)
   {
      poly_Free(&thePolygon);
      thePolygon = NULL;
   }
   else
   {
      POLYAREA* result=NULL;
      
      if (poly_Boolean(this->thePolygon,
                       rhs.thePolygon,
                       &result,
                       PBO_SUB) == err_ok) 
      {
         poly_Free(&thePolygon);
         thePolygon = result;
      }
      else
      {
         // errors so make it empty
         poly_Free(&thePolygon);
         thePolygon = NULL;
      }
   }

   return *this;
}

rspfPolyArea2d rspfPolyArea2d::operator +(const rspfPolyArea2d& rhs)const
{
   if(this == &rhs)
   {
      return *this;
   }
   POLYAREA* result=NULL;

   if (poly_Boolean(this->thePolygon,
                    rhs.thePolygon,
                    &result,
                    PBO_UNITE) == err_ok) 
   {
      return rspfPolyArea2d(result, false);
   }

   // if there were errors then we will just return an
   // empty polygon.
   //
   return rspfPolyArea2d();
}

rspfPolyArea2d& rspfPolyArea2d::operator +=(const rspfPolyArea2d& rhs)
{
   if(this == &rhs)
   {
      poly_Free(&thePolygon);
      thePolygon = NULL;
   }
   else
   {
      POLYAREA* result=NULL;
      if (poly_Boolean(this->thePolygon,
                       rhs.thePolygon,
                       &result,
                       PBO_UNITE) == err_ok) 
      {
         poly_Free(&thePolygon);
         thePolygon = result;
      }
      else
      {
         // errors so make it empty
         poly_Free(&thePolygon);
         thePolygon = NULL;
      }
   }

   return *this;
}

rspfPolyArea2d rspfPolyArea2d::operator &(const rspfPolyArea2d& rhs)const
{
   if(this == &rhs)
   {
      return rspfPolyArea2d(*this);
   }
   if(thePolygon&&rhs.thePolygon)
   {
      POLYAREA* result=NULL;
      
      if (poly_Boolean(this->thePolygon,
                       rhs.thePolygon,
                       &result,
                       PBO_ISECT) == err_ok) 
      {
         return rspfPolyArea2d(result, false);
      }
   }
   
   // if there were errors then we will just return an
   // empty polygon.
   //
   return rspfPolyArea2d();
}

rspfPolyArea2d& rspfPolyArea2d::operator &=(const rspfPolyArea2d& rhs)
{
   if(this != &rhs)
   {
      POLYAREA* result=NULL;
      if (poly_Boolean(this->thePolygon,
                       rhs.thePolygon,
                       &result,
                       PBO_ISECT) == err_ok) 
      {
         poly_Free(&thePolygon);
         thePolygon = result;
      }
      else
      {
         // errors so make it empty
         poly_Free(&thePolygon);
         thePolygon = NULL;
      }
   }

   return *this;
}

rspfPolyArea2d rspfPolyArea2d::operator ^(const rspfPolyArea2d& rhs)const
{
   if(this != &rhs)
   {
      POLYAREA* result=NULL;
      
      if (poly_Boolean(this->thePolygon,
                       rhs.thePolygon,
                       &result,
                       PBO_XOR) == err_ok) 
      {
         return rspfPolyArea2d(result, false);
      }
      
   }
   
   return rspfPolyArea2d();
}

rspfPolyArea2d& rspfPolyArea2d::operator ^=(const rspfPolyArea2d& rhs)
{
   if(this == &rhs)
   {
      poly_Free(&thePolygon);
      thePolygon = NULL;
   }
   else
   {
      POLYAREA* result=NULL;
      if (poly_Boolean(this->thePolygon,
                       rhs.thePolygon,
                       &result,
                       PBO_XOR) == err_ok) 
      {
         poly_Free(&thePolygon);
         thePolygon = result;
      }
      else
      {
         // errors so make it empty
         poly_Free(&thePolygon);
         thePolygon = NULL;
      }
   }
   
   return *this;
}

bool rspfPolyArea2d::isPointWithin(const rspfDpt& point)const
{
   if(!thePolygon)
   {
      return false;
   }
   Vector v;
   v[0] = point.x;
   v[1] = point.y;
   v[2] = 0;
   
   return (poly_CheckInside(thePolygon, v)==TRUE);
  
}

bool rspfPolyArea2d::isPointWithin(double x, double y)const
{
   if(!thePolygon)
   {
      return false;
   }
   Vector v;
   v[0] = x;
   v[1] = y;
   v[2] = 0;
   
   return (poly_CheckInside(thePolygon, v)==TRUE);
}

void rspfPolyArea2d::getBoundingRect(rspfDrect& rect)
{
   if(thePolygon)
   {
      PLINE*    cntr  = NULL;
      POLYAREA* curpa = NULL;
      
      curpa = thePolygon;
      rect = rspfDrect(curpa->contours->xmin,
                        curpa->contours->ymin,
                        curpa->contours->xmax,
                        curpa->contours->ymax);
      do
      {        
         for ( cntr = curpa->contours->next; cntr != NULL; cntr = cntr->next )
         {
            rspfDrect rect2 = rspfDrect(curpa->contours->xmin,
                                          curpa->contours->ymin,
                                          curpa->contours->xmax,
                                          curpa->contours->ymax);
            if(rect2.width() != 1 &&
               rect2.height() != 1)
            {
               rect = rect.combine(rect2);
            }
         }     
      }while ( (curpa = curpa->f) != thePolygon);
   }
   else
   {
      rect = rspfDrect(0,0,0,0);
   }
}

bool rspfPolyArea2d::saveState(rspfKeywordlist& kwl,
                               const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           "rspfPolyArea2d",
           true);
           
   if(!thePolygon)
   {
      return true;
   }
   int cnt;
   PLINE *cntr;
   POLYAREA * curpa;
   
   cnt = 0, curpa = thePolygon; 
   do
   {
      cnt++;
   } while ( (curpa = curpa->f) != thePolygon);
   
   curpa = thePolygon; 
   do
   {
      cnt = 1;
      for ( cntr = curpa->contours; cntr != NULL; cntr = cntr->next )
      {
         VNODE *cur;
         rspfString newPrefix = prefix;
         newPrefix+="contour";
         newPrefix += rspfString::toString(cnt);
         newPrefix += ".";
         kwl.add(newPrefix.c_str(),
                 "flags",
                 (int)cntr->Flags,
                 true);
         
         cur = &cntr->head;
         int pointCount = 1;
         do
         {
            rspfString pointString = "point";
            pointString += rspfString::toString(pointCount);
            rspfString points;
            points = rspfString::toString(cur->point[0]);
            points += " ";
            points += rspfString::toString(cur->point[1]);
            kwl.add(newPrefix.c_str(),
                    pointString,
                    points.c_str(),
                    true);
            ++pointCount;
         } while ( ( cur = cur->next ) != &cntr->head );
         ++cnt;
      }     
   }while ( (curpa = curpa->f) != thePolygon);
   
   return true;
}

bool rspfPolyArea2d::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   
   vector<rspfString> contourList = kwl.getSubstringKeyList(rspfString(prefix) + "contour[0-9]+\\.");
   if(thePolygon)
   {
      poly_Free(&thePolygon);
      thePolygon = NULL;
   }
   if(contourList.size() > 0)
   {
      thePolygon = poly_Create();
      vector<rspfString>::iterator currentContour = contourList.begin();
      
      Vector v;
      PLINE  *poly = NULL;
      rspf_uint32 currentContourIndex = 0;
      while(currentContour != contourList.end())
      {
         rspfString copyPrefix = *currentContour;
         rspf_int32 contourFlags = 0;
         
         const char* lookupFlags = kwl.find((*currentContour).c_str(),
                                             "flags");
         if(lookupFlags)
         {
            contourFlags = rspfString(lookupFlags).toInt32();
         }
         rspf_uint32 numberOfPoints = kwl.getNumberOfSubstringKeys(copyPrefix + "point[0-9]");
         rspf_uint32 numberOfMatches = 0;
         rspf_uint32 currentPoint = 0;
         while(numberOfMatches < numberOfPoints)
         {
            rspfString x,y;
            rspfString pointString = "point";
            pointString += rspfString::toString(currentPoint);
            const char* pointLookup = kwl.find((*currentContour).c_str(),
                                               pointString);
            if(pointLookup)
            {
               istringstream s(pointLookup);

               s >> x >> y;
               v[0] = x.toDouble();
               v[1] = y.toDouble();
               v[2] = 0;
               // initialize the poly if we are on the
               // first match point else we just add it.
               if(!numberOfMatches)
               {
                  poly = poly_NewContour(v);
               }
               else
               {
                  poly_InclVertex(poly->head.prev,
                                  poly_CreateNode(v));
               }
               
               ++numberOfMatches;
            }
            ++currentPoint;
         }
         poly->Flags = contourFlags;
         poly_PreContour(poly, TRUE);
         if ( (poly->Flags & PLF_ORIENT) !=
              (currentContourIndex ? PLF_INV : PLF_DIR) )
         {
            poly_InvContour(poly);
         }
         if(!poly_InclContour(thePolygon, poly))
         {
            poly_DelContour(&poly);
         }
         poly = NULL;
         ++currentContour;
         ++currentContourIndex;
      }
   }
   return true;
}

bool rspfPolyArea2d::getAllVisiblePolygons(vector<rspfPolygon>& polyList)const
{
   if(!thePolygon) return false;
   if(!poly_Valid(thePolygon))
   {
      return false;
   }
   polyList.clear();
   POLYAREA* curpa = thePolygon;
   PLINE*    cntr;
   int index = 0;
   do
   {
      for ( cntr = curpa->contours; cntr != NULL; cntr = cntr->next )
      {
         if(cntr->Flags & PLF_DIR)
         {
            polyList.push_back(rspfPolygon());
            VNODE* cur = &cntr->head;
            do
            {   
               polyList[index].addPoint(cur->point[0],
                                        cur->point[1]);
               
            } while ( ( cur = cur->next ) != &cntr->head );
            ++index;
         }
      }
   }while ( (curpa = curpa->f) != thePolygon);

   return (polyList.size()>0);
}
#endif
