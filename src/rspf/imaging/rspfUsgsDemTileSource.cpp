//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Contains class declaration for rspfUsgsDemTileSource.
//
//********************************************************************
// $Id: rspfUsgsDemTileSource.cpp 19640 2011-05-25 15:58:00Z oscarkramer $

#include <iostream>
#include <fstream>
#include <rspf/imaging/rspfUsgsDemTileSource.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/support_data/rspfDemHeader.h>
#include <rspf/support_data/rspfDemProfile.h>
#include <rspf/support_data/rspfDemGrid.h>
#include <rspf/support_data/rspfDemUtil.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>

RTTI_DEF1(rspfUsgsDemTileSource, "rspfUsgsDemTileSource", rspfImageHandler)

// Static trace for debugging
static rspfTrace traceDebug("rspfUsgsDemTileSource:debug");

static const char DEM_TYPE_KW[] = "dem_type";
static const char USGS_DEM_KW[] = "usgs_dem";


rspfUsgsDemTileSource::rspfUsgsDemTileSource()
   :
      rspfImageHandler(),
      theDem(0),
      theTile(0),
      theNullValue(0.0),
      theMinHeight(0.0),
      theMaxHeight(0.0),
      theFeetFlag(false),
      theIsDemFlag(false),
      theScalarType(RSPF_SINT16)
{
   // Construction not complete.  Users should call "open" method.
}

rspfUsgsDemTileSource::~rspfUsgsDemTileSource()
{
   if (theDem)
   {
      delete theDem;
      theDem = 0;
   }
   theTile = 0;
}

rspfRefPtr<rspfImageData> rspfUsgsDemTileSource::getTile(
   const  rspfIrect& tile_rect, rspf_uint32 resLevel)
{
   if (theTile.valid())
   {
      // Image rectangle must be set prior to calling getTile.
      theTile->setImageRectangle(tile_rect);
      
      if ( getTile( theTile.get(), resLevel ) == false )
      {
         if (theTile->getDataObjectStatus() != RSPF_NULL)
         {
            theTile->makeBlank();
         }
      }
   }
   
   return theTile;
}

bool rspfUsgsDemTileSource::getTile(rspfImageData* result,
                                     rspf_uint32 resLevel)
{
   bool status = false;

   //---
   // Not open, this tile source bypassed, or invalid res level,
   // return a blank tile.
   //---
   if( isOpen() && isSourceEnabled() && isValidRLevel(resLevel) &&
       result && (result->getNumberOfBands() == getNumberOfOutputBands()) )
   {
      result->ref(); // Increment ref count.

      //---
      // Check for overview tile.  Some overviews can contain r0 so always
      // call even if resLevel is 0.  Method returns true on success, false
      // on error.
      //---
      status = getOverviewTile(resLevel, result);
      
      if (status) // From old code.  Not sure if still needed.
      {
         result->setMinPix(theTile->getMinPix(), theTile->getNumberOfBands());
         result->setMaxPix(theTile->getMaxPix(), theTile->getNumberOfBands());
         result->setNullPix(theTile->getNullPix(), theTile->getNumberOfBands());
      }
      
      if (!status) // Did not get an overview tile.
      {
         status = true;
         
         rspfIrect tile_rect = result->getImageRectangle();
         
         rspfIrect image_rect = getImageRectangle(resLevel);
         
         //---
         // See if any point of the requested tile is in the image.
         //---
         if ( tile_rect.intersects(image_rect) )
         {
            rspfIrect clip_rect = tile_rect.clipToRect(image_rect);
            
            if ( !tile_rect.completely_within(clip_rect) )
            {
               // Start with a blank tile.
               result->makeBlank();
            }
            
            // Load the tile buffer with data from the dem cell.
            if (theScalarType == RSPF_FLOAT32)
            {
               status = fillBuffer(rspf_float32(0.0),
                                   tile_rect,
                                   clip_rect,
                                   result);
            }
            else
            {
               status = fillBuffer(rspf_sint16(0),
                                   tile_rect,
                                   clip_rect,
                                   result);
            }
            if (status == true)
            {
               result->validate();
            }
         }    
         else // No intersection:  if ( tile_rect.intersects(image_rect) )
         {
            result->makeBlank();
         }
      }

      result->unref(); // Decrement ref count.
   }

   return status;
}

