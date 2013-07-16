//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfCustomEditorWindowRegistry.cpp 17108 2010-04-15 21:08:06Z dburken $
#include <rspf/base/rspfCustomEditorWindowRegistry.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfCustomEditorWindowFactoryBase.h>
rspfCustomEditorWindowRegistry* rspfCustomEditorWindowRegistry::theInstance=0;

RTTI_DEF(rspfCustomEditorWindowRegistry, "rspfCustomEditorWindowRegistry");

rspfCustomEditorWindowRegistry::~rspfCustomEditorWindowRegistry()
{
   theInstance = 0;
}

rspfCustomEditorWindowRegistry* rspfCustomEditorWindowRegistry::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfCustomEditorWindowRegistry;
   }

   return theInstance;
}

bool rspfCustomEditorWindowRegistry::registerFactory(rspfCustomEditorWindowFactoryBase* factory)
{
   bool result = false;
   if(factory)
   {
      theFactoryList.push_back(factory);
      result = true;
   }

   return result;
}

rspfCustomEditorWindow* rspfCustomEditorWindowRegistry::createCustomEditor(rspfObject* obj,
                                                                             void* parent)const
{
   rspfCustomEditorWindow* result = 0;
   int i = 0;

   for(i = 0; (i < (int)theFactoryList.size())&&(!result); ++i)
   {
      result = theFactoryList[i]->createCustomEditor(obj, parent);
   }

   return result;
}
rspfCustomEditorWindow* rspfCustomEditorWindowRegistry::createCustomEditor(const rspfString& classType,
                                                                             void* parent)const
{
   rspfCustomEditorWindow* result = 0;
   int i = 0;

   for(i = 0; (i < (int)theFactoryList.size())&&(!result); ++i)
   {
      result = theFactoryList[i]->createCustomEditor(classType, parent);
   }

   return result;
}

rspfObject* rspfCustomEditorWindowRegistry::createObject(const rspfString& typeName)const
{
   rspfObject* result = 0;
   unsigned long index = 0;
   
   while((index < theFactoryList.size()) &&(!result))
   {
      result = theFactoryList[index]->createObject(typeName);
      ++index;
   }
   return result;
}

rspfObject* rspfCustomEditorWindowRegistry::createObject(const rspfKeywordlist& kwl,
                                                           const char* prefix)const
{
   rspfObject* result = 0;
   unsigned long index = 0;

   while((index < theFactoryList.size()) &&(!result))
   {
      result = theFactoryList[index]->createObject(kwl,
                                                   prefix);
      ++index;
   }

   return result;
}

void* rspfCustomEditorWindowRegistry::createPopupEditor(rspfObject* obj,
                                                         void* parent)const
{
   void* result = 0;
   unsigned long index = 0;

   while((index < theFactoryList.size()) &&(!result))
   {
      result = theFactoryList[index]->createPopupEditor(obj,
                                                        parent);
      ++index;
   }

   return result;
}
   
void* rspfCustomEditorWindowRegistry::createPanelEditor(rspfObject* obj,
                                                         void* parent)const
{
   void* result = 0;
   unsigned long index = 0;

   while((index < theFactoryList.size()) &&(!result))
   {
      result = theFactoryList[index]->createPanelEditor(obj,
                                                        parent);
      ++index;
   }

   return result;
}

void rspfCustomEditorWindowRegistry::getTypeNameList(std::vector<rspfString>& typeList)const
{
   std::vector<rspfCustomEditorWindowFactoryBase*>::const_iterator iter = theFactoryList.begin();
   std::vector<rspfString> result;
   
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

extern "C"
{
  void* rspfCustomEditorWindowRegistryGetInstance()
  {
    return rspfCustomEditorWindowRegistry::instance();
  }
}
