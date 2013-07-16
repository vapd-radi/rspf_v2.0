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
// $Id: rspfRgbToJpegYCbCrSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfRgbToJpegYCbCrSource_HEADER
#define rspfRgbToJpegYCbCrSource_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfRgbToJpegYCbCrSource : public rspfImageSourceFilter
{
public:
   
   rspfRgbToJpegYCbCrSource();
   rspfRgbToJpegYCbCrSource(rspfImageSource* inputSource);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
       
protected:
   virtual ~rspfRgbToJpegYCbCrSource();
   rspfRefPtr<rspfImageData> theBlankTile;

TYPE_DATA
};

#endif
