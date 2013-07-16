//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description:
//
// Contains class definition for JpegTileSource.
//*******************************************************************
//  $Id: rspfJpegTileSource.cpp 22117 2013-01-18 21:04:23Z dburken $

#if defined(__BORLANDC__)
#  include <iostream>
using std::size_t;
#  include <stdlib.h>
#  include <stdio.h>
#endif

extern "C"
{
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
}

#include <rspf/imaging/rspfJpegTileSource.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfJpegStdIOSrc.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/imaging/rspfU8ImageData.h>
#include <rspf/projection/rspfBilinearProjection.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/support_data/rspfXmpInfo.h>

//---
// Using windows .NET compiler there is a conflict in the libjpeg with INT32
// in the file jmorecfg.h.  Defining XMD_H fixes this.
//---

RTTI_DEF1_INST(rspfJpegTileSource, "rspfJpegTileSource", rspfImageHandler)

static rspfTrace traceDebug("rspfJpegTileSource:degug");  

class rspfJpegTileSource::PrivateData
{
public:
   PrivateData()
      :theCinfo(),
       theJerr()
   {
      
   }
   virtual ~PrivateData()
   {
      clear();
   }
   void clear()
   {
      jpeg_destroy_decompress( &theCinfo );
   }
   struct jpeg_decompress_struct theCinfo;
   struct jpeg_error_mgr         theJerr;
};

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfJpegTileSource::rspfJpegTileSource()
   :
      rspfImageHandler(),
      theTile(0),
      theCacheTile(0),
      theLineBuffer(0),
      theFilePtr(0),
      theBufferRect(0, 0, 0, 0),
      theImageRect(0, 0, 0, 0),
      theNumberOfBands(0),
      theCacheSize	(0),
      thePrivateData(0),
      theCacheId(-1)
{
}

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfJpegTileSource::rspfJpegTileSource(const rspfKeywordlist& kwl,
                                         const char* prefix)
   :
      rspfImageHandler(),
      theTile(0),
      theCacheTile(0),
      theLineBuffer(0),
      theFilePtr(0),
      theBufferRect(0, 0, 0, 0),
      theImageRect(0, 0, 0, 0),
      theNumberOfBands(0),
      theCacheSize	(0),
      thePrivateData(0),
      theCacheId(-1)
{
   if (loadState(kwl, prefix) == false)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
   }
}

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfJpegTileSource::rspfJpegTileSource(const char* jpeg_file)
   :
      rspfImageHandler(),
      theTile(0),
      theCacheTile(0),
      theLineBuffer(0),
      theFilePtr(0),
      theBufferRect(0, 0, 0, 0),
      theImageRect(0, 0, 0, 0),
      theNumberOfBands(0),
      theCacheSize(0),
      thePrivateData(0),
      theCacheId(-1)
{
   static const char MODULE[]
      = "rspfJpegTileSource::rspfJpegTileSource";

   if (!open())
   {

      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE
         << "\nCannot open:  " << jpeg_file
         << endl;
   }
}

//*******************************************************************
// Destructor:
//*******************************************************************
rspfJpegTileSource::~rspfJpegTileSource()
{
   destroy();
}

void rspfJpegTileSource::destroy()
{
   rspfAppFixedTileCache::instance()->deleteCache(theCacheId);

   theTile      = 0;
   theCacheTile = 0;

   if (theLineBuffer)
   {
      delete [] theLineBuffer;
      theLineBuffer = 0;
   }
   if (theFilePtr)
   {
      fclose(theFilePtr);
      theFilePtr = NULL;
   }
   if(thePrivateData)
   {
      delete thePrivateData;
      thePrivateData = 0;
   }
}

