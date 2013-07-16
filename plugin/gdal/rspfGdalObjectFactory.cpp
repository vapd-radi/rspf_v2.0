#include <rspfGdalObjectFactory.h>
#include <rspfShapeDatabase.h>
#include <rspfShapeFile.h>
#include <rspfEsriShapeFileFilter.h>
RTTI_DEF1(rspfGdalObjectFactory,
          "rspfGdalObjectFactory",
          rspfObjectFactory);
rspfGdalObjectFactory* rspfGdalObjectFactory::theInstance = 0;
rspfGdalObjectFactory* rspfGdalObjectFactory::instance()
{
   if ( !theInstance )
   {
      theInstance = new rspfGdalObjectFactory();
   }
   return theInstance;
}
rspfGdalObjectFactory::~rspfGdalObjectFactory()
{}
rspfObject* rspfGdalObjectFactory::createObject(
   const rspfString& typeName)const
{
   rspfObject* result = 0;
   if (typeName == "rspfShapeFile")
   {
      result = new rspfShapeFile();
   }
   else if (typeName == "rspfShapeDatabase")
   {
      result = new rspfShapeDatabase();
   }
   else if (typeName == "rspfEsriShapeFileFilter")
   {
      result = new rspfEsriShapeFileFilter();
   }
   
   return result;
   
}
rspfObject* rspfGdalObjectFactory::createObject(const rspfKeywordlist& kwl,
                                                  const char* prefix)const
{
   rspfObject* result = 0;
   const char* type = kwl.find(prefix, "type");
   if(type)
   {
      result = createObject(rspfString(type));
      if(result)
      {
         result->loadState(kwl, prefix);
      }
   }
   return result; 
}
   
void rspfGdalObjectFactory::getTypeNameList(
   std::vector<rspfString>& typeList)const
{
   typeList.push_back(rspfString("rspfShapeFile"));
   typeList.push_back(rspfString("rspfShapeDatabase"));
   typeList.push_back(rspfString("rspfEsriShapeFileFilter"));
}
rspfGdalObjectFactory::rspfGdalObjectFactory()
   : rspfObjectFactory()
{
}
rspfGdalObjectFactory::rspfGdalObjectFactory(
   const rspfGdalObjectFactory& /* rhs */)
   : rspfObjectFactory()
{
}
const rspfGdalObjectFactory& rspfGdalObjectFactory::operator=(
   const rspfGdalObjectFactory& /* rhs */ )
{
   return *this;
}
