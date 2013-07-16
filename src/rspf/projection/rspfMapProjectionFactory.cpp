#include <rspf/projection/rspfMapProjectionFactory.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/projection/rspfAlbersProjection.h>
#include <rspf/projection/rspfAzimEquDistProjection.h>
#include <rspf/projection/rspfBonneProjection.h>
#include <rspf/projection/rspfBngProjection.h>
#include <rspf/projection/rspfCassiniProjection.h>
#include <rspf/projection/rspfCylEquAreaProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfEckert4Projection.h>
#include <rspf/projection/rspfEckert6Projection.h>
#include <rspf/projection/rspfGnomonicProjection.h>
#include <rspf/projection/rspfLambertConformalConicProjection.h>
#include <rspf/projection/rspfLlxyProjection.h>
#include <rspf/projection/rspfMercatorProjection.h>
#include <rspf/projection/rspfMillerProjection.h>
#include <rspf/projection/rspfMollweidProjection.h>
#include <rspf/projection/rspfNewZealandMapGridProjection.h>
#include <rspf/projection/rspfObliqueMercatorProjection.h>
#include <rspf/projection/rspfOrthoGraphicProjection.h>
#include <rspf/projection/rspfPolarStereoProjection.h>
#include <rspf/projection/rspfPolyconicProjection.h>
#include <rspf/projection/rspfSinusoidalProjection.h>
#include <rspf/projection/rspfStereographicProjection.h>
#include <rspf/projection/rspfTransCylEquAreaProjection.h>
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/projection/rspfUpsProjection.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfVanDerGrintenProjection.h>
#include <rspf/projection/rspfSpaceObliqueMercatorProjection.h>
#include <rspf/support_data/rspfSpaceImagingGeom.h>
rspfMapProjectionFactory* rspfMapProjectionFactory::theInstance = 0;
rspfMapProjectionFactory* rspfMapProjectionFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfMapProjectionFactory;
   }
   return (rspfMapProjectionFactory*) theInstance;
}
rspfProjection*
rspfMapProjectionFactory::createProjection(const rspfFilename& filename,
                                            rspf_uint32 entryIdx)const
{
   if(!filename.exists())
   {
      return NULL;
   }
   rspfProjection* proj = createProjectionFromGeometryFile(filename,
                                                            entryIdx);
   if (proj)
   {
      return proj;
   }
   rspfFilename geomFile = filename;
   geomFile = geomFile.setExtension("geom");
   if(!geomFile.exists())
   {
      return NULL;
   }
   rspfKeywordlist kwl;
   if(kwl.addFile(geomFile))
   {
      return createProjection(kwl);
   }
   return NULL;
}
rspfProjection* rspfMapProjectionFactory::createProjection(const rspfKeywordlist &keywordList,
															 const char *prefix) const
{
   rspfProjection *result=NULL;
	
   const char *lookup = keywordList.find(prefix, rspfKeywordNames::TYPE_KW);
   const char *lookupSpaceImaging = keywordList.find(prefix, rspfSpaceImagingGeom::SIG_PRODUCER_KW);
   if(lookup)
   {
      result = createProjection(rspfString(lookup).trim());
      if(result)
      {
         result->loadState(keywordList, prefix);
      }
   }
   else
   {
      if(lookupSpaceImaging)
      {
		  rspfKeywordlist kwl;
		  rspfKeywordlist kwl2;
		  kwl.add(keywordList,
				  prefix,
				  true);
         
         rspfSpaceImagingGeom spaceImaging;
         
         spaceImaging.setGeometry(kwl);
         spaceImaging.exportToOssim(kwl2);
         result = rspfProjectionFactoryRegistry::instance()->createProjection(kwl2);
         if(result)
         {
            return result;
         }
      }
      lookup = keywordList.find(prefix, rspfKeywordNames::GEOM_FILE_KW);
      if(lookup)
      {
         rspfKeywordlist kwl;
         kwl.addFile(lookup);
         result = createProjection(kwl);
         
         if(!result)
         {
            result = createProjection(kwl, "projection.");
         }
      }      
   }
   return result;
}
rspfProjection* rspfMapProjectionFactory::createProjection(const rspfString &name) const
{
   if(name ==  STATIC_TYPE_NAME(rspfAlbersProjection))
      return new rspfAlbersProjection;
   if(name ==  STATIC_TYPE_NAME(rspfAzimEquDistProjection))
      return new rspfAzimEquDistProjection;
   if( name == STATIC_TYPE_NAME(rspfBonneProjection))
      return new rspfBonneProjection;
   if( name == STATIC_TYPE_NAME(rspfBngProjection))
      return new rspfBngProjection;
   if(name ==  STATIC_TYPE_NAME(rspfCassiniProjection))
      return new rspfCassiniProjection;
   if(name ==  STATIC_TYPE_NAME(rspfCylEquAreaProjection))
      return new rspfCylEquAreaProjection;
   if(name ==  STATIC_TYPE_NAME(rspfEquDistCylProjection))
      return new rspfEquDistCylProjection;
   if(name ==  STATIC_TYPE_NAME(rspfEckert4Projection))
      return new rspfEckert4Projection;
   if(name ==  STATIC_TYPE_NAME(rspfEckert6Projection))
      return new rspfEckert6Projection;
   if(name == STATIC_TYPE_NAME(rspfGnomonicProjection))
      return new rspfGnomonicProjection;
   if(name ==  STATIC_TYPE_NAME(rspfLambertConformalConicProjection))
      return new rspfLambertConformalConicProjection;
   if(name ==  STATIC_TYPE_NAME(rspfLlxyProjection))
      return new rspfLlxyProjection;
   if(name == STATIC_TYPE_NAME(rspfMercatorProjection))
      return new rspfMercatorProjection;
   if(name == STATIC_TYPE_NAME(rspfMillerProjection))
      return new rspfMillerProjection;
   if(name == STATIC_TYPE_NAME(rspfMollweidProjection))
      return new rspfMollweidProjection;
   if(name == STATIC_TYPE_NAME(rspfNewZealandMapGridProjection))
      return new rspfNewZealandMapGridProjection;
   if(name == STATIC_TYPE_NAME(rspfObliqueMercatorProjection))
      return new rspfObliqueMercatorProjection;
   if(name == STATIC_TYPE_NAME(rspfOrthoGraphicProjection))
      return new rspfOrthoGraphicProjection;
   if(name == STATIC_TYPE_NAME(rspfPolarStereoProjection))
      return new rspfPolarStereoProjection;
   if(name == STATIC_TYPE_NAME(rspfPolyconicProjection))
      return new rspfPolyconicProjection;
   if(name == STATIC_TYPE_NAME(rspfSinusoidalProjection))
      return new rspfSinusoidalProjection;
   if(name == STATIC_TYPE_NAME(rspfStereographicProjection))
      return new rspfStereographicProjection;
   if(name == STATIC_TYPE_NAME(rspfTransCylEquAreaProjection))
      return new rspfTransCylEquAreaProjection;
   if(name == STATIC_TYPE_NAME(rspfTransMercatorProjection))
      return new rspfTransMercatorProjection;
   if(name == STATIC_TYPE_NAME(rspfUpsProjection))
      return new rspfUpsProjection;
   if(name == STATIC_TYPE_NAME(rspfUtmProjection))
      return new rspfUtmProjection;
   if(name == STATIC_TYPE_NAME(rspfVanDerGrintenProjection))
      return new rspfVanDerGrintenProjection;
   if(name == STATIC_TYPE_NAME(rspfSpaceObliqueMercatorProjection))
      return new rspfSpaceObliqueMercatorProjection;
   return NULL;
}
rspfObject* rspfMapProjectionFactory::createObject(const rspfString& typeName)const
{
   return createProjection(typeName);
}
rspfObject* rspfMapProjectionFactory::createObject(const rspfKeywordlist& kwl,
                                                     const char* prefix)const
{
   return createProjection(kwl, prefix);
}
void rspfMapProjectionFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfAlbersProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfAzimEquDistProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfBonneProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfBngProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfCassiniProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfCylEquAreaProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfEquDistCylProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfEckert4Projection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfEckert6Projection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfGnomonicProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfLambertConformalConicProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfLlxyProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfMercatorProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfMillerProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfMollweidProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfNewZealandMapGridProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfObliqueMercatorProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfOrthoGraphicProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfPolarStereoProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfPolyconicProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfSinusoidalProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfStereographicProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfTransCylEquAreaProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfTransMercatorProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfUpsProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfUtmProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfVanDerGrintenProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfSpaceObliqueMercatorProjection)));
}
