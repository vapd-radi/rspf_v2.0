//*****************************************************************************
//
// License:  See top level LICENSE.txt file.
//
// AUTHOR: Garrett Potts
//
//*****************************************************************************
//  $Id: rspfActiveEdgeTable.h 14799 2009-06-30 08:54:44Z dburken $
#include <iostream>
#include <vector>
#include <list>
#include <rspf/base/rspfIrect.h>

class rspfPolygon;


class RSPFDLLEXPORT rspfActiveEdgeTableNode
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfActiveEdgeTableNode& rhs)
      {
          return out <<"| maxy="<<rhs.theMaxY<<" |dx="
                     <<rhs.theSlope<<" |"<<"x="
                     <<rhs.theCurrentX << "|"<< std::endl;
//          return out <<"|dy="<<rhs.theDeltaY<<" |dx="
//                     <<rhs.theSlope<<" |"<<"x="
//                     <<rhs.theCurrentX << "|"<<endl;
      }
   rspfActiveEdgeTableNode(rspf_int32 maxY      = 0,
                            double slope    = 0.0,
                            double currentX = 0.0)
//       :theDeltaY(deltaY),
      :theMaxY(maxY),
       theSlope(slope),
       theCurrentX(currentX)
      {
      }
   bool operator()(const rspfActiveEdgeTableNode& left,
                 const rspfActiveEdgeTableNode& right)const
      {
         return (left.theCurrentX < right.theCurrentX);
      }
   bool operator<(const rspfActiveEdgeTableNode& right)const
      {
         return (theCurrentX < right.theCurrentX);
      }
   bool operator>(const rspfActiveEdgeTableNode& right)const
      {
         return (theCurrentX > right.theCurrentX);
      }
                            
   rspf_int32  theMaxY;
//   rspf_int32  theDeltaY;
   double theSlope; 
   double theCurrentX;
};

class rspfActiveEdgeTableHelper;
/*!
 * This class is used in fast efficient scanliine rasterization.  It will take a polygon and insert
 * it into the table sorted by y's
 */
class rspfActiveEdgeTable
{
public:
   friend class rspfActiveEdgeTableHelper;
   rspfActiveEdgeTable();

   /*!
    * Currently will only work on a single polygon.  If you call this
    * twice it currently will use the last called polygon and
    * will wipe out the previous one.
    * 
    * Note: this is used for scanline rasterization and will round
    * to integer values all vertices as they are initially added
    * to the Active Edge Table.
    */
   void addPolygon(const rspfPolygon& polygon);

   const std::list<rspfActiveEdgeTableNode>& getActiveList()const;
   
//   bool getNextScanline(list<rspfActiveEdgeTable>::const_iterator& iterator)const;
   
   rspf_int32 getCurrentScanLine()const
      {
         return theCurrentScanLine;
      }
   rspf_int32 getYShift()const
      {
         return theRectOrigin.y;
      }
   
   void initializeActiveList();
   void updateActiveEdges();
   void mergeCurrentScanLine();
   void printActiveEdgeList()const;
   
protected:
   std::vector< std::list<rspfActiveEdgeTableNode> > theTable;
   std::list<rspfActiveEdgeTableNode> theActiveList;
   
   void createTable(const rspfPolygon& polygon);
   void addEdgeToTable(const rspfActiveEdgeTableNode& edge,
                       rspf_int32 scanLine);
   /*!
    * Used in computing the number of scanlines of the passed in polygon and
    * is also used to shift the y's relative to 0,0
    */
   rspfIrect  theBoundingRect;
   rspfIpt    theRectOrigin;
   rspf_int32 theLastScanLine;
   rspf_int32 theCurrentScanLine;
};

class rspfActiveEdgeTableHelper
{
public:
   rspfActiveEdgeTableHelper(rspfActiveEdgeTable* edgeTable);

   bool advanceScanLine();

   bool getNextLineSegment(rspfIpt& start,
                           rspfIpt& end);
   
   rspf_int32 getCurrentScanLine()const
      {
         if(theEdgeTable)
         {
            return theEdgeTable->getCurrentScanLine();
         }
         return 0;
      }
   rspf_int32 getYShift()const
      {
         if(theEdgeTable)
         {
            return theEdgeTable->getYShift();
         }
         return 0;         
      }
protected:
   rspfActiveEdgeTable* theEdgeTable;
   bool theTableInitializedFlag;

   std::list<rspfActiveEdgeTableNode>::const_iterator theIterator;
};
