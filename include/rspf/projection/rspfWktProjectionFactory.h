#ifndef rspfWktProjectionFactory_HEADER
#define rspfWktProjectionFactory_HEADER
#include <rspf/projection/rspfProjectionFactoryBase.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
class rspfProjection;
class rspfString;
class rspfMapProjection;
class RSPFDLLEXPORT rspfWktProjectionFactory : public rspfProjectionFactoryBase
{
public:
   /*!
    * METHOD: instance()
    * Instantiates singleton instance of this class:
    */
   static rspfWktProjectionFactory* instance();
   virtual rspfProjection* createProjection(const rspfFilename& filename,
                                             rspf_uint32 entryIdx) const;
   virtual rspfProjection* createProjection(const rspfString& name) const;
   virtual rspfProjection* createProjection(const rspfKeywordlist& kwl,
                                             const char* prefix = 0)const;
   virtual rspfObject* createObject(const rspfString& typeName)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl, const char* prefix=0)const;
   
   virtual void getTypeNameList(std::vector<rspfString>& typeList) const;
protected:
   rspfWktProjectionFactory();
   struct WktProjRecord : public rspfReferenced
   {
      WktProjRecord() : epsgCode(0), wktString(""), proj(0) {}
      rspf_uint32     epsgCode;
      rspfString      wktString;
      rspfRefPtr<rspfMapProjection> proj;
   };
   static rspfWktProjectionFactory*  m_instance; //!< Singleton implementation
   mutable std::vector<rspfRefPtr<WktProjRecord> > m_wktProjRecords;
};
#endif
