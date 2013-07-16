//******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: This file contains the Application cache algorithm
//
//***********************************
// $Id: rspfFixedTileCache.cpp 16276 2010-01-06 01:54:47Z gpotts $
#include <rspf/imaging/rspfFixedTileCache.h>
#include <algorithm>

rspfFixedTileCache::rspfFixedTileCache()
   : theTileBoundaryRect(),
     theTileSize(),
     theBoundaryWidthHeight(),
     theTilesHorizontal(0),
     theTilesVertical(0),
     theCacheSize(0),
     theMaxCacheSize(0),
     theTileMap(),
     theLruQueue(),
     theUseLruFlag(true)
{
   rspf::defaultTileSize(theTileSize);

   rspfIrect tempRect;
   tempRect.makeNan();

   setRect(tempRect);
   
   // theCacheSize    = 0;
   // theMaxCacheSize = 0;
   // theUseLruFlag = true;
}

rspfFixedTileCache::~rspfFixedTileCache()
{
   flush();
}

void rspfFixedTileCache::setRect(const rspfIrect& rect)
{
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
      rspf::defaultTileSize(theTileSize);
      theTileBoundaryRect      = rect;
      theTileBoundaryRect.stretchToTileBoundary(theTileSize);
      theBoundaryWidthHeight.x = theTileBoundaryRect.width();
      theBoundaryWidthHeight.y = theTileBoundaryRect.height();
      theTilesHorizontal       = theBoundaryWidthHeight.x/theTileSize.x;
      theTilesVertical         = theBoundaryWidthHeight.y/theTileSize.y;
   }
   flush();
}

void rspfFixedTileCache::setRect(const rspfIrect& rect,
                                  const rspfIpt& tileSize)
{
   {
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
      theTileBoundaryRect      = rect;
      theTileSize              = tileSize;
      theTileBoundaryRect.stretchToTileBoundary(theTileSize);
      theBoundaryWidthHeight.x = theTileBoundaryRect.width();
      theBoundaryWidthHeight.y = theTileBoundaryRect.height();
      theTilesHorizontal       = theBoundaryWidthHeight.x/theTileSize.x;
      theTilesVertical         = theBoundaryWidthHeight.y/theTileSize.y;
   }
   flush();
}


void rspfFixedTileCache::keepTilesWithinRect(const rspfIrect& rect)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   std::map<rspf_int32, rspfFixedTileCacheInfo>::iterator tileIter = theTileMap.begin();

   while(tileIter != theTileMap.end())
   {
      rspfIrect tileRect = (*tileIter).second.theTile->getImageRectangle();
      if(!tileRect.intersects(rect))
      {
         eraseFromLru(computeId((*tileIter).second.theTile->getOrigin()));
         theCacheSize -= (*tileIter).second.theTile->getDataSizeInBytes();
         (*tileIter).second.theTile = NULL;
         theTileMap.erase(tileIter);
      }
      ++tileIter;
   }
}

rspfRefPtr<rspfImageData> rspfFixedTileCache::addTile(rspfRefPtr<rspfImageData> imageData,
                                                         bool duplicateData)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfRefPtr<rspfImageData> result = NULL;
   if(!imageData.valid())
   {
      return result;
   }
   if(!imageData->getBuf())
   {
      return result;
   }
   
   rspf_int32 id = computeId(imageData->getOrigin());
   if(id < 0)
   {
      return result;
   }
   
   std::map<rspf_int32, rspfFixedTileCacheInfo>::iterator tileIter =
      theTileMap.find(id);

   if(tileIter==theTileMap.end())
   {
      if(duplicateData)
      {
         result = (rspfImageData*)imageData->dup();
      }
      else
      {
         result = imageData;
      }
      rspfFixedTileCacheInfo cacheInfo(result, id);
       
      theCacheSize += imageData->getDataSizeInBytes();
      theTileMap.insert(make_pair(id, cacheInfo));
      if(theUseLruFlag)
      {
         theLruQueue.push_back(id);
      }
   }
   
   return result;
}

rspfRefPtr<rspfImageData> rspfFixedTileCache::getTile(rspf_int32 id)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfRefPtr<rspfImageData> result = NULL;

   std::map<rspf_int32, rspfFixedTileCacheInfo>::iterator tileIter =
      theTileMap.find(id);
   if(tileIter!=theTileMap.end())
   {
      result = (*tileIter).second.theTile;
      adjustLru(id);
   }

   return result;
}

