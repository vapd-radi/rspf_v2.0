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
// $Id: rspfBandMergeSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfBandMergeSource_HEADER
#define rspfBandMergeSource_HEADER
#include <rspf/imaging/rspfImageCombiner.h>

class rspfImageSourceConnection;
class rspfImageSource;

class RSPF_DLL rspfBandMergeSource : public rspfImageCombiner
{
public:
   rspfBandMergeSource();
   rspfBandMergeSource(rspfConnectableObject::ConnectableObjectList& inputSources);
   
   /**
    * This will merge all the bands from the input source list
    * into one contiguous rspfImageData object.  This is very
    * useful when you want to split off an intesity channel in
    * HSI and then run some algorithms on it and then merge it back.
    * You can also try to do fusions by converting to an YCbCr and
    * replace the Y or intensity channel with maybe a high-pass
    * convolution on the pan.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);

   virtual void initialize();
   
   /**
    * Returns the number of bands in a tile returned from this TileSource.
    * Note: we are supporting sources that can have multiple data objects.
    * If you want to know the scalar type of an object you can pass in the 
    */
   virtual rspf_uint32 getNumberOfOutputBands() const;

   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   
protected:
   virtual ~rspfBandMergeSource();
   /** Performs allocation of "theTile". */
   void allocate();
   
   rspf_uint32                theNumberOfOutputBands;
   rspfRefPtr<rspfImageData> theTile;
   
   rspf_uint32 computeNumberOfInputBands()const;

TYPE_DATA
};
#endif /* #ifndef rspfBandMergeSource_HEADER */
