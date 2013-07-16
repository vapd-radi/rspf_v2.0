//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfTilePatch.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfTilePatch_HEADER
#define rspfTilePatch_HEADER
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/imaging/rspfDiscreteNearestNeighbor.h>
#include <rspf/imaging/rspfImageData.h>

class rspfImageSource;

class rspfTilePatch : public rspfObject
{
public:
   rspfTilePatch(rspfImageSource* input);
   /*!
    * The rect is assumed up right and all we need
    * to do is compute the even tile boundary rect
    */
   rspfTilePatch(rspfScalarType scalarType = RSPF_UCHAR,
                  long numberOfComponents = 1,
                  long width=128,
                  long height=128);
   
   /*!
    * return the tile boundary aligned rectangle for this
    * patch.
    */
   virtual rspfIrect getRect()const;

   /*!
    * The padding is for any convolution that must take
    * place.  
    */
   virtual void setRect(const rspfDpt &p1,
                        const rspfDpt &p2,
                        const rspfDpt &p3,
                        const rspfDpt &p4,
                        const rspfDpt &tile_size,
                        const rspfDpt &padding=rspfDpt(0,0));

   virtual void setRect(const rspfDrect &aRect,
                        const rspfDpt &tile_size,
                        const rspfDpt &padding=rspfDpt(0,0));

   virtual void setRect(const rspfDpt &center_of_rect,
                        const rspfDpt &rect_size,
                        const rspfDpt &tile_size,
                        const rspfDpt &padding=rspfDpt(0,0));

   /*!
    * This will fill the tile from the tile patch.
    */
   virtual void fillTile(rspfRefPtr<rspfImageData>& aTile)const;

   /*!
    * This will fill the tile from the tile patch and apply the convolution
    * kernel.  This is optimized for upright rectangles.  Use the bilinear
    * fill tile bellow for arbitrary 4-sided polygons.
    */
   virtual void fillTile(rspfRefPtr<rspfImageData>& result,
                         rspfDiscreteConvolutionKernel* kernel)const;

   /*!
    * We will assume that the patch is already
    * big enough to satisfy the kernel size.
    */
   virtual void fillTile(rspfRefPtr<rspfImageData>& result,
                         const rspfDpt   &ul,
                         const rspfDpt   &ur,
                         const rspfDpt   &deltaUl,
                         const rspfDpt   &deltaUr,
                         const rspfDpt   &length)const;


   virtual void fillPatch(rspfImageSource *imageSource,
                          long resLevel = 0);
   
   
   
//    static rspfIrect alignRectToBoundary(const rspfDrect &aRect,
//                                          const rspfDpt &tile_size);
   static rspfDrect findBoundingRect(const rspfDpt &p1,
                                      const rspfDpt &p2,
                                      const rspfDpt &p3,
                                      const rspfDpt &p4);
   /*!
    * Do not delete this data.  We give raw access to the buffer
    * if there are special things you wish to have done.
    */
   rspfRefPtr<rspfImageData> getData(){return thePatchData;}

   void setData(rspfRefPtr<rspfImageData>& patchData);
   /*!
    *  This will implement an in place convolution.  It will
    *  allocate a buffer pass it to the convolution algorithm
    * and then replace the internal buffer.
    */
   void convolve(rspfDiscreteConvolutionKernel* kernel);
   
protected:
   virtual ~rspfTilePatch();
   
   rspfRefPtr<rspfImageData>         thePatchData;
   static rspfDiscreteNearestNeighbor nearestNeighborKernel;
   


   template<class T>
   void fillTileTemplate(T dummyVariable,
                         rspfRefPtr<rspfImageData>& result,
                         const rspfDpt   &ul,
                         const rspfDpt   &ur,
                         const rspfDpt   &deltaUl,
                         const rspfDpt   &deltaUr,
                         const rspfDpt   &length)const;
   
   template<class T>
   void fillTileTemplate(T dummyVariable,
                         rspfRefPtr<rspfImageData>& result,
                         rspfDiscreteConvolutionKernel* kernel)const;
};

#endif
