//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken Copied from TiledImageHash.
//
// Description: Hashing function for tiled rectangles.  Will hash a
//              dpt or fpt to a single index value.
//
// NOTE:  Works on rectangles that are positive in the line up (y)
//        direction.  Use TiledImageHash for rectangles that are positive
//        in the line down direction.
//              
//*******************************************************************
//  $Id: rspfTileHash.h 9094 2006-06-13 19:12:40Z dburken $

#ifndef TileHash_HEADER
#define TileHash_HEADER

#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfPointHash.h>

class RSPFDLLEXPORT rspfTileHash : public rspfPointHash
{
public:
   rspfTileHash(const rspfDrect &imageRect,
                 double tileWidth,
                 double tileHeight);

   virtual ~rspfTileHash();

   virtual long operator()(const rspfDpt &aPoint);
   virtual long operator()(const rspfFpt &aPoint);
   
private:
   rspfDrect   theImageRect;
   double  theTileWidth;
   double  theTileHeight;
   long    theNumberOfHorizTiles;
   long    theNumberOfVertTiles;
};

#endif
