//**************************************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description: Class declaration of rspfImageGeometryRegistry. See .h file for class documentation.
//
//**************************************************************************************************
// $Id$
#ifndef rspfImageGeometryRegistry_HEADER
#define rspfImageGeometryRegistry_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/imaging/rspfImageGeometryFactoryBase.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/base/rspfFactoryListInterface.h>
class RSPF_DLL rspfImageGeometryRegistry : public rspfImageGeometryFactoryBase,
                                             public rspfFactoryListInterface<rspfImageGeometryFactoryBase,
                                                                              rspfImageGeometry>
{
public:
   virtual ~rspfImageGeometryRegistry(){m_instance=0;}
   static rspfImageGeometryRegistry* instance();
   
   
   virtual rspfImageGeometry* createGeometry(const rspfString& typeName)const;
   virtual rspfImageGeometry* createGeometry(const rspfKeywordlist& kwl,
                                              const char* prefix=0)const;
   virtual rspfImageGeometry* createGeometry(const rspfFilename& filename,
                                              rspf_uint32 entryIdx)const;
#if 0
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
#endif
   virtual bool extendGeometry(rspfImageHandler* handler)const;
   
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
   rspfImageGeometryRegistry()
   :rspfImageGeometryFactoryBase()
   {}
   
   rspfImageGeometryRegistry( const rspfImageGeometryRegistry& rhs )
   :rspfImageGeometryFactoryBase(rhs)
   {}
   void operator =(const rspfImageGeometryRegistry&){}
   static rspfImageGeometryRegistry* m_instance;
   
   TYPE_DATA
};
#endif
