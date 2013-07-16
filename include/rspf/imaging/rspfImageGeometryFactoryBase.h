//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class declaration of rspfImageGeometryFactoryBase. See .h file for class documentation.
//
//**************************************************************************************************
// $Id$
#ifndef rspfImageGeometryFactoryBase_HEADER
#define rspfImageGeometryFactoryBase_HEADER
#include <rspf/base/rspfBaseObjectFactory.h>
#include <rspf/imaging/rspfImageGeometry.h>

class rspfImageHandler;
class RSPF_DLL rspfImageGeometryFactoryBase : public rspfBaseObjectFactory
{
public: 
   /**
    * Creates an object given a type name.
    */
   virtual rspfObject* createObject(const rspfString& typeName)const
   {
      return createGeometry(typeName);
   }
   
   /**
    * Creates and object given a keyword list.
    */
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const
   {
      return createGeometry(kwl, prefix);
   }
   
   virtual rspfImageGeometry* createGeometry(const rspfString& typeName)const=0;
   virtual rspfImageGeometry* createGeometry(const rspfKeywordlist& kwl,
                                              const char* prefix=0)const=0;
   virtual rspfImageGeometry* createGeometry(const rspfFilename& filename,
                                              rspf_uint32 entryIdx)const = 0;
   virtual bool extendGeometry(rspfImageHandler* handler)const=0;
};
#endif
