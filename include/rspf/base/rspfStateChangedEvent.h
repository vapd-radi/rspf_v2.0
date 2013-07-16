#ifndef rspfStateChangedEvent_HEADER
#define rspfStateChangedEvent_HEADER

#include <string>

#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>

class RSPFDLLEXPORT rspfStateChangedEvent : public rspfEvent
{
public:
   rspfStateChangedEvent( rspfObject* object=NULL,
                           long id=RSPF_EVENT_STATE_CHANGED_ID );

   void setObjId( std::string objId );
   void setNewKeywordlist( rspfKeywordlist kwl );
   void setOldKeywordlist( rspfKeywordlist kwl );

   std::string getObjId();
   rspfKeywordlist getNewKeywordlist();
   rspfKeywordlist getOldKeywordlist();

   // HACK - Need to set keywordlists too!!!
   virtual rspfObject* dup()const{return new rspfStateChangedEvent(*this);}     

private:
   std::string theObjId;
   rspfKeywordlist theNewKeywordlist;
   rspfKeywordlist theOldKeywordlist;
};

#endif
