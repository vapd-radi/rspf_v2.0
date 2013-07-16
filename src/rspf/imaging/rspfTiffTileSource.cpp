//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//          Frank Warmerdam (warmerdam@pobox.com)
//
// Description:
//
// Contains class definition for TiffTileSource.
//
//*******************************************************************
//  $Id: rspfTiffTileSource.cpp 21745 2012-09-16 15:21:53Z dburken $

#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/support_data/rspfGeoTiff.h>
#include <rspf/support_data/rspfTiffInfo.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIoStream.h> /* for rspfIOMemoryStream */
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <xtiffio.h>
#include <geo_normalize.h>
#include <cstdlib> /* for abs(int) */

RTTI_DEF1(rspfTiffTileSource, "rspfTiffTileSource", rspfImageHandler)

static rspfTrace traceDebug("rspfTiffTileSource:debug");

#define RSPF_TIFF_UNPACK_R4(value) ( (value)&0x000000FF)
#define RSPF_TIFF_UNPACK_G4(value) ( ((value)>>8)&0x000000FF)
#define RSPF_TIFF_UNPACK_B4(value) ( ((value)>>16)&0x000000FF)
#define RSPF_TIFF_UNPACK_A4(value) ( ((value)>>24)&0x000000FF)

//---
// RSPF_BUFFER_SCAN_LINE_READS:
// If set to 1 rspfTiffTileSource::loadFromScanLine method will buffer image
// width by tile height.  If set to 0 one line will be read at a time which
// conserves memory on wide images or tall tiles.
//
// Buffered read is faster but uses more memory. Non-buffered slower less
// memory.
//
// Only affects reading strip tiffs.
//---
#define RSPF_BUFFER_SCAN_LINE_READS 1

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfTiffTileSource::rspfTiffTileSource()
   :
      rspfImageHandler(),
      theTiffPtr(0),
      theTile(0),
      theBuffer(0),
      theBufferSize(0),
      theBufferRect(0, 0, 0, 0),
      theBufferRLevel(0),
      theCurrentTileWidth(0),
      theCurrentTileHeight(0),
      theSamplesPerPixel(0),
      theBitsPerSample(0),
      theSampleFormatUnit(0),
      theMaxSampleValue(rspf::nan()),
      theMinSampleValue(rspf::nan()),
      theNullSampleValue(rspf::nan()),
      theNumberOfDirectories(0),
      theCurrentDirectory(0),
      theR0isFullRes(false),
      theBytesPerPixel(0),
      theScalarType(RSPF_SCALAR_UNKNOWN),
      theApplyColorPaletteFlag(true),
      theImageWidth(0),
      theImageLength(0),
      theReadMethod(0),
      thePlanarConfig(0),
      thePhotometric(0),
      theRowsPerStrip(0),
      theImageTileWidth(0),
      theImageTileLength(0),
      theImageDirectoryList(0),
      theCurrentTiffRlevel(0),
      theCompressionType(0),
      theOutputBandList(0)
{}

rspfTiffTileSource::~rspfTiffTileSource()
{
   close();
}

rspfRefPtr<rspfImageData> rspfTiffTileSource::getTile(
   const rspfIrect& tile_rect, rspf_uint32 resLevel )
{
   if ( theTile.valid() == false )
   {
      allocateTile(); // First time through...
   }
   
   if ( theTile.valid() )
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

bool rspfTiffTileSource::getTile(rspfImageData* result,
                                  rspf_uint32 resLevel)
{
   static const char MODULE[] = "rspfTiffTileSource::getTile(rspfImageData*, resLevel)";

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

      if (!status) // Did not get an overview tile.
      {
         status = true;
         
         rspf_uint32 level = resLevel;

         //---
         // If we have r0 our reslevels are the same as the callers so
         // no adjustment necessary.
         //---
         if (theStartingResLevel && !theR0isFullRes) // Used as overview.
         {
            //---
            // If we have r0 our reslevels are the same as the callers so
            // no adjustment necessary.
            //---
            if (level >= theStartingResLevel)
            {
               //---
               // Adjust the level to be relative to the reader using this
               // as overview.
               //---
               level -= theStartingResLevel; 
            }
         }

         rspfIrect tile_rect = result->getImageRectangle();

         //---
         // This should be the zero base image rectangle for this res level.
         // Note passed the non adjusted resLevel by design.
         //---
         rspfIrect image_rect = getImageRectangle(resLevel);

         // See if any point of the requested tile is in the image.
         if ( tile_rect.intersects(image_rect) )
         {
            // Initialize the tile if needed as we're going to stuff it.
            if (result->getDataObjectStatus() == RSPF_NULL)
            {
               result->initialize();
            }

            bool reallocateBuffer = false;   
            if ( (tile_rect.width()  != theCurrentTileWidth) ||
                 (tile_rect.height() != theCurrentTileHeight) )
            {
               // Current tile size must be set prior to allocatBuffer call.
               theCurrentTileWidth = tile_rect.width();
               theCurrentTileHeight = tile_rect.height();
               
               reallocateBuffer = true;
            }
            
            if (getCurrentTiffRLevel() != theImageDirectoryList[level])
            {
               status = setTiffDirectory(theImageDirectoryList[level]);
               if (status)
               {
                  reallocateBuffer = true;
               }
            }

            if (status)
            {
               if (reallocateBuffer)
               {
                  // NOTE: Using this buffer will be a thread issue. (drb) 
                  status = allocateBuffer();
               }
            }

            if ( status )
            {  
               rspfIrect clip_rect = tile_rect.clipToRect( image_rect );
               
               if ( !tile_rect.completely_within( clip_rect ) )
               {
                  //---
                  // We're not going to fill the whole tile so start with a
                  // blank tile.
                  //---
                  result->makeBlank();
               }
               
               // Load the tile buffer with data from the tif.
               if ( loadTile( tile_rect, clip_rect, result ) )
               {
                  result->validate();
                  status = true;
               }
               else
               {
                  // Would like to change this to throw rspfException.(drb)
                  status = false;
                  if(traceDebug())
                  {
                     // Error in filling buffer.
                     rspfNotify(rspfNotifyLevel_WARN)
                        << MODULE
                        << " Error filling buffer. Return status = false..."
                        << std::endl;
                  }
               }

            } // matches: if (status)
               
         } // matches:  if ( zeroBasedTileRect.intersects(image_rect) )
         else 
         {
            // No part of requested tile within the image rectangle.
            status = true; // Not an error.
            result->makeBlank();
         }
         
      } // matches: if (!status)
      
      result->unref(); // Decrement ref count.
      
   } // matches: if( isOpen() && isSourceEnabled() && isValidRLevel(level) )

   return status;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfTiffTileSource::saveState(rspfKeywordlist& kwl,
                                    const char* prefix) const
{
   bool result = rspfImageHandler::saveState(kwl, prefix);
   
   if ( result )
   {
      if ( isBandSelector() && theOutputBandList.size() )
      {
         if ( isIdentityBandList( theOutputBandList ) == false )
         {
            // If we're not identity output the bands.
            rspfString bandsString;
            rspf::toSimpleStringList(bandsString, theOutputBandList);
            kwl.add(prefix,
                    rspfKeywordNames::BANDS_KW,
                    bandsString,
                    true);
         }
      }
      
      kwl.add(prefix,
              "apply_color_palette_flag",
              theApplyColorPaletteFlag,
              true);
   }
   
   return result;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfTiffTileSource::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   bool result = false;
   theOutputBandList.clear();

   if (rspfImageHandler::loadState(kwl, prefix))
   {
      std::string pfx = ( prefix ? prefix : "" );
      std::string key = "apply_color_palette_flag";
      rspfString value;
      
      value.string() = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         theApplyColorPaletteFlag = value.toBool();
      }
      else
      {
         theApplyColorPaletteFlag = true;
      }

      key = rspfKeywordNames::BANDS_KW;
      value.string() = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         rspf::toSimpleVector( theOutputBandList, value );
      }
      
      if ( open() ) 
      {
         // Set the band list after open so that the overview also gets set.
         result = true;
      }
   }
   return result;
}

bool rspfTiffTileSource::open(const rspfFilename& image_file)
{
   if (theTiffPtr)
   {
     close();
   }
   theImageFile = image_file;
   return open();
}

void rspfTiffTileSource::close()
{
   if(theTiffPtr)
   {
      XTIFFClose(theTiffPtr);
      theTiffPtr = 0;
   }
   theImageWidth.clear();
   theImageLength.clear();
   theReadMethod.clear();
   thePlanarConfig.clear();
   thePhotometric.clear();
   theRowsPerStrip.clear();
   theImageTileWidth.clear();
   theImageTileLength.clear();
   if (theBuffer)
   {
      delete [] theBuffer;
      theBuffer = 0;
      theBufferSize = 0;
   }
   rspfImageHandler::close();
}

bool rspfTiffTileSource::open()
{
   static const char MODULE[] = "rspfTiffTileSource::open";

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " Entered..."
         << "\nFile:  " << theImageFile.c_str() << std::endl;
   }

   if(isOpen())
   {
     close();
   }

   // Check for empty file name.
   if (theImageFile.empty())
   {
      return false;
   }
#if 0  
   // First we do a quick test to see if the file looks like a tiff file.
   FILE		*fp;
   unsigned char header[2];

   fp = fopen( theImageFile.c_str(), "rb" );
   if( fp == NULL )
       return false;

   fread( header, 2, 1, fp );
   fclose( fp );

   if( (header[0] != 'M' || header[1] != 'M')
       && (header[0] != 'I' || header[1] != 'I') )
       return false;
