//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageSourceFactoryRegistry.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfImageSourceFactoryRegistry_HEADER
#define rspfImageSourceFactoryRegistry_HEADER
#include <rspf/imaging/rspfImageSourceFactoryBase.h>

class RSPF_DLL rspfImageSourceFactoryRegistry : public rspfImageSourceFactoryBase
{
public:
   static rspfImageSourceFactoryRegistry* instance();
   virtual ~rspfImageSourceFactoryRegistry();
   virtual rspfObject* createObject(const rspfString& name)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
   void registerFactory(rspfImageSourceFactoryBase* factory);
   void unregisterFactory(rspfImageSourceFactoryBase* factory);
   bool findFactory(rspfImageSourceFactoryBase* factory)const;
   
protected:
   rspfImageSourceFactoryRegistry(); // hide
   rspfImageSourceFactoryRegistry(const rspfImageSourceFactoryRegistry&);//hide
   void operator = (rspfImageSourceFactoryRegistry&);// hide
   
   static rspfImageSourceFactoryRegistry* theInstance;
   std::vector<rspfImageSourceFactoryBase*> theFactoryList;
TYPE_DATA
};

extern "C"
{
  RSPFDLLEXPORT void* rspfImageSourceFactoryRegistryGetInstance();
}
#endif
