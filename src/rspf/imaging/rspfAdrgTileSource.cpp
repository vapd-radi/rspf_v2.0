//*******************************************************************
//
// See top level LICENSE.txt file.
//
// Author: Ken Melero
// Contributor: David A. Horner (DAH) - http://dave.thehorners.com
// 
// Description: This class give the capability to access tiles from an
//              ADRG file.
//
//********************************************************************
// $Id: rspfAdrgTileSource.cpp 21631 2012-09-06 18:10:55Z dburken $

#include <rspf/imaging/rspfAdrgTileSource.h>
#include <rspf/imaging/rspfAdrgHeader.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <iostream>

RTTI_DEF1(rspfAdrgTileSource, "rspfAdrgTileSource", rspfImageHandler)

//***
// Static trace for debugging
//***
static rspfTrace traceDebug("rspfAdrgTileSource:debug");

//***
// NOTE:  Currently this tilesource is hard-coded to access tiles
//        from the ZDR image file. This will change as I wrote support
//        data classes for all the ADRG image files: ZDR, overview,
//        and legend support data.  Next step in development will
//        be to put in flags to access overview and legend image files.
//***

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfAdrgTileSource::rspfAdrgTileSource()
   :
      rspfImageHandler(),
      m_Tile(0),
      m_TileBuffer(0),
      m_FileStr(),
      m_AdrgHeader(0)
{
   // Construction not complete.  Users should call "open" method.
}

rspfAdrgTileSource::~rspfAdrgTileSource()
{
   if(m_AdrgHeader)
   {
      delete m_AdrgHeader;
      m_AdrgHeader = 0;
   }
   if (m_TileBuffer)
   {
      delete [] m_TileBuffer;
      m_TileBuffer = 0;
   }

   close();
}

rspfRefPtr<rspfImageData> rspfAdrgTileSource::getTile(
   const rspfIrect& rect,
   rspf_uint32 resLevel)
{
   if (m_Tile.valid())
   {
      // Image rectangle must be set prior to calling getTile.
      m_Tile->setImageRectangle(rect);
      
      if ( getTile( m_Tile.get(), resLevel ) == false )
      {
         if (m_Tile->getDataObjectStatus() != RSPF_NULL)
         {
            m_Tile->makeBlank();
         }
      }
   }
   
   return m_Tile;
}

bool rspfAdrgTileSource::getTile(rspfImageData* result,
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
      result->ref();  // Increment ref count.

      //---
      // Check for overview tile.  Some overviews can contain r0 so always
      // call even if resLevel is 0.  Method returns true on success, false
      // on error.
      //---
      status = getOverviewTile(resLevel, result);
      
      if (!status) // Did not get an overview tile.
      {
         status = true;
         
         rspfIrect tile_rect = result->getImageRectangle();
         
         rspfIrect image_rect = getImageRectangle(resLevel);
         
         result->makeBlank();
         
         //---
         // See if any point of the requested tile is in the image.
         //---
         if ( tile_rect.intersects(image_rect) )
         {
            rspfIrect clip_rect = tile_rect.clipToRect(image_rect);
            
            // Load the tile buffer with data from the adrg.
            status = fillBuffer(tile_rect, clip_rect, result);

            if (status)
            {
               result->validate();
            }
         }
      }

      result->unref();  // Decrement ref count.
   }

   return status;
}