#endif
   
   theImageDirectoryList.clear();

   //---
   // Note:  The 'm' in "rm" is to tell TIFFOpen to not memory map the file.
   //---
   theTiffPtr = XTIFFOpen(theImageFile.c_str(), "rm");
   if (!theTiffPtr)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " ERROR:\n"
            << "libtiff could not open..." << std::endl;
      }
      return false;
   }

   theCompressionType = COMPRESSION_NONE;
   //***
   // Get the general tiff info.
   //***
   if(!TIFFGetField(theTiffPtr, TIFFTAG_COMPRESSION, &theCompressionType))
   {
      theCompressionType = COMPRESSION_NONE;
   }

   //***
   // See if the first directory is of FILETYPE_REDUCEDIMAGE; if not,
   // the first level is considered to be full resolution data.
   // Note:  If the tag is not present, consider the first level full
   // resolution.
   //***
   theImageDirectoryList.push_back(0);
   rspf_uint32 sub_file_type;
   if ( !TIFFGetField( theTiffPtr,
                       TIFFTAG_SUBFILETYPE ,
                       &sub_file_type ) )
   {
      sub_file_type = 0;
   }

   if (sub_file_type == FILETYPE_REDUCEDIMAGE)
   {
      theR0isFullRes = false;
   }
   else
   {
      theR0isFullRes = true;
   }
   
   if( !TIFFGetField(theTiffPtr, TIFFTAG_BITSPERSAMPLE, &(theBitsPerSample)) )
   {
      theBitsPerSample = 8;
   }

   if( !TIFFGetField(theTiffPtr,
                     TIFFTAG_SAMPLESPERPIXEL,
                     &theSamplesPerPixel ) )
   {
      theSamplesPerPixel = 1; 
   }

   if ( !TIFFGetField( theTiffPtr,
                       TIFFTAG_SAMPLEFORMAT,
                       &theSampleFormatUnit ) )
   {
      theSampleFormatUnit = 0;
   }

   if ( !TIFFGetField( theTiffPtr,
                       TIFFTAG_SMAXSAMPLEVALUE,
                       &theMaxSampleValue ) )
   {
      uint16 maxValue = 0;
      if(!TIFFGetField( theTiffPtr,
                        TIFFTAG_MAXSAMPLEVALUE,
                        &maxValue))
      {
         //---
         // This will be reset in validateMinMax method.  Can't set right now because we
         // don't know the scalar type yet.
         //---
         theMaxSampleValue = rspf::nan();
      }
      else
      {
         theMaxSampleValue = maxValue;
      }
   }

   if ( !TIFFGetField( theTiffPtr,
                       TIFFTAG_SMINSAMPLEVALUE,
                       &theMinSampleValue ) )
   {
      uint16 minValue = 0;
      if(!TIFFGetField( theTiffPtr,
                        TIFFTAG_MINSAMPLEVALUE,
                        &minValue))
      {
         //---
         // This will be reset in validateMinMax method.  Can't set right now because we
         // don't know the scalar type yet.
         //--- 
         theMinSampleValue = rspf::nan();
      }
      else
      {
         theMinSampleValue = minValue;
      }
   }

   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\ntheMinSampleValue:  " << theMinSampleValue
           << "\ntheMaxSampleValue:  " << theMaxSampleValue
           << endl;
   }

   // Get the number of directories.
   theNumberOfDirectories = TIFFNumberOfDirectories(theTiffPtr);

   // Current dir.
   theCurrentDirectory = TIFFCurrentDirectory(theTiffPtr);

   theImageWidth.resize(theNumberOfDirectories);
   theImageLength.resize(theNumberOfDirectories);
   theReadMethod.resize(theNumberOfDirectories);
   thePlanarConfig.resize(theNumberOfDirectories);
   thePhotometric.resize(theNumberOfDirectories);
   theRowsPerStrip.resize(theNumberOfDirectories);
   theImageTileWidth.resize(theNumberOfDirectories);
   theImageTileLength.resize(theNumberOfDirectories);

   for (rspf_uint32 dir=0; dir<theNumberOfDirectories; ++dir)
   {
      if (setTiffDirectory(dir) == false)
      {
         return false;
      }

      // Note: Need lines, samples before acceptAsRrdsLayer check.
      
      // lines:
      if ( !TIFFGetField( theTiffPtr,
                          TIFFTAG_IMAGELENGTH,
                          &theImageLength[dir] ) )
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " Cannot determine image length."
            << endl;
      }

      // samples:
      if ( !TIFFGetField( theTiffPtr,
                          TIFFTAG_IMAGEWIDTH,
                          &theImageWidth[dir] ) )
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " Cannot determine image width."
            << endl;
      }
      
      if ( !TIFFGetField( theTiffPtr,
                         TIFFTAG_SUBFILETYPE ,
                         &sub_file_type ) )
      {
         sub_file_type = 0;
      }

      if (sub_file_type == FILETYPE_REDUCEDIMAGE)
      {
         //---
         // Check for a thumbnail image.  If present don't use as it will mess with
         // overviews.  Currently only checking if it's a two directory image, i.e. a full
         // res and a thumbnail.
         // 
         // Note this shuts off the thumbnail which someone may want to see.  We could make
         // this a reader prop if it becomes an issue. drb - 09 Jan. 2012.
         //---
         if ( dir!=0 )
         {
            bool acceptAsRrdsLayer = true;
            if ( ( theNumberOfDirectories == 2 ) && ( dir == 1 ) )
            {
               acceptAsRrdsLayer = isPowerOfTwoDecimation(dir);
            }

            if ( acceptAsRrdsLayer )
            {
               theImageDirectoryList.push_back(dir);
            }
         }
      }
      
      if( !TIFFGetField( theTiffPtr, TIFFTAG_PLANARCONFIG,
                         &(thePlanarConfig[dir]) ) )
      {
         thePlanarConfig[dir] = PLANARCONFIG_CONTIG;
      }
      
      if( !TIFFGetField( theTiffPtr, TIFFTAG_PHOTOMETRIC,
                         &(thePhotometric[dir]) ) )
      {
         thePhotometric[dir] = PHOTOMETRIC_MINISBLACK;
      }
      theLut = 0;
      // Check for palette.
      uint16* red;
      uint16* green;
      uint16* blue;
      if(TIFFGetField(theTiffPtr, TIFFTAG_COLORMAP, &red, &green, &blue))
      {
         if(theApplyColorPaletteFlag)
         {
            thePhotometric[dir] = PHOTOMETRIC_PALETTE;
            theSamplesPerPixel = 3;
         }
         populateLut();
      }

      if( TIFFIsTiled(theTiffPtr))
      {
         theRowsPerStrip[dir] = 0;
         if ( !TIFFGetField( theTiffPtr,
                             TIFFTAG_TILEWIDTH,
                             &theImageTileWidth[dir] ) )
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfTiffTileSource::getTiffTileWidth ERROR:"
               << "\nCannot determine tile width." << endl;
            theImageTileWidth[dir] = 0;
         }
         if ( !TIFFGetField( theTiffPtr,
                             TIFFTAG_TILELENGTH,
                             &theImageTileLength[dir] ) )
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;   
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfTiffTileSource::getTiffTileLength ERROR:"
               << "\nCannot determine tile length." << endl;
             theImageTileLength[dir] = 0;
         }
      }
      else
      {
         // Strip tiff:
         theImageTileWidth[dir]  = 0;
         theImageTileLength[dir] = 0;
         if( !TIFFGetField( theTiffPtr, TIFFTAG_ROWSPERSTRIP,
                            &(theRowsPerStrip[dir]) ) )
         {
            theRowsPerStrip[dir] = 1;
         }
         
         theImageTileWidth[dir]  = theImageWidth[dir];
         theImageTileLength[dir] = theRowsPerStrip[dir];

         //---
         // Let's default the tile size to something efficient.
         //
         // NOTE:
         //
         // This is not used by the strip reader method.  Only by the getImageTileHeight
         // and getImageTileHeight methods.
         //---
         if ( theImageTileWidth[dir] > 256 )
         {
            theImageTileWidth[dir] = 256;
         }
         else if( theImageTileWidth[dir] < 64 )
         {
            theImageTileWidth[dir]  = 64;
         }
         if( theImageTileLength[dir] > 256 )
         {
            theImageTileLength[dir] = 256;
         }
         else if(theImageTileLength[dir] < 64)
         {
            theImageTileLength[dir] = 64;
         }
      }
      
   } // End of "for (rspf_uint32 dir=0; dir<theNumberOfDirectories; dir++)"
   
   // Reset the directory back to "0".
   if (setTiffDirectory(0) == false)
   {
      return false;
   }

   //---
   // Get the scalar type.
   //---
   theScalarType = RSPF_SCALAR_UNKNOWN;
   if (theBitsPerSample == 16)
   {
      theBytesPerPixel = 2;

      if (theSampleFormatUnit == SAMPLEFORMAT_INT)
      {
         // this is currently causing pixel problems.  I am going to comment this out until we figure out a better solution
         //
#if 0         
         if (theMinSampleValue == 0) //  && (theMaxSampleValue > 36535) )
         {
            //---
            // This is a hack for RadarSat data which is has tag 339 set to
            // signed sixteen bit data with a min sample value of 0 and
            // sometimes a max sample value greater than 36535.
            //---
            theScalarType = RSPF_UINT16;
         }
         else
         {
            theScalarType = RSPF_SINT16;
         }
#else
         theScalarType = RSPF_SINT16;
#endif

      }
      else if (theSampleFormatUnit == SAMPLEFORMAT_UINT)
      {
         // ESH 03/2009 -- Changed "== 2047" to "<= 2047"
         if (theMaxSampleValue <= 2047) // 2^11-1
         {
            // 11 bit EO, i.e. Ikonos, QuickBird, WorldView, GeoEye.
            theScalarType = RSPF_USHORT11; // IKONOS probably...
         }
         else
         {
            theScalarType = RSPF_UINT16; 
         }
      }
      else
      {
         if (theMaxSampleValue <= 2047) // 2^11-1
         {
            // 11 bit EO, i.e. Ikonos, QuickBird, WorldView, GeoEye.
            theScalarType = RSPF_USHORT11; // IKONOS probably...
         }
         else
            theScalarType = RSPF_UINT16; // Default to unsigned...
      }
   }
   else if ( (theBitsPerSample == 32) &&
             (theSampleFormatUnit == SAMPLEFORMAT_UINT) )
   {
      theBytesPerPixel = 4;
      theScalarType = RSPF_UINT32;
   }
   else if ( (theBitsPerSample == 32) &&
             (theSampleFormatUnit == SAMPLEFORMAT_INT) )
   {
      theBytesPerPixel = 4;
      theScalarType = RSPF_SINT32;
   }
   else if (theBitsPerSample == 32 &&
            theSampleFormatUnit == SAMPLEFORMAT_IEEEFP)
   {
      theBytesPerPixel = 4;
      theScalarType = RSPF_FLOAT32;
   }
   else if(theBitsPerSample == 64 &&
	   theSampleFormatUnit == SAMPLEFORMAT_IEEEFP)
   {
      theBytesPerPixel = 8;
      theScalarType = RSPF_FLOAT64;
   }
   else if (theBitsPerSample <= 8)
   {
      theBytesPerPixel = 1;
      theScalarType = RSPF_UINT8;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Error:\nCannot determine scalar type.\n"
         << "Trace dump follows:\n";
      print(rspfNotify(rspfNotifyLevel_WARN));
      
      return false;
   }
   
   // Sanity check for min, max and null values.
   validateMinMaxNull();
   
   setReadMethod();
   
   // Establish raster pixel alignment type:
   GTIF* gtif = GTIFNew(theTiffPtr);
   rspf_uint16 raster_type;
   if (GTIFKeyGet(gtif, GTRasterTypeGeoKey, &raster_type, 0, 1) && (raster_type == 1))
      thePixelType = RSPF_PIXEL_IS_AREA;
   else
      thePixelType = RSPF_PIXEL_IS_POINT;
   GTIFFree(gtif);

   // Let base-class finish the rest:
   completeOpen();

   if ( isBandSelector() && theOutputBandList.size() && ( isIdentityBandList( theOutputBandList ) == false ) )
   {
      // This does range checking and will pass to overview if open.
      setOutputBandList( theOutputBandList );
   }
   
   //---
   // Note: Logic changed to leave theTile and theBuffer uninitialized until first getTile(...)
   // request. (drb)
   //---

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " Debug:";
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
   
   // Finished...
   return true;
}
   
