//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Class definition of Registry (singleton) for datum factories.
//
//----------------------------------------------------------------------------
// $Id$

#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfEpsgDatumFactory.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfString.h>

rspfDatumFactoryRegistry* rspfDatumFactoryRegistry::theInstance = 0;

rspfDatumFactoryRegistry::~rspfDatumFactoryRegistry()
{
}

rspfDatumFactoryRegistry* rspfDatumFactoryRegistry::instance()
{
   if (!theInstance)
   {
      theInstance =  new rspfDatumFactoryRegistry;
   }
   return theInstance;
}

void rspfDatumFactoryRegistry::registerFactory(
   rspfDatumFactoryInterface* factory)
{
   if (factory)
   {
      theFactoryList.push_back(factory);
   }
}

const rspfDatum* rspfDatumFactoryRegistry::create(
   const rspfString &code)const
{
   const rspfDatum* result = 0;

   std::vector<rspfDatumFactoryInterface*>::const_iterator i =
      theFactoryList.begin();

   while (i != theFactoryList.end())
   {
      result = (*i)->create(code);
      if (result)
      {
         break;
      }
      ++i; // go to next factory
   }
   return result;
}

const rspfDatum* rspfDatumFactoryRegistry::create(const rspfKeywordlist& kwl, 
                                                    const char *prefix)const
{
   const rspfDatum* result = 0;
   std::vector<rspfDatumFactoryInterface*>::const_iterator i = theFactoryList.begin();

   while (i != theFactoryList.end())
   {
      result = (*i)->create(kwl, prefix);
      if (result)
         return result;

      ++i; // go to next factory
   }
   return result;
}

void rspfDatumFactoryRegistry::getList(std::vector<rspfString>& list) const
{
   std::vector<rspfDatumFactoryInterface*>::const_iterator i =
      theFactoryList.begin();
   while (i != theFactoryList.end())
   {
      (*i)->getList(list);
      ++i;
   }
}

rspfDatumFactoryRegistry::rspfDatumFactoryRegistry()
   : theFactoryList()
{
   registerFactory(rspfDatumFactory::instance());
   registerFactory(rspfEpsgDatumFactory::instance());
}
