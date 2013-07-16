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
// $Id: rspfAppTileCache.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <rspf/imaging/rspfAppTileCache.h>
#include <rspf/imaging/rspfTileCache.h>
#include <rspf/base/rspfDataObject.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfPreferences.h>

rspfAppTileCache* rspfAppTileCache::theInstance = 0;

// we will need to grab this from the preferences
const rspf_uint32 rspfAppTileCache::DEFAULT_SIZE          = 80*1024*1024;
const rspf_uint32 rspfAppTileCache::DEFAULT_BUCKET_SIZE          = 293;
rspfAppTileCache::rspfAppCacheId rspfAppTileCache::theUniqueAppIdCounter = 1;

rspfAppTileCache *rspfAppTileCache::instance(rspf_uint32  maxSize)
{
   if(!theInstance)
   {
      if(maxSize < 1)
      {
         rspfString cacheSize = rspfPreferences::instance()->findPreference("cache_size");
         if(cacheSize!="")
         {
            maxSize = cacheSize.toUInt32()*1024*1024;
         }
         else
         {
            maxSize = DEFAULT_SIZE;
         }
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "Setting SIZE----------------------- " << maxSize << std::endl;
      }
      theInstance = new rspfAppTileCache(maxSize);
   }

   return theInstance;
}

rspfAppTileCache::~rspfAppTileCache()
{
   deleteAll();
}

rspfAppTileCache::rspfAppCacheId rspfAppTileCache::newTileCache(rspf_uint32 bucketSize)
{
   rspfTileCache *aCache = 0;
   rspfAppCacheId result = 0;

   aCache = new rspfTileCache(bucketSize);

   if(aCache)
   {
      theAppCache.insert(make_pair(theUniqueAppIdCounter, aCache));
      result = theUniqueAppIdCounter;
      ++theUniqueAppIdCounter;
   }

   return result;
}

/*!
 * Will retrieve a tile from the cache.
 */
rspfDataObject *rspfAppTileCache::get(rspfAppCacheId id,
                                        const rspfDpt3d &origin,
                                        rspf_uint32 resLevel)
{
   rspfDataObject* result = 0;
   if(id>0)
   {
      rspfTileCache *aCache = this->get(id);
      if(aCache)
      {
         result = aCache->get(origin,
                              resLevel);
         if(result)
         {
            adjustQueue(id, origin, resLevel);
         }
      }
   }

   return result;
}

rspfRefPtr<rspfDataObject> rspfAppTileCache::removeTile(rspfAppCacheId id,
                                              const rspfDpt3d &origin,
                                              unsigned long resLevel)
{
   rspfRefPtr<rspfDataObject> result = 0;
   if(id>0)
   {
      rspfTileCache *aCache = this->get(id);
      if(aCache)
      {
         // remove from cache
         result = aCache->remove(origin,
                                 resLevel);
         // adjust the byte count
         if(result.valid())
         {
            theCurrentCacheSize -= result->getDataSizeInBytes();
         }
         removeTileFromQueue(id, origin, resLevel);
      }
   }

   return result;
}

rspfRefPtr<rspfDataObject> rspfAppTileCache::insert(rspfAppCacheId appId,
                                                       const rspfDpt3d &origin,
                                                       const rspfDataObject* data,
                                                       rspf_uint32 resLevel)
{
   static const char MODULE[] = "rspfAppTileCache::insert";
   rspfDataObject *result = 0;

   // find the cache and if it's not there then return NULL
   rspfTileCache *aCache = this->get(appId);
   if(!aCache)
   {         
      return result;
   }
   
   rspfRefPtr<rspfDataObject> tileToInsert = 0;
   long dataSize = data->getDataSizeInBytes();
   
   if( (theCurrentCacheSize+dataSize) > theMaxCacheSize)
   {
      do
      {
         rspfRefPtr<rspfDataObject> tile = removeTile();
      }while((theCurrentCacheSize+dataSize) > theMaxCacheSize);
   }

   if(data)
   {
      tileToInsert = (rspfDataObject*)data->dup();

      result = aCache->insert(origin,
                              tileToInsert.get(),
                              resLevel);
      if(!result)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " ERROR: can't insert and should not happen"
            << endl;

         tileToInsert = 0;
      }
      else
      {
         theCurrentCacheSize += dataSize;
         theUsedQueue.push_back(rspfAppCacheTileInfo(appId,
                                                 origin,
                                                 resLevel));
      }
   }

   return result;
}