rspf_uint32 rspfTiffTileSource::getNumberOfLines( rspf_uint32 resLevel ) const
{
   rspf_uint32 result = 0;
   if ( theImageDirectoryList.size() && theTiffPtr && isValidRLevel(resLevel) )
   {
      //---
      // If we have r0 our reslevels are the same as the callers so
      // no adjustment necessary.
      //---
      if (!theStartingResLevel || theR0isFullRes) // not an overview or has r0.
      {
         //---
         // If we have r0 our reslevels are the same as the callers so
         // no adjustment necessary.
         //---
         if (resLevel < theImageDirectoryList.size())
         {
            result = theImageLength[theImageDirectoryList[resLevel]];
         }
         else if (theOverview.valid())
         {
            result = theOverview->getNumberOfLines(resLevel);
         }
      }
      else // this is an overview without r0
      {
         if (resLevel >= theStartingResLevel)
         {
            //---
            // Adjust the level to be relative to the reader using this as
            // overview.
            //---
            rspf_uint32 level = resLevel - theStartingResLevel;
            if (level < theImageDirectoryList.size())
            {
               result = theImageLength[theImageDirectoryList[level]];
            }
         }
      }
   }
   return result;
}

rspf_uint32 rspfTiffTileSource::getNumberOfSamples( rspf_uint32 resLevel ) const
{
   rspf_uint32 result = 0;
   if ( theImageDirectoryList.size() && theTiffPtr && isValidRLevel(resLevel) )
   {
      //---
      // If we have r0 our reslevels are the same as the callers so
      // no adjustment necessary.
      //---
      if (!theStartingResLevel||theR0isFullRes) // not an overview or has r0.
      {
         if (resLevel < theImageDirectoryList.size())
         {
            result = theImageWidth[theImageDirectoryList[resLevel]];
         }
         else if (theOverview.valid())
         {
            result = theOverview->getNumberOfSamples(resLevel);
         }
      }
      else // this is an overview.
      {
         if (resLevel >= theStartingResLevel)
         {
            //---
            // Adjust the level to be relative to the reader using this as
            // overview.
            //---
            rspf_uint32 level = resLevel - theStartingResLevel;
            if (level < theImageDirectoryList.size())
            {
               result = theImageWidth[theImageDirectoryList[level]];
            }
         }
      }
   }
   return result;
}

