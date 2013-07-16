//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageReconstructionFilterRegistry.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfImageReconstructionFilterRegistry_HEADER
#define rspfImageReconstructionFilterRegistry_HEADER
#include <rspf/imaging/rspfImageSourceFactoryBase.h>

class rspfImageReconstructionFilterRegistry : public rspfImageSourceFactoryBase
{
public:
   static rspfImageReconstructionFilterRegistry* instance();
   virtual rspfObject* createObject(const rspfString& name)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
   void registerFactory(rspfImageSourceFactoryBase* factory);
   void unregisterFactory(rspfImageSourceFactoryBase* factory);
   bool findFactory(rspfImageSourceFactoryBase* factory)const;
   
protected:
   virtual ~rspfImageReconstructionFilterRegistry();
   rspfImageReconstructionFilterRegistry(); // hide
   rspfImageReconstructionFilterRegistry(const rspfImageReconstructionFilterRegistry&){}//hide
   void operator = (rspfImageReconstructionFilterRegistry&){}// hide
   
   static rspfImageReconstructionFilterRegistry* theInstance;
   std::vector<rspfImageSourceFactoryBase*> theFactoryList;
TYPE_DATA
};

#endif
