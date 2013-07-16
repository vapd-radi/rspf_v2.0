//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageSourceFactory.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfImageSourceFactory_HEADER
#define rspfImageSourceFactory_HEADER
#include <rspf/imaging/rspfImageSourceFactoryBase.h>

class rspfImageSourceFactory : public rspfImageSourceFactoryBase
{
public:
   virtual ~rspfImageSourceFactory();
   static rspfImageSourceFactory* instance();
   virtual rspfObject* createObject(const rspfString& name)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;
   
protected:
   // Hide from use.
   rspfImageSourceFactory();
   rspfImageSourceFactory(const rspfImageSourceFactory&);
   const rspfImageSourceFactory& operator=(rspfImageSourceFactory&);

   static rspfImageSourceFactory* theInstance;
TYPE_DATA
};
#endif
