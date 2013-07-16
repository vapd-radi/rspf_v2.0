#include <rspf/elevation/rspfElevationDatabaseRegistry.h>
#include <rspf/elevation/rspfElevationDatabaseFactory.h>

rspfElevationDatabaseRegistry* rspfElevationDatabaseRegistry::m_instance = 0;
rspfElevationDatabaseRegistry* rspfElevationDatabaseRegistry::instance()
{
   if(!m_instance)
   {
      m_instance = new rspfElevationDatabaseRegistry;
      
      m_instance->registerFactory(rspfElevationDatabaseFactory::instance());
   }
   
   return m_instance;
}

rspfElevationDatabase* rspfElevationDatabaseRegistry::createDatabase(const rspfString& typeName)const
{
   rspfElevationDatabase* result = 0;
   rspf_uint32 idx = 0;
   for(;((idx < m_factoryList.size())&&!result); ++idx)
   {
      result = m_factoryList[idx]->createDatabase(typeName);
   }
   
   return result;
   
}
rspfElevationDatabase* rspfElevationDatabaseRegistry::createDatabase(const rspfKeywordlist& kwl,
                                                                       const char* prefix)const
{
   rspfElevationDatabase* result = 0;
   rspf_uint32 idx = 0;
   for(;((idx < m_factoryList.size())&&!result); ++idx)
   {
      result = m_factoryList[idx]->createDatabase(kwl, prefix);
   }
   
   return result;
}

rspfElevationDatabase* rspfElevationDatabaseRegistry::open(const rspfString& connectionString)
{
   rspfElevationDatabase* result = 0;
   rspf_uint32 idx = 0;
   for(;((idx < m_factoryList.size())&&!result); ++idx)
   {
      result = m_factoryList[idx]->open(connectionString);
   }
   
   return result;
}

