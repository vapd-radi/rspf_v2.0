#include <rspf/base/rspfROIEventListener.h>
#include <rspf/base/rspfROIEvent.h>


RTTI_DEF1(rspfROIEventListener, "rspfROIEventListener", rspfListener);

void rspfROIEventListener::processEvent(rspfEvent& event)
{
   if(event.isConsumed()) return;
   
   rspfROIEvent* roiEvent = PTR_CAST(rspfROIEvent,
                                      &event);
   
   if(roiEvent)
   {
      const int type = roiEvent->getEventType();
      
      switch ( type )
      {
      case rspfROIEvent::RSPF_RECTANGLE_ROI:
         handleRectangleROIEvent( *roiEvent );
         break;

      case rspfROIEvent::RSPF_POLYGON_ROI:
         handlePolygonROIEvent( *roiEvent );
         break;


      case rspfROIEvent::RSPF_POLYLINE_ROI:
         handlePolylineROIEvent( *roiEvent );
         break;

         default:
         break;
      }
   }
}

void rspfROIEventListener::handleRectangleROIEvent( rspfROIEvent& /* event */)
{
}

void rspfROIEventListener::handlePolygonROIEvent( rspfROIEvent& /* event */)
{
}

void rspfROIEventListener::handlePolylineROIEvent( rspfROIEvent& /* event */)
{
}
