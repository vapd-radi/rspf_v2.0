#ifndef rspfMiscProjectionFactory_HEADER
#define rspfMiscProjectionFactory_HEADER
#include <rspf/projection/rspfProjectionFactoryBase.h>
class rspfProjection;
class rspfString;
class rspfMiscProjectionFactory : public rspfProjectionFactoryBase
{
public:
   /*!
    * METHOD: instance()
    * Instantiates singleton instance of this class:
    */
   static rspfMiscProjectionFactory* instance();
   /*!
    * METHOD: create()
    * Attempts to create an instance of the projection specified by name.
    * Returns successfully constructed projection or NULL.
    */
   virtual rspfProjection* createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx)const;
   virtual rspfProjection* createProjection(const rspfString& name)const;
   virtual rspfProjection* createProjection(const rspfKeywordlist& kwl,
                                             const char* prefix = 0)const;
   virtual rspfObject* createObject(const rspfString& typeName)const;
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
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
protected:
   rspfMiscProjectionFactory() {}
   static rspfMiscProjectionFactory*  theInstance;
 
};
#endif
