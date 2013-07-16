//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfObjectFactoryRegistry.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfObjectFactoryRegistry_HEADER
#define rspfObjectFactoryRegistry_HEADER
#include <vector>

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfObjectFactory.h>
#include <rspf/base/rspfFactoryListInterface.h>

class rspfKeywordlist;
class rspfString;
class RSPFDLLEXPORT rspfObjectFactoryRegistry : public rspfObject,
                                                  public rspfFactoryListInterface<rspfObjectFactory, rspfObject>
{
public:
   virtual ~rspfObjectFactoryRegistry();
   static rspfObjectFactoryRegistry* instance();
   
   virtual rspfObject* createObject(const rspfString& name)const
   {
      return createObjectFromRegistry(name);
   }
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const
   {
      return createObjectFromRegistry(kwl, prefix);
   }
   
   /*!
    * Returns a type list of all objects that can be instantiated
    * through the createObjectMethods above.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const
   {
      getAllTypeNamesFromRegistry(typeList);
   }

   /*!
    * returns a list of objects that are of the passed in
    * base type.  for example:
    *
    *  If I want all rspfImageSource objects then I call:
    *
    *  rspfObjectFactoryRegistry::instance()->getTypeNameList(l,
    *                                                          "rspfImageSource");
    */ 
   virtual void getTypeNameList(std::vector<rspfString>& typeList,
                                const rspfString& baseType)const;
   
protected:
   rspfObjectFactoryRegistry();//hide
   rspfObjectFactoryRegistry(const rspfObjectFactoryRegistry&):rspfObject(){}//hide
   void operator =(const rspfObjectFactoryRegistry&){}//hide
   
//   std::vector<rspfObjectFactory*>   theFactoryList;

//   rspfObjectFactory* findFactory(rspfObjectFactory* factory)const;

TYPE_DATA
};

extern "C"
{
  RSPFDLLEXPORT void* rspfObjectFactoryRegistryGetInstance();
}
#endif
