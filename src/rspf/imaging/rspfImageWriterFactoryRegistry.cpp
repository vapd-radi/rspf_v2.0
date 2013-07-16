//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Frank Warmerdam (warmerda@home.com)
//
//*******************************************************************
//  $Id: rspfImageWriterFactoryRegistry.cpp 19907 2011-08-05 19:55:46Z dburken $

#include <rspf/imaging/rspfImageWriterFactory.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfImageFileWriter.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfString.h>
#include <algorithm>
#include <iterator>
#include <ostream>

rspfImageWriterFactoryRegistry* rspfImageWriterFactoryRegistry::theInstance = NULL;

rspfImageWriterFactoryRegistry::rspfImageWriterFactoryRegistry()
{
}

rspfImageWriterFactoryRegistry* rspfImageWriterFactoryRegistry::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfImageWriterFactoryRegistry;
      rspfObjectFactoryRegistry::instance()->registerFactory(theInstance);
      rspfImageWriterFactory::instance();
   }

   return theInstance;
}


rspfImageFileWriter* rspfImageWriterFactoryRegistry::createWriter(const rspfFilename& filename)const
{
   rspfImageFileWriter * writer = createWriterFromExtension(filename.ext().downcase());
   if(writer)
   {
      writer->setFilename(filename);
   }
   
   return writer;
}

rspfObject* rspfImageWriterFactoryRegistry::createObject(const rspfKeywordlist &kwl,
                                                           const char *prefix)const
{
   return createObjectFromRegistry(kwl, prefix);
}

rspfObject* rspfImageWriterFactoryRegistry::createObject(const rspfString& typeName)const
{
   return createObjectFromRegistry(typeName);
}

void rspfImageWriterFactoryRegistry::getTypeNameList(std::vector<rspfString>& typeList)const
{
   getAllTypeNamesFromRegistry(typeList);
}

rspfImageFileWriter *rspfImageWriterFactoryRegistry::createWriterFromExtension(const rspfString& fileExtension)const
{
   rspfImageFileWriter *writer = NULL;
   rspfImageWriterFactoryBase::ImageFileWriterList result;
   getImageFileWritersBySuffix(result, fileExtension);
   if(!result.empty())
   {
      writer = result[0].release();
      result.clear();
   }
   return writer;
}

rspfImageFileWriter *rspfImageWriterFactoryRegistry::createWriter(const rspfKeywordlist &kwl,
                                                                const char *prefix)const
{
   // let's see if we ned to return an object based on extension.
   // this is specified by the type to be a generic
   // rspfImageFileWriter
   //
   rspfString type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);

   if(type == "rspfImageFileWriter")
   {
      rspfFilename filename = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);

      if((filename != "")&&
         (filename.ext() != ""))
      {
         rspfImageFileWriter* writer = createWriterFromExtension(filename.ext());

         if(writer)
         {
            writer->setFilename(filename);
         }
         return writer;
      }
   }
   
   vector<rspfImageWriterFactoryBase*>::const_iterator factories;
   rspfImageFileWriter *result = NULL;

   factories = m_factoryList.begin();
   while(factories != m_factoryList.end())
   {
      result = (*factories)->createWriter(kwl, prefix);
      if(result)
      {
         return result;
      }
      ++factories;
   }

   return result;
}

rspfImageFileWriter *rspfImageWriterFactoryRegistry::createWriter(const rspfString& typeName)const
{
   vector<rspfImageWriterFactoryBase*>::const_iterator factories;
   rspfImageFileWriter *result = NULL;

   factories = m_factoryList.begin();
   while(factories != m_factoryList.end())
   {
      result = (*factories)->createWriter(typeName);
      if(result)
      {
         return result;
      }
      ++factories;
   }

   return result;
}


void rspfImageWriterFactoryRegistry::getImageTypeList(std::vector<rspfString>& typeList)const
{
   vector<rspfString> result;
   vector<rspfImageWriterFactoryBase*>::const_iterator iter = m_factoryList.begin();
   
   while(iter != m_factoryList.end())
   {
      result.clear();
      (*iter)->getImageTypeList(result);
      
      // now append to the end of the typeList.
      typeList.insert(typeList.end(),
                      result.begin(),
                      result.end());
      ++iter;
   }  
}