rspf_uint32 rspfTiffTileSource::getNumberOfDecimationLevels() const
{
   rspf_uint32 result = theImageDirectoryList.size();
   
   // If starting res level is not 0 then this is an overview.
   if (theStartingResLevel&&theR0isFullRes)
   {
      // Don't count r0.
      --result;
   }
   else if (theOverview.valid())
   {
      result += theOverview->getNumberOfDecimationLevels();
   }

   return result;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspfScalarType rspfTiffTileSource::getOutputScalarType() const
{
   return theScalarType;
}

bool rspfTiffTileSource::loadTile(const rspfIrect& tile_rect,
                                   const rspfIrect& clip_rect,
                                   rspfImageData* result)
{
   static const char MODULE[] = "rspfTiffTileSource::loadTile";

   bool status = true;

   if ( !theBuffer )
   {
      status = allocateBuffer();
   }

   if ( status )
   {
      switch(theReadMethod[theCurrentDirectory])
      {
         case READ_TILE:
            status = loadFromTile(clip_rect, result);
            break;
            
         case READ_SCAN_LINE:
            status = loadFromScanLine(clip_rect, result);
            break;
            
         case READ_RGBA_U8_TILE:
            status = loadFromRgbaU8Tile(tile_rect, clip_rect, result);
            break;
            
         case READ_RGBA_U8_STRIP:
            status = loadFromRgbaU8Strip(tile_rect, clip_rect, result);
            break;
            
         case READ_RGBA_U8A_STRIP:
            status = loadFromRgbaU8aStrip(tile_rect, clip_rect, result);
            break;
            
         default:
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " Unsupported tiff type!" << endl;
            status = false;
            break;
      }
   }
   
   return status;
}

bool rspfTiffTileSource::loadFromScanLine(const rspfIrect& clip_rect,
                                           rspfImageData* result)
{
#if RSPF_BUFFER_SCAN_LINE_READS
   rspfInterleaveType type =
      (thePlanarConfig[theCurrentDirectory] == PLANARCONFIG_CONTIG) ?
       RSPF_BIP : RSPF_BIL;
   
   if ( theBufferRLevel != getCurrentTiffRLevel() ||
        !clip_rect.completely_within(theBufferRect) )
   {
      //***
      // Must reload the buffer.  Grab enough lines to fill the depth of the
      // clip rectangle.
      //***
      theBufferRLevel = getCurrentTiffRLevel();
      theBufferRect   = getImageRectangle(theBufferRLevel);
      theBufferRect.set_uly(clip_rect.ul().y);
      theBufferRect.set_lry(clip_rect.lr().y);
      rspf_uint32 startLine = clip_rect.ul().y;
      rspf_uint32 stopLine  = clip_rect.lr().y;
      rspf_uint8* buf = theBuffer;

      if (thePlanarConfig[theCurrentDirectory] == PLANARCONFIG_CONTIG)
      {
         rspf_uint32 lineSizeInBytes = getNumberOfSamples(theBufferRLevel) *
                                  theBytesPerPixel * theSamplesPerPixel;

         for (rspf_uint32 line = startLine; line <= stopLine; ++line)
         {
            TIFFReadScanline(theTiffPtr, (void*)buf, line, 0);
            buf += lineSizeInBytes;
         }
      }
      else
      {
         rspf_uint32 lineSizeInBytes = getNumberOfSamples(theBufferRLevel) *
                                  theBytesPerPixel;

         for (rspf_uint32 line = startLine; line <= stopLine; ++line)
         {
            for (rspf_uint32 band = 0; band < theSamplesPerPixel; ++band)
            {
               TIFFReadScanline(theTiffPtr, (void*)buf, line, band);
               buf += lineSizeInBytes;
            }
         }
      }
   }

   //---
   // Since theTile's internal rectangle is relative to any sub image offset
   // we must adjust both the zero based "theBufferRect" and the zero base
   // "clip_rect" before passing to
   // theTile->loadTile method.
   //---
   result->loadTile(theBuffer, theBufferRect, clip_rect, type);
   return true;

#else
   rspfInterleaveType type =
      (thePlanarConfig[theCurrentDirectory] == PLANARCONFIG_CONTIG) ? RSPF_BIP : RSPF_BIL;

   rspf_int32 startLine = clip_rect.ul().y;
   rspf_int32 stopLine  = clip_rect.lr().y;
   rspf_int32 stopSamp  = static_cast<rspf_int32>(getNumberOfSamples(theBufferRLevel)-1);
   
   if (thePlanarConfig[theCurrentDirectory] == PLANARCONFIG_CONTIG)
   {
      for (rspf_int32 line = startLine; line <= stopLine; ++line)
      {
         TIFFReadScanline(theTiffPtr, (void*)theBuffer, line, 0);
         result->copyLine((void*)theBuffer, line, 0, stopSamp, type);
      }
   }
   else
   {
      rspf_uint32 lineSizeInBytes = getNumberOfSamples(theBufferRLevel) * theBytesPerPixel;
      for (rspf_int32 line = startLine; line <= stopLine; ++line)
      {
         rspf_uint8* buf = theBuffer;
         for (rspf_uint32 band = 0; band < theSamplesPerPixel; ++band)
         {
            TIFFReadScanline(theTiffPtr, (void*)buf, line, band);
            buf += lineSizeInBytes;
         }
         result->copyLine((void*)theBuffer, line, 0, stopSamp, type);
      }
   }
   return true;
#endif /* #if RSPF_BUFFER_SCAN_LINE_READS #else - Non buffered scan line reads. */
}

bool rspfTiffTileSource::loadFromTile(const rspfIrect& clip_rect,
                                       rspfImageData* result)
{
   static const char MODULE[] = "rspfTiffTileSource::loadFromTile";
   
   rspf_int32 tileSizeRead = 0;
   
   //---
   // Shift the upper left corner of the "clip_rect" to the an even tile
   // boundary.  Note this will shift in the upper left direction.
   //---
   rspfIpt tileOrigin = clip_rect.ul();
   adjustToStartOfTile(tileOrigin);
   rspfIpt ulTilePt       = tileOrigin;
//   rspfIpt subImageOffset = getSubImageOffset(getCurrentTiffRLevel()+theStartingResLevel);

   //---
   // Calculate the number of tiles needed in the line/sample directions.
   //---
   rspf_uint32 tiles_in_v_dir = (clip_rect.lr().x-tileOrigin.x+1) /
      theImageTileWidth[theCurrentDirectory];
   rspf_uint32 tiles_in_u_dir = (clip_rect.lr().y-tileOrigin.y+1) /
      theImageTileLength[theCurrentDirectory];

   if ( (clip_rect.lr().x-tileOrigin.x+1) %
        theImageTileWidth[theCurrentDirectory]  ) ++tiles_in_v_dir;
   if ( (clip_rect.lr().y-tileOrigin.y+1) %
        theImageTileLength[theCurrentDirectory] ) ++tiles_in_u_dir;


   // Tile loop in line direction.
   for (rspf_uint32 u=0; u<tiles_in_u_dir; ++u)
   {
      ulTilePt.x = tileOrigin.x;

      // Tile loop in sample direction.
      for (rspf_uint32 v=0; v<tiles_in_v_dir; ++v)
      {
         rspfIrect tiff_tile_rect(ulTilePt.x,
                                   ulTilePt.y,
                                   ulTilePt.x +
                                   theImageTileWidth[theCurrentDirectory]  - 1,
                                   ulTilePt.y +
                                   theImageTileLength[theCurrentDirectory] - 1);
         
         if (tiff_tile_rect.intersects(clip_rect))
         {
            rspfIrect tiff_tile_clip_rect
               = tiff_tile_rect.clipToRect(clip_rect);

            //---
            // Since theTile's internal rectangle is relative to any sub
            // image offset we must adjust both the zero based
            // "theBufferRect" and the zero based "clip_rect" before
            // passing to theTile->loadTile method.
            //---
            rspfIrect bufRectWithOffset = tiff_tile_rect;// + subImageOffset;
            rspfIrect clipRectWithOffset = tiff_tile_clip_rect;// + subImageOffset;
            
            if  (thePlanarConfig[theCurrentDirectory] == PLANARCONFIG_CONTIG)
            {
               tileSizeRead = TIFFReadTile(theTiffPtr,
                                           theBuffer,
                                           ulTilePt.x,
                                           ulTilePt.y,
                                           0,
                                           0);
               if (tileSizeRead > 0)
               {
                  result->loadTile(theBuffer,
                                  bufRectWithOffset,
                                  clipRectWithOffset,
                                  RSPF_BIP);
               }
               else if(tileSizeRead < 0)
               {
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_WARN)
                        << MODULE << " Read Error!"
                        << "\nReturning error...  " << endl;
                  }
                  theErrorStatus = rspfErrorCodes::RSPF_ERROR;
                  return false;
               }
            }
            else
            {
               if ( theOutputBandList.empty() )
               {
                  // This will set to identity.
                  rspfImageSource::getOutputBandList( theOutputBandList );
               }
               
               // band separate tiles...
               std::vector<rspf_uint32>::const_iterator bandIter = theOutputBandList.begin();
               rspf_uint32 destinationBand = 0;
               while ( bandIter != theOutputBandList.end() )
               {
                  tileSizeRead = TIFFReadTile( theTiffPtr,
                                               theBuffer,
                                               ulTilePt.x,
                                               ulTilePt.y,
                                               0,
                                               (*bandIter) );
                  if(tileSizeRead > 0)
                  {
                     result->loadBand( theBuffer,
                                       bufRectWithOffset,
                                       clipRectWithOffset,
                                       destinationBand );
                  }
                  else if (tileSizeRead < 0)
                  {
                     if(traceDebug())
                     {
                        rspfNotify(rspfNotifyLevel_WARN)
                           << MODULE << " Read Error!"
                           << "\nReturning error...  " << endl;
                     }
                     theErrorStatus = rspfErrorCodes::RSPF_ERROR;
                     return false;
                  }
                  ++bandIter; // Next band...
                  ++destinationBand;
               }
            }

         } // End of if (tiff_tile_rect.intersects(clip_rect))
         
         ulTilePt.x += theImageTileWidth[theCurrentDirectory];
         
      }  // End of tile loop in the sample direction.

      ulTilePt.y += theImageTileLength[theCurrentDirectory];
      
   }  // End of tile loop in the line direction.

   return true;
}

bool rspfTiffTileSource::loadFromRgbaU8Tile(const rspfIrect& tile_rect,
                                             const rspfIrect& clip_rect,
                                             rspfImageData* result)
{
   static const char MODULE[] = "rspfTiffTileSource::loadFromRgbaTile";

   if (theSamplesPerPixel != 3 || theBytesPerPixel!=1)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Error:"
         << "\nInvalid number of bands or bytes per pixel!" << endl;
   }
   
   //***
   // Shift the upper left corner of the "clip_rect" to the an even tile
   // boundary.  Note this will shift in the upper left direction.
   //***
   rspfIpt tileOrigin = clip_rect.ul();
   adjustToStartOfTile(tileOrigin);

   //---
   // Calculate the number of tiles needed in the line/sample directions
   // to fill the tile.
   //---
   rspf_uint32 tiles_in_v_dir = (clip_rect.lr().x-tileOrigin.x+1) /
      theImageTileWidth[theCurrentDirectory];
   rspf_uint32 tiles_in_u_dir = (clip_rect.lr().y-tileOrigin.y+1) /
      theImageTileLength[theCurrentDirectory];

   if ( (clip_rect.lr().x-tileOrigin.x+1) %
        theImageTileWidth[theCurrentDirectory]  ) ++tiles_in_v_dir;
   if ( (clip_rect.lr().y-tileOrigin.y+1) %
        theImageTileLength[theCurrentDirectory] ) ++tiles_in_u_dir;
   
   rspfIpt ulTilePt = tileOrigin;

#if 0
   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\ntile_rect:  " << tile_rect
           << "\nclip_rect:  " << clip_rect
           << "\ntiles_in_v_dir:  " << tiles_in_v_dir
           << "\ntiles_in_u_dir:  " << tiles_in_u_dir
           << endl;
   }
