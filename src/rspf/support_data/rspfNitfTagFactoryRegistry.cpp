//*******************************************************************
//
// LICENSE: See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfTagFactoryRegistry.cpp 9212 2006-06-28 23:28:53Z dburken $

#include <cstddef>   /* for NULL      */
#include <algorithm> /* for std::find */

#include <rspf/support_data/rspfNitfTagFactoryRegistry.h>
#include <rspf/support_data/rspfNitfRpfTagFactory.h>
#include <rspf/support_data/rspfNitfRegisteredTagFactory.h>
#include <rspf/support_data/rspfNitfUnknownTag.h>

rspfNitfTagFactoryRegistry* rspfNitfTagFactoryRegistry::theInstance=NULL;

rspfNitfTagFactoryRegistry::rspfNitfTagFactoryRegistry()
{
}

rspfNitfTagFactoryRegistry::~rspfNitfTagFactoryRegistry()
{
   theInstance = NULL;
}

void rspfNitfTagFactoryRegistry::registerFactory(rspfNitfTagFactory* aFactory)
{
   if(aFactory&&!exists(aFactory))
   {
      theFactoryList.push_back(aFactory);
   }
}

void rspfNitfTagFactoryRegistry::unregisterFactory
(rspfNitfTagFactory* aFactory)
{
   std::vector<rspfNitfTagFactory*>::iterator iter =
      std::find(theFactoryList.begin(),
                theFactoryList.end(),
                aFactory);
   if(iter != theFactoryList.end())
   {
      theFactoryList.erase(iter);
   }
}

rspfNitfTagFactoryRegistry* rspfNitfTagFactoryRegistry::instance()
{
   if(theInstance)
   {
      return theInstance;
   }
   else
   {
      theInstance = new rspfNitfTagFactoryRegistry;
      theInstance->initializeDefaults();
   }
   
   return theInstance;
}

rspfRefPtr<rspfNitfRegisteredTag> rspfNitfTagFactoryRegistry::create(const rspfString &tagName)const
{
   std::vector<rspfNitfTagFactory*>::const_iterator factory;
   rspfRefPtr<rspfNitfRegisteredTag> result = NULL;

   factory = theFactoryList.begin();
   while(factory != theFactoryList.end())
   {
      result = (*factory)->create(tagName);

      if(result.valid())
      {
         return result;
      }
      ++factory;
   }

   rspfRefPtr<rspfNitfRegisteredTag> tag = new rspfNitfUnknownTag;
   return tag;
}

void rspfNitfTagFactoryRegistry::initializeDefaults()
{
   theFactoryList.push_back(rspfNitfRegisteredTagFactory::instance());
   theFactoryList.push_back(rspfNitfRpfTagFactory::instance());
}

bool rspfNitfTagFactoryRegistry::exists(rspfNitfTagFactory* factory)const
{
   std::vector<rspfNitfTagFactory*>::const_iterator iter = std::find(theFactoryList.begin(),
                                                                      theFactoryList.end(),
                                                                      factory);

   return (iter != theFactoryList.end());
   
}

rspfNitfTagFactoryRegistry::rspfNitfTagFactoryRegistry(
   const rspfNitfTagFactoryRegistry&)
{
}

const rspfNitfTagFactoryRegistry& rspfNitfTagFactoryRegistry::operator=(
   const rspfNitfTagFactoryRegistry&)
{
   return *this;
}
