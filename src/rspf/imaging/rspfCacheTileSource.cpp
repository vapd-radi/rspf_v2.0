//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Garrett Potts 
//
// Description:  rspfCacheTileSource
// 
//*******************************************************************
//  $Id: rspfCacheTileSource.cpp 20459 2012-01-17 01:30:36Z gpotts $

#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/imaging/rspfCacheTileSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>

static rspfTrace traceDebug("rspfCacheTileSource:debug");

static const rspfString TILE_SIZE_XY_KW("tile_size_xy");
static const rspfString USE_INPUT_TILE_SIZE_KW("use_input_tile_size");

RTTI_DEF1(rspfCacheTileSource, "rspfCacheTileSource", rspfImageSourceFilter);
   
rspfCacheTileSource::rspfCacheTileSource()
   : rspfImageSourceFilter(),
     //theCacheId(rspfAppFixedTileCache::instance()->newTileCache()),
     theTile(0),
     theFixedTileSize(),
     theCachingEnabled(true),
     theEventProgressFlag(false),
     theUseInputTileSizeFlag(false)
{
   rspf::defaultTileSize(theFixedTileSize);
}

rspfCacheTileSource::~rspfCacheTileSource()
{
   deleteRlevelCache();
   //rspfAppFixedTileCache::instance()->deleteCache(theCacheId);
   
   //theCacheId = -1;
  // theBoundingRect.makeNan();
}

void rspfCacheTileSource::flush()
{
   //rspfAppFixedTileCache::instance()->flush(theCacheId);
   rspf_uint32 idx = 0;
   for(idx = 0; idx < theRLevelCacheList.size();++idx)
   {
      rspfAppFixedTileCache::instance()->flush(theRLevelCacheList[idx]);
   }

}

void rspfCacheTileSource::initialize()
{
   rspfImageSourceFilter::initialize();
   flush();
   theTile = 0;
}
   
void rspfCacheTileSource::allocate()
{
   theTile = 0;
   if(theInputConnection)
   {
      theTile = rspfImageDataFactory::instance()->create(this, this);
      theTile->initialize();
   }
}

rspfRefPtr<rspfImageData> rspfCacheTileSource::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> result = 0;
   
   if ( theInputConnection )
   {
      if ( isSourceEnabled() )
      {
         if(!theTile.valid())
         {
            allocate();
         }
         
         if (theTile.valid())
         {
            theTile->setImageRectangle(tileRect);
            theTile->makeBlank();
            // see if we can get a valid cache at the given resolution level
            if(getCacheId(resLevel) < 0)
            {
               return theInputConnection->getTile(tileRect, resLevel);
            }
            result = fillTile(resLevel);
         }
      }
      else // Not enabled...
      {
         result = theInputConnection->getTile(tileRect, resLevel);
      }

   } // End:  if ( theInputConnection )

   return result;
}

