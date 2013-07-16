//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description: Hashing function for tiled rectangles.  WIll hash a
//              dpt to a single index value.
//              
//*******************************************************************
//  $Id: rspfTiledImageHash.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <cfloat> // for FLT_EPSILON

#include <rspf/base/rspfTiledImageHash.h>


rspfTiledImageHash::rspfTiledImageHash(const rspfDrect &imageRect,
                                         double tileWidth,
                                         double tileHeight)
   :rspfPointHash(),
    theImageRect(imageRect)
{
   // make sure that the width of the tile is not larger than
   // the width of the image rectangle.
   if(theImageRect.width() < tileWidth)
   {
      theTileWidth = theImageRect.width();
      theNumberOfHorizTiles = 1;
   }
   else
   {
      theTileWidth          = tileWidth;
      double tempDiv        = theImageRect.width()/theTileWidth;
      double overFlow       = tempDiv  - static_cast<long>(tempDiv);
      theNumberOfHorizTiles = static_cast<long>(tempDiv);
      
      if(fabs(overFlow) >= FLT_EPSILON) // if the extent went beyond a tile
      {
         theNumberOfHorizTiles ++;      // we must say it has another tile
      }
   }

   // make sure the height of the tile is not larger than the
   // height of the image rectangle.
   if(theImageRect.height() < tileHeight)
   {
      theTileHeight = theImageRect.height();
      theNumberOfVertTiles = 1;
   }
   else
   {
      theTileHeight         = tileHeight;
      double tempDiv        = theImageRect.height()/theTileHeight;
      double overFlow       = tempDiv  - static_cast<long>(tempDiv);
      theNumberOfVertTiles = static_cast<long>(tempDiv);

      if(fabs(overFlow) >= FLT_EPSILON) // if the extent went beyond a tile
      {
         theNumberOfVertTiles ++;      // we must say it has another tile
      }
   }

}

rspfTiledImageHash::~rspfTiledImageHash()
{
}

long rspfTiledImageHash::operator()(const rspfDpt &aPoint)
{
   if(aPoint.x >= theImageRect.ul().x && aPoint.y >= theImageRect.ul().y)
   {
      // how far is the point horizontally  from the upper left corner
      double deltaWidth  = aPoint.x - theImageRect.ul().x;

      // how far is the point vertically from the upper left point
      double deltaHeight = aPoint.y - theImageRect.ul().y;

      // if deltas are negative then we are outside the
      // bounds
      if((deltaWidth < 0) || (deltaHeight < 0)) 
      {
         return -1;
      }

      // check if outside the rectangle
      if( (deltaWidth > theNumberOfHorizTiles*theTileWidth)||
          (deltaHeight > theNumberOfVertTiles*theTileHeight))
      {
         return -1;
      }
      // solve the horizontal and vertical index numbers
      long indexWidth  = static_cast<long>(deltaWidth  / theTileWidth);
      long indexHeight = static_cast<long>(deltaHeight / theTileHeight);

      // map to a linear array.  Just like you would index a 2-D array in memory
      return static_cast<long>(theNumberOfHorizTiles*indexHeight + indexWidth);
   }

   return -1;
}

long rspfTiledImageHash::operator()(const rspfFpt &aPoint)
{
   if(aPoint.x >= theImageRect.ul().x && aPoint.y >= theImageRect.ul().y)
   {
      // how far is the point horizontally  from the upper left corner
      double deltaWidth  = aPoint.x - theImageRect.ul().x;

      // how far is the point vertically from the upper left point
      double deltaHeight = aPoint.y - theImageRect.ul().y;

      // if deltas are negative then we are outside the
      // bounds
      if((deltaWidth < 0) || (deltaHeight < 0)) 
      {
         return -1;
      }

      // check if outside the rectangle
      if( (deltaWidth > theNumberOfHorizTiles*theTileWidth)||
          (deltaHeight > theNumberOfVertTiles*theTileHeight))
      {
         return -1;
      }
      // solve the horizontal and vertical index numbers
      long indexWidth  = static_cast<long>(deltaWidth  / theTileWidth);
      long indexHeight = static_cast<long>(deltaHeight / theTileHeight);

      // map to a linear array.  Just like you would index a 2-D array in memory
      return static_cast<long>(theNumberOfHorizTiles*indexHeight + indexWidth);
   }

   return -1;
}
