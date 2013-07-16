#include <rspf/projection/rspfProjectionViewControllerFactory.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/projection/rspfMapViewController.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
RTTI_DEF1(rspfProjectionViewControllerFactory, "rspfProjectionViewControllerFactory", rspfObjectFactory);
rspfProjectionViewControllerFactory* rspfProjectionViewControllerFactory::theInstance = NULL;
rspfProjectionViewControllerFactory* rspfProjectionViewControllerFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfProjectionViewControllerFactory;
   }
   return theInstance;
}
rspfViewController* rspfProjectionViewControllerFactory::createViewController(const rspfString& name)const
{
   if(name == STATIC_TYPE_NAME(rspfMapViewController))
   {
      return new rspfMapViewController;
   }
   return (rspfViewController*)NULL;
}
rspfViewController* rspfProjectionViewControllerFactory::createViewController(const rspfKeywordlist& kwl,
                                                                                const char* prefix)const
{
   const char* type = kwl.find(prefix,  rspfKeywordNames::TYPE_KW);
   rspfViewController* result = NULL;
   if(type)
   {
      result = createViewController(rspfString(type));
      if(result)
      {
         if(!result->loadState(kwl, prefix))
         {
            delete result;
            result = NULL;
         }
      }
   }
   
   return result;
}
rspfObject* rspfProjectionViewControllerFactory::createObject(const rspfString& typeName)const
{
   return createViewController(typeName);
}
rspfObject* rspfProjectionViewControllerFactory::createObject(const rspfKeywordlist& kwl,
                                                                const char* prefix)const
{
   return createViewController(kwl, prefix);
}
void rspfProjectionViewControllerFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back("rspfMapViewController");
}
