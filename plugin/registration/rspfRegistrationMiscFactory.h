#ifndef rspfRegistrationMiscFactory_HEADER
#define rspfRegistrationMiscFactory_HEADER
#include "rspfRegistrationExports.h"
#include <rspf/base/rspfObjectFactory.h>

class RSPF_REGISTRATION_DLL rspfRegistrationMiscFactory : public rspfObjectFactory
{
public:
   rspfRegistrationMiscFactory(){}
    virtual ~rspfRegistrationMiscFactory(){theInstance = 0;}
   
   static rspfRegistrationMiscFactory* instance();
   virtual rspfObject* createObject(const rspfString& typeName)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
protected:
   static rspfRegistrationMiscFactory* theInstance;
   
TYPE_DATA

  
};

#endif
