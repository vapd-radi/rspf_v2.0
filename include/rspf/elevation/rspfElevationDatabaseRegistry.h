#ifndef rspfElevationDatabaseRegistry_HEADER
#define rspfElevationDatabaseRegistry_HEADER
#include <rspf/elevation/rspfElevationDatabaseFactoryBase.h>
#include <rspf/elevation/rspfElevationDatabase.h>
#include <rspf/base/rspfFactoryListInterface.h>
class RSPF_DLL rspfElevationDatabaseRegistry : public rspfFactoryListInterface<rspfElevationDatabaseFactoryBase, rspfElevationDatabase>

{
public:
   rspfElevationDatabaseRegistry()
   {
      m_instance = 0;
   }
   static rspfElevationDatabaseRegistry* instance();
   void getTypeNameList(std::vector<rspfString>& typeList) const 
   {
      getAllTypeNamesFromRegistry(typeList);
   }
   rspfElevationDatabase* createDatabase(const rspfString& typeName)const;
   rspfElevationDatabase* createDatabase(const rspfKeywordlist& kwl,
                                          const char* prefix=0)const;
   rspfElevationDatabase* open(const rspfString& connectionString);

protected:
   static rspfElevationDatabaseRegistry* m_instance;
};
#endif
