//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfProjectionFactoryRegistry.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfProjectionFactoryRegistry_HEADER
#define rspfProjectionFactoryRegistry_HEADER
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/base/rspfFactoryListInterface.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryBase.h>

class rspfImageHandler;
class RSPF_DLL rspfProjectionFactoryRegistry : public rspfObjectFactory,
                                                 public rspfFactoryListInterface<rspfProjectionFactoryBase,
                                                                                 rspfProjection>
{
public:

   virtual ~rspfProjectionFactoryRegistry();
   
   static rspfProjectionFactoryRegistry* instance();

   
   rspfProjection* createProjection(const rspfFilename& filename,
                                     rspf_uint32 entryIdx)const;
   rspfProjection* createProjection(const rspfString& name)const
   {
      return createNativeObjectFromRegistry(name);
   }
   rspfProjection* createProjection(const rspfKeywordlist& kwl,
                                     const char* prefix=NULL)const;
   
   virtual rspfProjection* createProjection(rspfImageHandler* handler)const;
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
   
   /**
    * This should return the type name of all objects in all factories.
    * This is the name used to construct the objects dynamially and this
    * name must be unique.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const
   {
      getAllTypeNamesFromRegistry(typeList);
   }
      
protected:
   rspfProjectionFactoryRegistry();
   rspfProjectionFactoryRegistry(const rspfProjectionFactoryRegistry& rhs);
   void operator = (const rspfProjectionFactoryRegistry&);

   static rspfProjectionFactoryRegistry* m_instance;
   void initializeDefaults();
};

extern "C"
{
  RSPFDLLEXPORT void* rspfProjectionFactoryRegistryGetInstance();
}

#endif
