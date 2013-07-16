#ifndef rspfDisplayListEvent_HEADER
#define rspfDisplayListEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <map>

class RSPFDLLEXPORT rspfDisplayListEvent : public rspfEvent
{
public:
   rspfDisplayListEvent(rspfObject* obj = NULL)
      :rspfEvent(obj, RSPF_EVENT_DISPLAY_LIST_ID)
      {
         theDisplayIterator = theDisplayMap.end();
      }
   void addDisplay(void* display);
   virtual rspfObject* dup()const
  {
    return new rspfDisplayListEvent(*this);
  }
   
   void* firstDisplay();
   void* nextDisplay();
   void  clearList();
   
protected:
   std::map<void*, void*> theDisplayMap;
   std::map<void*, void*>::iterator theDisplayIterator;


TYPE_DATA
};

#endif
