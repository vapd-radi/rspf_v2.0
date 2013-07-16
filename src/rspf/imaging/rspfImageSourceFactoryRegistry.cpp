//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfImageSourceFactoryRegistry.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <algorithm>
#include <rspf/imaging/rspfImageSourceFactoryRegistry.h>
#include <rspf/imaging/rspfImageSourceFactory.h>
#include <rspf/imaging/rspfImageReconstructionFilterRegistry.h>
#include <rspf/base/rspfString.h>

rspfImageSourceFactoryRegistry* rspfImageSourceFactoryRegistry::theInstance = NULL;

RTTI_DEF1(rspfImageSourceFactoryRegistry, "rspfImageSourceFactoryRegistry", rspfImageSourceFactoryBase);

rspfImageSourceFactoryRegistry::rspfImageSourceFactoryRegistry()
{
   theInstance = this;
}

rspfImageSourceFactoryRegistry::~rspfImageSourceFactoryRegistry()
{
   theInstance->unregisterFactory(rspfImageSourceFactory::instance());
   
   theInstance = NULL;
}

rspfImageSourceFactoryRegistry* rspfImageSourceFactoryRegistry::instance()
{
  if(theInstance == NULL)
  {
    theInstance = new rspfImageSourceFactoryRegistry;
    theInstance->registerFactory(rspfImageSourceFactory::instance());
    theInstance->registerFactory(rspfImageReconstructionFilterRegistry::instance());
  }
  return theInstance;
}

rspfObject* rspfImageSourceFactoryRegistry::createObject(const rspfString& name)const
{
   rspfObject*                   result = NULL;
   std::vector<rspfImageSourceFactoryBase*>::const_iterator factory;

   factory = theFactoryList.begin();
   while((factory != theFactoryList.end()) && !result)
   {
     result = (*factory)->createObject(name);
     ++factory;
   }
   
   return result;
}

rspfObject* rspfImageSourceFactoryRegistry::createObject(const rspfKeywordlist& kwl,
							   const char* prefix)const
{
   rspfObject*                   result = NULL;
   std::vector<rspfImageSourceFactoryBase*>::const_iterator factory;

   factory = theFactoryList.begin();
   while((factory != theFactoryList.end()) && !result)
   {
     result = (*factory)->createObject(kwl, prefix);
     ++factory;
   }
   
   return result;
 
}

void rspfImageSourceFactoryRegistry::getTypeNameList(std::vector<rspfString>& typeList)const
{
   std::vector<rspfString> result;
   std::vector<rspfImageSourceFactoryBase*>::const_iterator iter = theFactoryList.begin();

   while(iter != theFactoryList.end())
   {
      result.clear();
      (*iter)->getTypeNameList(result);

      // now append to the end of the typeList.
      typeList.insert(typeList.end(),
                      result.begin(),
                      result.end());
      ++iter;
   }
  
}

void rspfImageSourceFactoryRegistry::registerFactory(rspfImageSourceFactoryBase* factory)
{
  if(factory&&!findFactory(factory))
  {
     theFactoryList.push_back(factory);
  }
}

void rspfImageSourceFactoryRegistry::unregisterFactory(rspfImageSourceFactoryBase* factory)
{
   std::vector<rspfImageSourceFactoryBase*>::iterator iter =  std::find(theFactoryList.begin(),
                                                                          theFactoryList.end(),
                                                                          factory);
   if(iter != theFactoryList.end())
   {
      theFactoryList.erase(iter);
   }
}

bool rspfImageSourceFactoryRegistry::findFactory(rspfImageSourceFactoryBase* factory)const
{
   return (std::find(theFactoryList.begin(),
                     theFactoryList.end(),
                     factory)!=theFactoryList.end());
}

void* rspfImageSourceFactoryRegistryGetInstance()
{
  return rspfImageSourceFactoryRegistry::instance();
}

// Hidden
rspfImageSourceFactoryRegistry::rspfImageSourceFactoryRegistry(const rspfImageSourceFactoryRegistry&)
   : rspfImageSourceFactoryBase()
{}

// Hidden
void rspfImageSourceFactoryRegistry::operator=(rspfImageSourceFactoryRegistry&)
{}
