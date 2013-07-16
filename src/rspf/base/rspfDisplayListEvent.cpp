#include <rspf/base/rspfDisplayListEvent.h>

RTTI_DEF1(rspfDisplayListEvent, "rspfDisplayListEvent", rspfEvent);

void rspfDisplayListEvent::addDisplay(void* display)
{
   std::map<void*, void*>::iterator iter = theDisplayMap.find(display);

   if(iter == theDisplayMap.end())
   {
      theDisplayMap.insert(std::make_pair(display, display));
   }
}

void* rspfDisplayListEvent::firstDisplay()
{
   theDisplayIterator = theDisplayMap.begin();

   if(theDisplayIterator != theDisplayMap.end())
   {
      return (*theDisplayIterator).first;
   }

   return NULL;
}

void* rspfDisplayListEvent::nextDisplay()
{
   ++theDisplayIterator;

   if(theDisplayIterator != theDisplayMap.end())
   {
      return (*theDisplayIterator).first;
   }

   return NULL;
}

void rspfDisplayListEvent::clearList()
{
   theDisplayMap.clear();
   theDisplayIterator = theDisplayMap.end();
}

