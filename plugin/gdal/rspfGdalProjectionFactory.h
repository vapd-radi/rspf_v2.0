#ifndef rspfGdalProjectionFactory_HEADER
#define rspfGdalProjectionFactory_HEADER
#include <rspf/projection/rspfProjectionFactoryBase.h>
#include "rspfOgcWktTranslator.h"
#include <rspfPluginConstants.h>
#include <list>
class rspfProjection;
class rspfString;
class rspfGdalProjectionFactory : public rspfProjectionFactoryBase
{
public:
   /*!
    * METHOD: instance()
    * Instantiates singleton instance of this class:
    */
   static rspfGdalProjectionFactory* instance();
   virtual rspfProjection* createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx)const;
   /*!
    * METHOD: create()
    * Attempts to create an instance of the projection specified by name.
    * Returns successfully constructed projection or NULL.
    */
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
   /*!
    * METHOD: getList()
    * Returns list of all projections represented by this factory:
    */
   virtual std::list<rspfString> getList()const;
protected:
   rspfGdalProjectionFactory() {}
   rspfOgcWktTranslator theWktTranslator;
   static rspfGdalProjectionFactory*  theInstance;
};
#endif
