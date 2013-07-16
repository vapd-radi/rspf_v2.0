//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Implementation of rspf2dTo2dTransformRegistry.
//
//**************************************************************************************************
// $Id$
#include <rspf/base/rspf2dTo2dTransformRegistry.h>
#include <rspf/base/rspf2dTo2dTransformFactory.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
rspf2dTo2dTransformRegistry* rspf2dTo2dTransformRegistry::m_instance = 0;
RTTI_DEF1(rspf2dTo2dTransformRegistry, "rspf2dTo2dTransformRegistry", rspfObjectFactory);
rspf2dTo2dTransformRegistry* rspf2dTo2dTransformRegistry::instance()
{
   if(!m_instance)
   {
      m_instance = new rspf2dTo2dTransformRegistry();
      m_instance->registerFactory(rspf2dTo2dTransformFactory::instance());
      rspfObjectFactoryRegistry::instance()->registerFactory(m_instance);
   }
   
   return m_instance;
}
