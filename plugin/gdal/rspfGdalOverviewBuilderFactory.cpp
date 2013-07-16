#include <rspfGdalOverviewBuilderFactory.h>
#include <rspfGdalOverviewBuilder.h>
rspfGdalOverviewBuilderFactory*
rspfGdalOverviewBuilderFactory::theInstance = 0;
rspfGdalOverviewBuilderFactory* rspfGdalOverviewBuilderFactory::instance()
{
   if ( !theInstance )
   {
      theInstance = new rspfGdalOverviewBuilderFactory();
   }
   return theInstance;
}
rspfGdalOverviewBuilderFactory::~rspfGdalOverviewBuilderFactory()
{
   theInstance = 0;
}
rspfOverviewBuilderBase* rspfGdalOverviewBuilderFactory::createBuilder(
   const rspfString& typeName) const
{
   rspfRefPtr<rspfOverviewBuilderBase> result = new  rspfGdalOverviewBuilder();
   if ( result->hasOverviewType(typeName) == true )
   {
      result->setOverviewType(typeName);
   }
   else
   {
      result = 0;
   }
   
   return result.release();
}
void rspfGdalOverviewBuilderFactory::getTypeNameList(
   std::vector<rspfString>& typeList) const
{
   rspfRefPtr<rspfOverviewBuilderBase> builder = new  rspfGdalOverviewBuilder();
   builder->getTypeNameList(typeList);
}
rspfGdalOverviewBuilderFactory::rspfGdalOverviewBuilderFactory()
{
}
rspfGdalOverviewBuilderFactory::rspfGdalOverviewBuilderFactory(
   const rspfGdalOverviewBuilderFactory& /* obj */)
{
}
void rspfGdalOverviewBuilderFactory::operator=(
   const rspfGdalOverviewBuilderFactory& /* rhs */)
{
}