rspfIpt rspfFixedTileCache::getTileOrigin(rspf_int32 tileId)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfIpt result;
   result.makeNan();

   if(tileId < 0)
   {
      return result;
   }
   rspf_int32 ty = (tileId/theTilesHorizontal);
   rspf_int32 tx = (tileId%theTilesVertical);
   
   rspfIpt ul = theTileBoundaryRect.ul();
   
   result = rspfIpt(ul.x + tx*theTileSize.x, ul.y + ty*theTileSize.y);

   return result;
}

rspf_int32 rspfFixedTileCache::computeId(const rspfIpt& tileOrigin)const
{
   rspfIpt idDiff = tileOrigin - theTileBoundaryRect.ul();

   if((idDiff.x < 0)||
      (idDiff.y < 0)||
      (idDiff.x >= theBoundaryWidthHeight.x)||
      (idDiff.y >= theBoundaryWidthHeight.y))
     {
       return -1;
     }
   rspf_uint32 y = idDiff.y/theTileSize.y;
   y*=theTilesHorizontal;

   rspf_uint32 x = idDiff.x/theTileSize.x;
   
   
   return (y + x);
}

void rspfFixedTileCache::deleteTile(rspf_int32 tileId)
{
   std::map<rspf_int32, rspfFixedTileCacheInfo>::iterator tileIter =
      theTileMap.find(tileId);

   if(tileIter != theTileMap.end())
   {
      if((*tileIter).second.theTile.valid())
      {
         theCacheSize -= (*tileIter).second.theTile->getDataSizeInBytes();
         (*tileIter).second.theTile = NULL;
      }
      theTileMap.erase(tileIter);
      eraseFromLru(tileId);
   }
}

rspfRefPtr<rspfImageData> rspfFixedTileCache::removeTile(rspf_int32 tileId)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfRefPtr<rspfImageData> result = NULL;
   
   std::map<rspf_int32, rspfFixedTileCacheInfo>::iterator tileIter =
      theTileMap.find(tileId);

   if(tileIter != theTileMap.end())
   {
      theCacheSize -= (*tileIter).second.theTile->getDataSizeInBytes();
      if((*tileIter).second.theTile.valid())
      {
         result =  (*tileIter).second.theTile;
      }
      theTileMap.erase(tileIter);
      eraseFromLru(tileId);
   }
   
   return result;
}

void rspfFixedTileCache::flush()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   std::map<rspf_int32, rspfFixedTileCacheInfo>::iterator tileIter =
      theTileMap.begin();

   while(tileIter != theTileMap.end())
   {
      if( (*tileIter).second.theTile.valid())
      {
         (*tileIter).second.theTile = 0;
      }
      ++tileIter;
   }
   theLruQueue.clear();
   theTileMap.clear();
   theCacheSize = 0;
}

void rspfFixedTileCache::deleteTile()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   if(theUseLruFlag)
   {
      if(theLruQueue.begin() != theLruQueue.end())
      {
         deleteTile(*(theLruQueue.begin()));
      }
   }
}

rspfRefPtr<rspfImageData> rspfFixedTileCache::removeTile()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   if(theUseLruFlag)
   {
      if(theLruQueue.begin() != theLruQueue.end())
      {
         return removeTile(*(theLruQueue.begin()));
      }
   }

   return NULL;
}

void rspfFixedTileCache::adjustLru(rspf_int32 id)
{
   if(theUseLruFlag)
   {
      std::list<rspf_int32>::iterator iter = std::find(theLruQueue.begin(), theLruQueue.end(), id);
      
      if(iter != theLruQueue.end())
      {
         rspf_int32 value = *iter;
         theLruQueue.erase(iter);
         theLruQueue.push_back(value);
      }
   }
}

void rspfFixedTileCache::eraseFromLru(rspf_int32 id)
{
   if(theUseLruFlag)
   {
      
      std::list<rspf_int32>::iterator iter = std::find(theLruQueue.begin(), theLruQueue.end(), id);
      
      if(iter != theLruQueue.end())
      {
         theLruQueue.erase(iter);
      }
   }
}


void rspfFixedTileCache::setTileSize(const rspfIpt& tileSize)
{
   setRect(theTileBoundaryRect, tileSize);
}
