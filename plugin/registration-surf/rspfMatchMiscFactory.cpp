#include "rspfMatchMiscFactory.h"
#include "rspfSurfMatch.h"
#include <rspf/base/rspfKeywordNames.h>

RTTI_DEF1(rspfMatchMiscFactory, "rspfMatchMiscFactory", rspfObjectFactory);

rspfMatchMiscFactory* rspfMatchMiscFactory::theInstance = NULL;

rspfMatchMiscFactory* rspfMatchMiscFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfMatchMiscFactory;
   }

   return theInstance;
}

rspfObject* rspfMatchMiscFactory::createObject(const rspfString& typeName)const
{
   if(typeName == STATIC_TYPE_NAME(rspfSurfMatch))
   {
      return new rspfSurfMatch;
   }

   return (rspfObject*)NULL;
}

rspfObject* rspfMatchMiscFactory::createObject(const rspfKeywordlist& kwl,
                                                  const char* prefix)const
{
   const char* type = kwl.find(prefix,
                               rspfKeywordNames::TYPE_KW);
   rspfString copyPrefix = prefix;
   if(type)
   {
      rspfObject* object = createObject(rspfString(type));
      if(object)
      {
         object->loadState(kwl, copyPrefix.c_str());
      }
      return object;
   }
   return NULL;  
}

void rspfMatchMiscFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfSurfMatch));
}