void rspfJpegTileSource::allocate()
{
   if(theLineBuffer)
   {
      delete [] theLineBuffer;
      theLineBuffer = 0;
   }
   // Make the cache tile the height of one tile by the image width.
   rspf::defaultTileSize(theCacheSize);
   theCacheSize.x = theImageRect.width();

   rspfAppFixedTileCache::instance()->deleteCache(theCacheId);
   theCacheId = rspfAppFixedTileCache::instance()->newTileCache(theImageRect, theCacheSize);

   theTile = rspfImageDataFactory::instance()->create(this, this);
   theCacheTile = (rspfImageData*)theTile->dup();
   theTile->initialize();
   
   rspfIrect cache_rect(theImageRect.ul().x,
                         theImageRect.ul().y,
                         theImageRect.ul().x + (theCacheSize.x-1),
                         theImageRect.ul().y + (theCacheSize.y-1));
   
   theCacheTile->setImageRectangle(cache_rect);
   theCacheTile->initialize();
   
   theLineBuffer = new rspf_uint8[theImageRect.width() * theNumberOfBands];

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfJpegTileSource::allocate DEBUG:"
         << "\ncache tile size:           " << theCacheSize
         << "\nimage width:               " << theImageRect.width()
         << "\nimage height:              " << theImageRect.height()
         << "\nnumber of bands:           " << theNumberOfBands
         << endl;
   }
}

rspfRefPtr<rspfImageData> rspfJpegTileSource::getTile(
   const rspfIrect& rect, rspf_uint32 resLevel)
{
   if (theTile.valid())
   {
      // Image rectangle must be set prior to calling getTile.
      theTile->setImageRectangle(rect);
      
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

bool rspfJpegTileSource::getTile(rspfImageData* result,
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

         if (getImageRectangle(0).intersects(tile_rect))
         {
            // Make a clip rect.
            rspfIrect clip_rect = tile_rect.clipToRect(getImageRectangle(0));
            
            fillTile(clip_rect, result);
         }
         else // No intersection...
         {
            result->makeBlank();
         }
      }

      result->unref();  // Decrement ref count.
   }

   return status;
}

void rspfJpegTileSource::fillTile(const rspfIrect& clip_rect,
                                   rspfImageData* tile)
{
   if (!theFilePtr) return;

   rspfIrect buffer_rect = clip_rect;
   buffer_rect.stretchToTileBoundary(theCacheSize);
   buffer_rect.set_ulx(0);
   buffer_rect.set_lrx(getImageRectangle(0).lr().x);

   // Check for a partial tile.
   if ( ! tile->getImageRectangle().completely_within(buffer_rect) )
   {
      tile->makeBlank();
   }

   rspf_int32 number_of_cache_tiles = buffer_rect.height()/theCacheSize.y;

#if 0
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "tiles high:  " << number_of_cache_tiles
         << endl;
   }
#endif

   rspfIpt origin = buffer_rect.ul();
   
   for (int tileIdx = 0; tileIdx < number_of_cache_tiles; ++tileIdx)
   {
      // See if it's in the cache already.
      rspfRefPtr<rspfImageData> tempTile;
      tempTile = rspfAppFixedTileCache::instance()->
         getTile(theCacheId, origin);
      if (tempTile.valid())
      {
         tile->loadTile(tempTile.get());
      }
      else
      {
         // Have to read from the jpeg file.
         rspf_uint32 start_line = static_cast<rspf_uint32>(origin.y);
         rspf_uint32 stop_line  = 
            static_cast<rspf_uint32>( min(origin.y+theCacheSize.y-1,
                                           getImageRectangle().lr().y) );
         rspfIrect cache_rect(origin.x,
                               origin.y,
                               origin.x+theCacheSize.x-1,
                               origin.y+theCacheSize.y-1);

         theCacheTile->setImageRectangle(cache_rect);

         if ( !theCacheTile->getImageRectangle().
              completely_within(getImageRectangle()) )
         {
            theCacheTile->makeBlank();
         }

         if (start_line < thePrivateData->theCinfo.output_scanline)
         {
            // Must restart the compression process again.
            restart();
         }
         
         // Get pointers to the cache tile buffers.
         JSAMPROW jbuf[1];
         std::vector<rspf_uint8*> buf(theNumberOfBands);
         rspf_uint32 band = 0;
         for (band = 0; band < theNumberOfBands; ++band)
         {
            buf[band] = theCacheTile->getUcharBuf(band);
         }

         const rspf_uint32 SAMPLES = getNumberOfSamples();
         jbuf[0] = (JSAMPROW) theLineBuffer;

         // Gobble any not needed lines.
         while (thePrivateData->theCinfo.output_scanline < start_line)
         {
            jpeg_read_scanlines(&thePrivateData->theCinfo, jbuf, 1);
         }

         while (thePrivateData->theCinfo.output_scanline <= stop_line)
         {
            // Read a line from the jpeg file.
            jpeg_read_scanlines(&thePrivateData->theCinfo, jbuf, 1);
            
            //---
            // Copy the line which if band interleaved by pixel the the band
            // separate buffers.
            //---
            rspf_uint32 index = 0;
            for (rspf_uint32 sample = 0; sample < SAMPLES; ++sample)
            {
               for (band = 0; band < theNumberOfBands; ++band)
               {
                  buf[band][sample] = theLineBuffer[index];
                  ++index;
               }
            }
            
            for (band = 0; band < theNumberOfBands; ++band)
            {
               buf[band] += SAMPLES;
            }
         }
         theCacheTile->validate();
         
         tile->loadTile(theCacheTile.get());
         
         // Add it to the cache for the next time.
         rspfAppFixedTileCache::instance()->addTile(theCacheId, theCacheTile);
         
      } // End of reading for jpeg file.
      
      origin.y += theCacheSize.y;
      
   } // for (int tile = 0; tile < number_of_cache_tiles; ++tile)

   tile->validate();
}

