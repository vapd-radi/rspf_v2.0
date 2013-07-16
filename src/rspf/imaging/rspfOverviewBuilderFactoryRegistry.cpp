//----------------------------------------------------------------------------
// 
// See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: The factory registry for overview builders.
//
//----------------------------------------------------------------------------
// $Id: rspfOverviewBuilderFactoryRegistry.cpp 19907 2011-08-05 19:55:46Z dburken $

#include <rspf/imaging/rspfOverviewBuilderFactoryRegistry.h>
#include <rspf/imaging/rspfOverviewBuilderFactoryBase.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <algorithm> /* for std::find */

rspfOverviewBuilderFactoryRegistry* rspfOverviewBuilderFactoryRegistry::m_instance = 0;

rspfOverviewBuilderFactoryRegistry* rspfOverviewBuilderFactoryRegistry::instance()
{
   if ( m_instance == 0 )
   {
      m_instance = new rspfOverviewBuilderFactoryRegistry();
      rspfObjectFactoryRegistry::instance()->registerFactory(m_instance);
   }
   return m_instance;
}

rspfObject* rspfOverviewBuilderFactoryRegistry::createObject(const rspfString& typeName)const
{
   return createObjectFromRegistry(typeName);
}

rspfObject* rspfOverviewBuilderFactoryRegistry::createObject(const rspfKeywordlist& kwl,
                                                               const char* prefix)const
{
   rspfRefPtr<NativeReturnType> result = 0;

   rspfString type = kwl.find(prefix, "type");
   if(!type.empty())
   {
      result = createBuilder(type);
      if(result.valid())
      {
         if(!result->loadState(kwl, prefix))
         {
            result = 0;
         }
      }
   }
   
   return result.release();
}

rspfOverviewBuilderBase*
rspfOverviewBuilderFactoryRegistry::createBuilder(
   const rspfString& typeName) const
{
   FactoryListType::const_iterator iter = m_factoryList.begin();
   NativeReturnType* result = 0;
   
   while(iter != m_factoryList.end())
   {
      result = (*iter)->createBuilder(typeName);
      if (result)
      {
         break;
      }
      ++iter;
   }
   
   return result;
}

void rspfOverviewBuilderFactoryRegistry::getTypeNameList(std::vector<rspfString>& typeList)const
{
   getAllTypeNamesFromRegistry(typeList);
}

rspfOverviewBuilderFactoryRegistry::rspfOverviewBuilderFactoryRegistry()
{
   m_instance = this;
}

rspfOverviewBuilderFactoryRegistry::rspfOverviewBuilderFactoryRegistry(
   const rspfOverviewBuilderFactoryRegistry& /* obj */ )
{
   m_instance = this;
}

void rspfOverviewBuilderFactoryRegistry::operator=(
   const rspfOverviewBuilderFactoryRegistry& /* rhs */ )
{
   
}