rspfRefPtr<rspfImageData> rspfCacheTileSource::fillTile(
   rspf_uint32 resLevel)
{
   rspfRefPtr<rspfImageData> tempTile = 0;
   fireProgressEvent(0.0);
   rspfIrect boundingRect = getBoundingRect(resLevel);
   rspfIrect tileRect = theTile->getImageRectangle();
   rspfIrect allignedRect = tileRect.clipToRect(boundingRect);
   rspfAppFixedTileCache::rspfAppFixedCacheId cacheId = getCacheId(resLevel); 
   rspfIpt cacheTileSize =  rspfAppFixedTileCache::instance()->getTileSize(cacheId);
   if ( !allignedRect.hasNans() )
   {
      allignedRect.stretchToTileBoundary(cacheTileSize);
     // rspfAppFixedTileCache::rspfAppFixedCacheId cacheId = theCacheId;
      
      // check to see if we need to loop
      if((allignedRect == tileRect)&&
         (static_cast<rspf_int32>(tileRect.width())  == cacheTileSize.x)&&
         (static_cast<rspf_int32>(tileRect.height()) == cacheTileSize.y))
      {
         rspfIpt origin = tileRect.ul();
         if(theCachingEnabled)
         {
            tempTile = rspfAppFixedTileCache::instance()->getTile(cacheId,
                                                                   origin);
         }
         if(!tempTile.valid())
         {
            tempTile = theInputConnection->getTile(tileRect, resLevel);
            
            if(tempTile.valid())
            {
               if((tempTile->getBuf())&&
                  (tempTile->getDataObjectStatus()!=RSPF_EMPTY)&&
                  theCachingEnabled)
               {
                  rspfAppFixedTileCache::instance()->addTile(cacheId,
                                                              tempTile);
               }
            }
         }
//         else
//         {
//            std::cout << "FOUND IN CACHE AT RES " << resLevel << "\n";
//         }
         
         if(tempTile.valid())
         {
            if((tempTile->getDataObjectStatus() != RSPF_NULL)&&
               (tempTile->getDataObjectStatus() != RSPF_EMPTY))
            {
               theTile->setDataObjectStatus(tempTile->getDataObjectStatus());
               theTile->loadTile(tempTile.get());
            }
         }
         fireProgressEvent(100.0);
      }
      else
      {
         
         rspf_int32 boundaryHeight = allignedRect.height();
         rspf_int32 boundaryWidth  = allignedRect.width();
         rspfIpt origin(allignedRect.ul());
         rspf_int32 totalTiles  = (boundaryHeight/cacheTileSize.y)*
            (boundaryWidth/theFixedTileSize.x);
         rspf_int32 currentTile = 0;
         for(rspf_int32 row = 0;
             row < boundaryHeight;
             row+=theFixedTileSize.y)
         {
            origin.x  =  allignedRect.ul().x;
            for(rspf_int32 col = 0;
                col < boundaryWidth;
                col+=theFixedTileSize.x)
            {
               ++currentTile;
               if(theCachingEnabled)
               {
                  tempTile =
                     rspfAppFixedTileCache::instance()->getTile(cacheId,
                                                                 origin);
               }
               else
               {
                  tempTile = 0;
               }
               if(!tempTile.valid())
               {
                  rspfIrect rect(origin.x,
                                  origin.y,
                                  origin.x + cacheTileSize.x-1,
                                  origin.y + cacheTileSize.y-1);
                  
                  tempTile = theInputConnection->getTile(rect, resLevel);
                  
                  if(tempTile.valid())
                  {
                     // drb if(theTile->getBuf()&&
                     if(tempTile->getBuf()&&                        
                        (tempTile->getDataObjectStatus()!=RSPF_EMPTY)&&
                        theCachingEnabled)
                     {
                        rspfAppFixedTileCache::instance()->
                           addTile(cacheId, tempTile);
                     }
                  }
               }
             //  else
             //  {
             //     std::cout << "FOUND IN CACHE AT RES " << resLevel << "\n";
             // }
               
               if(tempTile.valid())
               {
                  if(tempTile->getBuf()&&
                     (tempTile->getDataObjectStatus()!=RSPF_EMPTY))
                  {
                     theTile->loadTile(tempTile.get());
                  }
               }
               double percent = 100.0*((double)currentTile/(double)totalTiles);
               fireProgressEvent(percent);
               origin.x   += theFixedTileSize.x;
            }
            origin.y += theFixedTileSize.y;
         }
         theTile->validate();
         fireProgressEvent(100);
      }
      
   } // End of:  if ( !allignedRect.hasNans() )
   
   return theTile;
}

rspf_uint32 rspfCacheTileSource::getTileWidth() const
{
   return theFixedTileSize.x;
}

rspf_uint32 rspfCacheTileSource::getTileHeight() const
{
   return theFixedTileSize.y;  
}

bool rspfCacheTileSource::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   //rspfAppFixedTileCache::instance()->deleteCache(theCacheId);
   
   //theCacheId = rspfAppFixedTileCache::instance()->newTileCache();
   
   deleteRlevelCache();
   const char* lookup = kwl.find(prefix, rspfKeywordNames::ENABLE_CACHE_KW);
   if(lookup)
   {
      theCachingEnabled = rspfString(lookup).toBool();
   }
   lookup = kwl.find(prefix, USE_INPUT_TILE_SIZE_KW);
   if(lookup)
   {
      theUseInputTileSizeFlag = rspfString(lookup).toBool();
   }

   lookup = kwl.find(prefix, TILE_SIZE_XY_KW);
   if (lookup)
   {
      rspfIpt pt;
      pt.toPoint(std::string(lookup));
      setTileSize(pt);
   }
   
   bool result = rspfImageSourceFilter::loadState(kwl, prefix);

   initialize();

   return result;
}

bool rspfCacheTileSource::saveState(rspfKeywordlist& kwl,
                                     const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::ENABLE_CACHE_KW,
           theCachingEnabled,
           true);
   kwl.add(prefix,
           USE_INPUT_TILE_SIZE_KW,
           theUseInputTileSizeFlag,
           true);
   
   kwl.add(prefix,
           TILE_SIZE_XY_KW,
           theFixedTileSize.toString().c_str());
   
   return rspfImageSourceFilter::saveState(kwl, prefix);
}

rspfRefPtr<rspfProperty> rspfCacheTileSource::getProperty(
   const rspfString& name)const
{
   // Lock for the length of this method.
   
   if (name == TILE_SIZE_XY_KW)
   {
      rspfRefPtr<rspfProperty> result =
         new rspfStringProperty(name, theFixedTileSize.toString());
      result->setCacheRefreshBit();
      return result;
   }
   else if(name == rspfKeywordNames::ENABLE_CACHE_KW)
   {
      rspfRefPtr<rspfProperty> result = new rspfBooleanProperty(name,
                                                                   theCachingEnabled);
      result->setCacheRefreshBit();
      return result;
   }
   else if(name == USE_INPUT_TILE_SIZE_KW)
   {
      rspfRefPtr<rspfProperty> result = new rspfBooleanProperty(name,
                                                                   theUseInputTileSizeFlag);
      result->setCacheRefreshBit();
      return result;
   }
   return rspfImageSourceFilter::getProperty(name);
}

void rspfCacheTileSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if (!property) return;

   rspfString name = property->getName();
   if (name == TILE_SIZE_XY_KW)
   {
      rspfIpt pt;
      pt.toPoint(property->valueToString());

      if ( (pt.x > 7) && (pt.y > 7) )
      {
         setTileSize(pt);
      }
      else
      {
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "rspfCacheTileSource::setProperty NOTICE:"
            << "\nTile dimensions must be at least 8!"
            << "\nFormat = ( x, y )"
            << std::endl;
      }
   }
   else if(name == rspfKeywordNames::ENABLE_CACHE_KW)
   {
      setCachingEnabledFlag(property->valueToString().toBool());
   }
   else if(name == USE_INPUT_TILE_SIZE_KW)
   {
      theUseInputTileSizeFlag = property->valueToString().toBool();
      if(theUseInputTileSizeFlag)
      {
         deleteRlevelCache();
      }
     // setCachingEnabledFlag(property->valueToString().toBool());
   }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}

void rspfCacheTileSource::getPropertyNames(
   std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(TILE_SIZE_XY_KW);
   propertyNames.push_back(rspfKeywordNames::ENABLE_CACHE_KW);
   propertyNames.push_back(USE_INPUT_TILE_SIZE_KW);
   
   rspfImageSourceFilter::getPropertyNames(propertyNames);
}

rspfString rspfCacheTileSource::getLongName()const
{
   return rspfString("Tile Cache , cache for rspfImageData objects.");
}

rspfString rspfCacheTileSource::getShortName()const
{
   return rspfString("Tile Cache");
}

void rspfCacheTileSource::setCachingEnabledFlag(bool value)
{
   if(!value && theCachingEnabled)
   {
      flush();
   }
   theCachingEnabled = value;
}

void rspfCacheTileSource::setEventProgressFlag(bool value)
{
   theEventProgressFlag = value;
}

void rspfCacheTileSource::getTileSize(rspfIpt& size) const
{
   size = theFixedTileSize;
}

void rspfCacheTileSource::setTileSize(const rspfIpt& size)
{
   if (size != theFixedTileSize)
   {
      theTile = 0; // Force an allocate of new tile.
      theFixedTileSize = size;
      initializeRlevelCache();
     // rspfAppFixedTileCache::instance()->setTileSize(theCacheId, size);
   }
}

void rspfCacheTileSource::fireProgressEvent(double percentComplete)
{
   if(theEventProgressFlag)
   {
      rspfProcessProgressEvent event(this, percentComplete);
      fireEvent(event);
   }
}

rspfAppFixedTileCache::rspfAppFixedCacheId rspfCacheTileSource::getCacheId(rspf_uint32 resLevel)
{
   rspfAppFixedTileCache::rspfAppFixedCacheId result = -1;
   if(theRLevelCacheList.empty())
   {
      initializeRlevelCache();
   }
   if(resLevel < theRLevelCacheList.size())
   {
      if(theRLevelCacheList[resLevel] <0)
      {
         rspfIpt cacheTileSize(theFixedTileSize);
         if(theUseInputTileSizeFlag)
         {
            cacheTileSize = rspfIpt(rspf::min(static_cast<rspf_int32>(theInputConnection->getTileWidth()),
                                                static_cast<rspf_int32>(1024)), 
                                     rspf::min(static_cast<rspf_int32>(theInputConnection->getTileHeight()),
                                                static_cast<rspf_int32>(1024)));
            
         }

         //std::cout << cacheTileSize << std::endl;
         rspfIrect rect = getBoundingRect(resLevel);
         
         if((cacheTileSize.x > static_cast<rspf_int64>(rect.width()))&&
            (cacheTileSize.y > static_cast<rspf_int64>(rect.height())))
         {
            cacheTileSize.x = rspf::max(static_cast<rspf_int64>(rect.width()), 
                                         static_cast<rspf_int64>(64));
            cacheTileSize.y = rspf::max(static_cast<rspf_int64>(rect.height()), 
                                         static_cast<rspf_int64>(64));
         }

         rect.stretchToTileBoundary(theFixedTileSize);
         theRLevelCacheList[resLevel] = rspfAppFixedTileCache::instance()->newTileCache(rect, cacheTileSize); 
      }
      result = theRLevelCacheList[resLevel];
   }
   
   return result;
}

void rspfCacheTileSource::deleteRlevelCache()
{
   rspf_uint32 idx = 0;
   for(idx = 0; idx < theRLevelCacheList.size();++idx)
   {
      rspfAppFixedTileCache::instance()->deleteCache(theRLevelCacheList[idx]);
   }
   theRLevelCacheList.clear();
}

void rspfCacheTileSource::initializeRlevelCache()
{
   rspf_uint32 nLevels = getNumberOfDecimationLevels();
   deleteRlevelCache();
   
   if(nLevels > 0)
   {
      rspf_uint32 idx = 0;
      theRLevelCacheList.resize(nLevels);
      for(idx= 0; idx < theRLevelCacheList.size(); ++idx)
      {
         theRLevelCacheList[idx] = -1;
      }
   }
}

