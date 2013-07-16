//*******************************************************************
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfProjectionFactoryRegistry.cpp 20413 2012-01-03 19:03:59Z dburken $
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfProjectionFactoryBase.h>
#include <rspf/projection/rspfEpsgProjectionFactory.h>
#include <rspf/projection/rspfWktProjectionFactory.h>
#include <rspf/projection/rspfMapProjectionFactory.h>
#include <rspf/projection/rspfTiffProjectionFactory.h>
#include <rspf/projection/rspfNitfProjectionFactory.h>
#include <rspf/projection/rspfSensorModelFactory.h>
#include <rspf/projection/rspfMiscProjectionFactory.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <algorithm>
#include <vector>

rspfProjectionFactoryRegistry* rspfProjectionFactoryRegistry::m_instance = 0;


rspfProjectionFactoryRegistry::rspfProjectionFactoryRegistry()
{
   initializeDefaults();
   rspfObjectFactoryRegistry::instance()->registerFactory(this);
}

rspfProjectionFactoryRegistry::rspfProjectionFactoryRegistry(const rspfProjectionFactoryRegistry& rhs)
:
rspfObjectFactory(rhs)
{}

void rspfProjectionFactoryRegistry::operator=(const rspfProjectionFactoryRegistry&)
{}

rspfProjectionFactoryRegistry::~rspfProjectionFactoryRegistry()
{
}

rspfProjectionFactoryRegistry* rspfProjectionFactoryRegistry::instance()
{
   if(!m_instance)
   {
      m_instance = new rspfProjectionFactoryRegistry();
   }
   
   return m_instance;
}

rspfProjection*
rspfProjectionFactoryRegistry::createProjection(const rspfFilename& name,
                                                 rspf_uint32 entryIdx)const
{
   rspfProjection* result = 0;
   rspf_uint32 idx = 0;
   for(idx = 0; ((idx < m_factoryList.size())&&(!result)); ++idx)
   {
      result = m_factoryList[idx]->createProjection(name, entryIdx);
   }
   
   return result;
}

rspfProjection* rspfProjectionFactoryRegistry::createProjection(rspfImageHandler* handler)const
{
   rspfProjection* result = 0;
   rspf_uint32 idx = 0;
   for(idx = 0; ((idx < m_factoryList.size())&&(!result)); ++idx)
   {
      result = m_factoryList[idx]->createProjection(handler);
   }
   
   return result;
}

rspfProjection* rspfProjectionFactoryRegistry::createProjection(
   const rspfKeywordlist& kwl, const char* prefix)const
{
   rspfProjection* result = 0;//createNativeObjectFromRegistry(kwl, prefix); 
   rspf_uint32 idx = 0; 
   for(idx = 0; ((idx < m_factoryList.size())&&!result);++idx) 
   { 
      result = m_factoryList[idx]->createProjection(kwl, prefix); 
   } 
   
   if ( (result == 0) && (prefix == 0) )
   {
      //---
      // Check for an embedded prefix like "image0."
      //
      // This is a workaround for people who use the "image_info" application
      // to create a geometry file.
      //
      // If the image has an external geometry file and it has a prefix like
      // "image0." and someone does:
      //
      // theInputConnection->getImageGeometry(kwl);
      // rspfRefPtr<rspfProjection> proj =
      // rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
      //
      // It will fail because the factory doesn't know it has a prefix.
      //
      // ESH 01/2009: I've changed the following so that not just the first
      // line is checked for "image" and ".".  If the image_info .geom file
      // was created with the -m (metadata) option, the first lines of the
      // file will not have the expected structure.
      //---
      bool bFoundImageLine = false;
      rspfKeywordlist::KeywordMap::const_iterator i = kwl.getMap().begin();
      while ( (i != kwl.getMap().end()) && (bFoundImageLine == false) )
      {
         rspfString s1 = (*i).first;
         if ( s1.size() )
         {
            std::vector<rspfString> v;
            s1.split(v, rspfString("."));
            if ( v.size() )
            {
               if ( v[0].contains("image") )
               {
                  bFoundImageLine = true;
                  rspfString s2 = v[0];
                  s2 += ".";
                  idx = 0;
                  for(;((idx < m_factoryList.size())&&!result); ++idx)
                  {
                     result =  m_factoryList[idx]->createProjection(kwl, s2.c_str());
                  }
               }
            }
         }

         // Go to the next line of the .geom file
         ++i;
      }
   }

   return result;
}

void rspfProjectionFactoryRegistry::initializeDefaults()
{
   registerFactory(rspfSensorModelFactory::instance());
   registerFactory(rspfNitfProjectionFactory::instance());
   registerFactory(rspfTiffProjectionFactory::instance());
   registerFactory(rspfWktProjectionFactory::instance());
   registerFactory(rspfMapProjectionFactory::instance());
   registerFactory(rspfMiscProjectionFactory::instance());

   // KEEP THIS LAST PLEASE!
   // This factory constructs map projections from EPSG codes. An infinite loop will occur if this
   // is placed before the explicit (non-coded) factories, since this factory will invoke the above
   // factories via this registry after populating a KWL which includes a PCS code. If this factory
   // sees that request before the others, it will be caught in a loop.
   registerFactory(rspfEpsgProjectionFactory::instance()); 
}

extern "C"
{
  void* rspfProjectionFactoryRegistryGetInstance()
  {
    return rspfProjectionFactoryRegistry::instance();
  }

}
