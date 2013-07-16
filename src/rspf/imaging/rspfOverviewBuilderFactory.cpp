//----------------------------------------------------------------------------
// 
// See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Factory for overview builders.
//
//----------------------------------------------------------------------------
// $Id: rspfOverviewBuilderFactory.cpp 17709 2010-07-08 20:21:14Z dburken $


#include <rspf/imaging/rspfOverviewBuilderFactory.h>
#include <rspf/imaging/rspfTiffOverviewBuilder.h>

rspfOverviewBuilderFactory* rspfOverviewBuilderFactory::theInstance = 0;

rspfOverviewBuilderFactory* rspfOverviewBuilderFactory::instance()
{
   if ( !theInstance )
   {
      theInstance = new rspfOverviewBuilderFactory();
   }
   return theInstance;
}

rspfOverviewBuilderFactory::~rspfOverviewBuilderFactory()
{
   theInstance = 0;
}

rspfOverviewBuilderBase* rspfOverviewBuilderFactory::createBuilder(
   const rspfString& typeName) const
{
   rspfRefPtr<rspfOverviewBuilderBase> result = new rspfTiffOverviewBuilder();
   if ( result->hasOverviewType(typeName) == false )
   {
      result = 0;
   }
   if ( result.get() )
   {
      result->setOverviewType(typeName);
   }
   return result.release();
}

void rspfOverviewBuilderFactory::getTypeNameList(
   std::vector<rspfString>& typeList) const
{
   rspfRefPtr<rspfOverviewBuilderBase> builder = new  rspfTiffOverviewBuilder();
   builder->getTypeNameList(typeList);
}

rspfOverviewBuilderFactory::rspfOverviewBuilderFactory()
{
}

rspfOverviewBuilderFactory::rspfOverviewBuilderFactory(
   const rspfOverviewBuilderFactory& /* obj */)
{
}

void rspfOverviewBuilderFactory::operator=(
   const rspfOverviewBuilderFactory& /* rhs */)
{
}
