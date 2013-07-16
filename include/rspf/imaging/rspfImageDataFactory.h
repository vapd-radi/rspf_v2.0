//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description:
//
//*************************************************************************
// $Id: rspfImageDataFactory.h 13474 2008-08-22 14:20:42Z gpotts $
#ifndef rspfImageDataFactory_HEADER
#define rspfImageDataFactory_HEADER

#include <rspf/imaging/rspfImageData.h>
#include <rspf/base/rspfRefPtr.h>
#include <OpenThreads/Mutex>
class rspfSource;
class rspfImageSource;


/*!
 * This factory should be called by all image source producers to allocate
 * an image tile.
 */
class RSPF_DLL rspfImageDataFactory
{
public:
   virtual ~rspfImageDataFactory();
   static rspfImageDataFactory* instance();

   virtual rspfRefPtr<rspfImageData> create(rspfSource* owner,
                                              rspfScalarType scalar,
                                              rspf_uint32 bands = 1)const;

   virtual rspfRefPtr<rspfImageData> create(rspfSource* owner,
                                              rspfScalarType scalar,
                                              rspf_uint32 bands,
                                              rspf_uint32 width,
                                              rspf_uint32 height)const;
   
   virtual rspfRefPtr<rspfImageData> create(
      rspfSource* owner,
      rspf_uint32 bands,
      rspfImageSource* inputSource)const;
   
   
   virtual rspfRefPtr<rspfImageData> create(
      rspfSource* owner,
      rspfImageSource* inputSource)const;
   
protected:
   rspfImageDataFactory(); // hide
   rspfImageDataFactory(const rspfImageDataFactory&){}//hide
   void operator = (rspfImageDataFactory&){}// hide
   
   static rspfImageDataFactory* theInstance;
   static OpenThreads::Mutex theInstanceMutex;
};

#endif
