#include <gdal.h>
#include <rspf/plugin/rspfSharedObjectBridge.h>
#include <rspfPluginConstants.h>
#include <rspfGdalFactory.h>
#include <rspfGdalObjectFactory.h>
#include <rspfGdalImageWriterFactory.h>
#include <rspfGdalInfoFactory.h>
#include <rspfGdalProjectionFactory.h>
#include <rspfGdalOverviewBuilderFactory.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/imaging/rspfOverviewBuilderFactoryRegistry.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/support_data/rspfInfoFactoryRegistry.h>
static void setDescription(rspfString& description)
{
   description = "GDAL Plugin\n\n";
   
   int driverCount = GDALGetDriverCount();
   int idx = 0;
   description += "GDAL Supported formats\n";
   for(idx = 0; idx < driverCount; ++idx)
   {
      GDALDriverH driver = GDALGetDriver(idx);
      if(driver)
      {
         description += "  name: ";
         description += rspfString(GDALGetDriverShortName(driver)) + " " + rspfString(GDALGetDriverLongName(driver)) + "\n";
      }
   }
}
extern "C"
{
   rspfSharedObjectInfo  myInfo;
   rspfString theDescription;
   std::vector<rspfString> theObjList;
   const char* getDescription()
   {
      return theDescription.c_str();
   }
   int getNumberOfClassNames()
   {
      return (int)theObjList.size();
   }
   const char* getClassName(int idx)
   {
      if(idx < (int)theObjList.size())
      {
         return theObjList[0].c_str();
      }
      return (const char*)0;
   }
   /* Note symbols need to be exported on windoze... */ 
   RSPF_PLUGINS_DLL void rspfSharedLibraryInitialize(
               rspfSharedObjectInfo** info, 
               const char* options)
   {    
      myInfo.getDescription = getDescription;
      myInfo.getNumberOfClassNames = getNumberOfClassNames;
      myInfo.getClassName = getClassName;
      
      *info = &myInfo;
      rspfKeywordlist kwl;
      kwl.parseString(rspfString(options));
      /* Register the readers... */
     rspfImageHandlerRegistry::instance()->
        registerFactory(rspfGdalFactory::instance(), rspfString(kwl.find("read_factory.location")).downcase() == "front" );
     /* Register the writers... */
     rspfImageWriterFactoryRegistry::instance()->
        registerFactory(rspfGdalImageWriterFactory::instance(), rspfString(kwl.find("writer_factory.location")).downcase() == "front" );
     /* Register the overview builder factory. */
     rspfOverviewBuilderFactoryRegistry::instance()->
        registerFactory(rspfGdalOverviewBuilderFactory::instance());
     rspfProjectionFactoryRegistry::instance()->
        registerFactory(rspfGdalProjectionFactory::instance());
     /* Register generic objects... */
     rspfObjectFactoryRegistry::instance()->
        registerFactory(rspfGdalObjectFactory::instance());
     /* Register gdal info factoy... */
     rspfInfoFactoryRegistry::instance()->
       registerFactory(rspfGdalInfoFactory::instance());
     setDescription(theDescription);
  }
   /* Note symbols need to be exported on windoze... */ 
  RSPF_PLUGINS_DLL void rspfSharedLibraryFinalize()
  {
     rspfImageHandlerRegistry::instance()->
        unregisterFactory(rspfGdalFactory::instance());
     rspfImageWriterFactoryRegistry::instance()->
        unregisterFactory(rspfGdalImageWriterFactory::instance());
     rspfOverviewBuilderFactoryRegistry::instance()->
        unregisterFactory(rspfGdalOverviewBuilderFactory::instance());
     rspfProjectionFactoryRegistry::instance()->unregisterFactory(rspfGdalProjectionFactory::instance());
     rspfObjectFactoryRegistry::instance()->
        unregisterFactory(rspfGdalObjectFactory::instance());
     rspfInfoFactoryRegistry::instance()->
       unregisterFactory(rspfGdalInfoFactory::instance());
  }
}
