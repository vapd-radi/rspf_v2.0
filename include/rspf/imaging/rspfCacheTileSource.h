//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts
//
// Description:  rspfCacheTileSource
// 
//*******************************************************************
//  $Id: rspfCacheTileSource.h 20456 2012-01-13 19:39:30Z gpotts $
#ifndef rspfCacheTileSource_HEADER
#define rspfCacheTileSource_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfAppFixedTileCache.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <OpenThreads/Mutex>

/** Cache Tile Source */
class RSPFDLLEXPORT rspfCacheTileSource : public rspfImageSourceFilter
{
public:
   typedef std::vector<rspfAppFixedTileCache::rspfAppFixedCacheId> RLevelCacheList;
   /**
    * Will construct a new Application cache
    */
   rspfCacheTileSource();
   // rspfCacheTileSource(rspfImageSource* inputSource);

   virtual rspfString getLongName()  const;
   virtual rspfString getShortName() const;
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   virtual void initialize();
   virtual void flush();
   virtual void setCachingEnabledFlag(bool value);
   virtual void setEventProgressFlag(bool value);

   void getTileSize(rspfIpt& size) const;

   /**
    * @brief Set the tile size.  This changes underlying cache tile size.
    * @param Size of cache tile.
    */
   void setTileSize(const rspfIpt& size);
   
   virtual rspf_uint32 getTileWidth() const;
   virtual rspf_uint32 getTileHeight() const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   

   
protected:
   virtual ~rspfCacheTileSource();

   void allocate();
   void deleteRlevelCache();
   void initializeRlevelCache();
   
   rspfAppFixedTileCache::rspfAppFixedCacheId getCacheId(rspf_uint32 resLevel);

   rspfRefPtr<rspfImageData> theTile;
   rspfIpt                    theFixedTileSize;
   bool                        theCachingEnabled;
   bool                        theEventProgressFlag;
   bool                        theUseInputTileSizeFlag;
   RLevelCacheList             theRLevelCacheList;
   
   /** For lock and unlock. */

   
   virtual void fireProgressEvent(double percentComplete);
   
   virtual rspfRefPtr<rspfImageData> fillTile(rspf_uint32 resLevel);

TYPE_DATA
};


#endif /* end of "#ifndef rspfCacheTileSource_HEADER" */
