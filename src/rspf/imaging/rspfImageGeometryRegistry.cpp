//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class implementation of rspfImageGeometryRegistry. See .h file for class documentation.
//
//**************************************************************************************************
// $Id$
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/imaging/rspfImageGeometryFactory.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
rspfImageGeometryRegistry* rspfImageGeometryRegistry::m_instance = 0;
RTTI_DEF1(rspfImageGeometryRegistry, "rspfImageGeometryRegistry", rspfImageGeometryFactoryBase);
rspfImageGeometryRegistry* rspfImageGeometryRegistry::instance()
{
   if(!m_instance)
   {
      m_instance = new rspfImageGeometryRegistry();
      m_instance->registerFactory(rspfImageGeometryFactory::instance());
      rspfObjectFactoryRegistry::instance()->registerFactory(m_instance);
   }
   
   return m_instance;
}

bool rspfImageGeometryRegistry::extendGeometry(rspfImageHandler* handler)const
{
   bool result = false;
   rspf_uint32 idx = 0;
   for(;((idx < m_factoryList.size())&&!result); ++idx)
   {
      result = m_factoryList[idx]->extendGeometry(handler);
   }
   
   return result;
}

rspfImageGeometry* rspfImageGeometryRegistry::createGeometry(const rspfString& typeName)const
{
   rspfRefPtr<rspfObject> obj = createObjectFromRegistry(typeName);
   
   rspfRefPtr<rspfImageGeometry> geomResult = dynamic_cast<rspfImageGeometry*>(obj.get());
   
   obj = 0;
   
   return geomResult.release();
}

rspfImageGeometry* rspfImageGeometryRegistry::createGeometry(const rspfKeywordlist& kwl,
                                                               const char* prefix)const
{
   rspfRefPtr<rspfObject> obj = createObjectFromRegistry(kwl, prefix);
   
   rspfRefPtr<rspfImageGeometry> geomResult = dynamic_cast<rspfImageGeometry*>(obj.get());
   
   obj = 0;
   
   return geomResult.release();
}

rspfImageGeometry* rspfImageGeometryRegistry::createGeometry(const rspfFilename& filename,
                                                                       rspf_uint32 entryIdx)const
{
   rspfImageGeometry* result = 0;
   rspf_uint32 idx = 0;
   for(;((idx < m_factoryList.size())&&!result); ++idx)
   {
      result = m_factoryList[idx]->createGeometry(filename, entryIdx);
   }
   
   return result;
}
