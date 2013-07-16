//******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: This file contains the Application cache algorithm
//
//***********************************
// $Id: rspfFixedTileCache.h 16276 2010-01-06 01:54:47Z gpotts $
#ifndef rspfFixedTileCache_HEADER
#define rspfFixedTileCache_HEADER
#include <map>
#include <list>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageData.h>
#include <OpenThreads/Mutex>

class  rspfFixedTileCacheInfo
{
public:
   rspfFixedTileCacheInfo(rspfRefPtr<rspfImageData>& tile,
                           rspf_int32 tileId=-1)
      :theTile(tile),
      theTileId(tileId)
      {
      }
   
   bool operator <(const rspfFixedTileCacheInfo& rhs)const
      {
         return (theTileId < rhs.theTileId);
      }
   bool operator <(rspf_int32 tileId)const
      {
         return (theTileId < tileId);
      }
   bool operator >(const rspfFixedTileCacheInfo& rhs)const
      {
         return (theTileId > rhs.theTileId);
      }
   bool operator >(rspf_int32 tileId)const
      {
         return (theTileId > tileId);
      }
   bool operator ==(const rspfFixedTileCacheInfo& rhs)const
      {
         return (theTileId == rhs.theTileId);
      }
   bool operator ==(rspf_int32 tileId)const
      {
         return (theTileId == tileId);
      }
   
   rspfRefPtr<rspfImageData> theTile;
   rspf_int32 theTileId;
};

class rspfFixedTileCache : public rspfReferenced
{
public:
   rspfFixedTileCache();
   virtual void setRect(const rspfIrect& rect);
   virtual void setRect(const rspfIrect& rect,
                        const rspfIpt& tileSize);
   void keepTilesWithinRect(const rspfIrect& rect);
   virtual rspfRefPtr<rspfImageData> addTile(rspfRefPtr<rspfImageData> imageData,
                                               bool duplicateData=true);
   virtual rspfRefPtr<rspfImageData> getTile(rspf_int32 id);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIpt& origin)
      {
         return getTile(computeId(origin));
      }
   virtual void setUseLruFlag(bool flag)
      {
         theUseLruFlag = flag;
      }
   virtual bool getUseLruFlag()const
      {
         return theUseLruFlag;
      }
   virtual void flush();
   virtual void deleteTile(const rspfIpt& origin)
      {
         deleteTile(computeId(origin));
      }
   virtual void deleteTile(rspf_int32 tileId);
   virtual rspfRefPtr<rspfImageData> removeTile(const rspfIpt& origin)
      {
         return removeTile(computeId(origin));
      }
   virtual rspfRefPtr<rspfImageData> removeTile(rspf_int32 tileId);
   virtual const rspfIrect& getTileBoundaryRect()const
      {
         return theTileBoundaryRect;
      }
   virtual rspf_uint32 getNumberOfTiles()const
      {
         return (rspf_uint32)theTileMap.size();
      }
   virtual const rspfIpt& getTileSize()const
      {
         return theTileSize;
      }
   virtual rspf_uint32 getCacheSize()const
      {
         return theCacheSize;
      }
   virtual void deleteTile();
   virtual rspfRefPtr<rspfImageData> removeTile();
   
   virtual void setMaxCacheSize(rspf_uint32 cacheSize)
      {
         theMaxCacheSize = cacheSize;
      }

   rspf_uint32 getMaxCacheSize()const
      {
         return theMaxCacheSize;
      }
   
   virtual rspfIpt getTileOrigin(rspf_int32 tileId);
   virtual rspf_int32 computeId(const rspfIpt& tileOrigin)const;
   virtual void setTileSize(const rspfIpt& tileSize);
protected:
   virtual ~rspfFixedTileCache();
   OpenThreads::Mutex theMutex;
   rspfIrect   theTileBoundaryRect;
   rspfIpt     theTileSize;
   rspfIpt     theBoundaryWidthHeight;
   rspf_uint32 theTilesHorizontal;
   rspf_uint32 theTilesVertical;
   rspf_uint32 theCacheSize;
   rspf_uint32 theMaxCacheSize;
   std::map<rspf_int32, rspfFixedTileCacheInfo> theTileMap;
   std::list<rspf_int32> theLruQueue;
   bool                   theUseLruFlag;
   virtual void eraseFromLru(rspf_int32 id);
   void adjustLru(rspf_int32 id);
};

#endif
