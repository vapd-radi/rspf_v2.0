#include <rspf/projection/rspfMiscProjectionFactory.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/projection/rspfSonomaSensor.h>
#include <rspf/projection/rspfAffineProjection.h>
#include <rspf/projection/rspfBilinearProjection.h>
#include <rspf/projection/rspfQuadProjection.h>
#include <rspf/projection/rspfWarpProjection.h>
#include <rspf/projection/rspfRpcProjection.h>
#include <rspf/projection/rspfPolynomProjection.h>
rspfMiscProjectionFactory* rspfMiscProjectionFactory::theInstance = 0;
rspfMiscProjectionFactory* rspfMiscProjectionFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfMiscProjectionFactory;
   }
   return (rspfMiscProjectionFactory*) theInstance;
}
rspfProjection*
rspfMiscProjectionFactory::createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx) const
{
   return createProjectionFromGeometryFile(filename, entryIdx);
}
rspfProjection*
rspfMiscProjectionFactory::createProjection(const rspfKeywordlist &kwl,
                                             const char *prefix) const
{
   rspfProjection *result=NULL;
   
   const char *lookup = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if(lookup)
   {
      result = createProjection(rspfString(lookup).trim());
      if(result)
      {
         result->loadState(kwl, prefix);
      }
   }
   else
   {
      lookup = kwl.find(prefix, "geom_file");
      if(lookup)
      {
         rspfKeywordlist kwl2;
         kwl2.addFile(lookup);
         result = createProjection(kwl2, "projection.");
         if(!result)
         {
            result = createProjection(kwl2);
         }
      }      
   }
   return result;
}
rspfProjection*
rspfMiscProjectionFactory::createProjection(const rspfString &name) const
{
   if(name == STATIC_TYPE_NAME(rspfSonomaSensor))
   {
      return new rspfSonomaSensor;
   }
   if(name == STATIC_TYPE_NAME(rspfAffineProjection))
   {
      return new rspfAffineProjection;
   }
   if(name == STATIC_TYPE_NAME(rspfBilinearProjection))
   {
      return new rspfBilinearProjection;
   }
   if(name == STATIC_TYPE_NAME(rspfQuadProjection))
   {
      return new rspfQuadProjection;
   }
   if(name == STATIC_TYPE_NAME(rspfRpcProjection))
   {
      return new rspfRpcProjection;
   }
   if(name == STATIC_TYPE_NAME(rspfWarpProjection))
   {
      return new rspfWarpProjection;
   }
   if(name == STATIC_TYPE_NAME(rspfPolynomProjection))
   {
      return new rspfPolynomProjection;
   }
   return NULL;
}
rspfObject*
rspfMiscProjectionFactory::createObject(const rspfString& typeName)const
{
   return createProjection(typeName);
}
rspfObject*
rspfMiscProjectionFactory::createObject(const rspfKeywordlist& kwl,
                                                     const char* prefix)const
{
   return createProjection(kwl, prefix);
}
void rspfMiscProjectionFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfSonomaSensor)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfAffineProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfBilinearProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfRpcProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfQuadProjection)));   
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfWarpProjection)));
   typeList.push_back(rspfString(STATIC_TYPE_NAME(rspfPolynomProjection)));
}
