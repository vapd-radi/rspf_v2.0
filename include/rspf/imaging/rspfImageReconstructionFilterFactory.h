//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfImageReconstructionFilterFactory.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfImageReconstructionFilterFactory_HEADER
#define rspfImageReconstructionFilterFactory_HEADER
#include <rspf/imaging/rspfImageSourceFactoryBase.h>

class rspfImageReconstructionFilterFactory : public rspfImageSourceFactoryBase
{
public:
   virtual ~rspfImageReconstructionFilterFactory();
   static rspfImageReconstructionFilterFactory* instance();
   virtual rspfObject* createObject(const rspfString& name)const;
   virtual rspfObject* createObject(const rspfKeywordlist& kwl,
                                     const char* prefix=0)const;
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;

   
protected:
   rspfImageReconstructionFilterFactory(); // hide
   rspfImageReconstructionFilterFactory(const rspfImageReconstructionFilterFactory&){}//hide
   void operator = (rspfImageReconstructionFilterFactory&){}// hide

   static rspfImageReconstructionFilterFactory* theInstance;
   
TYPE_DATA
};
#endif
