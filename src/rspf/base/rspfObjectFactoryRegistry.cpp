//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfObjectFactoryRegistry.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/base/rspfObjectFactoryRegistry.h>

RTTI_DEF1(rspfObjectFactoryRegistry, "rspfObjectFactoryRegistry", rspfObject);
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/base/rspfBaseObjectFactory.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfRefPtr.h>
using namespace std;


rspfObjectFactoryRegistry::rspfObjectFactoryRegistry()
{
}

rspfObjectFactoryRegistry::~rspfObjectFactoryRegistry()
{
}

rspfObjectFactoryRegistry* rspfObjectFactoryRegistry::instance()
{
   static rspfObjectFactoryRegistry sharedInstance;
   
   return &sharedInstance;
}

#if 0
bool rspfObjectFactoryRegistry::addFactory(rspfObjectFactory* factory)
{
   return registerFactory(factory);
}

bool rspfObjectFactoryRegistry::registerFactory(rspfObjectFactory* factory)
{
   if(factory)
   {
      if(!findFactory(factory))
      {
         theFactoryList.push_back(factory);
         return true;
      }
      else
      {
         return true;
      }
   }
   
   return false;
}

void rspfObjectFactoryRegistry::unregisterFactory(rspfObjectFactory* factory)
{
   vector<rspfObjectFactory*>::iterator iter = theFactoryList.begin();

   while(iter != theFactoryList.end())
   {
      if( (*iter) == factory)
      {
         theFactoryList.erase(iter);
         return;
      }
      ++iter;
   }
}

rspfObject* rspfObjectFactoryRegistry::createObject(const rspfString& name)const
{
   rspfObject* result = NULL;
   unsigned long index = 0;
   
   while((index < theFactoryList.size()) &&(!result))
   {
      result = theFactoryList[index]->createObject(name);
      ++index;
   }
   return result;
}

rspfObject* rspfObjectFactoryRegistry::createObject(const rspfKeywordlist& kwl,
                                                      const char* prefix)const
{
   rspfObject* result = NULL;
   unsigned long index = 0;

   while((index < theFactoryList.size()) &&(!result))
   {
      result = theFactoryList[index]->createObject(kwl,
                                                   prefix);
      ++index;
   }

   return result;
}

void rspfObjectFactoryRegistry::getTypeNameList(std::vector<rspfString>& typeList)const
{
   vector<rspfString> result;
   vector<rspfObjectFactory*>::const_iterator iter = theFactoryList.begin();

   while(iter != theFactoryList.end())
   {
      result.clear();
      (*iter)->getTypeNameList(result);

      typeList.insert(typeList.end(),
                      result.begin(),
                      result.end());
      ++iter;
   }
}

rspfObjectFactory* rspfObjectFactoryRegistry::findFactory(rspfObjectFactory* factory)const
{
   vector<rspfObjectFactory*>::const_iterator iter = theFactoryList.begin();

   while(iter != theFactoryList.end())
   {
      if( (*iter) == factory)
      {
         return factory;
      }
      ++iter;
   }

   return NULL;
}
#endif

void rspfObjectFactoryRegistry::getTypeNameList(std::vector<rspfString>& typeList,
                                                 const rspfString& baseType)const
{
   vector<rspfString> allTypeList;

   getTypeNameList(allTypeList);

   if(baseType == "")
   {
      typeList.insert(typeList.end(),
                      allTypeList.begin(),
                      allTypeList.end());
   }
   else
   {
      int i = 0;
      rspfRefPtr<rspfObject> obj;
      for(i = 0; i < (int)allTypeList.size(); ++i)
      {
         obj = createObject(allTypeList[i]);
         if(obj.valid())
         {
            if(obj->canCastTo(baseType))
            {
               typeList.push_back(allTypeList[i]);
            }
         }
      }
   }
}


void* rspfObjectFactoryRegistryGetInstance()
{
  return rspfObjectFactoryRegistry::instance();
}
