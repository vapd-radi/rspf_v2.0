#include <rspf/elevation/rspfElevationDatabaseFactory.h>
#include <rspf/elevation/rspfDtedElevationDatabase.h>
#include <rspf/elevation/rspfSrtmElevationDatabase.h>
#include <rspf/elevation/rspfGeneralRasterElevationDatabase.h>
#include <rspf/elevation/rspfImageElevationDatabase.h>
#include <rspf/base/rspfKeywordNames.h>

rspfElevationDatabaseFactory* rspfElevationDatabaseFactory::m_instance = 0;
rspfElevationDatabaseFactory* rspfElevationDatabaseFactory::instance()
{
   if(!m_instance)
   {
      m_instance = new rspfElevationDatabaseFactory();
   }
   
   return m_instance;
}

rspfElevationDatabase* rspfElevationDatabaseFactory::createDatabase(const rspfString& typeName)const
{
   if((typeName == STATIC_TYPE_NAME(rspfDtedElevationDatabase)) ||
      (typeName == "dted")||
      (typeName == "dted_directory"))
      
   {
      return new rspfDtedElevationDatabase();
   }
   else if((typeName == STATIC_TYPE_NAME(rspfSrtmElevationDatabase)) ||
           (typeName == "srtm")||
           (typeName == "srtm_directory"))
      
   {
      return new rspfSrtmElevationDatabase();
   }
   else if((typeName == STATIC_TYPE_NAME(rspfGeneralRasterElevationDatabase)) ||
           (typeName == "general_raster")||
           (typeName == "general_raster_directory"))
      
   {
      return new rspfGeneralRasterElevationDatabase();
   }
   else if( (typeName == "rspfImageElevationDatabase") ||
            (typeName == "image")||
            (typeName == "image_directory"))
      
   {
      return new rspfImageElevationDatabase();
   }
   
   return 0;
}

rspfElevationDatabase* rspfElevationDatabaseFactory::createDatabase(const rspfKeywordlist& kwl,
                                                                      const char* prefix)const
{
   rspfRefPtr<rspfElevationDatabase> result = 0;
   rspfString type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if(!type.empty())
   {
      result = createDatabase(type);
      if(result.valid())
      {
         if(!result->loadState(kwl, prefix))
         {
            result = 0;
         }
      }
   }
   
   return result.release();
}

rspfElevationDatabase* rspfElevationDatabaseFactory::open(const rspfString& connectionString)const
{
   rspfRefPtr<rspfElevationDatabase> result = new rspfDtedElevationDatabase();
   if(!result->open(connectionString))
   {
      result = new rspfSrtmElevationDatabase;
      if(!result->open(connectionString))
      {
         result = new rspfGeneralRasterElevationDatabase;
         if(!result->open(connectionString))
         {
            result = 0;
         }
         
#if 0         
         //---
         // Commented out as the rspfImageElevationDatabase can load any type and the
         // elevation manager is passing in elevation. This stops it opening your entire
         // elevation directory if you happen to start you application where there is an
         // elevation directory in there. Note you can still explicitly declare a
         // "image_directory in your preferences.  drb - 20110509
         //---
         if(!result->open(connectionString))
         {
            result = new rspfImageElevationDatabase;
            if(!result->open(connectionString))
            {
               result = 0;
            }
         }
#endif
      }
   }
   return result.release();
}

void rspfElevationDatabaseFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfDtedElevationDatabase));
   typeList.push_back(STATIC_TYPE_NAME(rspfSrtmElevationDatabase));
   typeList.push_back(STATIC_TYPE_NAME(rspfGeneralRasterElevationDatabase));
   typeList.push_back(STATIC_TYPE_NAME(rspfImageElevationDatabase));
}
