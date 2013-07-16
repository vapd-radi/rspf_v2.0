#ifndef rspfProjectionViewControllerFactory_HEADER
#define rspfProjectionViewControllerFactory_HEADER
#include <rspf/base/rspfBaseObjectFactory.h>
#include <rspf/base/rspfString.h>
class rspfViewController;
class rspfProjectionViewControllerFactory : public rspfObjectFactory
{
public:
   static rspfProjectionViewControllerFactory* instance();
   virtual rspfViewController* createViewController(const rspfString& name)const;
   virtual rspfViewController* createViewController(const rspfKeywordlist& kwl,
                                                     const char* prefix)const;
   virtual rspfObject* createObject(const rspfString& typeName)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
protected:
   rspfProjectionViewControllerFactory(){}
   rspfProjectionViewControllerFactory(const rspfProjectionViewControllerFactory& /* rhs */){}
   static rspfProjectionViewControllerFactory* theInstance;
TYPE_DATA
};
#endif