//*******************************************************************
// Private Method:
//*******************************************************************
bool rspfAdrgTileSource::fillBuffer(const rspfIrect& /* tile_rect */,
                                     const rspfIrect& clip_rect,
                                     rspfImageData* tile)
{
   //***
   // Shift the upper left corner of the "clip_rect" to the an even chunk
   // boundry.
   //***
   rspfIpt tileOrigin = clip_rect.ul();
   adjustToStartOfTile(tileOrigin);

   //***
   // Calculate the number of tiles needed in the line/sample directions.
   //***
   rspf_int32 size_in_x = clip_rect.lr().x - tileOrigin.x + 1;
   rspf_int32 size_in_y = clip_rect.lr().y - tileOrigin.y + 1;
   
   rspf_int32 tiles_in_x_dir = size_in_x / ADRG_TILE_WIDTH  +
      (size_in_x % ADRG_TILE_WIDTH  ? 1 : 0);
   rspf_int32 tiles_in_y_dir = size_in_y / ADRG_TILE_HEIGHT +
      (size_in_y % ADRG_TILE_HEIGHT ? 1 : 0);


   rspfIpt ulTilePt = tileOrigin;
   
   // Chunk loop in line direction.
   for (int32 y=0; y<tiles_in_y_dir; y++)
   {
      ulTilePt.x = tileOrigin.x;

      // Tile loop in sample direction.
      for (int32 x=0; x<tiles_in_x_dir; x++)
      {
         rspfIrect adrg_tile_rect(ulTilePt.x,
                                   ulTilePt.y,
                                   ulTilePt.x + ADRG_TILE_WIDTH- 1,
                                   ulTilePt.y + ADRG_TILE_HEIGHT - 1);

         if (adrg_tile_rect.intersects(clip_rect))
         {
            rspfIrect tile_clip_rect = clip_rect.clipToRect(adrg_tile_rect);
            
            //---
            // Some point in the chip intersect the tile so grab the
            // data.
            //---
            rspf_int32 row = (rspf_int32) ulTilePt.y / ADRG_TILE_HEIGHT; 
            rspf_int32 col = (rspf_int32) ulTilePt.x / ADRG_TILE_WIDTH;
            rspf_int32 tileOffset = m_AdrgHeader->tim(row, col);

            if(tileOffset != 0)
            {
               // Get the data.
               int seek_position = (tileOffset - 1) * 49152 + 2048;
               int band;

               // seek to start of chip
               m_FileStr.seekg(seek_position, ios::beg);
               for (band=0; band<3; band++)
               {
                  //***
                  // Read the chip from the ccf file into the chunk buffer.
                  // This will get all the bands.  Bands are interleaved by
                  // chip.
                  //***
                  if (!m_FileStr.read((char*)m_TileBuffer,
                                       ADRG_TILE_SIZE))
                  {
                     theErrorStatus = rspfErrorCodes::RSPF_ERROR;
                     return false;
                  }

                  tile->loadBand(m_TileBuffer,
                                 adrg_tile_rect,
                                 tile_clip_rect,
                                 band);

               } // End of band loop.
               
            } // End of if (tileOffset != 0)
            
         } // End of if (adrg_tile_rect.intersects(clip_rect))
         
         ulTilePt.x += ADRG_TILE_WIDTH;
         
      }  // End of tile loop in the sample direction.

      ulTilePt.y += ADRG_TILE_HEIGHT;
      
   }  // End of tile loop in the line direction.

   return true;
}

rspf_uint32 rspfAdrgTileSource::getNumberOfOutputBands()const
{
   return getNumberOfInputBands();
}

//*******************************************************************
// Public Method:
//*******************************************************************
rspfIrect
rspfAdrgTileSource::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   return rspfIrect(0,                         // upper left x
                     0,                         // upper left y
                     getNumberOfSamples(reduced_res_level) - 1,  // lower right x
                     getNumberOfLines(reduced_res_level)   - 1); // lower right y
}

