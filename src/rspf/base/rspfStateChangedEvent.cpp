#include <rspf/base/rspfStateChangedEvent.h>

rspfStateChangedEvent::rspfStateChangedEvent( rspfObject* object,
                                                 long id )
   : rspfEvent( object, id )
{
}

void rspfStateChangedEvent::setNewKeywordlist( rspfKeywordlist kwl )
{
   theNewKeywordlist = kwl;
}

rspfKeywordlist rspfStateChangedEvent::getNewKeywordlist()
{
   return theNewKeywordlist;
}


void rspfStateChangedEvent::setOldKeywordlist( rspfKeywordlist kwl )
{
   theOldKeywordlist = kwl;
}

rspfKeywordlist rspfStateChangedEvent::getOldKeywordlist()
{
   return theOldKeywordlist;
}

void rspfStateChangedEvent::setObjId( std::string objId )
{
   theObjId = objId;
}

std::string rspfStateChangedEvent::getObjId()
{
   return theObjId;
}
