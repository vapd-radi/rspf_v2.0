#ifndef rspfDisplayRefreshEvent_HEADER
#define rspfDisplayRefreshEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <map>

class RSPFDLLEXPORT rspfDisplayRefreshEvent : public rspfEvent
{
public:
   rspfDisplayRefreshEvent(rspfObject* obj = NULL)
      :rspfEvent(obj, RSPF_EVENT_DISPLAY_REFRESH_ID)
      {}

TYPE_DATA
};

#endif
