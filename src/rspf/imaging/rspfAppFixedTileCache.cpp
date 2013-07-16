//******************************************************************
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
// $Id: rspfAppFixedTileCache.cpp 20127 2011-10-12 11:27:10Z gpotts $
#include <algorithm>
#include <sstream>
#include <rspf/imaging/rspfAppFixedTileCache.h>
#include <rspf/imaging/rspfFixedTileCache.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfTrace.h>
#include <OpenThreads/ScopedLock>

rspfAppFixedTileCache* rspfAppFixedTileCache::theInstance = 0;
rspfAppFixedTileCache::rspfAppFixedCacheId rspfAppFixedTileCache::theUniqueAppIdCounter = 0;
const rspf_uint32 rspfAppFixedTileCache::DEFAULT_SIZE = 1024*1024*80;

static const rspfTrace traceDebug("rspfAppFixedTileCache:debug");
std::ostream& operator <<(std::ostream& out, const rspfAppFixedTileCache& rhs)
{
   std::map<rspfAppFixedTileCache::rspfAppFixedCacheId, rspfFixedTileCache*>::const_iterator iter = rhs.theAppCacheMap.begin();

   if(iter == rhs.theAppCacheMap.end())
   {
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "***** APP CACHE EMPTY *****" << endl;
   }
   else
   {
      while(iter != rhs.theAppCacheMap.end())
      {
         out << "Cache id = "<< (*iter).first << " size = " << (*iter).second->getCacheSize() << endl;
         ++iter;
      }
   }

   return out;
}


rspfAppFixedTileCache::rspfAppFixedTileCache()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfAppFixedTileCache::rspfAppFixedTileCache() DEBUG: entered ..." << std::endl;
   }
   theInstance = this;
   theTileSize = rspfIpt(64, 64);
   theCurrentCacheSize = 0;

   // rspf::defaultTileSize(theTileSize);
   
   rspf_uint32 cacheSize = rspfString(rspfPreferences::instance()->findPreference("cache_size")).toUInt32()*(1024*1024);
   const char* tileSize = rspfPreferences::instance()->findPreference("tile_size");
   if(tileSize)
   {
      rspfString tempString(tileSize);
      std::vector<rspfString> splitString;
      tempString = tempString.trim();
      tempString.split(splitString, " ");
      if(splitString.size() > 1)
      {
         theTileSize.x = splitString[0].toInt32();
         theTileSize.y = splitString[1].toInt32();
      }
      else 
      {
         theTileSize = rspfIpt(64,64);
      }
         
    }
   if(cacheSize)
   {
      setMaxCacheSize(cacheSize);
   }
   else
   {
      setMaxCacheSize(DEFAULT_SIZE);
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG: cache tile size = " << theTileSize << std::endl
         << "Cache size = " << cacheSize << " bytes" << std::endl;
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfAppFixedTileCache::rspfAppFixedTileCache() DEBUG: leaving ..." << std::endl;
   }
}

rspfAppFixedTileCache::~rspfAppFixedTileCache()
{
   deleteAll();
}

rspfAppFixedTileCache *rspfAppFixedTileCache::instance(rspf_uint32  maxSize)
{
   if(!theInstance)
   {
      theInstance = new rspfAppFixedTileCache;
      if(maxSize)
	{
	  theInstance->setMaxCacheSize(maxSize);
	}
   }
   return theInstance;
}

void rspfAppFixedTileCache::setMaxCacheSize(rspf_uint32 cacheSize)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theMaxGlobalCacheSize = cacheSize;
   theMaxCacheSize = cacheSize;
   //   theMaxCacheSize      = (rspf_uint32)(theMaxGlobalCacheSize*.2);
}

void rspfAppFixedTileCache::flush()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   std::map<rspfAppFixedCacheId, rspfFixedTileCache*>::iterator currentIter = theAppCacheMap.begin();
   
   while(currentIter != theAppCacheMap.end())
   {
      (*currentIter).second->flush();
      ++currentIter;
   }
   theCurrentCacheSize = 0;
}

void rspfAppFixedTileCache::flush(rspfAppFixedCacheId cacheId)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfFixedTileCache* cache = getCache(cacheId);
   {
      if(cache)
      {
         theCurrentCacheSize -= cache->getCacheSize();
         cache->flush();
      }
   }
}