#endif
   
   
   // Tile loop in line direction.
   for (rspf_uint32 u=0; u<tiles_in_u_dir; u++)
   {
      ulTilePt.x = tileOrigin.x;

      // Tile loop in sample direction.
      for (rspf_uint32 v=0; v<tiles_in_v_dir; v++)
      {
         rspfIrect tiff_tile_rect
            = rspfIrect(ulTilePt.x,
                         ulTilePt.y,
                         ulTilePt.x +
                         theImageTileWidth[theCurrentDirectory]  - 1,
                         ulTilePt.y +
                         theImageTileLength[theCurrentDirectory] - 1);

         if ( getCurrentTiffRLevel() != theBufferRLevel ||
              tiff_tile_rect != theBufferRect)
         {
            // Need to grab a new tile.
            // Read a tile into the buffer.
            if ( !TIFFReadRGBATile(theTiffPtr,
                                   ulTilePt.x,
                                   ulTilePt.y,
                                   (uint32*)theBuffer) ) // use tiff typedef
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << MODULE << " Read Error!"
                  << "\nReturning error..." << endl;
               theErrorStatus = rspfErrorCodes::RSPF_ERROR;
               return false;
            }

            // Capture the rectangle.
            theBufferRect   = tiff_tile_rect;
            theBufferRLevel = getCurrentTiffRLevel();
         }

         rspfIrect tile_clip_rect = clip_rect.clipToRect(theBufferRect);
         
         //***
         // Get the offset to the first valid pixel.
         // 
         // Note: The data in the tile buffer is organized bottom up.  So the
         //       coordinate must be negated in the line direction since
         //       the met assumes an origin of upper left.
         //***
         rspf_uint32 in_buf_offset =
              (tiff_tile_rect.lr().y-tile_clip_rect.ul().y)*
              theImageTileWidth[theCurrentDirectory]*4 +
              ((tile_clip_rect.ul().x - ulTilePt.x)*4);
         
         rspf_uint32 out_buf_offset =
            (tile_clip_rect.ul().y - tile_rect.ul().y) *
            ((rspf_int32)result->getWidth()) +
            tile_clip_rect.ul().x - tile_rect.ul().x;
         
         //
         // Get a pointer positioned at the first valid pixel in buffers.
         //
         rspf_uint32* s = (rspf_uint32*)(theBuffer + in_buf_offset);  // s for source...
//         rspf_uint8* s = theBuffer + in_buf_offset;  // s for source...
         rspf_uint8* r = static_cast<rspf_uint8*>(result->getBuf(0))+
            out_buf_offset;
         rspf_uint8* g = static_cast<rspf_uint8*>(result->getBuf(1))+
            out_buf_offset;
         rspf_uint8* b = static_cast<rspf_uint8*>(result->getBuf(2))+
            out_buf_offset;
         
         rspf_uint32 lines2copy = tile_clip_rect.lr().y-tile_clip_rect.ul().y+1;
         rspf_uint32 samps2copy = tile_clip_rect.lr().x-tile_clip_rect.ul().x+1;
         
         // Line loop through valid portion of the tiff tile.         
         for (rspf_uint32 line = 0; line < lines2copy; line++)
         {
            // Sample loop through the tiff tile.
            rspf_uint32 i=0;
            rspf_uint32 j=0;
            
            // note the bands from the TIFF READ are stored in a, b, g, r ordering.
            // we must reverse the bands and skip the first byte.
            for (rspf_uint32 sample = 0; sample < samps2copy; sample++)
            {
               r[i] = (rspf_uint8)RSPF_TIFF_UNPACK_R4(s[j]);
               g[i] = (rspf_uint8)RSPF_TIFF_UNPACK_G4(s[j]);
               b[i] = (rspf_uint8)RSPF_TIFF_UNPACK_B4(s[j]);
               i++;
               ++j;
            }
            
            // Increment the pointers by one line.
            const rspf_uint32 OUTPUT_TILE_WIDTH = result->getWidth();
            r += OUTPUT_TILE_WIDTH;
            g += OUTPUT_TILE_WIDTH;
            b += OUTPUT_TILE_WIDTH;
            s -= theImageTileWidth[theCurrentDirectory];
         }
      
         ulTilePt.x += theImageTileWidth[theCurrentDirectory];
      
      }  // End of tile loop in the sample direction.
      
      ulTilePt.y += theImageTileLength[theCurrentDirectory];
      
   }  // End of tile loop in the line direction.
   
   return true;
}

bool rspfTiffTileSource::loadFromRgbaU8Strip(const rspfIrect& tile_rect,
                                              const rspfIrect& clip_rect,
                                              rspfImageData* result)
{
   static const char MODULE[] = "rspfTiffTileSource::loadFromRgbaU8Strip";

   if (theSamplesPerPixel > 4 || theBytesPerPixel != 1)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Error:"
         << "\nInvalid number of bands or bytes per pixel!" << endl;
   }
   
   //***
   // Calculate the number of strips to read.
   //***
   const rspf_uint32 OUTPUT_TILE_WIDTH = result->getWidth();

   rspf_uint32 starting_strip = clip_rect.ul().y /
      theRowsPerStrip[theCurrentDirectory];
   rspf_uint32 ending_strip   = clip_rect.lr().y /
      theRowsPerStrip[theCurrentDirectory];
   rspf_uint32 strip_width    = theImageWidth[theCurrentDirectory]*4;   
   rspf_uint32 output_tile_offset = (clip_rect.ul().y - tile_rect.ul().y) *
                                OUTPUT_TILE_WIDTH + clip_rect.ul().x -
                                tile_rect.ul().x;

#if 0 /* Please keep for debug: */
   CLOG << "DEBUG:"
        << "\nsamples:         " << theSamplesPerPixel
        << "\ntile_rect:       " << tile_rect
        << "\nclip_rect:       " << clip_rect
        << "\nstarting_strip:  " << starting_strip
        << "\nending_strip:    " << ending_strip
        << "\nstrip_width:     " << strip_width
        << "\noutput_tile_offset:  " << output_tile_offset
        << endl;
#endif
   
   //***
   // Get the pointers positioned at the first valid pixel in the buffers.
   // s = source
   // d = destination
   //***
   rspf_uint32 band;

   rspf_uint8** d = new rspf_uint8*[theSamplesPerPixel];
   for (band = 0; band < theSamplesPerPixel; band++)
   {
      d[band] = static_cast<rspf_uint8*>(result->getBuf(band))+output_tile_offset;
   }

   // Loop through strips...
   for (rspf_uint32 strip=starting_strip; strip<=ending_strip; strip++)
   {
      if ( (theBufferRLevel != theCurrentDirectory) ||
           ( clip_rect.completely_within( theBufferRect ) == false ) )
      {
         if (TIFFReadRGBAStrip(theTiffPtr,
                               (strip*theRowsPerStrip[theCurrentDirectory]),
                               (uint32*)theBuffer) == 0) // use tiff typedef
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " Error reading strip!" <<endl;
            delete [] d;
            return false;
         }

         // Capture rect and rlevel of buffer:
         theBufferRLevel = theCurrentDirectory;
         theBufferRect = rspfIrect(
            0,
            starting_strip,
            theImageWidth[theCurrentDirectory]-1,
            (ending_strip-starting_strip) ? (ending_strip-starting_strip) *
            theRowsPerStrip[theCurrentDirectory] - 1 :
            theRowsPerStrip[theCurrentDirectory] - 1 );
      }
      
      //***
      // If the last strip is a partial strip then the first line of the
      // strip will be the last line of the image.
      //***
      rspf_uint32 last_line = theImageLength[theCurrentDirectory] - 1;

      rspf_uint32 strip_offset
         = ((strip * theRowsPerStrip[theCurrentDirectory]) +
            theRowsPerStrip[theCurrentDirectory] - 1) <
         last_line ?  0 :
         ((strip * theRowsPerStrip[theCurrentDirectory]) +
          theRowsPerStrip[theCurrentDirectory] - 1) - last_line;

      rspf_uint32 total_rows = theRowsPerStrip[theCurrentDirectory] -
         strip_offset;
      
      for (rspf_uint32 row=0; row<total_rows; row++)
      {
         // Write the line if it's in the clip rectangle.
         rspf_int32 current_line = strip * theRowsPerStrip[theCurrentDirectory]
            + row;
         if  (current_line >= clip_rect.ul().y &&
              current_line <= clip_rect.lr().y)
         {
            //
            // Position the stip pointer to the correct spot.
            // 
            // Note:
            // A strip is organized from bottom to top and the raster buffer is
            // orgainized from top to bottom so the lineBuf must be offset
            // accordingly.
            //
            rspf_uint32* s = (rspf_uint32*)(theBuffer+ ((theRowsPerStrip[theCurrentDirectory] - row -
                                                           strip_offset - 1) * strip_width + clip_rect.ul().x * 4));
            
            // Copy the data to the output buffer.
            rspf_uint32 i=0;
                                                 
            for (int32 sample=clip_rect.ul().x;
                 sample<=clip_rect.lr().x;
                 sample++)
            {
               d[0][i] = RSPF_TIFF_UNPACK_R4(*s);
               d[1][i] = RSPF_TIFF_UNPACK_G4(*s);
               d[2][i] = RSPF_TIFF_UNPACK_B4(*s);
               ++i;
               ++s;
            }

            for (band = 0; band < theSamplesPerPixel; band++)
            {
               d[band] += OUTPUT_TILE_WIDTH;
            }
         }
      }  // End of loop through rows in a strip.

   }  // End of loop through strips.

   delete [] d;
               
   return true;
}

