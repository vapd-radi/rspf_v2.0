//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// AUTHOR: Garrett Potts (gpotts@imagelinks.com)
//
//*****************************************************************************
//  $Id: rspfActiveEdgeTable.cpp 11347 2007-07-23 13:01:59Z gpotts $
#include <rspf/base/rspfActiveEdgeTable.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/base/rspfLine.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfNotifyContext.h>
#include <iterator>
using namespace std;


rspfActiveEdgeTable::rspfActiveEdgeTable()
{
   
}

void rspfActiveEdgeTable::addPolygon(const rspfPolygon& polygon)
{
   createTable(polygon);
   if(theTable.size())
   {
      rspf_int32 iNextY, iY;
      rspfActiveEdgeTableNode edge;
      rspf_uint32 numPts = polygon.getVertexCount();
      theLastScanLine = 0;
      
      for (rspf_uint32 i=0; i<numPts; ++i)
      {
         // CURRENT EDGE IS FROM polygon[i] to polygon[(i+1)%numPts]
         int iNext = (i+1)%numPts;  // INDEX FOR SECOND EDGE VERTEX
         iNextY = rspf::round<rspf_int32>(polygon[iNext].y-theRectOrigin.y);
         iY = rspf::round<rspf_int32>(polygon[i].y-theRectOrigin.y);
         int dy = (iNextY - iY);
         if (dy != 0)   // ADD ONLY IF EDGE NOT HORIZONTAL
         {
            if (dy> 0)  // local max
            {
               double m = (polygon[iNext].x-polygon[i].x)/
                          (polygon[iNext].y-polygon[i].y);
                                 
               edge = rspfActiveEdgeTableNode(iNextY+1,
                                               m,
                                               polygon[i].x);

               addEdgeToTable(edge, iY);
                                               
            }
            else         // local min so 
            {
               double m = (polygon[i].x-polygon[iNext].x)/
                          (polygon[i].y-polygon[iNext].y);

               edge = rspfActiveEdgeTableNode(iY,
                                               m,
                                               polygon[iNext].x);
               
               addEdgeToTable(edge, iNextY);
            }
            
         }
      }
   }
}

void rspfActiveEdgeTable::createTable(const rspfPolygon& polygon)
{
   theBoundingRect.makeNan();
   theRectOrigin.makeNan();
   if(polygon.getVertexCount() >2)
   {
      theTable.clear();
      polygon.getBoundingRect(theBoundingRect);
      if(!theBoundingRect.hasNans())
      {
         theTable.resize(theBoundingRect.height()+1);
         theRectOrigin = theBoundingRect.ul();
      }
   }   
}

void rspfActiveEdgeTable::addEdgeToTable(const rspfActiveEdgeTableNode& edge,
                                          rspf_int32 scanLine)
{
   theTable[scanLine].push_back(edge);
}

void rspfActiveEdgeTable::initializeActiveList()
{
   int i = 0;
   for(i=0;i<(int)theTable.size();++i)
   {
      if(theTable[i].size())
      {
         theActiveList = theTable[i];
         theTable[i].clear();
         theCurrentScanLine = i;
         theActiveList.sort();
         return;
      }
   }
}

void rspfActiveEdgeTable::updateActiveEdges()
{
   std::list<rspfActiveEdgeTableNode>::iterator current = theActiveList.begin();

   while(current != theActiveList.end())
   {
      (*current).theCurrentX += (*current).theSlope;

      if((*current).theMaxY == theCurrentScanLine)
      {
         current = theActiveList.erase(current);
      }
      else
      {
         ++current;
      }
   }
};

void rspfActiveEdgeTable::mergeCurrentScanLine()
{
   if((theCurrentScanLine < (int)theTable.size())&&
      theTable[theCurrentScanLine].size())
   {
      theTable[theCurrentScanLine].sort();
      theActiveList.merge(theTable[theCurrentScanLine]);
      theActiveList.sort();

      theTable[theCurrentScanLine].clear();
   }
}

void rspfActiveEdgeTable::printActiveEdgeList()const
{
   copy(theActiveList.begin(),
        theActiveList.end(),
        ostream_iterator<rspfActiveEdgeTableNode>(rspfNotify(rspfNotifyLevel_INFO) << "->"));
   rspfNotify(rspfNotifyLevel_INFO) << "NULL\n";
}

rspfActiveEdgeTableHelper::rspfActiveEdgeTableHelper(rspfActiveEdgeTable* edgeTable)
   :theEdgeTable(edgeTable),
    theTableInitializedFlag(false)
{
}

bool rspfActiveEdgeTableHelper::advanceScanLine()
{
   if(!theEdgeTable) return false;
   
   if(!theTableInitializedFlag)
   {
      theEdgeTable->initializeActiveList();
      theTableInitializedFlag = true;
   }
   else
   {
      ++theEdgeTable->theCurrentScanLine;
      theEdgeTable->updateActiveEdges();
      theEdgeTable->mergeCurrentScanLine();
   }
   
   theIterator = theEdgeTable->theActiveList.begin();
   
   return (theEdgeTable->theActiveList.size());
}

bool rspfActiveEdgeTableHelper::getNextLineSegment(rspfIpt& start,
                                                    rspfIpt& end)
{
   if(!theEdgeTable) return false;
   if(!theTableInitializedFlag) advanceScanLine();
   
   if(theIterator == theEdgeTable->theActiveList.end())
   {
      return false;
   }
   else
   {
      int y = theEdgeTable->getCurrentScanLine()+getYShift();
      
      start = rspfIpt(rspf::round<int>((*theIterator).theCurrentX),
                       y);
      ++theIterator;
      if(theIterator == theEdgeTable->theActiveList.end())
      {
         end = start;
      }
      else
      {
         end = rspfIpt( rspf::round<int>((*theIterator).theCurrentX),
                         y);
         
         ++theIterator;
      }
   }

   if(start.x > end.x)
   {
      swap(start.x, end.x);
   }
   return true;
}
