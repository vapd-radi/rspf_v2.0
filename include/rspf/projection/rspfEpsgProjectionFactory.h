#ifndef rspfEpsgProjectionFactory_HEADER
#define rspfEpsgProjectionFactory_HEADER
#include <rspf/projection/rspfEpsgProjectionDatabase.h>
#include <rspf/projection/rspfProjectionFactoryBase.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
class rspfProjection;
class rspfString;
class rspfMapProjection;
class RSPFDLLEXPORT rspfEpsgProjectionFactory : public rspfProjectionFactoryBase
{
public:
   /*!
    * METHOD: instance()
    * Instantiates singleton instance of this class:
    */
   static rspfEpsgProjectionFactory* instance();
   virtual rspfProjection* createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx) const;
   virtual rspfProjection* createProjection(const rspfString& name) const;
   virtual rspfProjection* createProjection(const rspfKeywordlist& kwl,
                                             const char* prefix = 0)const;
   virtual rspfObject* createObject(const rspfString& typeName)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl, const char* prefix=0)const;
   
   virtual void getTypeNameList(std::vector<rspfString>& typeList) const;
protected:
   rspfEpsgProjectionFactory();
   rspfMapProjection* createProjFromAutoCode(const std::vector<rspfString>& spec) const;
   rspfRefPtr<rspfEpsgProjectionDatabase>    m_projDatabase;
   static rspfEpsgProjectionFactory*  m_instance; //!< Singleton implementation
 
};
#endif
