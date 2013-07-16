#include <rspfGdalFactory.h>
#include <rspfGdalTileSource.h>
#include <rspfOgrGdalTileSource.h>
#include <rspfOgrVectorTileSource.h>
#include <rspfHdfReader.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfString.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordNames.h>
#include <gdal.h>
#include <ogrsf_frmts.h>
static const rspfTrace traceDebug("rspfGdalFactory:debug");
RTTI_DEF1(rspfGdalFactory, "rspfGdalFactory", rspfImageHandlerFactoryBase);
rspfGdalFactory* rspfGdalFactory::theInstance = 0;
rspfGdalFactory::~rspfGdalFactory()
{
   theInstance = (rspfGdalFactory*)NULL;
}
rspfGdalFactory* rspfGdalFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfGdalFactory;
      CPLSetErrorHandler((CPLErrorHandler)CPLQuietErrorHandler);
      GDALAllRegister();
      OGRRegisterAll();
   }
   return theInstance;
}
   
rspfImageHandler* rspfGdalFactory::open(const rspfFilename& fileName,
                                          bool openOverview)const
{
   
   if(fileName.ext().downcase() == "nui") return 0;
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalFactory::open(filename) DEBUG: entered..." << std::endl;
   }
   
   rspfRefPtr<rspfImageHandler> result;
   if (traceDebug())
   {
     rspfNotify(rspfNotifyLevel_DEBUG)
       << "rspfGdalFactory::open(filename) DEBUG:"
       << "\ntrying rspfHdfReader"
       << std::endl;
   }
   result = new rspfHdfReader;
   result->setOpenOverviewFlag(openOverview);
   if(result->open(fileName))
   {
     return result.release();
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalFactory::open(filename) DEBUG:"
         << "\ntrying rspfGdalTileSource"
         << std::endl;
   }
   result = new rspfGdalTileSource;
   result->setOpenOverviewFlag(openOverview);   
   if(result->open(fileName))
   {
      return result.release();
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalFactory::open(filename) DEBUG:"
         << "\ntrying rspfOgrVectorTileSource\n";
   }
   result = new rspfOgrVectorTileSource;
   if(result->open(fileName))
   {
     return result.release();
   }
   
   if(!fileName.exists() || 
       fileName.before(":", 3).upcase() == "SDE" || 
       fileName.before(":", 4).upcase() == "GLTP" || 
       fileName.ext().downcase() == "mdb")
   {
      result = 0;
      return result.release();
   }
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalFactory::open(filename) DEBUG:"
         << "\ntrying rspfOgrGdalTileSource\n";
   }
   
   result = new rspfOgrGdalTileSource;
   if(result->open(fileName))
   {
      return result.release();
   }
   result = 0;
   return result.release();
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalFactory::open(filename) DEBUG: leaving..."
         << std::endl;
   }
   
   return (rspfImageHandler*)NULL;
}
rspfImageHandler* rspfGdalFactory::open(const rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   rspfRefPtr<rspfImageHandler> result;
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalFactory::open(kwl, prefix) DEBUG: entered..." << std::endl;
   }
   if(traceDebug())
   {
     rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalFactory::open(kwl, prefix) DEBUG: trying rspfHdfReader" << std::endl;
   }
   result = new rspfHdfReader;
   if(result->loadState(kwl, prefix))
   {
     return result.release();
   }
   result = 0;
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalFactory::open(kwl, prefix) DEBUG: trying rspfGdalTileSource" << std::endl;
   }
   result = new rspfGdalTileSource;
   if(result->loadState(kwl, prefix))
   {
      return result.release();
   }
   result = 0;
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalFactory::open(kwl, prefix) DEBUG: trying rspfOgrGdalTileSource" << std::endl;
   }
   
   result = new rspfOgrGdalTileSource;
   if(result->loadState(kwl, prefix))
   {
      return result.release();
   }
   result = 0;
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalFactory::open(kwl, prefix) DEBUG: leaving..." << std::endl;
   }
   
   return result.release();
}
rspfObject* rspfGdalFactory::createObject(const rspfString& typeName)const
{
   if(STATIC_TYPE_NAME(rspfHdfReader) == typeName)
   {
     return new rspfHdfReader();
   }
   if(STATIC_TYPE_NAME(rspfGdalTileSource) == typeName)
   {
      return new rspfGdalTileSource();
   }
   if(STATIC_TYPE_NAME(rspfOgrGdalTileSource) == typeName)
   {
      return new rspfOgrGdalTileSource();
   }
   
   return (rspfObject*)0;
}
rspfObject* rspfGdalFactory::createObject(const rspfKeywordlist& kwl,
                                                    const char* prefix)const
{
   rspfRefPtr<rspfObject> result;
   const char* type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if(type)
   {
      if (rspfString(type).trim() == STATIC_TYPE_NAME(rspfImageHandler))
      {
         const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
         if (lookup)
         {
            result = this->open(kwl, prefix);//rspfFilename(lookup));
         }
      }
      else
      {
         result = createObject(rspfString(type));
         if(result.valid())
         {
            result->loadState(kwl, prefix);
         }
      }
   }
   return result.release();
}
 
void rspfGdalFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfHdfReader));
   typeList.push_back(STATIC_TYPE_NAME(rspfGdalTileSource));
   typeList.push_back(STATIC_TYPE_NAME(rspfOgrGdalTileSource));
}
void rspfGdalFactory::getSupportedExtensions(rspfImageHandlerFactoryBase::UniqueStringList& extensionList)const
{
   int driverCount = GDALGetDriverCount();
   int idx = 0;
   
   for(idx = 0; idx < driverCount; ++idx)
   {
      GDALDriverH driver =  GDALGetDriver(idx);  
      
      if(driver)
      {
         const char* metaData = GDALGetMetadataItem(driver, GDAL_DMD_EXTENSION, 0);
         int nMetaData = metaData ? strlen(metaData) : 0;
         if(metaData && nMetaData>0 )
         {
            std::vector<rspfString> splitArray;
            rspfString(metaData).split(splitArray, " /");
            rspf_uint32 idxExtension = 0;
            for(idxExtension = 0; idxExtension < splitArray.size(); ++idxExtension)
            {
               extensionList.push_back(splitArray[idxExtension].downcase());
            }
         }
      }
   }
   if(GDALGetDriverByName("AAIGrid"))
   {
      extensionList.push_back("adf");
   }
}
void rspfGdalFactory::getImageHandlersBySuffix(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& ext)const
{
   rspfImageHandlerFactoryBase::UniqueStringList extList;
   getSupportedExtensions(extList);
   
   rspfString testExt = ext.downcase();
   
   if(std::find(extList.getList().begin(),
                extList.getList().end(), testExt) != extList.getList().end())
   {
      result.push_back(new rspfGdalTileSource);
   }
}
void rspfGdalFactory::getImageHandlersByMimeType(rspfImageHandlerFactoryBase::ImageHandlerList& result, const rspfString& mimeType)const
{
   int driverCount = GDALGetDriverCount();
   int idx = 0;
   
   for(idx = 0; idx < driverCount; ++idx)
   {
      GDALDriverH driver =  GDALGetDriver(idx);  
      
      if(driver)
      {
         const char* metaData = GDALGetMetadataItem(driver, GDAL_DMD_MIMETYPE, 0);
         int nMetaData = metaData ? strlen(metaData) : 0;
         if(metaData && nMetaData>0 )
         {
            if(rspfString(metaData) == mimeType)
            {
               result.push_back(new rspfGdalTileSource());
               return;
            }
         }
      }
   }
}
