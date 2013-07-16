//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Hashing function for tiled rectangles.  Will hash a
//              dpt to a single index value.
//              
//*******************************************************************
//  $Id: rspfTiledImageHash.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfTiledImageHash_HEADER
#define rspfTiledImageHash_HEADER
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfPointHash.h>

class RSPFDLLEXPORT rspfTiledImageHash : public rspfPointHash
{
public:
   rspfTiledImageHash(const rspfDrect &imageRect,
                       double tileWidth,
                       double tileHeight);

   virtual ~rspfTiledImageHash();

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
