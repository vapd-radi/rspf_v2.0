//----------------------------------------------------------------------------
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//----------------------------------------------------------------------------
// $Id: rspfImageMetaDataWriterRegistry.cpp 17206 2010-04-25 23:20:40Z dburken $


#include <rspf/imaging/rspfImageMetaDataWriterRegistry.h>
#include <rspf/imaging/rspfImageMetaDataWriterFactory.h>
#include <rspf/imaging/rspfMetadataFileWriter.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <algorithm>

//rspfImageMetaDataWriterRegistry*
//rspfImageMetaDataWriterRegistry::theInstance= NULL;

rspfImageMetaDataWriterRegistry::rspfImageMetaDataWriterRegistry()
{
   registerFactory(rspfImageMetaDataWriterFactory::instance());
   rspfObjectFactoryRegistry::instance()->registerFactory(this);
}

rspfImageMetaDataWriterRegistry::~rspfImageMetaDataWriterRegistry()
{
   rspfObjectFactoryRegistry::instance()->unregisterFactory(this);
}

rspfImageMetaDataWriterRegistry* rspfImageMetaDataWriterRegistry::instance()
{
   static rspfImageMetaDataWriterRegistry sharedInstance;

   return &sharedInstance;
}

void rspfImageMetaDataWriterRegistry::registerFactory(
   rspfImageMetaDataWriterFactoryBase* factory)
{
   if(factory&&!findFactory(factory))
   {
      theFactoryList.push_back(factory);
   }
}

void rspfImageMetaDataWriterRegistry::unregisterFactory(
   rspfImageMetaDataWriterFactoryBase* factory)
{
   std::vector<rspfImageMetaDataWriterFactoryBase*>::iterator iter =
      std::find(theFactoryList.begin(),
                theFactoryList.end(),
                factory);
   if(iter != theFactoryList.end())
   {
      theFactoryList.erase(iter);
   }
}

bool rspfImageMetaDataWriterRegistry::findFactory(
   rspfImageMetaDataWriterFactoryBase* factory)const
{
   return (std::find(theFactoryList.begin(),
                     theFactoryList.end(),
                     factory)!=theFactoryList.end());
}


rspfObject* rspfImageMetaDataWriterRegistry::createObject(
   const rspfString& typeName) const
{
   rspfObject* result = NULL;
   std::vector<rspfImageMetaDataWriterFactoryBase*>::const_iterator factory;
   
   factory = theFactoryList.begin();
   while((factory != theFactoryList.end()) && !result)
   {
      result = (*factory)->createObject(typeName);
      ++factory;
   }
   
   return result;
}

rspfObject* rspfImageMetaDataWriterRegistry::createObject(
   const rspfKeywordlist& kwl,
   const char* prefix) const
{
   rspfObject* result = NULL;
   std::vector<rspfImageMetaDataWriterFactoryBase*>::const_iterator factory;

   factory = theFactoryList.begin();
   while((factory != theFactoryList.end()) && !result)
   {
     result = (*factory)->createObject(kwl, prefix);
     ++factory;
   }
   return result;
}

rspfRefPtr<rspfMetadataFileWriter>
rspfImageMetaDataWriterRegistry::createWriter(const rspfString& type) const
{
   rspfRefPtr<rspfMetadataFileWriter> result = NULL;
   
   std::vector<rspfImageMetaDataWriterFactoryBase*>::const_iterator factory;
   factory = theFactoryList.begin();
   while((factory != theFactoryList.end()) && !result)
   {
     result = (*factory)->createWriter(type);
     ++factory;
   }
   return result;
}

void rspfImageMetaDataWriterRegistry::getTypeNameList(
   std::vector<rspfString>& typeList) const
{
   std::vector<rspfString> result;
   std::vector<rspfImageMetaDataWriterFactoryBase*>::const_iterator iter =
      theFactoryList.begin();

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

void rspfImageMetaDataWriterRegistry::getMetadatatypeList(
   std::vector<rspfString>& metadatatypeList) const
{
   //---
   // Since this is the master registry for all meta data writer factories
   // we will start with a cleared list.
   //---
   metadatatypeList.clear();
   
   std::vector<rspfImageMetaDataWriterFactoryBase*>::const_iterator iter =
      theFactoryList.begin();
   
   while(iter != theFactoryList.end())
   {
      (*iter)->getMetadatatypeList(metadatatypeList);
      ++iter;
   } 
}

rspfImageMetaDataWriterRegistry::rspfImageMetaDataWriterRegistry(
   const rspfImageMetaDataWriterRegistry& /* rhs */)
{
}

const rspfImageMetaDataWriterRegistry&
rspfImageMetaDataWriterRegistry::operator=(
   const rspfImageMetaDataWriterRegistry& /* rhs */)
{
   return *this;
}

extern "C"
{
   void* rspfImageMetaDataWriterRegistryGetInstance()
   {
      return rspfImageMetaDataWriterRegistry::instance();
   }
}
