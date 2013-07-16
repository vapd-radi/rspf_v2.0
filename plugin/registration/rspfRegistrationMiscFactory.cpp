#include "rspfRegistrationMiscFactory.h"
#include "rspfImageCorrelator.h"
#include "rspfOutlierRejection.h"
#include "rspfModelOptimizer.h"
#include <rspf/base/rspfKeywordNames.h>

RTTI_DEF1(rspfRegistrationMiscFactory, "rspfRegistrationMiscFactory", rspfObjectFactory);

rspfRegistrationMiscFactory* rspfRegistrationMiscFactory::theInstance = NULL;

rspfRegistrationMiscFactory* rspfRegistrationMiscFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfRegistrationMiscFactory;
   }

   return theInstance;
}

rspfObject* rspfRegistrationMiscFactory::createObject(const rspfString& typeName)const
{
   if(typeName == STATIC_TYPE_NAME(rspfImageCorrelator))
   {
      return new rspfImageCorrelator;
   }
   if(typeName == STATIC_TYPE_NAME(rspfOutlierRejection))
   {
      return new rspfOutlierRejection;
   }
   if(typeName == STATIC_TYPE_NAME(rspfModelOptimizer))
   {
      return new rspfModelOptimizer;
   }

   return (rspfObject*)NULL;
}

rspfObject* rspfRegistrationMiscFactory::createObject(const rspfKeywordlist& kwl,
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

void rspfRegistrationMiscFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfImageCorrelator));
   typeList.push_back(STATIC_TYPE_NAME(rspfOutlierRejection));
   typeList.push_back(STATIC_TYPE_NAME(rspfModelOptimizer));
}
