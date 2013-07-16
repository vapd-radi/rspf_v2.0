/* -*-c++-*- libwms - Copyright (C) since 2004 Garrett Potts
*/
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfNotifyContext.h>

rspfReferenced::~rspfReferenced()
{
   if(theRefMutex)
   {
      delete theRefMutex;
      theRefMutex = 0;
   }
   if (theRefCount>0)
   {
      rspfNotify(rspfNotifyLevel_WARN)<<"Warning: deleting still referenced object "<<this<<std::endl;
      rspfNotify(rspfNotifyLevel_WARN)<<"         the final reference count was "<<theRefCount
                                        <<", memory corruption possible."<<std::endl;
   }
}
