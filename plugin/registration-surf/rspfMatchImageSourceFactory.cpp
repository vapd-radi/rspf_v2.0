#include "rspfMatchImageSourceFactory.h"
#include <rspf/base/rspfTrace.h>
#include "rspfSurfMatch.h"

#include <rspf/base/rspfObjectFactoryRegistry.h>

RTTI_DEF1(rspfMatchImageSourceFactory, "rspfMatchImageSourceFactory", rspfImageSourceFactoryBase);
rspfMatchImageSourceFactory* rspfMatchImageSourceFactory::theInstance = 0;

static rspfTrace traceDebug("rspfMatchImageSourceFactory:debug");

rspfMatchImageSourceFactory::rspfMatchImageSourceFactory()
{
   theInstance = this;
}

rspfMatchImageSourceFactory::rspfMatchImageSourceFactory(const rspfMatchImageSourceFactory&)
{
}

const rspfMatchImageSourceFactory& rspfMatchImageSourceFactory::operator=(rspfMatchImageSourceFactory&)
{
   return *this;
}

rspfMatchImageSourceFactory::~rspfMatchImageSourceFactory()
{
   theInstance = 0;
   rspfObjectFactoryRegistry::instance()->unregisterFactory(this);
}

rspfMatchImageSourceFactory* rspfMatchImageSourceFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfMatchImageSourceFactory;
   }

   return theInstance;
}

rspfObject* rspfMatchImageSourceFactory::createObject(const rspfString& name)const
{
   if(name == STATIC_TYPE_NAME(rspfSurfMatch))
   {
      return new rspfSurfMatch;
   }

   return 0;
}

rspfObject* rspfMatchImageSourceFactory::createObject(const rspfKeywordlist& kwl,
                                                               const char* prefix)const
{
   static const char* MODULE = "rspfImageSourceFactory::createSource";
   
   rspfString copyPrefix = prefix;
   rspfObject* result = NULL;
   
   if(traceDebug())
   {
      CLOG << "looking up type keyword for prefix = " << copyPrefix << endl;
   }

   const char* lookup = kwl.find(copyPrefix, "type");
   if(lookup)
   {
      rspfString name = lookup;
      result           = createObject(name);
      
      if(result)
      {
         if(traceDebug())
         {
            CLOG << "found source " << result->getClassName() << " now loading state" << endl;
         }
         result->loadState(kwl, copyPrefix.c_str());
      }
      else
      {
         if(traceDebug())
         {
            CLOG << "type not found " << lookup << endl;
         }
      }
   }
   else
   {
      if(traceDebug())
      {
         CLOG << "type keyword not found" << endl;
      }
   }
   return result;
}

void rspfMatchImageSourceFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfSurfMatch));
}
