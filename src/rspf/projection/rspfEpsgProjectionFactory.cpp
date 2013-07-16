#include <rspf/projection/rspfEpsgProjectionFactory.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfOrthoGraphicProjection.h>
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <sstream>
rspfEpsgProjectionFactory* rspfEpsgProjectionFactory::m_instance = 0;
rspfEpsgProjectionFactory* rspfEpsgProjectionFactory::instance()
{
   if(!m_instance)
      m_instance = new rspfEpsgProjectionFactory;
   return m_instance;
}
rspfEpsgProjectionFactory::rspfEpsgProjectionFactory()
{
   m_projDatabase = rspfEpsgProjectionDatabase::instance();
}
rspfProjection* rspfEpsgProjectionFactory::createProjection(const rspfKeywordlist &keywordList,
                                                             const char *prefix) const
{
    rspfProjection* proj = 0;
   rspfString proj_spec = keywordList.find(prefix, rspfKeywordNames::PCS_CODE_KW);
   if(!proj_spec.empty())
      proj = createProjection(proj_spec);
  
   if (!proj)
   {
      proj_spec = keywordList.find(prefix, rspfKeywordNames::PROJECTION_KW);
      proj = createProjection(proj_spec);
   }
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
rspfProjection* rspfEpsgProjectionFactory::createProjection(const rspfString &spec) const
{
   std::vector<rspfString> split_line;
   
   if (rspfString(spec).downcase().contains("auto"))
   {
      split_line = spec.after(":").explode(",");
      return createProjFromAutoCode(split_line);
   }
   rspfString s = spec;
   s.trim(rspfString("|"));
   return m_projDatabase->findProjection(s);
}
rspfMapProjection* 
rspfEpsgProjectionFactory::createProjFromAutoCode(const std::vector<rspfString>& spec) const
{
   if (spec.size() != 4)
      return 0;
   rspf_uint32 code = spec[0].toUInt32();
   rspfGpt origin (spec[3].toDouble(), spec[2].toDouble()) ;
   switch(code)
   {
      case 42001:
      {
         rspfUtmProjection* utm = new rspfUtmProjection;
         utm->setZone(origin);
         utm->setHemisphere(origin);
         utm->setOrigin(origin);
         utm->update();
         utm->setPcsCode(42001);
         return utm;
      }
      
      case 42002:
      {
         rspfTransMercatorProjection* transMerc = new rspfTransMercatorProjection;
         transMerc->setFalseNorthing(origin.latd()>=0.0?0.0:10000000.0);
         transMerc->setOrigin(rspfGpt(0.0, origin.lond()));
         transMerc->setScaleFactor(.9996);
         transMerc->update();
         transMerc->setPcsCode(42002);
         return transMerc;
      }
      
      case 42003:
      {
         rspfOrthoGraphicProjection* ortho = new rspfOrthoGraphicProjection;
         ortho->setOrigin(origin);
         ortho->update();
         ortho->setPcsCode(42003);
         return ortho;
      }
      
      case 42004:
      {
         rspfEquDistCylProjection* geographic = new rspfEquDistCylProjection;
         geographic->setOrigin(origin);
         geographic->update();
         geographic->setPcsCode(42004);
         return geographic;
      }
   }
   return 0;
}
rspfObject* rspfEpsgProjectionFactory::createObject(const rspfString& typeName)const
{
   return createProjection(typeName);
}
rspfObject* rspfEpsgProjectionFactory::createObject(const rspfKeywordlist& kwl,
                                                     const char* prefix)const
{
   return createProjection(kwl, prefix);
}
rspfProjection* rspfEpsgProjectionFactory::createProjection(const rspfFilename& /* filename */,
                                                              rspf_uint32 /* entryIdx */)const
{
   return 0;
}
void rspfEpsgProjectionFactory::getTypeNameList(std::vector<rspfString>& typeList) const
{
   m_projDatabase->getProjectionsList(typeList);
   return;
}
