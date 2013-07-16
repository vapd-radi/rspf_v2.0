#include <rspf/projection/rspfWktProjectionFactory.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfOrthoGraphicProjection.h>
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/projection/rspfEpsgProjectionDatabase.h>
#include <sstream>
rspfWktProjectionFactory* rspfWktProjectionFactory::m_instance = 0;
static const rspfString WKT_MAGIC ("WKT_PCS");
rspfWktProjectionFactory* rspfWktProjectionFactory::instance()
{
   if(!m_instance)
      m_instance = new rspfWktProjectionFactory;
   return m_instance;
}
rspfWktProjectionFactory::rspfWktProjectionFactory()
{
   rspfFilename db_name = 
      rspfPreferences::instance()->preferencesKWL().find("wkt_database_file");
   if (!db_name.isReadable())
      return;
   rspfString format_id;
   rspfString line;
   std::ifstream db_stream (db_name.chars());
   bool good_file = false;
   if (db_stream.good())
   {
      std::getline(db_stream, format_id.string());
      format_id.trim();
      if (format_id == WKT_MAGIC)
         good_file = true;
   }
   if (!good_file)
   {
      rspfNotify(rspfNotifyLevel_WARN)<<"rspfWktProjectionDatabase Constructor -- "
         "Encountered bad WKT database file <"<<db_name<<">. Skipping this file."<<endl;
      db_stream.close();
      return;
   }
   std::getline(db_stream, line.string());
   while (!db_stream.eof())
   {
      rspfRefPtr<WktProjRecord> db_record = new WktProjRecord;
      std::getline(db_stream, line.string());
      std::vector<rspfString> csvRecord = line.explode(","); // ONLY CSV FILES CONSIDERED HERE
      if (csvRecord.size())
      {
         db_record->epsgCode  = csvRecord[0].toUInt32();
         db_record->wktString = csvRecord[1];
         m_wktProjRecords.push_back(db_record);
      }
   }
   db_stream.close();
}
rspfProjection* rspfWktProjectionFactory::createProjection(const rspfKeywordlist &keywordList,
                                                             const char *prefix) const
{
    rspfProjection* proj = 0;
   rspfString proj_spec = keywordList.find(prefix, rspfKeywordNames::PROJECTION_KW);
   if(!proj_spec.empty())
      proj = createProjection(proj_spec);
  
   if (!proj)
      return NULL;
   rspfKeywordlist proj_kwl;
   proj->saveState(proj_kwl, prefix);
   proj_kwl.remove(prefix, rspfKeywordNames::PIXEL_SCALE_XY_KW);
   proj_kwl.remove(prefix, rspfKeywordNames::PIXEL_SCALE_UNITS_KW);
   proj_kwl.remove(prefix, rspfKeywordNames::TIE_POINT_XY_KW);
   proj_kwl.remove(prefix, rspfKeywordNames::TIE_POINT_UNITS_KW);
   proj_kwl.addList(keywordList, false); // false: do not override existing items
   proj->loadState(proj_kwl, prefix);
   
   return proj;
}
rspfProjection* rspfWktProjectionFactory::createProjection(const rspfString &spec) const
{
   rspfProjection* proj = 0;
   
   std::vector<rspfRefPtr<WktProjRecord> >::iterator db_iter = m_wktProjRecords.begin();
   while ((db_iter != m_wktProjRecords.end()) && !proj)
   {
      if ((*db_iter)->wktString == spec)
      {
         if ((*db_iter)->proj.valid())
            proj = (rspfProjection*) (*db_iter)->proj->dup();
         else
            proj = rspfEpsgProjectionDatabase::instance()->findProjection((*db_iter)->epsgCode);
      }
      db_iter++;
   }
   return proj;
}
rspfObject* rspfWktProjectionFactory::createObject(const rspfString& typeName)const
{
   return createProjection(typeName);
}
rspfObject* rspfWktProjectionFactory::createObject(const rspfKeywordlist& kwl,
                                                     const char* prefix)const
{
   return createProjection(kwl, prefix);
}
rspfProjection* rspfWktProjectionFactory::createProjection(const rspfFilename& /* filename */,
                                                              rspf_uint32 /* entryIdx */)const
{
   return 0;
}
void rspfWktProjectionFactory::getTypeNameList(std::vector<rspfString>& typeList) const
{
   std::vector<rspfRefPtr<WktProjRecord> >::iterator db_iter = m_wktProjRecords.begin();
   while (db_iter != m_wktProjRecords.end())
   {
      typeList.push_back((*db_iter)->wktString);
      db_iter++;
   }
   return;
}
