#include <rspf/base/rspfIdManager.h>
#include <time.h>
#ifndef NULL
#include <stddef.h>
#endif


rspf_int64 rspfIdManager::theCurrentId           = rspfId::INVALID_ID;
rspfIdManager* rspfIdManager::theInstance = NULL;

rspfIdManager::rspfIdManager()
{
   theCurrentId=rspfId::INVALID_ID;
}

rspfIdManager::rspfIdManager(const rspfIdManager& /* rhs */)
{
   theCurrentId=rspfId::INVALID_ID;
}

rspfIdManager::~rspfIdManager()
{
   theCurrentId=rspfId::INVALID_ID;   
}

rspfIdManager* rspfIdManager::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfIdManager;
      theCurrentId = -1;
   }

   return theInstance;
}

rspfId rspfIdManager::generateId()
{
   return rspfId(++theCurrentId);
}

rspfId rspfIdManager::generateId(rspf_int64 customId)
{
   if(customId <= theCurrentId)
   {
      return rspfId(++theCurrentId);
   }
   theCurrentId = customId;
   return rspfId(theCurrentId);
}

void rspfIdManager::setCurrentId(rspf_int64 currentId)
{
   theCurrentId = (currentId >=0) ? currentId: theCurrentId;
}

void  rspfIdManager::operator=(const rspfIdManager& /* rhs */)
{}