//*******************************************************************
// Private Method:
//*******************************************************************
bool rspfTiffTileSource::loadFromRgbaU8aStrip(const rspfIrect& tile_rect,
                                               const rspfIrect& clip_rect,
                                               rspfImageData* result)
{
   static const char MODULE[] = "rspfTiffTileSource::loadFromRgbaU8aStrip";

   //***
   // Specialized for one bit data to handle null values.
   //***
   const rspf_uint32 OUTPUT_TILE_WIDTH = result->getWidth();
   const rspf_uint8 NULL_PIX = static_cast<rspf_uint8>(result->getNullPix(0));
   const rspf_uint8 MIN_PIX  = static_cast<rspf_uint8>(result->getMinPix(0));

   if (theSamplesPerPixel > 4 || theBytesPerPixel!= 1)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Error:"
         << "\nInvalid number of bands or bytes per pixel!" << endl;
   }
   
   //***
   // Calculate the number of strips to read.
   //***
   rspf_uint32 starting_strip = clip_rect.ul().y /
        theRowsPerStrip[theCurrentDirectory];
   rspf_uint32 ending_strip   = clip_rect.lr().y /
      theRowsPerStrip[theCurrentDirectory];
   rspf_uint32 output_tile_offset = (clip_rect.ul().y - tile_rect.ul().y) *
                                OUTPUT_TILE_WIDTH + clip_rect.ul().x -
                                tile_rect.ul().x;

#if 0
   if (traceDebug())
   {
      CLOG << "DEBUG:"
           << "\ntile_rect:       " << tile_rect
           << "\nclip_rect:       " << clip_rect
           << "\nstarting_strip:  " << starting_strip
           << "\nending_strip:    " << ending_strip
           << "\nstrip_width:     " << strip_width
           << "\noutput_tile_offset:     " << output_tile_offset
           << "\nsamples:         " << theSamplesPerPixel
           << endl;
   }
#endif
   
   //***
   // Get the pointers positioned at the first valid pixel in the buffers.
   // s = source
   // d = destination
   //***
   rspf_uint32 band;

   rspf_uint8** d = new rspf_uint8*[theSamplesPerPixel];
   for (band = 0; band < theSamplesPerPixel; band++)
   {
      d[band] = static_cast<rspf_uint8*>(result->getBuf(band))+output_tile_offset;
   }

   // Loop through strips...
   for (rspf_uint32 strip=starting_strip; strip<=ending_strip; strip++)
   {
      if (TIFFReadRGBAStrip(theTiffPtr,
                            (strip*theRowsPerStrip[theCurrentDirectory]),
                            (uint32*)theBuffer) == 0) // use tiff typedef
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " Error reading strip!" <<endl;
         delete [] d;
         return false;
      }

      //***
      // If the last strip is a partial strip then the first line of the
      // strip will be the last line of the image.
      //***
      rspf_uint32 last_line = theImageLength[theCurrentDirectory] - 1;

      rspf_uint32 strip_offset
         = ((strip * theRowsPerStrip[theCurrentDirectory]) +
            theRowsPerStrip[theCurrentDirectory] - 1) < last_line ?  0 :
         ((strip * theRowsPerStrip[theCurrentDirectory]) +
          theRowsPerStrip[theCurrentDirectory] - 1) - last_line;

      rspf_uint32 total_rows = theRowsPerStrip[theCurrentDirectory] -
         strip_offset;
      
      for (rspf_uint32 row=0; row<total_rows; row++)
      {
         // Write the line if it's in the clip rectangle.
         rspf_int32 current_line = strip * theRowsPerStrip[theCurrentDirectory]
            + row;
         if  (current_line >= clip_rect.ul().y &&
              current_line <= clip_rect.lr().y)
         {
            //***
            // Position the stip pointer to the correct spot.
            // 
            // Note:
            // A strip is organized from bottom to top and the raster buffer is
            // orgainized from top to bottom so the lineBuf must be offset
            // accordingly.
            //***
            rspf_uint8* s = theBuffer;
            s += (theRowsPerStrip[theCurrentDirectory] - row -
                  strip_offset - 1) *
                 theImageWidth[theCurrentDirectory] * 4 +
                 clip_rect.ul().x * 4;
            
            // Copy the data to the output buffer.
            rspf_uint32 i=0;
            rspf_uint32 j=0;
            for (int32 sample=clip_rect.ul().x;
                 sample<=clip_rect.lr().x;
                 sample++)
            {
               for (band = 0; band < theSamplesPerPixel; band++)
               {
                  rspf_uint8 pix = s[j + band];
                  d[band][i] = pix != NULL_PIX ? pix : MIN_PIX;
               }
               ++i;
               j += 4;
            }

            for (band = 0; band < theSamplesPerPixel; band++)
            {
               d[band] += OUTPUT_TILE_WIDTH;
            }
         }
      }  // End of loop through rows in a strip.

   }  // End of loop through strips.

   delete [] d;
               
   return true;
}

void rspfTiffTileSource::adjustToStartOfTile(rspfIpt& pt) const
{
   //***
   // Notes:
   // - Assumes an origin of (0,0)
   // - Shifts in to the upper left direction.
   //***
   rspf_int32 tw =
      static_cast<rspf_int32>(theImageTileWidth[theCurrentDirectory]);
   rspf_int32 th =
      static_cast<rspf_int32>(theImageTileLength[theCurrentDirectory]);
   
   if (pt.x > 0)
   {
      pt.x = (pt.x/tw) * tw;
   }
   else if (pt.x < 0)
   {
      pt.x = std::abs(pt.x) < tw ? -tw : (pt.x/tw)*tw;
   }

   if (pt.y > 0)
   {
      pt.y = (pt.y/th) * th;
   }
   else if (pt.y < 0)
   {
      pt.y = std::abs(pt.y) < th ? -th : (pt.y/th)*th;
   }
}

bool rspfTiffTileSource::isValidRLevel(rspf_uint32 resLevel) const
{
   bool result = false;
   
   const rspf_uint32 LEVELS = getNumberOfDecimationLevels();

   //---
   // If we have r0 our reslevels are the same as the callers so
   // no adjustment necessary.
   //---
   if ( !theStartingResLevel || theR0isFullRes) // Not an overview or has r0.
   {
      result = (resLevel < LEVELS);
   }
   else if (resLevel >= theStartingResLevel) // Used as overview.
   {
      result = ( (resLevel - theStartingResLevel) < LEVELS);
   }
   
   return result;
}

rspf_uint32 rspfTiffTileSource::getCurrentTiffRLevel() const
{
   return theCurrentTiffRlevel;
//   return theCurrentDirectory;
}

rspfString rspfTiffTileSource::getReadMethod(rspf_uint32 directory) const
{
   switch (theReadMethod[directory])
   {
      case READ_RGBA_U8_TILE:
         return rspfString("READ_RGBA_U8_TILE");
      case READ_RGBA_U8_STRIP:
         return rspfString("READ_RGBA_U8_STRIP");
      case READ_RGBA_U8A_STRIP:
         return rspfString("READ_RGBA_U8A_STRIP");
      case READ_SCAN_LINE:
         return rspfString("READ_SCAN_LINE");
      case READ_TILE:
         return rspfString("READ_TILE");
      case UNKNOWN:
      default:
         return rspfString("UNKNOWN");
   }
}      

rspf_uint32 rspfTiffTileSource::getNumberOfDirectories() const
{
   return theNumberOfDirectories;
}

rspf_uint32 rspfTiffTileSource::getImageTileWidth() const
{
   rspf_uint32 result = 0;
   if(isOpen())
   {
      if(theCurrentDirectory < theImageTileWidth.size())
      {
         result = theImageTileWidth[theCurrentDirectory];
      }
   }   
   return result;
}

rspf_uint32 rspfTiffTileSource::getImageTileHeight() const
{
   rspf_uint32 result = 0;
   if(isOpen())
   {
      if(theCurrentDirectory < theImageTileLength.size())
      {
         result = theImageTileLength[theCurrentDirectory];
      }
   }
   return result;
}

rspf_uint32 rspfTiffTileSource::getTileWidth() const
{
   rspf_uint32 result = getImageTileWidth();
   if (!result)
   {
      rspfIpt tileSize;
      rspf::defaultTileSize(tileSize);
      result = tileSize.x;
   }
   return result;
}

rspf_uint32 rspfTiffTileSource::getTileHeight() const
{
   rspf_uint32 result = getImageTileHeight();
   if (!result)
   {
      rspfIpt tileSize;
      rspf::defaultTileSize(tileSize);
      result = tileSize.y;
   }
   return result;
}

void rspfTiffTileSource::setApplyColorPaletteFlag(bool flag)
{
   theApplyColorPaletteFlag = flag;
   
   if(isColorMapped())
   {
      if(theApplyColorPaletteFlag)
      {
         thePhotometric[0] = PHOTOMETRIC_PALETTE;
         theSamplesPerPixel = 3;
      }
      else
      {
         thePhotometric[0] = PHOTOMETRIC_MINISBLACK;
         theSamplesPerPixel = 1; 
      }
      
      setReadMethod();

      theTile = 0;
      if (theBuffer)
      {
         delete [] theBuffer;
         theBuffer = 0;
      }
   }
}

bool rspfTiffTileSource::getApplyColorPaletteFlag()const
{
   return theApplyColorPaletteFlag;
}

rspfString rspfTiffTileSource::getLongName()const
{
   return rspfString("TIFF Image Handler");
}

rspfString rspfTiffTileSource::getShortName()const
{
   return rspfString("TIFF Image Handler");
}


