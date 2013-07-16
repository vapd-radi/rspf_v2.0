#ifndef rspfElevationManagerEventListener_HEADER
#define ossmiElevationManagerEventListener_HEADER

#include <rspf/base/rspfListener.h>
#include <rspf/base/rspfElevationManagerEvent.h>

class RSPFDLLEXPORT rspfElevationManagerEventListener : public rspfListener
{
 public:
   rspfElevationManagerEventListener();
   virtual ~rspfElevationManagerEventListener();
   virtual void processEvent( rspfEvent& event );
   virtual void processEvent( rspfElevationManagerEvent& event );

TYPE_DATA
};

#endif
