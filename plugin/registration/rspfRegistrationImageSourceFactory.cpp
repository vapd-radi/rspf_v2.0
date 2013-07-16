#include "rspfRegistrationImageSourceFactory.h"
#include <rspf/base/rspfTrace.h>
#include "rspfChipMatch.h"
#include "rspfDensityReducer.h"
#include "rspfExtremaFilter.h"
#include "rspfHarrisCorners.h"
#include "rspfMultiplier.h"
#include "rspfSquareFunction.h"

#include <rspf/base/rspfObjectFactoryRegistry.h>

RTTI_DEF1(rspfRegistrationImageSourceFactory, "rspfRegistrationImageSourceFactory", rspfImageSourceFactoryBase);
rspfRegistrationImageSourceFactory* rspfRegistrationImageSourceFactory::theInstance = 0;

static rspfTrace traceDebug("rspfRegistrationImageSourceFactory:debug");

rspfRegistrationImageSourceFactory::rspfRegistrationImageSourceFactory()
{
   theInstance = this;
}

rspfRegistrationImageSourceFactory::rspfRegistrationImageSourceFactory(const rspfRegistrationImageSourceFactory&)
{
}

const rspfRegistrationImageSourceFactory& rspfRegistrationImageSourceFactory::operator=(rspfRegistrationImageSourceFactory&)
{
   return *this;
}

rspfRegistrationImageSourceFactory::~rspfRegistrationImageSourceFactory()
{
   theInstance = 0;
   rspfObjectFactoryRegistry::instance()->unregisterFactory(this);
}

rspfRegistrationImageSourceFactory* rspfRegistrationImageSourceFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfRegistrationImageSourceFactory;
   }

   return theInstance;
}

rspfObject* rspfRegistrationImageSourceFactory::createObject(const rspfString& name)const
{
   if(name == STATIC_TYPE_NAME(rspfChipMatch))
   {
      return new rspfChipMatch;
   }
   else if(name == STATIC_TYPE_NAME(rspfDensityReducer))
   {
      return new rspfDensityReducer;
   }
   else if(name == STATIC_TYPE_NAME(rspfExtremaFilter))
   {
      return new rspfExtremaFilter;
   }
   else if(name == STATIC_TYPE_NAME(rspfHarrisCorners))
   {
      return new rspfHarrisCorners;
   }
   else if(name == STATIC_TYPE_NAME(rspfMultiplier))
   {
      return new rspfMultiplier;
   }
   else if(name == STATIC_TYPE_NAME(rspfSquareFunction))
   {
      return new rspfSquareFunction;
   }

   return 0;
}

rspfObject* rspfRegistrationImageSourceFactory::createObject(const rspfKeywordlist& kwl,
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

void rspfRegistrationImageSourceFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfChipMatch));
   typeList.push_back(STATIC_TYPE_NAME(rspfDensityReducer));
   typeList.push_back(STATIC_TYPE_NAME(rspfExtremaFilter));
   typeList.push_back(STATIC_TYPE_NAME(rspfHarrisCorners));
   typeList.push_back(STATIC_TYPE_NAME(rspfMultiplier));
   typeList.push_back(STATIC_TYPE_NAME(rspfSquareFunction));
}
