

#include <rspf/base/rspfPropertyInterfaceRegistry.h>
#include <rspf/base/rspfPropertyInterfaceFactory.h>
#include <rspf/base/rspfString.h>
using namespace std;

RTTI_DEF1( rspfPropertyInterfaceRegistry, "rspfPropertyInterfaceRegistry", rspfObject );

rspfPropertyInterfaceRegistry* rspfPropertyInterfaceRegistry::theInstance = NULL;

rspfPropertyInterfaceRegistry::~rspfPropertyInterfaceRegistry()
{
   theInstance = NULL;
}

rspfPropertyInterface* rspfPropertyInterfaceRegistry::createInterface( const rspfObject* obj ) const
{
   vector<rspfPropertyInterfaceFactory*>::const_iterator i = theFactoryList.begin();
   rspfPropertyInterface* result = NULL;
   
   while ( ( i != theFactoryList.end() ) && ( ! result ) )
   {
      result = (*i)->createInterface( obj );

      ++i;
   }

   return result;
}

rspfPropertyInterfaceRegistry* rspfPropertyInterfaceRegistry::instance()
{
   if ( !theInstance)
   {
      theInstance = new rspfPropertyInterfaceRegistry;
   }

   return theInstance;
}

rspfObject* rspfPropertyInterfaceRegistry::createObject(const rspfString& typeName)const
{
   rspfObject* result = (rspfObject*)NULL;
   vector<rspfPropertyInterfaceFactory*>::const_iterator i = theFactoryList.begin();

   while ( ( i != theFactoryList.end() ) && ( ! result ) )
   {
      result = (*i)->createObject(typeName);

      ++i;
   }
   
   return result;
}

rspfObject* rspfPropertyInterfaceRegistry::createObject(const rspfKeywordlist& kwl,
                                                          const char* prefix)const
{
   rspfObject* result = (rspfObject*)NULL;
   vector<rspfPropertyInterfaceFactory*>::const_iterator i = theFactoryList.begin();

   while ( ( i != theFactoryList.end() ) && ( ! result ) )
   {
      result = (*i)->createObject(kwl, prefix);

      ++i;
   }
   
   return result;
}

void rspfPropertyInterfaceRegistry::getTypeNameList(std::vector<rspfString>& typeList)const
{
   vector<rspfPropertyInterfaceFactory*>::const_iterator i = theFactoryList.begin();

   while ( i != theFactoryList.end() )
   {
      std::vector<rspfString> tempTypeList;
      
      (*i)->getTypeNameList(tempTypeList);
      typeList.insert(typeList.end(),
                      tempTypeList.begin(),
                      tempTypeList.end());
      ++i;
   }
   
}

bool rspfPropertyInterfaceRegistry::addFactory( rspfPropertyInterfaceFactory* factory )
{
   return registerFactory(factory);
}

bool rspfPropertyInterfaceRegistry::registerFactory(rspfPropertyInterfaceFactory* factory)
{
   bool result = false;
   
   if ( factory )
   {
      theFactoryList.push_back( factory );
      result = true;
   }

   return result;
}

extern "C"
{
  void* rspfPropertyInterfaceRegistryGetInstance()
  {
    return rspfPropertyInterfaceRegistry::instance();
  }
}
