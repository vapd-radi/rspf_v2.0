//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//
//*******************************************************************
//  $Id: rspfStreamFactory.cpp 18652 2011-01-10 13:30:31Z dburken $
//
#include <rspf/base/rspfStreamFactory.h>
#include <fstream>
#include <rspf/rspfConfig.h>
#include <rspf/base/rspfFilename.h>
#if RSPF_HAS_LIBZ
#include <rspf/base/rspfGzStream.h>
#endif


rspfStreamFactory* rspfStreamFactory::theInstance = 0;

rspfStreamFactory::rspfStreamFactory()
   : rspfStreamFactoryBase()
{
}

rspfStreamFactory::~rspfStreamFactory()
{
}

rspfStreamFactory* rspfStreamFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfStreamFactory;
   }

   return theInstance;
}

rspfRefPtr<rspfIFStream> rspfStreamFactory::createNewIFStream(
   const rspfFilename& file,
   std::ios_base::openmode openMode) const
{
   rspfRefPtr<rspfIFStream> result = 0;
   
#if RSPF_HAS_LIBZ
   rspfFilename copyFile = file;

   if(!copyFile.exists())
   {
      rspfString ext = copyFile.ext();
      copyFile.setExtension("gz");
      if(!copyFile.exists())
      {
         copyFile.setExtension(ext);
         copyFile += ".gz";

         if(!copyFile.exists())
         {
            return result;
         }
      }
   }
   
   std::ifstream in(copyFile.c_str(), std::ios::in|std::ios::binary);

   if(!in) return result;

   unsigned char buf[2];

   in.read((char*)buf, 2);
   in.close();
   // check for gzip magic number
   //
   if((buf[0] == 0x1F) &&
      (buf[1] == 0x8B))
   {
      result = new rspfIgzStream(copyFile.c_str(), openMode);
   }
#endif
   return result;
}

rspfStreamFactory::rspfStreamFactory(const rspfStreamFactory&)
   : rspfStreamFactoryBase()
{}



