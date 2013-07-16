//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*******************************************************************
//  $Id: rspfStreamFactoryRegistry.cpp 11177 2007-06-07 19:47:04Z dburken $
//
#include <rspf/base/rspfStreamFactoryRegistry.h>
#include <rspf/base/rspfStreamFactory.h>
#include <rspf/base/rspfIoStream.h>
#include <rspf/base/rspfFilename.h>

#include <fstream>
#include <algorithm>

rspfStreamFactoryRegistry* rspfStreamFactoryRegistry::theInstance = 0;

rspfStreamFactoryRegistry::rspfStreamFactoryRegistry()
{
}

rspfStreamFactoryRegistry::~rspfStreamFactoryRegistry()
{
}

rspfStreamFactoryRegistry* rspfStreamFactoryRegistry::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfStreamFactoryRegistry();
      theInstance->registerFactory(rspfStreamFactory::instance());
   }

   return theInstance;
}

rspfRefPtr<rspfIFStream>
rspfStreamFactoryRegistry::createNewIFStream(
   const rspfFilename& file,
   std::ios_base::openmode openMode) const
{
   rspf_uint32 idx = 0;
   rspfRefPtr<rspfIFStream> result = 0;
   for(idx = 0; ((idx < theFactoryList.size())&&(!result)); ++idx)
   {
      result = theFactoryList[idx]->createNewIFStream(file, openMode);
   }

   if(!result)
   {
      result = new rspfIFStream(file.c_str(),
                                 openMode);
//       result = new std::ifstream(file.c_str(),
//                                  openMode);
   }
   
   return result;
}


void rspfStreamFactoryRegistry::registerFactory(rspfStreamFactoryBase* factory)
{
   std::vector<rspfStreamFactoryBase*>::iterator iter = std::find(theFactoryList.begin(),
                                                                  theFactoryList.end(),
                                                                  factory);
   if(iter == theFactoryList.end())
   {
      theFactoryList.push_back(factory);
   }
}

rspfStreamFactoryRegistry::rspfStreamFactoryRegistry(const rspfStreamFactoryRegistry&)
{}
