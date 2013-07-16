//*******************************************************************
// Copyright (C) 2005 David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//*******************************************************************
//  $Id: rspfMatchPluginInit.cpp 18968 2011-02-25 19:43:42Z gpotts $
#include <rspf/plugin/rspfSharedObjectBridge.h>
#include "rspfMatchExports.h"
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/imaging/rspfImageSourceFactoryRegistry.h>
#include <rspf/imaging/rspfImageWriterFactoryRegistry.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include "rspfMatchImageSourceFactory.h"

static void setDescription(rspfString& description)
{
   description = "Matching Plugin\n\n";
}

extern "C"
{
   static rspfSharedObjectInfo  myInfo;
   static rspfString theDescription;
   static std::vector<rspfString> theObjList;
   static const char* getDescription()
   {
      return theDescription.c_str();
   }
   static int getNumberOfClassNames()
   {
      return (int)theObjList.size();
   }
   static const char* getClassName(int idx)
   {
      if(idx < (int)theObjList.size())
      {
         return theObjList[0].c_str();
      }
      return (const char*)0;
   }

   /* Note symbols need to be exported on windoze... */ 
   RSPF_REGISTRATION_DLL void rspfSharedLibraryInitialize(
                                                            rspfSharedObjectInfo** info, 
                                                            const char* options)
   {
      myInfo.getDescription = getDescription;
      myInfo.getNumberOfClassNames = getNumberOfClassNames;
      myInfo.getClassName = getClassName;
      setDescription(theDescription);
      rspfImageSourceFactoryRegistry::instance()->registerFactory(rspfMatchImageSourceFactory::instance());

      if(!theObjList.size())
      {
         rspfMatchImageSourceFactory::instance()->getTypeNameList(theObjList);
      }
      *info = &myInfo;
  }

   /* Note symbols need to be exported on windoze... */ 
  RSPF_REGISTRATION_DLL void rspfSharedLibraryFinalize()
  {
     rspfImageSourceFactoryRegistry::instance()->unregisterFactory(rspfMatchImageSourceFactory::instance());     
  }
}