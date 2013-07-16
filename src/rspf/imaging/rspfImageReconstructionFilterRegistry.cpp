//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfImageReconstructionFilterRegistry.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <algorithm>
#include <vector>
#include <rspf/imaging/rspfImageReconstructionFilterRegistry.h>
#include <rspf/imaging/rspfImageReconstructionFilterFactory.h>
#include <rspf/base/rspfString.h>

rspfImageReconstructionFilterRegistry* rspfImageReconstructionFilterRegistry::theInstance = 0;

RTTI_DEF1(rspfImageReconstructionFilterRegistry, "rspfImageReconstructionFilterRegistry", rspfImageSourceFactoryBase);

rspfImageReconstructionFilterRegistry::rspfImageReconstructionFilterRegistry()
{
   theInstance = this;
}

rspfImageReconstructionFilterRegistry::~rspfImageReconstructionFilterRegistry()
{
   theInstance = NULL;
}

rspfImageReconstructionFilterRegistry* rspfImageReconstructionFilterRegistry::instance()
{
  if(theInstance == NULL)
  {
     theInstance = new rspfImageReconstructionFilterRegistry;
     theInstance->registerFactory(rspfImageReconstructionFilterFactory::instance());
  }
  return theInstance;
}

rspfObject* rspfImageReconstructionFilterRegistry::createObject(const rspfString& name)const
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

rspfObject* rspfImageReconstructionFilterRegistry::createObject(const rspfKeywordlist& kwl,
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

void rspfImageReconstructionFilterRegistry::getTypeNameList(std::vector<rspfString>& typeList)const
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

void rspfImageReconstructionFilterRegistry::registerFactory(rspfImageSourceFactoryBase* factory)
{
  if(factory&&!findFactory(factory))
  {
     theFactoryList.push_back(factory);
  }
}

void rspfImageReconstructionFilterRegistry::unregisterFactory(rspfImageSourceFactoryBase* factory)
{
   std::vector<rspfImageSourceFactoryBase*>::iterator iter =  std::find(theFactoryList.begin(),
                                                                          theFactoryList.end(),
                                                                          factory);
   if(iter != theFactoryList.end())
   {
      theFactoryList.erase(iter);
   }
}

bool rspfImageReconstructionFilterRegistry::findFactory(rspfImageSourceFactoryBase* factory)const
{
   return (std::find(theFactoryList.begin(),
                     theFactoryList.end(),
                     factory)!=theFactoryList.end());
}
