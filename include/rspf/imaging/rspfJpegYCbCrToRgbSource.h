//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfJpegYCbCrToRgbSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfJpegYCbCrToRgbSource_HEADER
#define rspfJpegYCbCrToRgbSource_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfJpegYCbCrToRgbSource : public rspfImageSourceFilter
{
public:
   rspfJpegYCbCrToRgbSource();
   rspfJpegYCbCrToRgbSource(rspfImageSource* inputSource);
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                               rspf_uint32 resLevel=0);

   virtual void initialize();
       
protected:
   virtual ~rspfJpegYCbCrToRgbSource();

   virtual void allocate();
   
   rspfRefPtr<rspfImageData> theBlankTile;

TYPE_DATA
};

#endif
