//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfImageReconstructionFilterFactory.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/imaging/rspfImageReconstructionFilterFactory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfMeanMedianFilter.h>
#include <rspf/base/rspfKeywordNames.h>

RTTI_DEF1(rspfImageReconstructionFilterFactory, "rspfImageReconstructionFilterFactory",  rspfImageSourceFactoryBase);

rspfImageReconstructionFilterFactory* rspfImageReconstructionFilterFactory::theInstance = 0;

rspfImageReconstructionFilterFactory::~rspfImageReconstructionFilterFactory()
{
   theInstance = NULL;
}

rspfImageReconstructionFilterFactory* rspfImageReconstructionFilterFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfImageReconstructionFilterFactory;
   }

   return theInstance;
}

rspfImageReconstructionFilterFactory::rspfImageReconstructionFilterFactory()
{
   theInstance = this;
}

rspfObject* rspfImageReconstructionFilterFactory::createObject(const rspfString& name)const
{
   if(STATIC_TYPE_NAME(rspfMeanMedianFilter) == name)
   {
      return new rspfMeanMedianFilter;
   }

   return 0;
}

rspfObject* rspfImageReconstructionFilterFactory::createObject(const rspfKeywordlist& kwl,
                                                                 const char* prefix)const
{
   const char* typeKw = kwl.find(prefix, rspfKeywordNames::TYPE_KW);

   if(typeKw)
   {
      rspfObject* obj = createObject(rspfString(typeKw).trim());

      if(obj)
      {
         obj->loadState(kwl,
                        prefix);
         
         return obj;
      }
   }
   
   return 0;
}

void rspfImageReconstructionFilterFactory::getTypeNameList(std::vector<rspfString>& typeList)const
{
   typeList.push_back(STATIC_TYPE_NAME(rspfMeanMedianFilter));
}
