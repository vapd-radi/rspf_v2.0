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
// $Id: rspfRgbToHsvSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfRgbToHsvSource_HEADER
#define rspfJpegRgbToYCbCrSource_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfRgbToHsvSource : public rspfImageSourceFilter
{
public:
   rspfRgbToHsvSource();
   rspfRgbToHsvSource(rspfImageSource* inputSource);
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   
   virtual rspfScalarType getOutputScalarType() const;
   virtual rspf_uint32    getNumberOfOutputBands()const;
   virtual double getNullPixelValue()const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   virtual void initialize();
       
protected:
   virtual ~rspfRgbToHsvSource();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   rspfRefPtr<rspfImageData> theBlankTile;
   rspfRefPtr<rspfImageData> theTile;

TYPE_DATA
};

#endif
