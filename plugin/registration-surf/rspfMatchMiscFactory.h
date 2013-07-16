#ifndef rspfMatchMiscFactory_HEADER
#define rspfMatchMiscFactory_HEADER
#include "rspfMatchExports.h"
#include <rspf/base/rspfObjectFactory.h>

class RSPF_REGISTRATION_DLL rspfMatchMiscFactory : public rspfObjectFactory
{
public:
   rspfMatchMiscFactory(){}
    virtual ~rspfMatchMiscFactory(){theInstance = 0;}
   
   static rspfMatchMiscFactory* instance();
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
   static rspfMatchMiscFactory* theInstance;
   
TYPE_DATA

  
};

#endif
