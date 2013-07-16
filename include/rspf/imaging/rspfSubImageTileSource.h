//*****************************************************************************
// FILE: rspfSubImageTileSource.h
//
// Copyright (C) 2001 ImageLinks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Contains declaration of class rspfSubImageTileSource.
//
// LIMITATIONS: None.
//
//*****************************************************************************
//  $Id: rspfSubImageTileSource.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfSubImageTileSource_HEADER
#define rspfSubImageTileSource_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

/*!****************************************************************************
 *
 *  CLASS: rspfSubImageTileSource
 *
 *  This tile source permits specifying an offset that is to be applied to the
 *  tile origin for all getTile() requests. It is intended for converting
 *  a full-image space coordinate to a sub-image coordinate.
 *
 *****************************************************************************/
class rspfSubImageTileSource : public rspfImageSourceFilter
{
public:
   /*!
    * Default constructor initializes the sub-image offset to zero.
    */
   rspfSubImageTileSource();

   /*!
    * Constructor accepts an input source pointer. Offset is defaulted to (0,0).
    */
   rspfSubImageTileSource(rspfImageSource* inputSource);

   /*!
    * Constructs with specified input source and offset.
    */
   rspfSubImageTileSource(rspfImageSource* inputSource,
                           const rspfIpt&   offset);

   /*!
    * Permits setting the image's offset relative to the full image space.
    */
   void setSubImageOffset(const rspfIpt& offset) { theSubImageOffset=offset; }

   rspfIpt getSubImageOffset()const{ return theSubImageOffset;}
 
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32  rr_level=0);

   /*!
    * Fetches the sub-image's bounding rect in FULL-IMAGE SPACE.
    */
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0) const;
   
   /*!
    * Fetches the sub-image's valid image vertices in FULL-IMAGE SPACE.
    */
   virtual void getValidImageVertices(vector<rspfIpt>& validVertices,
                                      rspfVertexOrdering ordering=RSPF_CLOCKWISE_ORDER,
                                      rspf_uint32 resLevel=0)const;

   virtual void initialize();
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

private:
   virtual ~rspfSubImageTileSource();
   
   rspfIpt                    theSubImageOffset;
   rspfRefPtr<rspfImageData> theTile;
   TYPE_DATA
};

#endif
