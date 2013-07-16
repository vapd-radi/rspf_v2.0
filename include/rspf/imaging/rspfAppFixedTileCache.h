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
// $Id: rspfAppFixedTileCache.h 20127 2011-10-12 11:27:10Z gpotts $
#ifndef rspfAppFixedTileCache_HEADER
#define rspfAppFixedTileCache_HEADER
#include <map>
#include <list>
#include <iostream>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfIrect.h>
#include <OpenThreads/ReadWriteMutex>

class rspfFixedTileCache;
class rspfImageData;

class RSPF_DLL rspfAppFixedTileCache
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfAppFixedTileCache& rhs);
   static const rspf_uint32 DEFAULT_SIZE;
   typedef rspf_int32 rspfAppFixedCacheId;
   static rspfAppFixedTileCache *instance(rspf_uint32  maxSize   = 0);
   virtual ~rspfAppFixedTileCache();
   
   /*!
    * Will flush all cache registered
    */
   virtual void flush();
   virtual void flush(rspfAppFixedCacheId cacheId);
   virtual void deleteCache(rspfAppFixedCacheId cacheId);
   /*!
    * Will create a new Tile cache for this application if the tile size is 0,0 it will
    * use the default tile size.  Will
    * return 0 if not successful.
    */
   rspfAppFixedCacheId newTileCache(const rspfIrect& tileBoundaryRect,
                                     const rspfIpt& tileSize=rspfIpt(0,0));
   rspfAppFixedCacheId newTileCache();

   virtual void setRect(rspfAppFixedCacheId cacheId,
                        const rspfIrect& boundaryTileRect);
   virtual void setTileSize(rspfAppFixedCacheId cacheId,
                            const rspfIpt& tileSize);
   
   rspfRefPtr<rspfImageData> getTile(rspfAppFixedCacheId cacheId,
                           const rspfIpt& origin);
   rspfRefPtr<rspfImageData> addTile(rspfAppFixedCacheId cacheId,
                                       rspfRefPtr<rspfImageData> data,
                                       bool duplicateData=true);
   
   rspfRefPtr<rspfImageData> removeTile(rspfAppFixedCacheId cacheId,
                                          const rspfIpt& origin);
   void deleteTile(rspfAppFixedCacheId cacheId,
                   const rspfIpt& origin);
   
   const rspfIpt& getTileSize(rspfAppFixedCacheId cacheId);
   
   virtual void setMaxCacheSize(rspf_uint32 cacheSize);
   
protected:
//    struct rspfAppFixedCacheTileInfo
//    {
//    public:
//       rspfAppFixedCacheTileInfo(rspfAppFixedCacheId appId,
//                                  const rspfIpt& origin)
//          :theAppCacheId(appId),
//           theOrigin(origin)
//          {}
//       rspfAppFixedCacheId  theAppCacheId;
//       rspfIpt       theOrigin;
      
//       bool operator ==(const rspfAppFixedCacheTileInfo &rhs)const
//          {
//             return (theAppCacheId == rhs.theAppCacheId &&
//                     theOrigin     == rhs.theOrigin);
//          } 
//    };
   
   rspfAppFixedTileCache();
   
   rspfFixedTileCache* getCache(rspfAppFixedCacheId cacheId);

   void shrinkGlobalCacheSize(rspf_int32 byteCount);
   void shrinkCacheSize(rspfAppFixedCacheId id,
                        rspf_int32 byteCount);
   void shrinkCacheSize(rspfFixedTileCache* cache,
                        rspf_int32 byteCount);
   void deleteAll();
   
   static rspfAppFixedTileCache *theInstance;
   
   /*!
    * Will hold the current unique Application id.
    */
   static rspfAppFixedCacheId    theUniqueAppIdCounter;
   rspfIpt                       theTileSize;
   rspf_uint32                   theMaxCacheSize;
   rspf_uint32                   theMaxGlobalCacheSize;
   rspf_uint32                   theCurrentCacheSize;

   std::map<rspfAppFixedCacheId, rspfFixedTileCache*> theAppCacheMap;

   OpenThreads::Mutex theMutex;
};

#endif
