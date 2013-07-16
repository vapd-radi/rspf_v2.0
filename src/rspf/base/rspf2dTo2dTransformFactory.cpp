//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Implementation of rspf2dTo2dTransformFactory.
//
//**************************************************************************************************
// $Id$
#include <rspf/base/rspf2dTo2dTransformFactory.h>
#include <rspf/base/rspf2dBilinearTransform.h>
#include <rspf/base/rspf2dTo2dShiftTransform.h>
#include <rspf/base/rspf2dTo2dIdentityTransform.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfRefPtr.h>

rspf2dTo2dTransformFactory* rspf2dTo2dTransformFactory::m_instance = 0;
rspf2dTo2dTransformFactory* rspf2dTo2dTransformFactory::instance()
{
   if(!m_instance)
   {
      m_instance = new rspf2dTo2dTransformFactory();
   }
   return m_instance;
}

rspf2dTo2dTransform* rspf2dTo2dTransformFactory::createTransform(const rspfString& name)const
{
   rspf2dTo2dTransform* result = 0;
   if(name == STATIC_TYPE_NAME(rspf2dBilinearTransform))
   {
      result = new rspf2dBilinearTransform();
   }
   else if(name == STATIC_TYPE_NAME(rspf2dTo2dShiftTransform))
   {
      result = new rspf2dTo2dShiftTransform();
   }
   else if(name == STATIC_TYPE_NAME(rspf2dTo2dIdentityTransform))
   {
      result = new rspf2dTo2dIdentityTransform();
   }
   
   return result;
}

rspf2dTo2dTransform* rspf2dTo2dTransformFactory::createTransform(const rspfKeywordlist& kwl,
                                                                   const char* prefix)const
{
   rspfString type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   rspfRefPtr<rspf2dTo2dTransform> result =  createTransform(type);
   if(result.valid()&&!result->loadState(kwl, prefix))
   {
      result = 0;
   }
   
   return result.release();
}

void rspf2dTo2dTransformFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspf2dBilinearTransform));
   typeList.push_back(STATIC_TYPE_NAME(rspf2dTo2dShiftTransform));
   typeList.push_back(STATIC_TYPE_NAME(rspf2dTo2dIdentityTransform));
}
