//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class declaration of rspf2dTo2dTransformFactory.
//
//**************************************************************************************************
// $Id$
#ifndef rspf2dTo2dTransformFactory_HEADER
#define rspf2dTo2dTransformFactory_HEADER
#include <rspf/base/rspf2dTo2dTransformFactoryBase.h>

class rspf2dTo2dTransform;
class RSPF_DLL rspf2dTo2dTransformFactory : public rspf2dTo2dTransformFactoryBase
{
public:
   rspf2dTo2dTransformFactory(){}
   static rspf2dTo2dTransformFactory* instance();
   
   /**
    * Take a transform type name.
    */
   virtual rspf2dTo2dTransform* createTransform(const rspfString& name)const;
   
   /**
    * Take a keywordlist.
    */
   virtual rspf2dTo2dTransform* createTransform(const rspfKeywordlist& kwl,
                                                 const char* prefix)const;
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
  
protected:
   static rspf2dTo2dTransformFactory* m_instance;
};

#endif
