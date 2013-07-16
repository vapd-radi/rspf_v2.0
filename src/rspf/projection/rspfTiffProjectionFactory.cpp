#include <rspf/projection/rspfTiffProjectionFactory.h>
#include <rspf/support_data/rspfGeoTiff.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <fstream>
rspfTiffProjectionFactory* rspfTiffProjectionFactory::theInstance = 0;
rspfTiffProjectionFactory* rspfTiffProjectionFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfTiffProjectionFactory;
   }
   return (rspfTiffProjectionFactory*) theInstance;
}
rspfProjection*
rspfTiffProjectionFactory::createProjection(const rspfFilename& filename,
                                              rspf_uint32 entryIdx)const
{
   if(!filename.exists())
   {
      return 0;
   }
   if(isTiff(filename))
   {
      rspfGeoTiff geotiff(filename, entryIdx);
      rspfKeywordlist kwl;
      
      if(geotiff.addImageGeometry(kwl))
      {
         return rspfProjectionFactoryRegistry::instance()->
            createProjection(kwl);
      }
   }
   
   return 0;
}
rspfProjection*
rspfTiffProjectionFactory::createProjection(const rspfKeywordlist& /* keywordList */,
                                             const char* /* prefix */) const
{
   return 0;
}
rspfProjection* rspfTiffProjectionFactory::createProjection(const rspfString& /* name */) const
{
   return 0;
}
rspfProjection* rspfTiffProjectionFactory::createProjection(rspfImageHandler* handler)const
{
   rspfTiffTileSource* tiff = dynamic_cast<rspfTiffTileSource*> (handler);
   
   if(tiff)
   {
      rspfGeoTiff geotiff;
      rspfKeywordlist kwl;
      
      geotiff.readTags(tiff->tiffPtr(), tiff->getCurrentEntry(), false);
      
      if(geotiff.addImageGeometry(kwl))
      {
         return rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
      }
      
   }
   
   return 0;
}
rspfObject* rspfTiffProjectionFactory::createObject(const rspfString& typeName)const
{
   return (rspfObject*)createProjection(typeName);
}
rspfObject* rspfTiffProjectionFactory::createObject(const rspfKeywordlist& kwl,
                                                     const char* prefix)const
{
   return createProjection(kwl, prefix);
}
void rspfTiffProjectionFactory::getTypeNameList(std::vector<rspfString>& /* typeList */)const
{
}
bool rspfTiffProjectionFactory::isTiff(const rspfFilename& filename)const
{
   std::ifstream in;
   unsigned char header[2];
   in.open( filename.c_str(), ios::in|ios::binary);
   if( !in)
       return false;
   in.read( (char*)header, 2);
   if( (header[0] != 'M' || header[1] != 'M')
       && (header[0] != 'I' || header[1] != 'I') )
   {
      return false;
   }
   return true;
}
