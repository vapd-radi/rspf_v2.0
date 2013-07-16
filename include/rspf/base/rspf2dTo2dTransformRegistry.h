//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class declaration of rspf2dTo2dTransformRegistry.
//
//**************************************************************************************************
// $Id$
#ifndef rspf2dTo2dTransformRegistry_HEADER
#define rspf2dTo2dTransformRegistry_HEADER
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspf2dTo2dTransformFactoryBase.h>
#include <rspf/base/rspf2dTo2dTransform.h>
#include <vector>
#include <rspf/base/rspfFactoryListInterface.h>

class RSPF_DLL rspf2dTo2dTransformRegistry : public rspfObjectFactory,
                                               public rspfFactoryListInterface<rspf2dTo2dTransformFactoryBase,
                                                                                rspf2dTo2dTransform>
{
public:
   virtual ~rspf2dTo2dTransformRegistry(){}
   static rspf2dTo2dTransformRegistry* instance();
   
   /*!
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const
   {
      return createObjectFromRegistry(typeName);
   }
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const
   {
      return createObjectFromRegistry(kwl, prefix);
   }
   /*!
    * Creates an object given a type name.
    */
   virtual rspf2dTo2dTransform* createTransform(const rspfString& typeName)const
   {
      return createNativeObjectFromRegistry(typeName);
   }
   
   /*!
    * Creates and object given a keyword list.
    */
   virtual rspf2dTo2dTransform* createTransform(const rspfKeywordlist& kwl,
                                                 const char* prefix=0)const
   {
      return createNativeObjectFromRegistry(kwl, prefix);
   }
   
   /*!
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const
   {
      getAllTypeNamesFromRegistry(typeList);
   }
   
protected:
   rspf2dTo2dTransformRegistry()
   :rspfObjectFactory()
   {}
   
   rspf2dTo2dTransformRegistry( const rspf2dTo2dTransformRegistry& rhs )
   :rspfObjectFactory(rhs)
   {}
   void operator =(const rspf2dTo2dTransformRegistry&){}
   static rspf2dTo2dTransformRegistry* m_instance;
   
   TYPE_DATA
};
#endif
