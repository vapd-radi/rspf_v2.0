//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  Scott Bortman
//
// Description:   Region Of Interest (ROI) Event
//
// $Id: rspfROIEvent.cpp 9094 2006-06-13 19:12:40Z dburken $
//----------------------------------------------------------------------------
#include <rspf/base/rspfROIEvent.h>
#include <rspf/base/rspfPolygon.h>
#include <rspf/base/rspfPolyLine.h>
#include <rspf/base/rspfIrect.h>

RTTI_DEF1(rspfROIEvent, "rspfROIEvent", rspfEvent);

rspfROIEvent::rspfROIEvent(rspfObject* object,
                             long id)
   : rspfEvent( object, id ),
     theType(RSPF_RECTANGLE_ROI),
     theMovingFlag(false)
   
{
}

rspfROIEvent::rspfROIEvent( const rspfROIEvent& rhs )
   : rspfEvent    ( rhs ),
     theType       ( rhs.theType ),
     theMovingFlag ( rhs.theMovingFlag)
{
}

rspfObject* rspfROIEvent::dup() const
{
   return new rspfROIEvent( *this );
}

const vector<rspfIpt>& rspfROIEvent::getPoints()const
{
   return thePoints;
}

void rspfROIEvent::getPoint(vector<rspfIpt>& points)const 
{
   points = thePoints;
}

void rspfROIEvent::setPoints( const vector<rspfIpt>& points )
{
   thePoints = points;
}

void rspfROIEvent::setRect(const rspfIrect& rect)
{
   thePoints.clear();
   thePoints.push_back(rect.ul());
   thePoints.push_back(rect.lr());
}

void rspfROIEvent::setEventType( rspfRegionType eventType )
{
   theType = eventType;
}

rspfROIEvent::rspfRegionType rspfROIEvent::getEventType()const
{
   return theType;
}

void rspfROIEvent::setTypeToPolygon()
{
   theType = RSPF_POLYGON_ROI;
}

void rspfROIEvent::setTypeToRectangle()
{
   theType = RSPF_RECTANGLE_ROI;
}

void rspfROIEvent::setTypeToPolyline()
{
   theType = RSPF_POLYLINE_ROI;
}

bool rspfROIEvent::isRectangleRegion()const
{
   return (theType == RSPF_RECTANGLE_ROI);
}

bool rspfROIEvent::isPolygonRegion()const
{
   return (theType == RSPF_POLYGON_ROI);
}

bool rspfROIEvent::isPolylineRegion()const
{
   return (theType == RSPF_POLYLINE_ROI);
}

void rspfROIEvent::getRect(rspfIrect& rect)const
{
   rect = rspfIrect(thePoints);
}

void rspfROIEvent::getPolygon(rspfPolygon& polygon)const
{
   polygon = rspfPolygon(thePoints);
}

void rspfROIEvent::getPolyLine(rspfPolyLine& polyline)const
{
   polyline = rspfPolyLine(thePoints);
}

void rspfROIEvent::setMovingFlag(bool flag)
{
   theMovingFlag = flag;
}

bool rspfROIEvent::getMovingFlag() const
{
   return theMovingFlag;
}
