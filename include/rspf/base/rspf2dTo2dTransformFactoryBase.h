#ifndef rspf2dTo2dTransformFactoryBase_HEADER
#define rspf2dTo2dTransformFactoryBase_HEADER
#include <rspf/base/rspfObjectFactory.h>
//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class declaration of rspf2dTo2dTransformFactoryBase.
//
//**************************************************************************************************
// $Id$
#include <rspf/base/rspf2dTo2dTransform.h>

class RSPF_DLL rspf2dTo2dTransformFactoryBase : public rspfObjectFactory
{
public:
   
   /**
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const
   {
      return createTransform(typeName);
   }
   
   /**
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const
   {
      return createTransform(kwl, prefix);
   }
   
   /**
    * Take a transform type name.
    */
   virtual rspf2dTo2dTransform* createTransform(const rspfString& name)const=0;
   
   /**
    * Take a keywordlist.
    */
   virtual rspf2dTo2dTransform* createTransform(const rspfKeywordlist& kwl,
                                                 const char* prefix)const=0;
protected:
};
#endif
