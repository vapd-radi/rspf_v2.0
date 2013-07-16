//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Kenneth Melero
//
//*******************************************************************
//  $Id: rspfImageMetaDataWriterFactory.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <rspf/imaging/rspfImageMetaDataWriterFactory.h>

#include <rspf/imaging/rspfEnviHeaderFileWriter.h>
#include <rspf/imaging/rspfERSFileWriter.h>
#include <rspf/imaging/rspfFgdcFileWriter.h>
#include <rspf/imaging/rspfGeomFileWriter.h>
#include <rspf/imaging/rspfReadmeFileWriter.h>
#include <rspf/imaging/rspfWorldFileWriter.h>

#include <rspf/base/rspfKeywordNames.h>

RTTI_DEF1(rspfImageMetaDataWriterFactory,
          "rspfImageMetaDataWriterFactory",
          rspfImageMetaDataWriterFactoryBase);

rspfImageMetaDataWriterFactory*
rspfImageMetaDataWriterFactory::theInstance = NULL;

rspfImageMetaDataWriterFactory::rspfImageMetaDataWriterFactory()
{
   theInstance = this;
}

rspfImageMetaDataWriterFactory::~rspfImageMetaDataWriterFactory()
{
   theInstance = NULL;
}

rspfImageMetaDataWriterFactory* rspfImageMetaDataWriterFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfImageMetaDataWriterFactory;
   }

   return theInstance;
}

rspfObject* rspfImageMetaDataWriterFactory::createObject(
   const rspfString& typeName) const
{
   // Use the type name to instantiate the class.
   rspfObject* result = (rspfObject*)NULL;
   
   if(STATIC_TYPE_NAME(rspfEnviHeaderFileWriter) == typeName)
   {
     result = new rspfEnviHeaderFileWriter;
   }
   if(STATIC_TYPE_NAME(rspfERSFileWriter) == typeName)
   {
     result = new rspfERSFileWriter;
   }
   else if(STATIC_TYPE_NAME(rspfFgdcFileWriter) == typeName)
   {
     result = new rspfFgdcFileWriter;
   }
   else if(STATIC_TYPE_NAME(rspfGeomFileWriter) == typeName)
   {
     result = new rspfGeomFileWriter;
   }
   else if(STATIC_TYPE_NAME(rspfReadmeFileWriter) == typeName)
   {
     result = new rspfReadmeFileWriter;
   }
   //---
   // Special case for backwards compatibility. rspfJpegWorldFileWriter and
   // rspfTiffWorldFileWriter same; hence, moved to rspfWorldFileWriter.
   //---
   else if( (STATIC_TYPE_NAME(rspfWorldFileWriter)  == typeName) ||
            (rspfString("rspfJpegWorldFileWriter") == typeName) ||
            (rspfString("rspfTiffWorldFileWriter") == typeName) )
   {
     result = new rspfWorldFileWriter;
   }

   return result;
}

rspfObject* rspfImageMetaDataWriterFactory::createObject(
   const rspfKeywordlist& kwl, const char* prefix)const
{
   //---
   // Check the type keyword found in kwl.  Use the create by class name
   // above and then call load state.
   //---
   rspfObject* result = (rspfObject*)NULL;

   const char* type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);

   if(type)
   {
      result = createObject(rspfString(type));
      if(result)
      {
         result->loadState(kwl, prefix);
      }
   }
   
   return result;
}

rspfRefPtr<rspfMetadataFileWriter>
rspfImageMetaDataWriterFactory::createWriter(const rspfString& type)const
{
   rspfRefPtr<rspfMetadataFileWriter> writer;
   
   writer = new rspfEnviHeaderFileWriter;
   if ( (writer->getClassName() == type) ||
        (writer->hasMetadataType(type)) )
   {
      return writer;
   }

   writer = new rspfERSFileWriter;
   if ( (writer->getClassName() == type) ||
        (writer->hasMetadataType(type)) )
   {
      return writer;
   }

  writer = new rspfFgdcFileWriter;
   if ( (writer->getClassName() == type) ||
        (writer->hasMetadataType(type)) )
   {
      return writer;
   }

   writer = new rspfGeomFileWriter;
   if ( (writer->getClassName() == type) ||
        (writer->hasMetadataType(type)) )
   {
      return writer;
   }
   
   writer = new rspfReadmeFileWriter;
   if ( (writer->getClassName() == type) ||
        (writer->hasMetadataType(type)) )
   {
      return writer;
   }

   //---
   // Special case for backwards compatibility. rspfJpegWorldFileWriter and
   // rspfTiffWorldFileWriter same; hence, moved to rspfWorldFileWriter.
   //---
   writer = new rspfWorldFileWriter;
   if ( (writer->getClassName() == type) ||
        (writer->hasMetadataType(type))  ||
        (rspfString("rspfJpegWorldFileWriter") == type) ||
        (rspfString("rspfTiffWorldFileWriter") == type) )
   {
      return writer;
   }

   // Not in factory.
   writer = NULL;
   return writer;
}

void rspfImageMetaDataWriterFactory::getTypeNameList(
   std::vector<rspfString>& typeList) const
{
   // add each object's RTTI name here
   typeList.push_back(STATIC_TYPE_NAME(rspfEnviHeaderFileWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfERSFileWriter));   
   typeList.push_back(STATIC_TYPE_NAME(rspfFgdcFileWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfGeomFileWriter));
   typeList.push_back(STATIC_TYPE_NAME(rspfReadmeFileWriter));   
   typeList.push_back(STATIC_TYPE_NAME(rspfWorldFileWriter));
}

void rspfImageMetaDataWriterFactory::getMetadatatypeList(
   std::vector<rspfString>& metadatatypeList) const
{
   rspfRefPtr<rspfMetadataFileWriter> writer;

   writer = new rspfEnviHeaderFileWriter;
   writer->getMetadatatypeList(metadatatypeList);

   writer = new rspfERSFileWriter;
   writer->getMetadatatypeList(metadatatypeList);

   writer = new rspfFgdcFileWriter;
   writer->getMetadatatypeList(metadatatypeList);
   
   writer = new rspfGeomFileWriter;
   writer->getMetadatatypeList(metadatatypeList);

   writer = new rspfReadmeFileWriter;
   writer->getMetadatatypeList(metadatatypeList);

   writer = new rspfWorldFileWriter;
   writer->getMetadatatypeList(metadatatypeList);
}

rspfImageMetaDataWriterFactory::rspfImageMetaDataWriterFactory(
   const rspfImageMetaDataWriterFactory& /* rhs */)
{
}

const rspfImageMetaDataWriterFactory&
rspfImageMetaDataWriterFactory::operator=(
   const rspfImageMetaDataWriterFactory& /* rhs */)
{
   return *this;
}