rspfTileCache* rspfAppTileCache::get(rspfAppCacheId id)
{
   rspfTileCache *result=0;
   
   AppIdIterator anIterator = theAppCache.find(id);
   
   if(anIterator != theAppCache.end())
   {
      result = (*anIterator).second;
   }

   return result;
}

void rspfAppTileCache::deleteCache(rspfAppCacheId appId)
{
   // first delete the cache
   AppIdIterator anIterator = theAppCache.find(appId);
   if(anIterator != theAppCache.end())
   {
      rspfTileCache *aCache = (*anIterator).second;
      theCurrentCacheSize -= aCache->sizeInBytes();

      delete aCache;

      theAppCache.erase(anIterator);      
   }
   // now delete all occurences of the appCacheId in the queue
   // used for LRU algorithm.
   deleteAppCacheFromQueue(appId);
}

void rspfAppTileCache::deleteAll()
{
   AppIdIterator iter = theAppCache.begin();

   while(iter != theAppCache.end())
   {
      delete (*iter).second;
      ++iter;
   }
   
   theAppCache.clear();
}

void rspfAppTileCache::removeTileFromQueue(rspfAppCacheId appId,
                                       const rspfDpt3d &origin,
                                       rspf_uint32 resLevel)
{
   list<rspfAppCacheTileInfo>::iterator anIterator;

   anIterator = theUsedQueue.begin();
   while(anIterator != theUsedQueue.end())
   {
      if( ((*anIterator).theAppCacheId == appId) &&
          ((*anIterator).theOrigin     == origin)&&
          ((*anIterator).theResLevel   == resLevel))
      {
         theUsedQueue.erase(anIterator);
         return;
      }
      else
      {
         ++anIterator;
      }
   }
}

void rspfAppTileCache::deleteAppCacheFromQueue(rspfAppCacheId appId)
{
   list<rspfAppCacheTileInfo>::iterator anIterator;

   anIterator = theUsedQueue.begin();
   while(anIterator != theUsedQueue.end())
   {
      if( (*anIterator).theAppCacheId == appId)
      {
         anIterator = theUsedQueue.erase(anIterator);
      }
      else
      {
         ++anIterator;
      }
   }
}

rspfRefPtr<rspfDataObject> rspfAppTileCache::removeTile()
{
   rspfRefPtr<rspfDataObject> result;
   if(!theUsedQueue.empty())
   {
      rspfAppCacheTileInfo &info = *(theUsedQueue.begin());

      rspfTileCache *aCache = get(info.theAppCacheId);
      if(aCache)
      {
         result            = aCache->remove(info.theOrigin,
                                            info.theResLevel);
         theCurrentCacheSize -= result->getDataSizeInBytes();
      }
      theUsedQueue.erase(theUsedQueue.begin());
   }

   return result;
}

void rspfAppTileCache::adjustQueue(rspfAppCacheId id,
                                    const rspfDpt3d &origin,
                                    rspf_uint32 resLevel)
{
   list<rspfAppCacheTileInfo>::iterator anIterator = theUsedQueue.begin();
   rspfAppCacheTileInfo info(id, origin, resLevel);
   
   while(anIterator != theUsedQueue.end())
   {
     
      if((*anIterator) == info)
      {
         theUsedQueue.erase(anIterator);
         theUsedQueue.push_back(info);
         return;
      }
      ++anIterator;
   }              
}