void rspfAppFixedTileCache::deleteCache(rspfAppFixedCacheId cacheId)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfRefPtr<rspfFixedTileCache> cache = getCache(cacheId);
   {
      std::map<rspfAppFixedCacheId, rspfFixedTileCache*>::iterator iter = theAppCacheMap.find(cacheId);
      
      if(cache.valid())
      {
         theAppCacheMap.erase(iter);
         theCurrentCacheSize -= cache->getCacheSize();
      }
      cache = 0;
   }
}

rspfAppFixedTileCache::rspfAppFixedCacheId rspfAppFixedTileCache::newTileCache(const rspfIrect& tileBoundaryRect,
                                                                                  const rspfIpt& tileSize)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfAppFixedCacheId result = -1; 
   rspfFixedTileCache* newCache = new rspfFixedTileCache;
   if(tileSize.x == 0 ||
      tileSize.y == 0)
   {
      // newCache->setRect(tileBoundaryRect, theTileSize);
      newCache->setRect(tileBoundaryRect,
                        newCache->getTileSize());
   }
   else
   {
      newCache->setRect(tileBoundaryRect, tileSize);
   }
   result = theUniqueAppIdCounter;
   theAppCacheMap.insert(std::make_pair(result, newCache));
   ++theUniqueAppIdCounter;
   
   return result;
}

rspfAppFixedTileCache::rspfAppFixedCacheId rspfAppFixedTileCache::newTileCache()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfAppFixedCacheId result = -1;
   rspfFixedTileCache* newCache = new rspfFixedTileCache;
   
   {
      result = theUniqueAppIdCounter;
      theAppCacheMap.insert(std::make_pair(result, newCache));
      ++theUniqueAppIdCounter;
   }
   
   return result;
   
}

void rspfAppFixedTileCache::setRect(rspfAppFixedCacheId cacheId,
                                     const rspfIrect& boundaryTileRect)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfFixedTileCache* cache = getCache(cacheId);
   if(cache)
   {
      
      rspf_uint32 cacheSize = cache->getCacheSize();
      // cache->setRect(boundaryTileRect, theTileSize);
      cache->setRect(boundaryTileRect,
                     cache->getTileSize());      
      theCurrentCacheSize += (cache->getCacheSize() - cacheSize);
   }
}

void rspfAppFixedTileCache::setTileSize(rspfAppFixedCacheId cacheId,
                                         const rspfIpt& tileSize)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfFixedTileCache* cache = getCache(cacheId);
   if(cache)
   {
      rspf_uint32 cacheSize = cache->getCacheSize();
      cache->setRect(cache->getTileBoundaryRect(), tileSize);
      theCurrentCacheSize += (cache->getCacheSize() - cacheSize);
      theTileSize = cache->getTileSize();
   }
}

rspfRefPtr<rspfImageData> rspfAppFixedTileCache::getTile(
   rspfAppFixedCacheId cacheId,
   const rspfIpt& origin)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfRefPtr<rspfImageData> result = 0;
   rspfFixedTileCache* cache = getCache(cacheId);
   if(cache)
   {
      result = cache->getTile(origin);
   }

   return result;
}


rspfRefPtr<rspfImageData> rspfAppFixedTileCache::addTile(
                                                            rspfAppFixedCacheId cacheId,
                                                            rspfRefPtr<rspfImageData> data,
                                                            bool duplicateData)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfRefPtr<rspfImageData> result = 0;
   rspfFixedTileCache *aCache = this->getCache(cacheId);
   if(!aCache)
   {         
      return result;
   }
   rspf_uint32 dataSize = data->getDataSizeInBytes();

   if( (theCurrentCacheSize+dataSize) > theMaxGlobalCacheSize)
   {
      shrinkGlobalCacheSize((rspf_int32)(theMaxGlobalCacheSize*0.1));
   }

   rspf_uint32 cacheSize = 0;
   {
      cacheSize = aCache->getCacheSize();
   }
   if(cacheSize > theMaxCacheSize)
   {
//       shrinkCacheSize(aCache,
//                       (rspf_int32)(aCache->getCacheSize()*.1));
      shrinkCacheSize(aCache,
                      (rspf_int32)(1024*1024));
   }
   {
      cacheSize = aCache->getCacheSize();
      result    = aCache->addTile(data, duplicateData);
   
      theCurrentCacheSize += (aCache->getCacheSize() - cacheSize);
   }
   
   return result;
}

