#ifndef rspfElevationDatabaseFactory_HEADER
#define rspfElevationDatabaseFactory_HEADER
#include "rspfElevationDatabaseFactoryBase.h"

class RSPF_DLL rspfElevationDatabaseFactory : public rspfElevationDatabaseFactoryBase
{
public:
   rspfElevationDatabaseFactory()
   {
      m_instance = this;
   }
   static rspfElevationDatabaseFactory* instance();
   
   rspfElevationDatabase* createDatabase(const rspfString& typeName)const;
   rspfElevationDatabase* createDatabase(const rspfKeywordlist& kwl,
                                          const char* prefix=0)const;
   virtual rspfElevationDatabase* open(const rspfString& connectionString)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
protected:
   static rspfElevationDatabaseFactory* m_instance;
};
#endif
