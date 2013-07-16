//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts
//
// Description:
//
// Contains class definition for ImageHandlerRegistry.
//
//*******************************************************************
//  $Id: rspfImageHandlerRegistry.cpp 22228 2013-04-12 14:11:45Z dburken $

#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfString.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerFactory.h>
#include <rspf/imaging/rspfImageHandlerFactoryBase.h>
#include <algorithm>
using namespace std;

RTTI_DEF1(rspfImageHandlerRegistry, "rspfImageHandlerRegistry", rspfObjectFactory);

//rspfImageHandlerRegistry* rspfImageHandlerRegistry::theInstance = 0;

rspfImageHandlerRegistry::rspfImageHandlerRegistry()
{
   rspfObjectFactoryRegistry::instance()->registerFactory(this);
   registerFactory(rspfImageHandlerFactory::instance());
}

rspfImageHandlerRegistry* rspfImageHandlerRegistry::instance()
{
   static rspfImageHandlerRegistry sharedInstance;
   
   return &sharedInstance;
}

rspfImageHandlerRegistry::~rspfImageHandlerRegistry()
{
   unregisterAllFactories();
}


rspfObject* rspfImageHandlerRegistry::createObject(const rspfKeywordlist& kwl,
                                                     const char* prefix)const
{
   rspfObject* result = createObjectFromRegistry(kwl, prefix);
   if(!result)
   {
      result = open(kwl, prefix);
   }
   return result;
}

rspfRefPtr<rspfImageHandler> rspfImageHandlerRegistry::openBySuffix(const rspfFilename& file,
                                                                       bool openOverview)const
{
   std::vector<rspfRefPtr<rspfImageHandler> > handlers;
   
   getImageHandlersBySuffix(handlers, file.ext());
   rspf_uint32 idx = 0;
   rspf_uint32 size = (rspf_uint32) handlers.size();
   
   for(idx = 0; idx < size; ++idx)
   {
      handlers[idx]->setOpenOverviewFlag(openOverview);
      if(handlers[idx]->open(file))
      {
         return handlers[idx];
      }
   }
   
   return rspfRefPtr<rspfImageHandler>(0);
}

void rspfImageHandlerRegistry::getImageHandlersBySuffix(rspfImageHandlerFactoryBase::ImageHandlerList& result,
                                                         const rspfString& ext)const
{
   vector<rspfImageHandlerFactoryBase*>::const_iterator iter = m_factoryList.begin();
   rspfImageHandlerFactoryBase::ImageHandlerList temp;
   while(iter != m_factoryList.end())
   {
      temp.clear();
      (*iter)->getImageHandlersBySuffix(temp, ext);
      
      if(!temp.empty())
      {
         
         // now append to the end of the typeList.
         result.insert(result.end(),
                       temp.begin(),
                       temp.end());
      }
      ++iter;
   }
}

void rspfImageHandlerRegistry::getImageHandlersByMimeType(
   rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& mimeType)const
{
   vector<rspfImageHandlerFactoryBase*>::const_iterator iter = m_factoryList.begin();
   rspfImageHandlerFactoryBase::ImageHandlerList temp;
   while(iter != m_factoryList.end())
   {
      temp.clear();
      (*iter)->getImageHandlersByMimeType(temp, mimeType);
      
      if(!temp.empty())
      {
         
         // now append to the end of the typeList.
         result.insert(result.end(),
                       temp.begin(),
                       temp.end());
      }
      ++iter;
   }
}

void rspfImageHandlerRegistry::getTypeNameList( std::vector<rspfString>& typeList ) const
{
   getAllTypeNamesFromRegistry(typeList);
}

void rspfImageHandlerRegistry::getSupportedExtensions(
   rspfImageHandlerFactoryBase::UniqueStringList& extensionList)const
{
   vector<rspfString> result;
   vector<rspfImageHandlerFactoryBase*>::const_iterator iter = m_factoryList.begin();

   while(iter != m_factoryList.end())
   {
      (*iter)->getSupportedExtensions(extensionList);

      ++iter;
   }
   
}

rspfImageHandler* rspfImageHandlerRegistry::open(const rspfFilename& fileName,
                                                   bool trySuffixFirst,
                                                   bool openOverview)const
{
   if(trySuffixFirst)
   {
      rspfRefPtr<rspfImageHandler> h = openBySuffix(fileName, openOverview);
      if(h.valid())
      {
         return h.release();
      }
   }
   
   // now try magic number opens
   //
   rspfImageHandler*                   result = NULL;
   vector<rspfImageHandlerFactoryBase*>::const_iterator factory;

   factory = m_factoryList.begin();
   while((factory != m_factoryList.end()) && !result)
   {
      result = (*factory)->open(fileName, openOverview);
      ++factory;
   }
   
   return result;
}

rspfImageHandler* rspfImageHandlerRegistry::open(const rspfKeywordlist& kwl,
                                                   const char* prefix)const
{
   rspfImageHandler*                   result = NULL;
   vector<rspfImageHandlerFactoryBase*>::const_iterator factory;
   
   factory = m_factoryList.begin();
   while((factory != m_factoryList.end()) && !result)
   {
      result = (*factory)->open(kwl, prefix);
      ++factory;
   }
   
   return result;
}

rspfRefPtr<rspfImageHandler> rspfImageHandlerRegistry::openOverview(
   const rspfFilename& file ) const
{
   rspfRefPtr<rspfImageHandler> result = 0;
   vector<rspfImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
   while( factory != m_factoryList.end() )
   {
      result = (*factory)->openOverview( file );
      if ( result.valid() )
      {
         break;
      }
      ++factory;
   }  
   return result;
}

rspfObject* rspfImageHandlerRegistry::createObject(const rspfString& typeName) const
{
   return createObjectFromRegistry(typeName);
}

std::ostream& rspfImageHandlerRegistry::printReaderProps(std::ostream& out) const
{
   // Loop through factories:
   vector<rspfImageHandlerFactoryBase*>::const_iterator factory = m_factoryList.begin();
   while( factory != m_factoryList.end() )
   {
      out << "factory: " << (*factory)->getClassName() << "\n";
      
      // Loop through factory image handlers:
      std::vector<rspfString> readerList;
      (*factory)->getTypeNameList(readerList);

      std::vector<rspfString>::const_iterator i = readerList.begin();
      while ( i != readerList.end() )
      {
         rspfRefPtr<rspfImageHandler> ih =
            dynamic_cast<rspfImageHandler*>( (*factory)->createObject( (*i) ) );
         if ( ih.valid() )
         {
            std::vector<rspfString> propertyList;
            ih->getPropertyNames(propertyList);
            out << "reader: " << ih->getClassName() << "\n";
            
            if ( propertyList.size() )
            {
               // Loop through image handler properties:
               out << "properties:\n";
               std::vector<rspfString>::const_iterator p = propertyList.begin();
               while ( p != propertyList.end() )
               {
                  out << (*p) << "\n";
                  ++p;
               }
            }
         }
         ++i;
      }
      ++factory;
   }
   out << std::endl;
   return out;
}

rspfImageHandlerRegistry::rspfImageHandlerRegistry(const rspfImageHandlerRegistry& /* rhs */)
   :  rspfObjectFactory()
{}

const rspfImageHandlerRegistry&
rspfImageHandlerRegistry::operator=(const rspfImageHandlerRegistry& rhs)
{ return rhs; }

extern "C"
{
  void* rspfImageHandlerRegistryGetInstance()
  {
    return rspfImageHandlerRegistry::instance();
  }
}