void rspfImageWriterFactoryRegistry::getImageFileWritersBySuffix(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                                                  const rspfString& ext)const
{
   rspfImageWriterFactoryBase::ImageFileWriterList tempResult;
   vector<rspfImageWriterFactoryBase*>::const_iterator iter = m_factoryList.begin();
   
   while(iter != m_factoryList.end())
   {
      result.clear();
      (*iter)->getImageFileWritersBySuffix(tempResult, ext);
      
      // now append to the end of the typeList.
      result.insert(result.end(),
                      tempResult.begin(),
                      tempResult.end());
      ++iter;
   }  
   
}

void rspfImageWriterFactoryRegistry::getImageFileWritersByMimeType(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                                                    const rspfString& mimeType)const
{
   rspfImageWriterFactoryBase::ImageFileWriterList tempResult;
   vector<rspfImageWriterFactoryBase*>::const_iterator iter = m_factoryList.begin();
   
   while(iter != m_factoryList.end())
   {
      result.clear();
      (*iter)->getImageFileWritersByMimeType(tempResult, mimeType);
      
      // now append to the end of the typeList.
      result.insert(result.end(),
                    tempResult.begin(),
                    tempResult.end());
      ++iter;
   }  
}

std::ostream& rspfImageWriterFactoryRegistry::printImageTypeList(
   std::ostream& out)const
{
   std::vector<rspfString> outputType;
   
   this->getImageTypeList(outputType);
   std::copy(outputType.begin(),
             outputType.end(),
             std::ostream_iterator<rspfString>(out, "\n"));
   out << std::endl;
   return out;
}

std::ostream& rspfImageWriterFactoryRegistry::printWriterProps(std::ostream& out)const
{
   // Loop through factories:
   vector<rspfImageWriterFactoryBase*>::const_iterator factoryIter = m_factoryList.begin();
   while( factoryIter != m_factoryList.end() )
   {
      out << "factory: " << (*factoryIter)->getClassName() << "\n\n";

      // Loop through writer classes in factory.
      std::vector<rspfString> typeNames;
      (*factoryIter)->getTypeNameList(typeNames);
      std::vector<rspfString>::const_iterator typeNamesIter = typeNames.begin();
      while (typeNamesIter != typeNames.end())
      {
         rspfRefPtr<rspfImageFileWriter> writer = (*factoryIter)->createWriter(*typeNamesIter);
         if ( writer.valid() )
         {
            out << "writer:\n" << writer->getClassName() << "\n";

            // Loop through writer types, e.g. tiff_tiled_band_separate
            std::vector<rspfString> imageTypeList;
            writer->getImageTypeList(imageTypeList);
            std::vector<rspfString>::const_iterator imageTypeListIter = imageTypeList.begin();
            out << "\ntypes:\n";
            while ( imageTypeListIter != imageTypeList.end() )
            {
               out << (*imageTypeListIter) << "\n";
               ++imageTypeListIter;
            }

            // Loop through writer properties, e.g. compression_quality.
            out << "\nproperties:\n";
            std::vector<rspfString> propNames;
            writer->getPropertyNames(propNames);
            std::vector<rspfString>::const_iterator propNamesIter = propNames.begin();
            while ( propNamesIter != propNames.end() )
            {
               out << (*propNamesIter) << "\n";
               
               ++propNamesIter;
            }
            out << "\n";
         }

         ++typeNamesIter;
      }
      
      ++factoryIter;
   }
   return out;
}

extern "C"
{
  void* rspfImageWriterFactoryRegistryGetInstance()
  {
    return rspfImageWriterFactoryRegistry::instance();
  }
}

rspfImageWriterFactoryRegistry::rspfImageWriterFactoryRegistry(const rspfImageWriterFactoryRegistry&)
   :
      rspfObjectFactory()
{
}

void rspfImageWriterFactoryRegistry::operator=(const rspfImageWriterFactoryRegistry&)
{
}
