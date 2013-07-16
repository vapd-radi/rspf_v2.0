//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// Description: This is a tmeporary filename class it will create a temporary
//              file and will also delete it upon destruction
//
//*************************************************************************
// $Id: rspfTempFilename.cpp 15524 2009-09-30 01:02:26Z dburken $
#include <stdlib.h>
#include <fstream>
#include <rspf/base/rspfTempFilename.h>
#include <rspf/base/rspfEnvironmentUtility.h>
#include <time.h>

rspfTempFilename::rspfTempFilename(const rspfString& tempDir,
                                     const rspfString& prefix,
                                     const rspfString& extension,
                                     bool autodelete,
                                     bool useWildcardDelete)
   :theTempDir(tempDir),
    thePrefix(prefix),
    theExtension(extension),
    theAutoDeleteFlag(autodelete),
    theWildCardDeleteFlag(useWildcardDelete)
{
}

rspfTempFilename::~rspfTempFilename()
{
   if(*((rspfFilename*)this) != "")
   {
      
      if(theAutoDeleteFlag)
      {
         if(theWildCardDeleteFlag)
         {
            wildcardRemove(*this + ".*");
         }
         else
         {
            remove();
         }
      }
   }
}

void  rspfTempFilename::generateRandomFile()
{
   generate(false);
}

void rspfTempFilename::generateRandomDir()
{
   generate(true);
}

void rspfTempFilename::generate(bool createAsDirectoryFlag)
{
   srand(time(0));
   rspfString tempDirCopy = theTempDir;

   if(tempDirCopy == "")
   {
      tempDirCopy = rspfEnvironmentUtility::instance()->getEnvironmentVariable("TEMP");
      if(tempDirCopy=="")
      {
         tempDirCopy  = rspfEnvironmentUtility::instance()->getEnvironmentVariable("TMP");
      }
      if(tempDirCopy == "")
      {
         if(rspfFilename("/tmp").exists())
         {
            tempDirCopy = "/tmp";
         }
      }
   }

   int count = 0;
   int randNumber1 = rand();
   rspfFilename prefixDir = rspfFilename(tempDirCopy);
   rspfFilename result = prefixDir.dirCat(thePrefix+
                                           rspfString::toString(randNumber1));
   
   while((count < RAND_MAX)&&result.exists())
   {
      randNumber1 = rand();
      result = prefixDir.dirCat(thePrefix+
                                rspfString::toString(randNumber1));
      
      ++count;
   }

   if(theExtension != "")
   {
      result = result.setExtension(theExtension);
   }
   *((rspfFilename*)this) = result;
   if(result != "")
   {
      if(createAsDirectoryFlag)
      {
         createDirectory();
      }
      else
      {
         std::ofstream out(result.c_str());
         out.close();
      }
   }
}
