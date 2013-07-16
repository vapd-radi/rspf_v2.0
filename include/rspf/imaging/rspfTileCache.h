//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description: This file contains the cache algorithm
//
//***********************************
// $Id: rspfTileCache.h 17207 2010-04-25 23:21:14Z dburken $

#ifndef DataCache_HEADER
#define DataCache_HEADER
#include <map>
#include <stack>
using namespace std;

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfDataObject.h>
class RSPF_DLL rspfTileCache
{
public:
   
   rspfTileCache(long numberOfBuckets  = 10);
   virtual ~rspfTileCache();

   /*!
    * This will return a tile if found.  Note this tile is still
    * owned by the cache.
    */
   virtual rspfDataObject* get(const rspfDpt3d &origin,
                               unsigned long resLevel=0);

   /*!
    * Will remove the tile from the cache and will not delete.  Note:
    * the cache no longer owns the tile.
    */
   virtual rspfDataObject* remove(const rspfDpt3d &origin,
                                  unsigned long resLevel=0);
   /*!
    * This will not duplicate the tile.  Note the tile is owned
    * by the cache.
    */
   virtual rspfDataObject* insert(const rspfDpt3d &origin,
                                  rspfDataObject* data,
                                  unsigned long resLevel=0);
   /*!
    * Not implemented
    */
   void invalidate();
   /*!
    * Not implemented
    */
   virtual void invalidate(const rspfDpt3d &origin,
                           rspf_uint32 resLevel);


   
   virtual long numberOfItems()const{return theCache?(long)theCache->size():(long)0;}
   virtual void display()const;

   virtual rspf_uint32 sizeInBytes(){return theSizeInBytes;}
   
protected:

   struct rspfTileInformation
   {
   public:
      friend ostream& operator <<(ostream &out,
                                  const rspfTileInformation& /* info */)
         {

            return out;
         }
      rspfTileInformation(rspfDataObject* aTile,
                           const rspfDpt3d &origin = rspfDpt(0,0),
                           unsigned long resLevel = 0)
         : theCachedTile(aTile),
           theOrigin(origin),
           theResLevel(resLevel)
         {}
      rspfRefPtr<rspfDataObject>    theCachedTile;
      rspfDpt3d         theOrigin;
      unsigned long      theResLevel;
   };

   typedef multimap<rspf_uint32, rspfTileInformation*>::iterator Iterator;
   typedef rspfTileInformation* CacheDataPtr;
   typedef rspfTileInformation  CacheData;
   
   void deleteAll();
   virtual rspf_uint32 bucketHash(const rspfDpt3d &aPt);
   virtual rspf_uint32 tileId(const rspfDpt3d &aPt);
   
   multimap<rspf_uint32, rspfTileInformation*>  *theCache;
   long                                            theNumberOfBuckets;
   rspf_uint32                                    theSizeInBytes;
};

#endif
