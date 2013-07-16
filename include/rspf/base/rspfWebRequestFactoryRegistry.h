#ifndef rspfWebFactoryRegistry_HEADER
#define rspfWebFactoryRegistry_HEADER
#include <rspf/base/rspfFactoryListInterface.h>
#include <rspf/base/rspfWebRequestFactoryBase.h>
#include <rspf/base/rspfWebRequest.h>


/**
 * This will later be changed fro rspfHttpRequest to rspfWebRequest so we have 
 * a base class for general schema access such as ftp, sftp,smtp, ... etc and not
 * just restricted to http and https schema definitions.
 */
class RSPF_DLL rspfWebRequestFactoryRegistry : public rspfWebRequestFactoryBase,
public rspfFactoryListInterface<rspfWebRequestFactoryBase, rspfWebRequest>

{
public:
   static rspfWebRequestFactoryRegistry* instance();
 
   virtual rspfWebRequest* create(const rspfUrl& url);
   virtual rspfHttpRequest* createHttp(const rspfUrl& url);
   
   /*!
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const
   {
      return createObjectFromRegistry(typeName);
   }
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const
   {
      getAllTypeNamesFromRegistry(typeList);
   }
protected:
   rspfWebRequestFactoryRegistry();
   static rspfWebRequestFactoryRegistry* m_instance;
};
#endif