//*******************************************************************
// Private Method:
//*******************************************************************
template <class T>
bool rspfUsgsDemTileSource::fillBuffer(T,
                                        const rspfIrect& tile_rect,
                                        const rspfIrect& clip_rect,
                                        rspfImageData* tile)
{
   const uint32 TILE_WIDTH = tile->getWidth();

   T* d = static_cast<T*>(tile->getBuf());
   
   rspf_float32 spatialResZ = theDem->getHeader().getSpatialResZ();
   if (spatialResZ == 0.0) spatialResZ = 1.0;

   // Compute the starting offset.
   uint32 offset = (clip_rect.ul().y - tile_rect.ul().y) * TILE_WIDTH +
                   clip_rect.ul().x - tile_rect.ul().x;

   for (int32 line = clip_rect.ul().y; line <= clip_rect.lr().y; ++line)
   {
      // Loop in the longitude or sample direction.
      int samp_offset = 0;
      for (int32 samp = clip_rect.ul().x; samp <= clip_rect.lr().x; ++samp)
      {
         rspf_float32 p = theDem->getElevation(samp, line);
         if (p != theNullValue)
         {
            p *= spatialResZ;
            if (theFeetFlag) p = p * MTRS_PER_FT;
         }
         d[offset + samp_offset] = static_cast<T>(p);
         ++samp_offset;
      }
      offset += TILE_WIDTH;
   }

   return true;
}

rspfIrect
rspfUsgsDemTileSource::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   return rspfIrect(0,
                     0,
                     getNumberOfSamples(reduced_res_level) - 1,
                     getNumberOfLines(reduced_res_level)   - 1);
}

bool rspfUsgsDemTileSource::open()
{
   static const char MODULE[] = "rspfUsgsDemTileSource::open";

   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\nAttempting to parse file:  " << theImageFile.c_str()
           << std::endl;
   }

   theIsDemFlag = rspfDemUtil::isUsgsDem(theImageFile);

   if (theIsDemFlag)
   {
      // Open up the file for reading.
      std::ifstream is(theImageFile.c_str(),
                       std::ios_base::in | std::ios_base::binary);
      if (is.good())
      {
         // Start out with a fresh dem.
         if (theDem) delete theDem;
         
         //---
         // Set the null to -32768.  This will also be the missing data values.
         //---
         theNullValue = RSPF_DEFAULT_NULL_PIX_SINT16;
         
         //---
         // Read the dem.
         // 
         // NOTE:  This defines the missing data value.  It should be the
         // same as null for mosaicing and min/max calculations.
         //---
         theDem = new rspfDemGrid(theNullValue);
         theDem->read(is);
         
         is.close();
      }
      else
      {
         theIsDemFlag = false;
      }
   }
   
   if (theIsDemFlag)
   {
      theFeetFlag  = (theDem->getHeader().getElevationUnits() == 1) ?
         true : false;
      
      theMinHeight = theFeetFlag ? theDem->getHeader().getMinimumElev() *
         MTRS_PER_FT : theDem->getHeader().getMinimumElev();
      theMaxHeight = theFeetFlag ? theDem->getHeader().getMaximumElev() *
         MTRS_PER_FT : theDem->getHeader().getMaximumElev();
      
      completeOpen();
      
      //---
      // Set up the tiles.  Note the blank tile will not get initialized to
      // save memory.  This includes setting the min and max pixel value
      // gathered from the statistics.
      //---
      theTile = rspfImageDataFactory::instance()->create(this, this);
      theTile->initialize();
      
      if (traceDebug())
      {
         CLOG << setiosflags(ios::fixed) << setprecision(5) << "DEBUG:"
              << "\nNull pix:  " << (theTile->getNullPix(0))
              << "\nMin  pix:  " << (theTile->getMinPix(0))
              << "\nMax  pix:  " << (theTile->getMaxPix(0))
              << "\nlines:     " << theDem->getHeight()
              << "\nsamples:   " << theDem->getWidth()
              << std::endl;
      }
   }

   return theIsDemFlag;
}

bool rspfUsgsDemTileSource::saveState(rspfKeywordlist& kwl,
                                       const char* prefix) const
{
   // Save of the dem type keyword.
   kwl.add(prefix, DEM_TYPE_KW, USGS_DEM_KW, true);

   // Save the scalar type.
   kwl.add(prefix,
           rspfKeywordNames::SCALAR_TYPE_KW,
           rspfScalarTypeLut::instance()->getEntryString(theScalarType));
   
   return rspfImageHandler::saveState(kwl, prefix);
}

bool rspfUsgsDemTileSource::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   if (rspfImageHandler::loadState(kwl, prefix))
   {
      //***
      // Look for "dem_type: usgs_dem" key value pair.
      // Note:  If not in the keyword list the "open" will look for it on
      //        disk.
      //***
      const char* lookup = kwl.find(prefix, DEM_TYPE_KW);
      if (lookup)
      {
         rspfString s = lookup;
         s.downcase();
         if (s == USGS_DEM_KW) theIsDemFlag = true;
      }

      //---
      // Look for scalar type override.
      //
      // Note: We only allow float or signed 16 bit.
      //---
      lookup = kwl.find(prefix, rspfKeywordNames::SCALAR_TYPE_KW);
      if (lookup)
      {
         rspfScalarType st =
            static_cast<rspfScalarType>(rspfScalarTypeLut::instance()->
                                         getEntryNumber(lookup));
         if ( (st == RSPF_FLOAT32) || (st == RSPF_SINT16))
         {
            theScalarType = st;
         }
         else
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfUsgsDemTileSource::loadState WARNING:"
                  << "\nInvalid scalar type: "
                  << rspfScalarTypeLut::instance()->
                  getEntryString(st)
                  << std::endl;
            }
         }
      }

      if (open())
      {
         return true;
      }
   }

   return false;
}

