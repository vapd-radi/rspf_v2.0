//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file
//
// Author:  David Burken
//
// Description: Class definition of registry (singleton) for info factories.
//
//----------------------------------------------------------------------------
// $Id$

#include <rspf/base/rspfFilename.h>
#include <rspf/support_data/rspfInfoFactoryRegistry.h>
#include <rspf/support_data/rspfInfoFactoryInterface.h>
#include <rspf/support_data/rspfInfoFactory.h>

#include <algorithm> /* for std::find */

rspfInfoFactoryRegistry* rspfInfoFactoryRegistry::m_instance = 0;

rspfInfoFactoryRegistry::~rspfInfoFactoryRegistry()
{
   m_factoryList.clear();
}

rspfInfoFactoryRegistry* rspfInfoFactoryRegistry::instance()
{
   if ( !m_instance )
   {
      m_instance = new rspfInfoFactoryRegistry;
   }
   return m_instance;
}

void rspfInfoFactoryRegistry::registerFactory(
   rspfInfoFactoryInterface* factory)
{
   if (factory)
   {
      m_mutex.lock();
      m_factoryList.push_back(factory);
      m_mutex.unlock();
   }
}

void rspfInfoFactoryRegistry::registerFactoryToFront(
   rspfInfoFactoryInterface* factory)
{
   if (factory)
   {
      m_mutex.lock();
      m_factoryList.insert( m_factoryList.begin(), factory );
      m_mutex.unlock();
   }
}

void rspfInfoFactoryRegistry::unregisterFactory(
   rspfInfoFactoryInterface* factory)
{
   m_mutex.lock();
   std::vector<rspfInfoFactoryInterface*>::iterator i =
      std::find(m_factoryList.begin(), m_factoryList.end(), factory);
   
   if( i != m_factoryList.end() )
   {
      m_factoryList.erase(i);
   }
   m_mutex.unlock();
}

rspfInfoBase* rspfInfoFactoryRegistry::create(
   const rspfFilename& file) const
{
   rspfInfoBase* result = 0;
   
   std::vector<rspfInfoFactoryInterface*>::const_iterator i =
      m_factoryList.begin();

   while ( i != m_factoryList.end() )
   {
      result = (*i)->create(file);
      if ( result )
      {
         break;
      }
      ++i;
   }

   return result;
}

/** hidden from use default constructor */
rspfInfoFactoryRegistry::rspfInfoFactoryRegistry()
   : m_factoryList(),
     m_mutex()
{
   this->registerFactory(rspfInfoFactory::instance());
}

/** hidden from use copy constructor */
rspfInfoFactoryRegistry::rspfInfoFactoryRegistry(
   const rspfInfoFactoryRegistry& /* obj */)
{}

/** hidden from use operator = */
const rspfInfoFactoryRegistry& rspfInfoFactoryRegistry::operator=(
   const rspfInfoFactoryRegistry& /* rhs */ )
{
   return *this;
}