//*******************************************************************
// Public Method:
//*******************************************************************
rspfIrect
rspfJpegTileSource::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   return rspfIrect(0,
                     0,
                     getNumberOfSamples(reduced_res_level) - 1,
                     getNumberOfLines(reduced_res_level)   - 1);
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfJpegTileSource::saveState(rspfKeywordlist& kwl,
                               const char* prefix) const
{
   return rspfImageHandler::saveState(kwl, prefix);
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfJpegTileSource::loadState(const rspfKeywordlist& kwl,
                               const char* prefix)
{
   if (rspfImageHandler::loadState(kwl, prefix))
   {
      return open();
   }

   return false;
}

void rspfJpegTileSource::close()
{
   destroy();
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfJpegTileSource::open(const rspfFilename& jpeg_file)
{
   theImageFile = jpeg_file;

   return open();
}

//*******************************************************************
// Private method:I have problems

//*******************************************************************
bool rspfJpegTileSource::open()
{
   static const char MODULE[] = "rspfJpegTileSource::open";

   // Start with a clean slate.
   destroy();
   // Check for empty filename.
   if (theImageFile.empty())
   {
      return false;
   }
   
   // Open Jpeg file.
   if((theFilePtr = fopen(theImageFile.c_str(), "rb")) == NULL)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;

      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << "\nERROR:\n"
            << "Could not open:  " << theImageFile.c_str()
            << endl;
      }
      
      return false;
   }

   //***
   // Verify the file is a jpeg by checking the first two bytes.
   //***
   rspf_uint8 c[2];
   fread(c, 2, 1, theFilePtr);
   if( c[0] != 0xFF || c[1] != 0xD8 )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " NOTICE:\n"
            << "Not a jpeg file..." << endl;
      }
      
      fclose(theFilePtr);
      theFilePtr = NULL;
      return false;
   }

   thePrivateData = new PrivateData();
   rewind(theFilePtr);

   //---
   // Step 1: allocate and initialize JPEG decompression object
   // We set up the normal JPEG error routines, then override error_exit.
   //---   
   thePrivateData->theCinfo.err = jpeg_std_error(&thePrivateData->theJerr);

   // Initialize the JPEG decompression object.
   jpeg_create_decompress(&thePrivateData->theCinfo);

   // Specify data source.
   //jpeg_stdio_src(&thePrivateData->theCinfo, theFilePtr);
   rspfJpegStdIOSrc(&thePrivateData->theCinfo, theFilePtr);

   // Read the file parameters with jpeg_read_header.
   jpeg_read_header(&thePrivateData->theCinfo, TRUE);

   jpeg_start_decompress(&thePrivateData->theCinfo);

   theNumberOfBands = thePrivateData->theCinfo.output_components;

   theImageRect = rspfIrect(0,
                             0,
                             thePrivateData->theCinfo.output_width  - 1,
                             thePrivateData->theCinfo.output_height - 1);
   
   theBufferRect.set_lrx(thePrivateData->theCinfo.output_width  - 1);
   
   completeOpen();

   // Allocate memory...
   allocate();

   return true;
}