void rspfAdrgTileSource::close()
{
   if(m_AdrgHeader)
   {
      delete m_AdrgHeader;
      m_AdrgHeader = 0;
   }
   if(m_FileStr.is_open())
   {
      m_FileStr.close();
   }
   m_Tile = 0;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfAdrgTileSource::open()
{
   static const char MODULE[] = "rspfAdrgTileSource::open";

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << "Entered..."<<std::endl;
   }
   if(isOpen())
   {
      close();
   }
   if(m_AdrgHeader)
   {
      delete m_AdrgHeader;
      m_AdrgHeader = 0;
   }
   // Instantiate support data class to parse header file.
   m_AdrgHeader = new rspfAdrgHeader(theImageFile);

   // Check for errors.
   if (m_AdrgHeader->errorStatus() == rspfErrorCodes::RSPF_ERROR)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " Error in rspfAdrg header detected." << std::endl;
      }

      close();
      return false;
   }

   m_FileStr.open(m_AdrgHeader->imageFile().c_str(),
                   ios::in | ios::binary);

   // Check the file pointer.
   if(!m_FileStr)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << MODULE << "\nCannot open:  "
              << m_AdrgHeader->imageFile().c_str() << std::endl;
      }
      close();
      
      return false;
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE
         << "File is opened -> "<< m_AdrgHeader->imageFile()<<std::endl;
   }

   // allow the base handler to check for other overrides
   completeOpen();
   // Allocate memory.
   m_Tile      = rspfImageDataFactory::instance()->create(this, this);
   m_Tile->initialize();
   m_TileBuffer  = new rspf_uint8[ADRG_TILE_SIZE];

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nminLon:   " << m_AdrgHeader->minLon()
         << "\nminLond:  " << m_AdrgHeader->minLongitude() 
         << "\nminLat:   " << m_AdrgHeader->minLat()
         << "\nminLatd:  " << m_AdrgHeader->minLatitude()
         << "\nmaxLon:   " << m_AdrgHeader->maxLon()
         << "\nmaxLond:  " << m_AdrgHeader->maxLongitude()
         << "\nmaxLat:   " << m_AdrgHeader->maxLat()
         << "\nmaxLatd:  " << m_AdrgHeader->maxLatitude()
         << std::endl;
   }
   
   return true;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfAdrgTileSource::saveState(rspfKeywordlist& kwl,
                                    const char* prefix) const
{
   // Currently nothing to do here.
   return rspfImageHandler::saveState(kwl, prefix);
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfAdrgTileSource::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   if (rspfImageHandler::loadState(kwl, prefix))
   {
      if (open())
      {
         return true;
      }
   }

   return false;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspfRefPtr<rspfImageGeometry> rspfAdrgTileSource::getImageGeometry()
{
   if ( !theGeometry )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();
      
      if ( !theGeometry )
      {
         // origin of latitude
         rspf_float64 originLatitude = (m_AdrgHeader->maxLatitude() +
                                         m_AdrgHeader->minLatitude()) / 2.0;
         
         // central meridian.
         rspf_float64 centralMeridian = (m_AdrgHeader->maxLongitude() +
                                          m_AdrgHeader->minLongitude()) / 2.0;
         
         //---
         // Compute the pixel size in latitude and longitude direction.  This will
         // be full image extents divided by full image lines and samples.
         //---
         
         // Samples in full image (used to compute degPerPixelX).
         rspf_float64 samples = m_AdrgHeader->samples();
         
         // Lines in full image (used to compute degPerPixelX).
         rspf_float64 lines = m_AdrgHeader->lines();
         
         // Degrees in latitude direction of the full image.
         rspf_float64 degrees_in_lat_dir = m_AdrgHeader->maxLatitude() -
            m_AdrgHeader->minLatitude();
         
         // Degrees in longitude direction of the full image.
         rspf_float64 degrees_in_lon_dir = m_AdrgHeader->maxLongitude() -
            m_AdrgHeader->minLongitude();
         
         rspf_float64 degPerPixelY = degrees_in_lat_dir / lines;
         rspf_float64 degPerPixelX = degrees_in_lon_dir / samples;
         
         //---
         // The tie is determined with the following assumptions that need to be
         // verified:
         // 1) Rows and columns start at 1.
         // 2) The min / max latitudes longitudes go to the edge of the pixel.
         // 3) Latitude decreases by degPerPixelY with each line.
         // 4) Longitude increases by degPerPixelX with each sample.
         //---
         rspf_float64 ul_lat = (m_AdrgHeader->maxLatitude() - 
                                 ( (m_AdrgHeader->startRow() - 1) *
                                   degPerPixelY ) - ( degPerPixelY * 0.5 ) );
         rspf_float64 ul_lon = (m_AdrgHeader->minLongitude() +
                                 ( (m_AdrgHeader->startCol() -1) *
                                   degPerPixelX ) +  ( degPerPixelX * 0.5 ) );
         
         // projection type
         rspfKeywordlist kwl;
         const char* prefix = 0;
         kwl.add(prefix,
                 rspfKeywordNames::TYPE_KW,
                 "rspfEquDistCylProjection",
                 true);
         
         // datum.
         kwl.add(prefix,
                 rspfKeywordNames::DATUM_KW,
                 "WGE",
                 true);
         
         // origin latitude
         kwl.add(prefix,
                 rspfKeywordNames::ORIGIN_LATITUDE_KW,
                 originLatitude,
                 true);

         // central meridin
         kwl.add(prefix,
                 rspfKeywordNames::CENTRAL_MERIDIAN_KW,
                 centralMeridian,
                 true);

         // Save the tie point.
         kwl.add(prefix,
                 rspfKeywordNames::TIE_POINT_XY_KW,
                 rspfDpt(ul_lon, ul_lat).toString().c_str(),
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::TIE_POINT_UNITS_KW,
                 rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES),
                 true);

         // Save the scale.
         kwl.add(prefix,
                 rspfKeywordNames::TIE_POINT_LAT_KW,
                 ul_lat,
                 true);
   
         kwl.add(prefix,
                 rspfKeywordNames::TIE_POINT_LON_KW,
                 ul_lon,
                 true);

         // Save the scale.
         kwl.add(prefix,
                 rspfKeywordNames::PIXEL_SCALE_XY_KW,
                 rspfDpt(degPerPixelX, degPerPixelY).toString().c_str(),
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::PIXEL_SCALE_UNITS_KW,
                 rspfUnitTypeLut::instance()->getEntryString(RSPF_DEGREES),
                 true);  

         // lines
         kwl.add(prefix,
                 rspfKeywordNames::NUMBER_LINES_KW,
                 getNumberOfLines());

         // samples
         kwl.add(prefix,
                 rspfKeywordNames::NUMBER_SAMPLES_KW,
                 getNumberOfSamples());

         // res sets
         kwl.add(prefix,
                 rspfKeywordNames::NUMBER_REDUCED_RES_SETS_KW,
                 getNumberOfDecimationLevels());

         // bands
         kwl.add(prefix,
                 rspfKeywordNames::NUMBER_INPUT_BANDS_KW,
                 getNumberOfInputBands());

         // bands
         kwl.add(prefix,
                 rspfKeywordNames::NUMBER_OUTPUT_BANDS_KW,
                 getNumberOfOutputBands());
   
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "\nminLon:             " << m_AdrgHeader->minLon()
               << "\nminLond:            " << m_AdrgHeader->minLongitude() 
               << "\nminLat:             " << m_AdrgHeader->minLat()
               << "\nminLatd:            " << m_AdrgHeader->minLatitude()
               << "\nmaxLon:             " << m_AdrgHeader->maxLon()
               << "\nmaxLond:            " << m_AdrgHeader->maxLongitude()
               << "\nmaxLat:             " << m_AdrgHeader->maxLat()
               << "\nmaxLatd:            " << m_AdrgHeader->maxLatitude()
               << "\nstartRow:           " << m_AdrgHeader->startRow()
               << "\nstartCol:           " << m_AdrgHeader->startCol()
               << "\nstopRow:            " << m_AdrgHeader->stopRow()
               << "\nstopCol:            " << m_AdrgHeader->stopCol()
               << "\nfull image lines:   " << lines
               << "\nfull image samples: " << samples
               << "\nkwl:\n"               << kwl
               << std::endl;
         }

         rspfProjection* new_proj = rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
         theGeometry = new rspfImageGeometry;
         theGeometry->setProjection(new_proj);  // assumes management of projection instance
         
      } // matches (after getExternalImageGeometry()):  if ( !theGeometry ) 
      
      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
      
   } // matches: if ( !theGeometry )

   return theGeometry;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspfScalarType rspfAdrgTileSource::getOutputScalarType() const
{
  return RSPF_UCHAR;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfAdrgTileSource::getTileWidth() const
{
   return ( m_Tile.valid() ? m_Tile->getWidth() : 0 );
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfAdrgTileSource::getTileHeight() const
{
   return ( m_Tile.valid() ? m_Tile->getHeight() : 0 );
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfAdrgTileSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   if ( (reduced_res_level == 0) && m_AdrgHeader )
   {
      return (m_AdrgHeader->stopRow() - m_AdrgHeader->startRow()) + 1;
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfLines(reduced_res_level);
   }

   return 0;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfAdrgTileSource::getNumberOfSamples(rspf_uint32 reduced_res_level) const
{
   if ( (reduced_res_level == 0) && m_AdrgHeader )
   {
      return (m_AdrgHeader->stopCol() - m_AdrgHeader->startCol()) + 1;
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfSamples(reduced_res_level);
   }

   return 0;
}

rspf_uint32 rspfAdrgTileSource::getImageTileWidth() const
{
   return ADRG_TILE_WIDTH;
}

rspf_uint32 rspfAdrgTileSource::getImageTileHeight() const
{
   return ADRG_TILE_HEIGHT;
}

rspfRefPtr<rspfProperty> rspfAdrgTileSource::getProperty(const rspfString& name)const
{
	if(name == "file_type")
	{
		return new rspfStringProperty(name, "ADRG");
	}
	
	return rspfImageHandler::getProperty(name);
}

void rspfAdrgTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
	rspfImageHandler::getPropertyNames(propertyNames);
	propertyNames.push_back("file_type");
}

rspfString rspfAdrgTileSource::getShortName()const
{
   return rspfString("adrg");
}

rspfString rspfAdrgTileSource::getLongName()const
{
   return rspfString("adrg reader");
}

rspf_uint32 rspfAdrgTileSource::getNumberOfInputBands() const
{
   return m_AdrgHeader->numberOfBands();
}

bool rspfAdrgTileSource::isOpen()const
{
   return (m_AdrgHeader!=0);
}

void rspfAdrgTileSource::adjustToStartOfTile(rspfIpt& pt) const
{
   pt.x &= 0xffffff80;
   pt.y &= 0xffffff80;
}
