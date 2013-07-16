//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: This file contains the cache algorithm
//
//***********************************
// $Id: rspfTileCache.cpp 17206 2010-04-25 23:20:40Z dburken $

#include <rspf/imaging/rspfTileCache.h>
#include <rspf/base/rspfDataObject.h>

void rspfTileCache::display()const
{
   rspfTileCache::Iterator tiles;

   for(long buckets = 0; buckets < theNumberOfBuckets; buckets++)
   {
      tiles = theCache[buckets].begin();
      while(tiles != theCache[buckets].end())
      {
         cout << ((*tiles).second) << endl;
         ++tiles;
      }
   }
}

rspfTileCache::rspfTileCache(long numberOfBuckets)
   : theCache(NULL),
     theNumberOfBuckets(numberOfBuckets>0?numberOfBuckets:255),
     theSizeInBytes(0)
{
   theCache = new multimap<rspf_uint32, rspfTileInformation*>[theNumberOfBuckets];
}

rspfTileCache::~rspfTileCache()
{
   deleteAll();
}

rspfDataObject* rspfTileCache::get(const rspfDpt3d &origin,
                                    unsigned long resLevel)
{
   rspfDataObject* result = NULL;
   Iterator anIterator;

   rspf_uint32 bucket = bucketHash(origin);
   
   anIterator = theCache[bucket].find(tileId(origin));
   while(anIterator != theCache[bucket].end())
   {
      CacheDataPtr info = (*anIterator).second;
      if(info)
      {
         if(info->theOrigin   == origin &&
            info->theResLevel == resLevel)
         {
            return info->theCachedTile.get();
         }
      }

      ++anIterator;
   }
   return result;
}

rspfDataObject* rspfTileCache::remove(const rspfDpt3d &origin,
                                  unsigned long resLevel)
{
   Iterator anIterator;
   rspfDataObject *result;
   rspf_uint32 bucket = bucketHash(origin);
   
   anIterator = theCache[bucket].find(tileId(origin));
   while(anIterator != theCache[bucket].end())
   {
      CacheDataPtr info = (*anIterator).second;
      if(info)
      {
         if(info->theOrigin   == origin &&
            info->theResLevel == resLevel)
         {
            theCache[bucket].erase(anIterator);
            theSizeInBytes -= info->theCachedTile->getDataSizeInBytes();

            result = info->theCachedTile.get();

            delete info;
            return result;
         }
      }

      ++anIterator;
   }
   
   return NULL;   
}

rspfDataObject* rspfTileCache::insert(const rspfDpt3d &origin,
                                       rspfDataObject* data,
                                       unsigned long resLevel)
{   
   rspf_uint32 bucket = bucketHash(origin);

   // make sure we keep up with the current size of the
   // cache in bytes.  This is only the count of the data
   // and not any overhead required by the cache.
   theSizeInBytes += data->getDataSizeInBytes();
   theCache[bucket].insert(make_pair(tileId(origin),
                                     new CacheData(data,
                                                   origin,
                                                   resLevel)));
   
   return data;
}

void rspfTileCache::invalidate()
{
   
}

void rspfTileCache::invalidate(const rspfDpt3d& /* origin */,
                                rspf_uint32 /* resLevel */)
{
}



rspf_uint32 rspfTileCache::tileId(const rspfDpt3d &aPt)
{
   const unsigned char *bufx = (unsigned char*)(&aPt.x);
   const unsigned char *bufy = (unsigned char*)(&aPt.y);
   const unsigned char *bufz = (unsigned char*)(&aPt.z);

   // this just multiplies each byte by some prime number
   // and add together.
   return (rspf_uint32)(bufx[0]*101 + bufx[1]*103 +
                    bufx[2]*107 + bufx[3]*109 +
                    bufx[4]*113 + bufx[5]*127 +
                    bufx[6]*131 + bufx[7]*137 +
                    bufy[0]*139 + bufy[1]*149 +
                    bufy[2]*151 + bufy[3]*157 +
                    bufy[4]*163 + bufy[5]*167 +
                    bufy[6]*173 + bufy[7]*179 +
                    bufz[0]*181 + bufz[1]*191 +
                    bufz[2]*193 + bufz[3]*197 +
                    bufz[4]*199 + bufz[5]*211 +
                    bufz[6]*223 + bufz[6]*227);       
}

rspf_uint32 rspfTileCache::bucketHash(const rspfDpt3d &aPt)
{   
   return tileId(aPt)%theNumberOfBuckets;
}

void rspfTileCache::deleteAll()
{
   Iterator anIterator;

   for(long bucket = 0; bucket < theNumberOfBuckets; bucket++)
   {
      anIterator = theCache[bucket].begin();
      while(anIterator != theCache[bucket].end())
      {
         CacheDataPtr info = (*anIterator).second;
         delete info;
         
         ++anIterator;
      }
      theCache[bucket].clear();
   }
   delete [] theCache;
}