rspfRefPtr<rspfProperty> rspfJpegTileSource::getProperty(const rspfString& name)const
{
	if(name == "file_type")
	{
		return new rspfStringProperty("file_type", "JPEG");
	}
	return rspfImageHandler::getProperty(name);
}

void rspfJpegTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
	rspfImageHandler::getPropertyNames(propertyNames);
	propertyNames.push_back("file_type");
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfJpegTileSource::getTileWidth() const
{
   return ( theTile.valid() ? theTile->getWidth() : 0 );
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfJpegTileSource::getTileHeight() const
{
   return ( theTile.valid() ? theTile->getHeight() : 0 );
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfJpegTileSource::isValidRLevel(rspf_uint32 reduced_res_level) const
{
   static const char MODULE[] = "rspfJpegTileSource::isValidRLevel";

   if (reduced_res_level == 0)
   {
      return true;
   }
   else if (theOverview.valid())
   {
      return theOverview->isValidRLevel(reduced_res_level);
   }
   else
   {
      cerr << MODULE << " Invalid reduced_res_level:  " << reduced_res_level
           << "\nHighest available:  " << (getNumberOfDecimationLevels() - 1)
           << endl;
   }
   return false;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfJpegTileSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      return theImageRect.lr().y - theImageRect.ul().y + 1;
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
rspf_uint32 rspfJpegTileSource::getNumberOfSamples(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      return theImageRect.lr().x - theImageRect.ul().x + 1;;
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfSamples(reduced_res_level);
   }

   return 0;
}

rspf_uint32 rspfJpegTileSource::getImageTileWidth() const
{
   return 0;
}

rspf_uint32 rspfJpegTileSource::getImageTileHeight() const
{
   return 0;
}

rspfString rspfJpegTileSource::getShortName()const
{
   return rspfString("jpg");
}
   
rspfString rspfJpegTileSource::getLongName()const
{
   return rspfString("jpg reader");
}

rspf_uint32 rspfJpegTileSource::getNumberOfInputBands() const
{
   return theNumberOfBands;
}

rspf_uint32 rspfJpegTileSource::getNumberOfOutputBands()const
{
   return getNumberOfInputBands();
}

rspfScalarType rspfJpegTileSource::getOutputScalarType() const
{
   return RSPF_UCHAR;
}

bool rspfJpegTileSource::isOpen()const
{
   return (theFilePtr != NULL);
}

void rspfJpegTileSource::restart()
{
   jpeg_abort_decompress( &thePrivateData->theCinfo );
   jpeg_destroy_decompress( &thePrivateData->theCinfo );
   
   // Put the theFilePtr back to the start...
   rewind(theFilePtr);

   // Initialize the JPEG decompression object.
   jpeg_create_decompress(&thePrivateData->theCinfo);

   // Specify data source.
   rspfJpegStdIOSrc(&thePrivateData->theCinfo, theFilePtr);

   // Read the file parameters with jpeg_read_header.
   jpeg_read_header(&thePrivateData->theCinfo, TRUE);

   jpeg_start_decompress(&thePrivateData->theCinfo);
}

rspfRefPtr<rspfImageGeometry> rspfJpegTileSource::getImageGeometry()
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

rspfRefPtr<rspfImageGeometry> rspfJpegTileSource::getInternalImageGeometry() const
{
   rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry();

   // See if we can pull a projection from the XMP APP1 XML block if present:
   rspfXmpInfo info;
   if ( info.open( getFilename() ) )
   {
      rspfDrect imageRect = getImageRectangle( 0 );
      rspfRefPtr<rspfProjection> proj = info.getProjection( imageRect );
      if ( proj.valid() )
      {
         geom->setProjection( proj.get() );
      }
   }

   return geom;
}
