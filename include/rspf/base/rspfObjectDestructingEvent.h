#ifndef rspfObjectDestructingEvent_HEADER
#define rspfObjectDestructingEvent_HEADER

#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfEventIds.h>

class rspfObject;

class RSPFDLLEXPORT rspfObjectDestructingEvent : public rspfEvent
{
public:
   rspfObjectDestructingEvent(rspfObject* objectDestructing=NULL)
      :rspfEvent(objectDestructing, RSPF_EVENT_OBJECT_DESTRUCTING_ID)
      {}
   virtual rspfObject* dup()const{return new rspfObjectDestructingEvent(*this);}

TYPE_DATA
};
#endif
