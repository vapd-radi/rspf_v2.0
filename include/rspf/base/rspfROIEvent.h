//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Scott Bortman
//
// Description:   Region Of Interest (ROI) Event
//
// $Id: rspfROIEvent.h 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------
#ifndef rspfROIEvent_HEADER
#define rspfROIEvent_HEADER

#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfIpt.h>
#include <vector>

class rspfPolygon;
class rspfPolyLine;
class rspfIrect;
class RSPFDLLEXPORT rspfROIEvent : public rspfEvent
{
public:
   rspfROIEvent( rspfObject* object=NULL,
                  long id=RSPF_EVENT_NULL_ID );
   rspfROIEvent( const rspfROIEvent& rhs );
   virtual rspfObject* dup() const;
   
   enum rspfRegionType
   {
      RSPF_RECTANGLE_ROI      = 0,
      RSPF_POLYGON_ROI        = 1,
      RSPF_POLYLINE_ROI       = 2
   };

   const std::vector<rspfIpt>& getPoints()const;
   void getPoint(std::vector<rspfIpt>& points)const ;
   void setPoints( const std::vector<rspfIpt>& points );
   void setRect(const rspfIrect& rect);
   void setEventType( rspfRegionType eventType );
   rspfRegionType getEventType()const;
   void setTypeToPolygon();
   void setTypeToRectangle();
   void setTypeToPolyline();
   bool isRectangleRegion()const;
   bool isPolygonRegion()const;
   bool isPolylineRegion()const;
   void getRect(rspfIrect& rect)const;
   void getPolygon(rspfPolygon& polygon)const;
   void getPolyLine(rspfPolyLine& polyline)const;

   /**
    * @param flag If true indicates region of interest is moving and size is
    * not to change.
    */
   void setMovingFlag(bool flag);

   /**
    * @return Returns theMoving flag.  If true indicates region of interest
    * is moving and size is not to change.
    */
   bool getMovingFlag() const;
   
 protected:
   std::vector<rspfIpt> thePoints;
   rspfRegionType  theType;

   // Indicates moving so keep the same size on a redraw.
   bool             theMovingFlag;

TYPE_DATA
};

#endif