void rspfAppFixedTileCache::deleteAll()
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   theCurrentCacheSize = 0;
   theAppCacheMap.clear();
}

rspfRefPtr<rspfImageData> rspfAppFixedTileCache::removeTile(
   rspfAppFixedCacheId cacheId,
   const rspfIpt& origin)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfRefPtr<rspfImageData> result = 0;
   
   rspfFixedTileCache* cache = getCache(cacheId);
   if(cache)
   {
      rspf_uint32 cacheSize = cache->getCacheSize();
      result = cache->removeTile(origin);
      theCurrentCacheSize += (cache->getCacheSize() - cacheSize);
   }

   return result;
}

void rspfAppFixedTileCache::deleteTile(rspfAppFixedCacheId cacheId,
                                        const rspfIpt& origin)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfFixedTileCache* cache = getCache(cacheId);
   if(cache)
   {
      rspf_uint32 cacheSize = cache->getCacheSize();
      cache->deleteTile(origin);
      theCurrentCacheSize += (cache->getCacheSize() - cacheSize);
   }
}

rspfFixedTileCache* rspfAppFixedTileCache::getCache(
   rspfAppFixedCacheId cacheId)
{   
   std::map<rspfAppFixedCacheId, rspfFixedTileCache*>::const_iterator
      currentIter = theAppCacheMap.find(cacheId);
   rspfFixedTileCache* result = 0;
   
   if(currentIter != theAppCacheMap.end())
   {
      result = (*currentIter).second;
   }

   return result;
}

void rspfAppFixedTileCache::shrinkGlobalCacheSize(rspf_int32 byteCount)
{
   if(static_cast<rspf_uint32>(byteCount) >= theCurrentCacheSize)
   {
      flush();
   }
   else
   {
      while(byteCount > 0)
      {
         std::map<rspfAppFixedCacheId, rspfFixedTileCache*>::iterator iter = theAppCacheMap.begin();
         while( (iter != theAppCacheMap.end())&&(byteCount>0))
         {
            rspfFixedTileCache* cache = getCache((*iter).first);
            if(cache)
            {
               rspf_uint32 before = cache->getCacheSize();
               cache->deleteTile();
               rspf_uint32 after = cache->getCacheSize();
               rspf_uint32 delta = (before - after);
               byteCount -= delta;
               theCurrentCacheSize -= (delta);
            }
            ++iter;
         }
      }
   }
}

void rspfAppFixedTileCache::shrinkCacheSize(rspfAppFixedCacheId id,
                                             rspf_int32 byteCount)
{
   rspfFixedTileCache* cache = getCache(id);

   if(cache)
   {
      shrinkCacheSize(cache, byteCount);
   }
}

void rspfAppFixedTileCache::shrinkCacheSize(rspfFixedTileCache* cache,
                                             rspf_int32 byteCount)
{
   if(cache)
   {
      rspf_int32 cacheSize = cache->getCacheSize();
      if(cacheSize <= byteCount)
      {
         cache->flush();
      }
      else
      {
         while(byteCount > 0)
         {
            rspf_uint32 before = cache->getCacheSize();
            cache->deleteTile();
            rspf_uint32 after = cache->getCacheSize();
            rspf_uint32 delta = std::abs((int)(before - after));
            if(delta)
            {
               byteCount -= delta;
               theCurrentCacheSize -= (delta);
            }
            else
            {
               byteCount = 0;
            }
         }
      }
   }
}

const rspfIpt& rspfAppFixedTileCache::getTileSize(rspfAppFixedCacheId cacheId)
{
   OpenThreads::ScopedLock<OpenThreads::Mutex> lock(theMutex);
   rspfFixedTileCache* cache = getCache(cacheId);
   if(cache)
   {
      return cache->getTileSize();
   }
   return theTileSize;
}
