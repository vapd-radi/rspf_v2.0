#ifndef rspfPropertyInterfaceRegistry_HEADER
#define rspfPropertyInterfaceRegistry_HEADER
#include <rspf/base/rspfObjectFactory.h>

class rspfPropertyInterfaceFactory;
class rspfPropertyInterface;

#include <rspf/base/rspfRtti.h>

#include <vector>
using namespace std;

class RSPFDLLEXPORT rspfPropertyInterfaceRegistry : public rspfObjectFactory
{
public:
   virtual ~rspfPropertyInterfaceRegistry();
   
   static rspfPropertyInterfaceRegistry* instance();
   
   virtual rspfPropertyInterface* createInterface( const rspfObject* obj ) const;

   /*!
    * Creates an object given a type name.
    */
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
   
   bool addFactory( rspfPropertyInterfaceFactory* factory );
   bool registerFactory(rspfPropertyInterfaceFactory* factory);
protected:
   rspfPropertyInterfaceRegistry()
      :rspfObjectFactory()
      {}

   rspfPropertyInterfaceRegistry( const rspfPropertyInterfaceRegistry& rhs )
      :rspfObjectFactory(rhs)
      {}
   void operator =(const rspfPropertyInterfaceRegistry&){}
   static rspfPropertyInterfaceRegistry* theInstance;
   vector<rspfPropertyInterfaceFactory*> theFactoryList;

TYPE_DATA
};

extern "C"
{
  RSPFDLLEXPORT void* rspfPropertyInterfaceRegistryGetInstance();
}
#endif
