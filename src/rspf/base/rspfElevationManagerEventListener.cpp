#include <rspf/base/rspfElevationManagerEventListener.h>
#include <rspf/base/rspfElevationManagerEvent.h>

RTTI_DEF1(rspfElevationManagerEventListener, "rspfElevationManagerEventListener",
          rspfListener);

rspfElevationManagerEventListener::rspfElevationManagerEventListener()
      : rspfListener()
{
}


rspfElevationManagerEventListener::~rspfElevationManagerEventListener()
{
}

void rspfElevationManagerEventListener::processEvent( rspfEvent& event )
{
   rspfElevationManagerEvent* elevationManagerEvent =
      PTR_CAST( rspfElevationManagerEvent, &event );

   if ( elevationManagerEvent )
   {
      processEvent( *elevationManagerEvent );
   }
}

void rspfElevationManagerEventListener::processEvent( rspfElevationManagerEvent& /* event */)
{

}