std::ostream& rspfTiffTileSource::print(std::ostream& os) const
{
   //***
   // Use a keyword format.
   //***
   os << "image_file:                    " << theImageFile
      << "\nsamples_per_pixel:           " << theSamplesPerPixel
      << "\nbits_per_sample:             " << theBitsPerSample
      << "\nsample_format_unit:          " << theSampleFormatUnit
      << "\nmin_sample_value:            " << theMinSampleValue
      << "\nmax_sample_value:            " << theMaxSampleValue
      << "\nnull_sample_value:           " << theNullSampleValue
      << "\ntheNumberOfDirectories:      " << theNumberOfDirectories
      << "\nr0_is_full_res:              " << theR0isFullRes;

   
   for (rspf_uint32 i=0; i<theNumberOfDirectories; ++i)
   {
      os << "\ndirectory[" << i << "]"
         << "\nimage width:     " << theImageWidth[i]
         << "\nimage_length:    " << theImageLength[i]
         << "\nread method:     " << getReadMethod(i).c_str()
         << "\nplanar:          " << thePlanarConfig[i]
         << "\nphotometric:     " << thePhotometric[i];
      if (theRowsPerStrip[i])
      {
         os << "\nrows per strip:  " << theRowsPerStrip[i];
      }
      if (theImageTileWidth[i])
      {
         os << "\ntile_width:      " << theImageTileWidth[i];
      }
      if (theImageTileLength[i])
      {
         os << "\ntile_length:     " << theImageTileLength[i];
      }
      os << endl;
   }

   if (theTile.valid())
   {
      os << "\nOutput tile dump:\n" << *theTile << endl;
   }

   if (theOverview.valid())
   {
      os << "\nOverview file:\n";
      theOverview->print(os);
   }

   os << endl;
   
   return rspfSource::print(os);
}

rspf_uint32 rspfTiffTileSource::getNumberOfInputBands() const
{
   return theSamplesPerPixel;
}

rspf_uint32 rspfTiffTileSource::getNumberOfOutputBands () const
{
   rspf_uint32 bands = theOutputBandList.size();
   if ( !bands )
   {
      bands = getNumberOfInputBands();
   }
   return bands;
}

bool rspfTiffTileSource::isBandSelector() const
{
   bool result = false;
   if ( isOpen() && theReadMethod.size() && ( theReadMethod.size() == thePlanarConfig.size() ) )
   {
      // Tiled band separate currently is only coded to be band selector.
      result = true;
      for ( rspf_uint32 i = 0; i < theReadMethod.size(); ++i )
      {
         if ( ( theReadMethod[i] != READ_TILE ) ||
              ( thePlanarConfig[i] == PLANARCONFIG_CONTIG ) )
         {
            result = false;
            break;
         }
      }
      if ( result && theOverview.valid() )
      {
         result = theOverview->isBandSelector();
      }
   }
   return result;
}

bool rspfTiffTileSource::setOutputBandList(const std::vector<rspf_uint32>& band_list)
{
   bool result = false;
   if ( isBandSelector() )
   {
      // Making a copy as passed in list could be our m_outputBandList.
      std::vector<rspf_uint32> inputList = band_list;
      result = rspfImageHandler::setOutputBandList( inputList, theOutputBandList );
      if ( result && theTile.valid() )
      {
         if ( theTile->getNumberOfBands() != theOutputBandList.size() )
         {
            theTile = 0; // Force a reinitialize on next getTile.
         }
      }
   }
   return result;
}

void rspfTiffTileSource::getOutputBandList(std::vector<rspf_uint32>& bandList) const
{
   if ( theOutputBandList.size() )
   {
      bandList = theOutputBandList;
   }
   else
   {
      rspfImageSource::getOutputBandList( bandList );
   }
}

bool rspfTiffTileSource::isOpen()const
{
   return (theTiffPtr!=NULL);
}

bool rspfTiffTileSource::hasR0() const
{
   return theR0isFullRes;
}

rspf_float64 rspfTiffTileSource::getMinPixelValue(rspf_uint32 band)const
{
   if(theMetaData.getNumberOfBands())
   {
      return rspfImageHandler::getMinPixelValue(band);
   }
   return theMinSampleValue;
}

rspf_float64 rspfTiffTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   if(theMetaData.getNumberOfBands())
   {
      return rspfImageHandler::getMaxPixelValue(band);
   }
   return theMaxSampleValue;
}

rspf_float64 rspfTiffTileSource::getNullPixelValue(rspf_uint32 band)const
{
   if(theMetaData.getNumberOfBands())
   {
      return rspfImageHandler::getNullPixelValue(band);
   }
   return theNullSampleValue;
}

bool rspfTiffTileSource::isColorMapped() const
{
   bool result = false;
   if ( isOpen() )
   {
      uint16* red;
      uint16* green;
      uint16* blue;
      
      result = static_cast<bool>(TIFFGetField(theTiffPtr,
                                              TIFFTAG_COLORMAP,
                                              &red, &green, &blue));
   }
   return result;
}

void rspfTiffTileSource::setReadMethod()
{
   for (rspf_uint32 dir=0; dir<theNumberOfDirectories; ++dir)
   {
      if (setTiffDirectory(dir) == false)
      {
         return;
      }
      
      //---
      // Establish how this tiff directory will be read.
      //---
      if (TIFFIsTiled(theTiffPtr)) 
      {
         if ( ( thePhotometric[dir] == PHOTOMETRIC_YCBCR ||
                thePhotometric[dir] == PHOTOMETRIC_PALETTE ) &&
              (theSamplesPerPixel <= 3) &&
              (theBitsPerSample   <= 8 ))
         {
            theReadMethod[dir] = READ_RGBA_U8_TILE;
         }
         else
         {
            theReadMethod[dir] = READ_TILE;
         }
      }
      else // Not tiled...
      {
         if ( (thePhotometric[dir] == PHOTOMETRIC_PALETTE ||
               thePhotometric[dir] == PHOTOMETRIC_YCBCR ) &&
              theSamplesPerPixel <= 3 &&
              theBitsPerSample   <= 8 )
         {
            theReadMethod[dir] = READ_RGBA_U8_STRIP;
         }
         else if (theSamplesPerPixel <= 3 && theBitsPerSample == 1)
         {
            //---
            // Note:  One bit data expands to zeroes and 255's so run it through
            //        a specialized method to flip zeroes to one's since zeroes
            //        are usually reserved for null value.
            //---
            theReadMethod[dir] = READ_RGBA_U8A_STRIP;
         }
         else if((theCompressionType == COMPRESSION_NONE)||
                 (theRowsPerStrip[dir]==1))
         {
            theReadMethod[dir] = READ_SCAN_LINE;
         }
         else if((theCompressionType!=COMPRESSION_NONE)&&
                 (theSamplesPerPixel <= 3) &&
                 (theBitsPerSample   <= 8) )
         {
            theReadMethod[dir] = READ_RGBA_U8_STRIP;
         }
         else
         {
            theReadMethod[dir] = UNKNOWN;
         }
      }
      
   } // End of loop through directories.

   // Reset the directory back to "0".
   setTiffDirectory(0);
}

void rspfTiffTileSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid())
   {
      return;
   }
   if(property->getName() == "apply_color_palette_flag")
   {
      // Assuming first directory...
      setApplyColorPaletteFlag(property->valueToString().toBool());
   }
   else
   {
      rspfImageHandler::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfTiffTileSource::getProperty(const rspfString& name)const
{
   if(name == "apply_color_palette_flag")
   {
      rspfBooleanProperty* property = new rspfBooleanProperty("apply_color_palette_flag",
                                                                theApplyColorPaletteFlag);
      property->clearChangeType();
      property->setFullRefreshBit();
      return property;
   }
   else if(name == "file_type")
	{
		return new rspfStringProperty(name, "TIFF");
	}
	
   return rspfImageHandler::getProperty(name);
}

void rspfTiffTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageHandler::getPropertyNames(propertyNames);
   propertyNames.push_back("file_type");
   propertyNames.push_back("apply_color_palette_flag");
}

bool rspfTiffTileSource::setTiffDirectory(rspf_uint16 directory)
{
   bool status = true;
   theCurrentTiffRlevel = 0;
   if (theCurrentDirectory != directory)
   {
      status = TIFFSetDirectory(theTiffPtr, directory);
      if (status == true)
      {
         theCurrentDirectory = directory;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfTiffTileSource::setTiffDirectory ERROR setting directory "
            << directory << "!" << endl;
      }
   }
   
   rspf_uint32 idx = 0;
   for(idx = 0; idx<theImageDirectoryList.size();++idx)
   {
      if(theImageDirectoryList[idx] == directory)
      {
         theCurrentTiffRlevel = idx;
         break;
      }
   }
   return status;
}

void rspfTiffTileSource::populateLut()
{
   rspf_uint16* r;
   rspf_uint16* g;
   rspf_uint16* b;
   if(TIFFGetField(theTiffPtr, TIFFTAG_COLORMAP, &r, &g, &b))
   {
      rspf_uint32 numEntries = 256;
      rspfScalarType scalarType = RSPF_UINT8;
      if(theBitsPerSample == 16)
      {
         numEntries = 65536;
         scalarType = RSPF_UINT16;
      }
      theLut = new rspfNBandLutDataObject(numEntries,
                                           3,
                                           scalarType,
                                           0);
      rspf_uint32 entryIdx = 0;
      for(entryIdx = 0; entryIdx < numEntries; ++entryIdx)
      {
         if(scalarType == RSPF_UINT8)
         {
            (*theLut)[entryIdx][0] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(((*r)/65535.0)*255.0);
            (*theLut)[entryIdx][1] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(((*g)/65535.0)*255.0);
            (*theLut)[entryIdx][2] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(((*b)/65535.0)*255.0);
         }
         else
         {
            (*theLut)[entryIdx][0] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(*r);
            (*theLut)[entryIdx][1] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(*g);
            (*theLut)[entryIdx][2] = (rspfNBandLutDataObject::LUT_ENTRY_TYPE)(*b);
         }
         ++r;++g;++b;
      }
   }
}

