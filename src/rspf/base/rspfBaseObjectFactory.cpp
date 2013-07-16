//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfBaseObjectFactory.cpp 19900 2011-08-04 14:19:57Z dburken $
#include <rspf/base/rspfBaseObjectFactory.h>
#include <rspf/base/rspfConnectableContainer.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
#include <vector>

RTTI_DEF1(rspfBaseObjectFactory, "rspfBaseObjectFactory", rspfObjectFactory);

rspfBaseObjectFactory* rspfBaseObjectFactory::theInstance = NULL;

rspfBaseObjectFactory* rspfBaseObjectFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfBaseObjectFactory;
   }

   return theInstance;
}

rspfObject* rspfBaseObjectFactory::createObject(const rspfString& typeName)const
{
   if(typeName == STATIC_TYPE_NAME(rspfConnectableContainer))
   {
      return new rspfConnectableContainer;
   }

   return (rspfObject*)NULL;
}

rspfObject* rspfBaseObjectFactory::createObject(const rspfKeywordlist& kwl,
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

void rspfBaseObjectFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfConnectableContainer));
}
