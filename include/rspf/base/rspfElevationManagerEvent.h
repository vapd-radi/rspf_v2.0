#ifndef rspfElevationManagerEvent_HEADER
#define rspfElevationManagerEvent_HEADER

#include <rspf/base/rspfEvent.h>

class rspfObject;

class RSPFDLLEXPORT rspfElevationManagerEvent : public rspfEvent
{
 public:
   rspfElevationManagerEvent( rspfObject* object=NULL,
                                    long id=RSPF_EVENT_NULL_ID )
      : rspfEvent( object, id )
      {
      }
   rspfElevationManagerEvent( const rspfElevationManagerEvent& rhs )
      : rspfEvent( rhs )
      {
      }
   virtual rspfObject* dup() const
      {
         return new rspfElevationManagerEvent( *this );
      }

TYPE_DATA   
};

#endif
