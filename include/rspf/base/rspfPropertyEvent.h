#ifndef rspfPropertyEvent_HEADER
#define rspfPropertyEvent_HEADER
#include <rspf/base/rspfEvent.h>

class RSPFDLLEXPORT rspfPropertyEvent : public rspfEvent
{
public:
   rspfPropertyEvent(rspfObject* object) // the event id
      :rspfEvent(object, RSPF_EVENT_PROPERTY_ID)
      {}
   virtual rspfObject* dup()const{return new rspfPropertyEvent(*this);}
};

#endif
