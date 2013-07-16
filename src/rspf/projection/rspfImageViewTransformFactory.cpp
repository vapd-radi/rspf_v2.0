#include <rspf/projection/rspfImageViewTransformFactory.h>
#include <rspf/projection/rspfImageViewProjectionTransform.h>
#include <rspf/projection/rspfImageViewAffineTransform.h>
#include <rspf/base/rspfKeywordlist.h>
RTTI_DEF1(rspfImageViewTransformFactory, "rspfImageViewTransformFactory", rspfObject)
rspfImageViewTransformFactory* rspfImageViewTransformFactory::theInstance = 0;
rspfImageViewTransformFactory* rspfImageViewTransformFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfImageViewTransformFactory;
   }
   return theInstance;
}
rspfImageViewTransform* rspfImageViewTransformFactory::createTransform(const rspfKeywordlist& kwl,
                                                                         const char* prefix)
{
   rspfImageViewTransform* result=NULL;
   
   const char *lookup = kwl.find(prefix, "type");
   if(lookup)
   {
      if(rspfString(lookup) == STATIC_TYPE_NAME(rspfImageViewProjectionTransform))
      {
         result = new rspfImageViewProjectionTransform;
      }
      if(rspfString(lookup) == STATIC_TYPE_NAME(rspfImageViewAffineTransform))
      {
         result = new rspfImageViewAffineTransform;
      }
   }
   if(result)
   {
      result->loadState(kwl, prefix);
   }
   
   return result;
}
   
