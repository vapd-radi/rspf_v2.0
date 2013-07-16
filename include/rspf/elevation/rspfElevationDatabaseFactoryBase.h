#ifndef rspfElevationDatabaseFactoryBase_HEADER
#define rspfElevationDatabaseFactoryBase_HEADER
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/elevation/rspfElevationDatabase.h>

class RSPF_DLL rspfElevationDatabaseFactoryBase : public rspfObjectFactory
{
public:
   /**
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const
   {
      return createDatabase(typeName);
   }
   
   /**
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const
   {
      return createDatabase(kwl, prefix);
   }
   
   virtual rspfElevationDatabase* createDatabase(const rspfString& typeName)const=0;
   virtual rspfElevationDatabase* createDatabase(const rspfKeywordlist& kwl,
                                                  const char* prefix=0)const=0;
   virtual rspfElevationDatabase* open(const rspfString& connectionString)const=0;
};

#endif
