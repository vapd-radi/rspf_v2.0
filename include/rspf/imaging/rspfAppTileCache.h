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
// $Id: rspfAppTileCache.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAppTileCache_HEADER
#define rspfAppTileCache_HEADER
#include <map>
#include <list>
using namespace std;

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfRefPtr.h>
// class rspfTile;
class rspfTileCache;
class rspfDataObject;

class rspfAppTileCache
{
public:
   static const rspf_uint32 DEFAULT_SIZE;

   static const rspf_uint32 DEFAULT_BUCKET_SIZE;

   typedef rspf_int32 rspfAppCacheId;
   typedef rspf_int32 rspfTileId;
   typedef map<rspfAppCacheId, rspfTileCache*>::iterator AppIdIterator;

   ~rspfAppTileCache();

   /*!
    *  We will force a singleton on this class.
    */
   static rspfAppTileCache *instance(rspf_uint32  maxSize   = 0);

   /*!
    * Will create a new Tile cache for this application.  Will
    * return 0 if not successful.
    */
   rspfAppCacheId newTileCache(rspf_uint32 bucketSize=DEFAULT_BUCKET_SIZE);

   /*!
    * Will retrieve a tile from the cache.  Will return NULL
    * if not found.
    */
   rspfDataObject *get(rspfAppCacheId id,
                        const rspfDpt3d &origin,
                        rspf_uint32 resLevel=0);

   /*!
    * Will remove a tile completly from the cache.  The
    * returned tile is no longer owned by the cache.
    */
   rspfRefPtr<rspfDataObject> removeTile(rspfAppCacheId id,
                                           const rspfDpt3d &origin,
                                           unsigned long resLevel);
   /*!
    * Will insert a tile into the cache.
    */
   rspfRefPtr<rspfDataObject> insert(rspfAppCacheId id,
                                       const rspfDpt3d &origin,
                                       const rspfDataObject* data,
                                       rspf_uint32 resLevel=0);
   
   /*!
    * This will delete the specified cache. The LRU queue will
    * be updated accordingly
    */
   void deleteCache(rspfAppCacheId appId);
protected:

   struct rspfAppCacheTileInfo
   {
   public:
      rspfAppCacheTileInfo(rspfAppCacheId appId,
                       const rspfDpt3d& origin,
                       rspf_uint32    resLevel)
         :theAppCacheId(appId),
          theOrigin(origin),
          theResLevel(resLevel)
         {}
      rspfAppCacheId  theAppCacheId;
      rspfDpt3d         theOrigin;
      rspf_uint32     theResLevel;

      bool operator ==(const rspfAppCacheTileInfo &rhs)
         {
            return (theAppCacheId == rhs.theAppCacheId &&
                    theOrigin     == rhs.theOrigin &&
                    theResLevel   == rhs.theResLevel);
         } 
  
   };

   rspfAppTileCache(rspf_uint32  maxSize   = DEFAULT_SIZE)
      :
         theMaxCacheSize(maxSize),
         theCurrentCacheSize(0)
      {}

   void deleteAll();
   /*!
    * Used to get access to the App cache.
    */
  rspfTileCache* get(rspfAppCacheId id);

   /*!
    * will remove all occurances of the appId from the
    * queue
    */
   void deleteAppCacheFromQueue(rspfAppCacheId appId);

   /*!
    * Will remove a single instance of a tile from the queue.
    */
   void removeTileFromQueue(rspfAppCacheId appId,
                            const rspfDpt3d &origin,
                            rspf_uint32 resLevel);

   void adjustQueue(rspfAppCacheId id,
                    const rspfDpt3d &origin,
                    rspf_uint32 resLevel);
   /*!
    * will pop the queue and remove that tile from its cache
    * and return it to the caller. This is used by the insert
    * when the max cache size is exceeded.
    */
   rspfRefPtr<rspfDataObject> removeTile();


   static rspfAppTileCache *theInstance;
   /*!
    * Will hold the current unique Application id.
    */
   static rspfAppCacheId                   theUniqueAppIdCounter;
   /*!
    * Will hold the list of application caches
    */
   map<rspfAppCacheId, rspfTileCache*>  theAppCache;

   /*!
    * Is the maximum size of the cache.
    */
   rspf_uint32                      theMaxGlobalCacheSize;
   rspf_uint32                      theMaxCacheSize;
   

   /*!
    * This holds the current cache size.
    */
   rspf_uint32                      theCurrentCacheSize;
   

   /*!
    * Is used in an Least recently used algorithm
    */
   list<rspfAppCacheTileInfo>      theUsedQueue;
   
};

#endif
