#ifndef rspfROIEventListener_HEADER
#define rspfROIEventListener_HEADER

#include <rspf/base/rspfListener.h>
#include <rspf/base/rspfROIEvent.h>
#include <rspf/base/rspfROIEvent.h>

class RSPFDLLEXPORT rspfROIEventListener : public rspfListener
{
public:
   rspfROIEventListener()
      {
      }
   virtual void processEvent(rspfEvent& event);
   virtual void handleRectangleROIEvent( rspfROIEvent& event );
   virtual void handlePolygonROIEvent( rspfROIEvent& event );
   virtual void handlePolylineROIEvent( rspfROIEvent& event );

TYPE_DATA   
};

#endif
