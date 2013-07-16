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
// $Id: rspfHsiToRgbSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfHsiToRgbSource_HEADER
#define rspfHsiToRgbSource_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfHsiToRgbSource : public rspfImageSourceFilter
{
public:
   rspfHsiToRgbSource();
   rspfHsiToRgbSource(rspfImageSource* inputSource);
   virtual rspfRefPtr<rspfImageData> getTile(const  rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);

   virtual rspfScalarType getOutputScalarType() const;

   virtual double getNullPixelValue()const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   virtual void initialize();
protected:
   virtual ~rspfHsiToRgbSource();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   rspfRefPtr<rspfImageData> theBlankTile;
   rspfRefPtr<rspfImageData> theTile;

   void initializeBuffers(rspfRefPtr<rspfImageData>& data);
   
TYPE_DATA
};

#endif /* #ifndef rspfHsiToRgbSource_HEADER */

