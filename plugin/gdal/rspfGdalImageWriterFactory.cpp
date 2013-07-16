#include "rspfGdalImageWriterFactory.h"
#include "rspfGdalWriter.h"
#include <rspf/base/rspfKeywordNames.h>
#include <gdal_priv.h>
rspfGdalImageWriterFactory* rspfGdalImageWriterFactory::theInstance = (rspfGdalImageWriterFactory*)NULL;
rspfGdalImageWriterFactory* rspfGdalImageWriterFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfGdalImageWriterFactory;
   }
   return theInstance;
}
rspfGdalImageWriterFactory::~rspfGdalImageWriterFactory()
{
   theInstance = (rspfGdalImageWriterFactory*)NULL;
}
rspfImageFileWriter*
rspfGdalImageWriterFactory::createWriter(const rspfKeywordlist& kwl,
                                          const char *prefix)const
{
   rspfString type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   rspfImageFileWriter* result = (rspfImageFileWriter*)NULL;
   if(type != "")
   {
      result = createWriter(type);
      if (result)
      {
         if (result->hasImageType(type))
         {
            rspfKeywordlist kwl2(kwl);
            kwl2.add(prefix,
                     rspfKeywordNames::IMAGE_TYPE_KW,
                     type,
                     true);
         
            result->loadState(kwl2, prefix);
         }
         else
         {
            result->loadState(kwl, prefix);
         }
      }
   }
   return result;
}
rspfImageFileWriter* rspfGdalImageWriterFactory::createWriter(const rspfString& typeName)const
{
   rspfRefPtr<rspfGdalWriter> writer = new rspfGdalWriter;
   if (writer->getClassName() == typeName)
   {
      return writer.release();
   }
   if (writer->hasImageType(typeName))
   {
      writer->setOutputImageType(typeName);
      return writer.release();
   }
   writer = 0;
   
   return writer.release();
}
rspfObject* rspfGdalImageWriterFactory::createObject(const rspfKeywordlist& kwl,
                                                   const char *prefix)const
{
   return createWriter(kwl, prefix);
}
rspfObject* rspfGdalImageWriterFactory::createObject(const rspfString& typeName)const
{
   return createWriter(typeName);
}
void rspfGdalImageWriterFactory::getExtensions(std::vector<rspfString>& result)const
{
   result.push_back("img");
   result.push_back("jp2");
   result.push_back("png");
}
void rspfGdalImageWriterFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   getImageTypeList(typeList);
}
void rspfGdalImageWriterFactory::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   rspfGdalWriter* writer = new rspfGdalWriter;
   writer->getImageTypeList(imageTypeList);
   delete writer;
}
void rspfGdalImageWriterFactory::getImageFileWritersBySuffix(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                                              const rspfString& ext)const
{
   int c = GDALGetDriverCount();
   int idx = 0;
   for(idx = 0; idx < c; ++idx)
   {
      
      GDALDriverH h = GDALGetDriver(idx);
      if(canWrite(h))
      {
         rspfString  driverName = GDALGetDriverShortName(h);
         driverName = "gdal_" + driverName.upcase();
         rspfString metaData(GDALGetMetadataItem(h, GDAL_DMD_EXTENSION, 0));
         if(!metaData.empty())
         {
            std::vector<rspfString> splitArray;
            metaData.split(splitArray, " /");
            
            rspf_uint32 idxExtension = 0;
            
            for(idxExtension = 0; idxExtension < splitArray.size(); ++idxExtension)
            {
               if(ext == splitArray[idxExtension])
               {
                  rspfGdalWriter* writer = new rspfGdalWriter;
                  writer->setOutputImageType(driverName);
                  result.push_back(writer);
                  if ( driverName == "gdal_JP2KAK" )
                  {
                     rspfKeywordlist kwl;
                     kwl.add("property0.name", "QUALITY");
                     kwl.add("property0.value", "100");
                     writer->loadState(kwl, NULL);
                  }
                  return;
               }
            }
         }
      }
   }
}
void rspfGdalImageWriterFactory::getImageFileWritersByMimeType(rspfImageWriterFactoryBase::ImageFileWriterList& result,
                                                                const rspfString& mimeType)const
{
   int c = GDALGetDriverCount();
   int idx = 0;
   for(idx = 0; idx < c; ++idx)
   {
      
      GDALDriverH h = GDALGetDriver(idx);
      if(canWrite(h))
      {
         rspfString  driverName = GDALGetDriverShortName(h);
         driverName = "gdal_" + driverName.upcase();
         rspfString metaData(GDALGetMetadataItem(h, GDAL_DMD_MIMETYPE, 0));
         if(!metaData.empty())
         {
            if(metaData == mimeType)
            {
               rspfGdalWriter* writer = new rspfGdalWriter;
               writer->setOutputImageType(driverName);
               result.push_back(writer);
               if ( driverName == "gdal_JP2KAK" )
               {
                  rspfKeywordlist kwl;
                  kwl.add("property0.name", "QUALITY");
                  kwl.add("property0.value", "100");
                  writer->loadState(kwl, NULL);
               }
               return;
            }
         }
      }
   }
}
bool rspfGdalImageWriterFactory::canWrite(GDALDatasetH handle)const
{
   return ( GDALGetMetadataItem(handle, GDAL_DCAP_CREATE, 0)||  GDALGetMetadataItem(handle, GDAL_DCAP_CREATECOPY, 0));
}