rspfRefPtr<rspfImageGeometry> rspfUsgsDemTileSource::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();

      if ( !theGeometry )
      {
         // Check the internal geometry first to avoid a factory call.
         theGeometry = getInternalImageGeometry();

         // At this point it is assured theGeometry is set.

         //---
         // WARNING:
         // Must create/set theGeometry at this point or the next call to
         // rspfImageGeometryRegistry::extendGeometry will put us in an infinite loop
         // as it does a recursive call back to rspfImageHandler::getImageGeometry().
         //---         

         // Check for set projection.
         if ( !theGeometry->getProjection() )
         {
            // Try factories for projection.
            rspfImageGeometryRegistry::instance()->extendGeometry(this);
         }
      }

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
   }

   return theGeometry;
}


rspfRefPtr<rspfImageGeometry> rspfUsgsDemTileSource::getInternalImageGeometry() const
{
   rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry();
   
   if (theDem)
   {
      const rspfDemHeader HDR = theDem->getHeader();
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfUsgsDemTileSource::getInternalImageGeometry DEBUG:"
            << "\nDEM Header:"
            << std::endl;
         HDR.print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
      
      // The DEM's image geometry is a map projection, obtained here via KWL:
      rspfKeywordlist proj_kwl;
      if ( HDR.getImageGeometry(proj_kwl) ) 
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfUsgsDemTileSource::getInternalImageGeometry DEBUG:"
               << "keyword list:\n" << proj_kwl
               << std::endl;
         }
         
         // Capture for next time.
         rspfRefPtr<rspfProjection> proj =
            rspfProjectionFactoryRegistry::instance()->
            createProjection(proj_kwl);
         if ( proj.valid() )
         {
            geom->setProjection(proj.get());
         }
      }
   }
   
   return geom;
}

rspfScalarType rspfUsgsDemTileSource::getOutputScalarType() const
{
   return theScalarType;
}

rspf_uint32 rspfUsgsDemTileSource::getTileWidth() const
{
   return ( theTile.valid() ? theTile->getWidth() : 0 );
}

rspf_uint32 rspfUsgsDemTileSource::getTileHeight() const
{
   return ( theTile.valid() ? theTile->getHeight() : 0 );
}

rspf_uint32 rspfUsgsDemTileSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      if (theDem)
      {
         return theDem->getHeight();
      }
      else
      {
         cerr << "rspfUsgsDemTileSource::getNumberOfLines ERROR:"
              << "\nNot initialized!" << std::endl;
         return 0;
      }
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfLines(reduced_res_level);
   }

   return 0;
}

rspf_uint32 rspfUsgsDemTileSource::getNumberOfSamples(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      if (theDem)
      {
         return theDem->getWidth();
      }
      else
      {
         cerr << "rspfUsgsDemTileSource::getNumberOfSamples ERROR:"
              << "\nNot initialized!" << std::endl;
         return 0;
      }
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfSamples(reduced_res_level);
   }

   return 0;
}

rspf_uint32 rspfUsgsDemTileSource::getImageTileWidth() const
{
   return 0;
}

rspf_uint32 rspfUsgsDemTileSource::getImageTileHeight() const
{
   return 0;
}

rspfString rspfUsgsDemTileSource::getShortName() const
{
   return rspfString("usgs dem");
}

rspfString rspfUsgsDemTileSource::getLongName() const
{
   return rspfString("usgs dem reader");
}

rspf_uint32 rspfUsgsDemTileSource::getNumberOfInputBands() const
{
   return 1;
}

rspf_uint32 rspfUsgsDemTileSource::getNumberOfOutputBands()const
{
   return 1;
}

double rspfUsgsDemTileSource::getNullPixelValue(rspf_uint32 /* band */)const 
{
   return theNullValue;
}

double rspfUsgsDemTileSource::getMinPixelValue(rspf_uint32 band)const 
{
   if(band < theMetaData.getNumberOfBands())
   {
      return theMetaData.getMinPix(band);
   }
   return theMinHeight; 
}

double rspfUsgsDemTileSource::getMaxPixelValue(rspf_uint32 band)const 
{
   if(theMetaData.getNumberOfBands())
   {
      return theMetaData.getMaxPix(band);
   }
   return theMaxHeight; 
}

bool rspfUsgsDemTileSource::isOpen()const
{
   return (theDem ? true : false );
}

const rspfUsgsDemTileSource& rspfUsgsDemTileSource::operator=(const  rspfUsgsDemTileSource& rhs)
{
   return rhs;
} 

rspfUsgsDemTileSource::rspfUsgsDemTileSource(const rspfUsgsDemTileSource&)
{
}