void rspfTiffTileSource::validateMinMaxNull()
{
   rspf_float64 tempNull = rspf::defaultNull(theScalarType);
   rspf_float64 tempMax  = rspf::defaultMax(theScalarType);
   rspf_float64 tempMin  = rspf::defaultMin(theScalarType);
   
   if( (theMinSampleValue == tempNull) || rspf::isnan(theMinSampleValue) ) 
   {
      theMinSampleValue = tempMin;
   }
   if( (theMaxSampleValue == tempNull) || rspf::isnan(theMaxSampleValue) )
   {
      theMaxSampleValue = tempMax;
   }
   if ( rspf::isnan(theNullSampleValue) )
   {
      theNullSampleValue = tempNull;
   }

   if (theScalarType == RSPF_FLOAT32)
   {
      std::ifstream inStr(theImageFile.c_str(), std::ios::in|std::ios::binary);
      if ( inStr.good() )
      {   
         // Do a print to a memory stream in key:value format.
         rspfTiffInfo ti;
         rspfIOMemoryStream memStr;
         ti.print(inStr, memStr);

         // Make keywordlist with all the tags.
         rspfKeywordlist gtiffKwl;
         if ( gtiffKwl.parseStream(memStr) )
         {
#if 0 /* Please keep for debug. (drb) */
            if ( traceDebug() )
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "rspfTiffTileSource::validateMinMaxNull kwl:\n" << gtiffKwl
                  << endl;
            }
#endif
            const char* lookup;

            lookup = gtiffKwl.find("tiff.image0.gdal_nodata");
            bool nullFound = false;
            if ( lookup )
            {
               rspfString os = lookup;
               theNullSampleValue = os.toFloat32();
               nullFound = true;
            }
            lookup = gtiffKwl.find("tiff.image0.vertical_citation");
            if ( lookup )
            {     
               //---
               // Format of string this handles:
               // "Null: -9999.000000, Non-Null Min: 12.428605, 
               // Non-Null Avg: 88.944082, Non-Null Max: 165.459558|"
               rspfString citation = lookup;
               std::vector<rspfString> array;
               citation.split( array, rspfString(",") );
               if ( array.size() == 4 )
               {
                  std::vector<rspfString> array2;

                  if ( !nullFound )
                  {
                     // null
                     array[0].split( array2, rspfString(":") );
                     if ( array2.size() == 2 )
                     {
                        rspfString os = array2[0].downcase();
                        if ( os.contains( rspfString("null") ) )
                        {
                           if ( array2[1].size() )
                           {
                              theNullSampleValue = array2[1].toFloat64(); 
                              nullFound = true;
                           }
                        }
                     }
                  }

                  // min
                  array2.clear();
                  array[1].split( array2, rspfString(":") );
                  if ( array2.size() == 2 )
                  {  
                     rspfString os = array2[0].downcase();
                     if ( os.contains( rspfString("min") ) )
                     {
                        if ( array2[1].size() )
                        {
                           theMinSampleValue = array2[1].toFloat64();
                        }
                     }
                  }

                  // Skipping average.

                  // max
                  array2.clear();
                  array[3].split( array2, rspfString(":") );
                  if ( array2.size() == 2 )
                  {
                     rspfString os = array2[0].downcase();
                     if ( os.contains( rspfString("max") ) )
                     {
                        if ( array2[1].size() )
                        {
                           array2[1].trim( rspfString("|") );
                           theMaxSampleValue = array2[1].toFloat64();   
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

#if 0
rspfImageGeometry* rspfTiffTileSource::getImageGeometry()
{
   //---
   // Call base class getImageGeometry which will check for external geometry
   // or an already set geometry.
   //---
   rspfImageGeometry* result = rspfImageHandler::getImageGeometry();

   if (result)
   {
      //---
      // TODO: Add transform from tags.
      //---

      
      // if ( !result->getTransform() )
      // {
      //    if ( transform.valid() )
      //    {
      //       result->setTransform( transform.get() );
      //    }
      // }
      //else
      //{
      //   rspfImageGeometryRegistry::instance()->createTransform(this);
      //}
      
      if ( !result->getProjection() )
      {
         // Get the projection from the tags.
         
         rspfRefPtr<rspfProjection> proj = 0;

         if (theTiffPtr)
         {
            rspfGeoTiff geotiff;

            //---
            // Note: must pass false to readTags so it doesn't close our
            // tiff pointer.
            //---
            geotiff.readTags(theTiffPtr, getCurrentEntry(), false);
            rspfKeywordlist kwl;
            if(geotiff.addImageGeometry(kwl))
            {
               proj = rspfProjectionFactoryRegistry::instance()->
                  createProjection(kwl);
            }
            
            if ( proj.valid() )
            {
               result->setProjection( proj.get() );
            }
            //else
            //{
            // rspfImageGeometryRegistry::instance()->createProjection(this);
            //}
         }
      }
      
   } // matches: if (result)

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiffTileSource::createImageGeometry DEBUG:\n";

      if (result)
      {
         result->print(rspfNotify(rspfNotifyLevel_DEBUG)) << "\n";
      }
   }

   return result;
}
#endif

bool rspfTiffTileSource::isPowerOfTwoDecimation(rspf_uint32 level) const
{
   // Check size of this level against last level to see if it's half the previous.
   bool result = false;
   if ( (level > 0) && ( theImageWidth.size() > level ) && ( theImageLength.size() > level ) )
   {
      rspf_uint32 i = level-1; // previous level
      
      if ( ( ( theImageWidth[i]/2 == theImageWidth[level] ) ||
             ( (theImageWidth[i]+1)/2 == theImageWidth[level] ) ) &&
           ( ( theImageLength[i]/2 == theImageLength[level] ) ||
             ( (theImageLength[i]+1)/2 == theImageLength[level] ) ) )
      {
         result = true;
      }
   }
   return result;
}

void rspfTiffTileSource::allocateTile()
{
   theTile = 0;
   rspf_uint32 bands = 0;
   if ( theOutputBandList.empty() )
   {
      bands = getNumberOfOutputBands();
   }
   else
   {
      bands = theOutputBandList.size();
   }

   if ( bands )
   {
      theTile = rspfImageDataFactory::instance()->create( this, getOutputScalarType(), bands );
      if ( theTile.valid() )
      {
         theTile->initialize();
         
         // The width and height mus be set prior to call to allocateBuffer.
         theCurrentTileWidth  = theTile->getWidth();
         theCurrentTileHeight = theTile->getHeight();
      }
   }
}

bool rspfTiffTileSource::allocateBuffer()
{
   bool bSuccess = true;

   // Allocate memory for a buffer to hold data grabbed from the tiff file.
   rspf_uint32 buffer_size=0;
   switch (theReadMethod[theCurrentDirectory])
   {
      case READ_RGBA_U8_TILE:
      {
         buffer_size = theImageTileWidth[theCurrentDirectory]*
            theImageTileWidth[theCurrentDirectory]*theBytesPerPixel*4;
         break;
      }  
      case READ_TILE:
      {
         if (thePlanarConfig[theCurrentDirectory] == PLANARCONFIG_CONTIG)
         {
            buffer_size = theImageTileWidth[theCurrentDirectory] *
               theImageTileLength[theCurrentDirectory] *
               theBytesPerPixel * theSamplesPerPixel;
         }
         else
         {
            buffer_size = theImageTileWidth[theCurrentDirectory] *
               theImageTileLength[theCurrentDirectory] *
               theBytesPerPixel;
         }
         break;
      }  
      case READ_RGBA_U8_STRIP:
      case READ_RGBA_U8A_STRIP:
      {
         buffer_size = theImageWidth[0]*theRowsPerStrip[theCurrentDirectory]*
            theBytesPerPixel*4;
         break;
      }  
      case READ_SCAN_LINE:
      {
#if RSPF_BUFFER_SCAN_LINE_READS
         // Buffer a image width by tile height.
         buffer_size = theImageWidth[0] * theBytesPerPixel *
            theSamplesPerPixel * theCurrentTileHeight;
#else
         buffer_size = theImageWidth[0] * theBytesPerPixel * theSamplesPerPixel;
#endif
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "Unknown read method!" << endl;
         print(rspfNotify(rspfNotifyLevel_WARN));
         bSuccess = false;
      }
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiffTileSource::allocateBuffer DEBUG:"
         << "\nbuffer_size:  " << buffer_size
         << endl;
   }

   theBufferRect.makeNan();
   theBufferRLevel = theCurrentDirectory;

   if ( bSuccess && ( buffer_size != theBufferSize ) )
   {
      theBufferSize = buffer_size;
      if (theBuffer)
      {
         delete [] theBuffer;
      }

      // ESH 05/2009 -- Fix for ticket #738:  
      // image_info crashing on aerial_ortho image during ingest
      try
      {
         theBuffer = new rspf_uint8[buffer_size];
      }
      catch(...)
      {
         theBuffer = 0;
         bSuccess = false;
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfTiffTileSource::allocateBuffer WARN:"
               << "\nNot enough memory: buffer_size:  " << buffer_size
               << endl;
         }
      }
   }

   return bSuccess;
}
