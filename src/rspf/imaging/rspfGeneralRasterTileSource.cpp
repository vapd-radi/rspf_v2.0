//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Contains class definition for rspfGeneralRasterTileSource.
//*******************************************************************
//  $Id: rspfGeneralRasterTileSource.cpp 21962 2012-11-30 15:44:32Z dburken $

#include <rspf/imaging/rspfGeneralRasterTileSource.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfInterleaveTypeLut.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfStreamFactoryRegistry.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageGeometryRegistry.h>
#include <rspf/projection/rspfMapProjectionFactory.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/support_data/rspfFgdcXmlDoc.h>

RTTI_DEF1_INST(rspfGeneralRasterTileSource,
               "rspfGeneralRasterTileSource",
               rspfImageHandler)

static rspfTrace traceDebug("rspfGeneralRasterTileSource:debug");

// For interleave type enum to string conversions.
static const rspfInterleaveTypeLut ILUT;

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfGeneralRasterTileSource::rspfGeneralRasterTileSource()
   :
      rspfImageHandler(),
      m_tile(0),
      m_buffer(0),
      m_lineBuffer(0),
      m_bufferInterleave(RSPF_BIL),
      m_fileStrList(0),
      m_rasterInfo(),
      m_bufferRect(0, 0, 0, 0),
      m_swapBytesFlag(false),
      m_bufferSizeInPixels(0),
      m_outputBandList(0)
{}

rspfGeneralRasterTileSource::~rspfGeneralRasterTileSource()
{
   close();
}

rspfRefPtr<rspfImageData> rspfGeneralRasterTileSource::getTile(
   const rspfIrect& tile_rect, rspf_uint32 resLevel)
{
   if ( m_tile.valid() == false )
   {
      allocateTile(); // First time through...
   }
   
   if (m_tile.valid())
   {
      // Image rectangle must be set prior to calling getTile.
      m_tile->setImageRectangle(tile_rect);
      
      if ( getTile( m_tile.get(), resLevel ) == false )
      {
         if (m_tile->getDataObjectStatus() != RSPF_NULL)
         {
            m_tile->makeBlank();
         }
      }
   }
   
   return m_tile;
}

bool rspfGeneralRasterTileSource::getTile(rspfImageData* result,
                                           rspf_uint32 resLevel)
{
   bool status = false;
   
   //---
   // Not open, this tile source bypassed, or invalid res level,
   // return a blank tile.
   //---
   if( isOpen() && isSourceEnabled() && isValidRLevel(resLevel)  &&
       result && (result->getNumberOfBands() == getNumberOfOutputBands()) )
   {
      //---
      // Check for overview tile.  Some overviews can contain r0 so always
      // call even if resLevel is 0.  Method returns true on success, false
      // on error.
      //---
      status = getOverviewTile(resLevel, result);
      if (status)
      {
         if(getOutputScalarType() == RSPF_USHORT11)
         {
            //---
            // Temp fix:
            // The overview handler could return a tile of RSPF_UINT16 if
            // the max sample value was not set to 2047.
            //---
            result->setScalarType(RSPF_USHORT11);
         }
      }
      
      if (!status) // Did not get an overview tile.
      {
         status = true;
         
         //---
         // Subtract any sub image offset to get the zero based image space
         // rectangle.
         //---
         rspfIrect tile_rect = result->getImageRectangle();
         
         // This should be the zero base image rectangle for this res level.
         rspfIrect image_rect = getImageRectangle(resLevel);
         
         //---
         // See if any point of the requested tile is in the image.
         //---
         if ( tile_rect.intersects(image_rect) )
         {
            // Make the tile rectangle zero base.
            result->setImageRectangle(tile_rect);

            // Initialize the tile if needed as we're going to stuff it.
            if (result->getDataObjectStatus() == RSPF_NULL)
            {
               result->initialize();
            }

            rspfIrect clip_rect = tile_rect.clipToRect(image_rect);

            if ( ! tile_rect.completely_within(m_bufferRect) )
            {
               // A new buffer must be loaded.
               if ( !tile_rect.completely_within(clip_rect) )
               {
                  //---
                  // Start with a blank tile since the whole tile buffer will
                  // not be
                  // filled.
                  //---
                  result->makeBlank();
               }

               // Reallocate the buffer if needed.
               if ( m_bufferSizeInPixels != result->getSize() )
               {
                  allocateBuffer( result );
               }

               rspfIpt size(static_cast<rspf_int32>(result->getWidth()),
                             static_cast<rspf_int32>(result->getHeight()));

               if( !fillBuffer(clip_rect.ul(), size) )
               {
                  rspfNotify(rspfNotifyLevel_WARN)
                     << "Error from fill buffer..."
                     << std::endl;
                  //---
                  // Error in filling buffer.
                  //---
                  setErrorStatus();
                  status = false;
               }
            }
            
            result->loadTile(m_buffer,
                             m_bufferRect,
                             clip_rect,
                             m_bufferInterleave);
            result->validate();

            // Set the rectangle back.
            result->setImageRectangle(tile_rect);
            
         }
         else // No intersection.
         {
            result->makeBlank();
         }
      }
   }
   return status;
}

bool rspfGeneralRasterTileSource::fillBuffer(const rspfIpt& origin, const rspfIpt& size)
{

   static const char MODULE[] = "rspfGeneralRasterTileSource::fillBuffer";

   // Note:  InterleaveType enumerations in "constants.h" file.
   bool status = false;
   switch ( m_rasterInfo.interleaveType() )
   {
      case RSPF_BIP:
      {
         status = fillBIP(origin, size);
         break;
      }
      case RSPF_BIL:
      {
         status = fillBIL(origin, size);
         break;
      }
      case RSPF_BSQ:
      {
         status = fillBSQ(origin, size);
         break;
      }
      case RSPF_BSQ_MULTI_FILE:
      {
         status = fillBsqMultiFile(origin, size);
         break;
      }
      default:
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " ERROR:\n"
            << " Unsupported interleave type:  "
            << ILUT.getEntryString(m_rasterInfo.interleaveType())
            << std::endl;
      }
   }
   
   if (status && m_swapBytesFlag)
   {
      rspfEndian oe;
      oe.swap(m_rasterInfo.getImageMetaData().getScalarType(),
              m_buffer,
              m_bufferSizeInPixels);
   }

   return status;
}

bool rspfGeneralRasterTileSource::fillBIP(const rspfIpt& origin, const rspfIpt& size )
{
   static const char MODULE[] = "rspfGeneralRasterTileSource::fillBIP ";

   m_bufferRect.set_ul(origin);
   m_bufferRect.set_lry(min( (origin.y + size.y -1),
                              m_rasterInfo.imageRect().lr().y));
   m_bufferRect.set_lrx(min( (origin.x + size.x -1),
                              m_rasterInfo.imageRect().lr().x));

   const rspf_int32 WIDTH                  = static_cast<rspf_int32>( m_bufferRect.width() ); 
   const rspf_int32 HEIGHT                 = static_cast<rspf_int32>( m_bufferRect.height() ); 
   const rspf_int32 INPUT_BANDS            = m_rasterInfo.numberOfBands();
   const rspf_int32 OUTPUT_BANDS           = static_cast<rspf_int32>( m_outputBandList.size() );
   const rspf_int32 BYTES_PER_PIXEL        = m_rasterInfo.bytesPerPixel();
   const rspf_int32 INPUT_BYTES_PER_SAMPLE = BYTES_PER_PIXEL * INPUT_BANDS;
   const rspf_int32 OUTPUT_BYTES_PER_SAMPLE = BYTES_PER_PIXEL * OUTPUT_BANDS;
   
   // Seek position.
   std::streamoff rasterOffset = m_rasterInfo.offsetToFirstValidSample() +
        origin.y * m_rasterInfo.bytesPerRawLine() +
        origin.x * INPUT_BYTES_PER_SAMPLE;

    // Input line buffer, all bands.
   std::streamsize inputLineBufferWidth = WIDTH * INPUT_BYTES_PER_SAMPLE;
  
   // Output buffer width:
   std::streamsize outputLineBufferWidth = WIDTH * OUTPUT_BYTES_PER_SAMPLE;

#if 0 /* Please keep for debug. (drb) */
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nDEBUG:"
         << "\norigin:                 " << origin
         << "\nSeek position:          " << rasterOffset
         << "\ninputLineBufferWidth:   " << inputLineBufferWidth
         << "\noutputLineBufferWidth:  " << outputLineBufferWidth
         << "\nINPUT_BANDS:            " << INPUT_BANDS
         << "\nOUTPUT_BANDS:           " << OUTPUT_BANDS
         << std::endl;
#endif
   
   rspf_int32 bufferOffset = 0;   
   
   // Line loop:
   rspf_int32 currentLine = 0;
   while ( currentLine < HEIGHT )
   {
      // Seek to line.
      m_fileStrList[0]->seekg(rasterOffset, ios::beg);
      if (!(*m_fileStrList[0]))
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " ERROR:\n"
            << " Seek error!  Returning with error..." << std::endl;
         return false;
      }

      // Read image data from line for all bands into line buffer.   
      m_fileStrList[0]->read( (char*)m_lineBuffer, inputLineBufferWidth );
      if ( m_fileStrList[0]->gcount() != inputLineBufferWidth ) 
      {
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << "\nERROR:  Reading image line." << std::endl;
         return false;
      }
      
      // Sample loop:
      for ( rspf_int32 sample = 0; sample < WIDTH; ++sample )
      {
         // Band loop:
         for ( rspf_int32 band = 0; band < OUTPUT_BANDS; ++band )
         {
            rspf_int32 selectedBand = static_cast<rspf_int32>(m_outputBandList[band]);
            memcpy( (void*)(m_buffer + bufferOffset +
                            sample * OUTPUT_BYTES_PER_SAMPLE +
                            band * BYTES_PER_PIXEL),
                    (void*)(m_lineBuffer +
                            sample * INPUT_BYTES_PER_SAMPLE + 
                            selectedBand * BYTES_PER_PIXEL),
                    BYTES_PER_PIXEL );
         }
      }
               
      ++currentLine;
      bufferOffset += outputLineBufferWidth;
      rasterOffset += m_rasterInfo.bytesPerRawLine();
   }
   
   return true;
   
} // End: bool rspfGeneralRasterTileSource::fillBipBandSelect(...

bool rspfGeneralRasterTileSource::fillBIL(const rspfIpt& origin,
                                           const rspfIpt& size)
{
   static const char MODULE[] = "rspfGeneralRasterTileSource::fillBIL";
   
   //***
   // This will fill a buffer the full width of valid samples * tileHeight().
   //***
   m_bufferRect.set_ul(origin);
   m_bufferRect.set_lry(min((origin.y + size.y - 1),
                             m_rasterInfo.imageRect().lr().y));
   m_bufferRect.set_lrx(min((origin.x + size.x - 1),
                             m_rasterInfo.imageRect().lr().x));
   
   rspf_sint64 currentLine = origin.y;

   // Start seek position.
   std::streamoff offset = ( m_rasterInfo.offsetToFirstValidSample() +
                             currentLine * m_rasterInfo.bytesPerRawLine() *
                             m_rasterInfo.numberOfBands() +
                             origin.x * m_rasterInfo.bytesPerPixel() );

   //---
   // Loop through and process lines. 
   //---
   rspf_uint64 linesProcessed = 0;
   
   std::streamsize buffer_width = m_bufferRect.width() * m_rasterInfo.bytesPerPixel();
   
   rspf_uint8* buf = m_buffer;

#if 0 /* Please leave for debug.  (drb) */
   rspfNotify(rspfNotifyLevel_DEBUG)
      << "\nDEBUG:"
      << "\norigin:                 " << origin
      << "\nSeek position:          " << offset
      << "\nStarting line number:   " << currentLine
      << "\nbuffer_width:           " << buffer_width << std::endl;
#endif

   rspf_uint64 height    = size.y;
   rspf_sint64 num_bands = m_rasterInfo.numberOfBands();
   
   while ((currentLine <= static_cast<rspf_sint64>(m_rasterInfo.imageRect().lr().y)) &&
          linesProcessed < height)
   {
      for (rspf_int32 band = 0; band < num_bands; ++band)
      {
         //***
         // Seek to line.
         //***
         m_fileStrList[0]->seekg(offset, ios::beg);
         if (!m_fileStrList[0])
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:\n"
               << " Seek error!  Returning with error..." << std::endl;
            return false;
         }
         
         // Read the line of image data.   
         m_fileStrList[0]->read( (char*)buf, buffer_width );

         if ( m_fileStrList[0]->gcount() != buffer_width ) 
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN) << MODULE << "\nERROR:  Reading image line."
                                               << "\ncurrentLine:  " << currentLine << std::endl;
            return false;
         }

         buf += buffer_width;
         offset += m_rasterInfo.bytesPerRawLine();

      } // End of band loop.
      
      ++linesProcessed;
      ++currentLine;
   }
   
   return true;
}
   
//*******************************************************************
// Private Method:
//*******************************************************************
bool rspfGeneralRasterTileSource::fillBSQ(const rspfIpt& origin,
                                           const rspfIpt& size)
{
   static const char MODULE[] = "rspfGeneralRasterTileSource::fillBSQ";

   // This will fill a buffer the full width of valid samples * tileHeight().

   m_bufferRect.set_ul(origin);
   
   m_bufferRect.set_lry(min((origin.y + size.y -1),
                             m_rasterInfo.imageRect().lr().y));
   m_bufferRect.set_lrx(min((origin.x + size.x - 1),
                             m_rasterInfo.imageRect().lr().x));
   
   // Start seek position.
   std::streamoff startSeekPosition
      = m_rasterInfo.offsetToFirstValidSample() +
        origin.y * m_rasterInfo.bytesPerRawLine() +
        origin.x * m_rasterInfo.bytesPerPixel();

   std::streamsize buffer_width = m_bufferRect.width() * m_rasterInfo.bytesPerPixel();
   
   rspf_uint8* buf = (rspf_uint8*)m_buffer;

   std::streamoff bandOffset
      = m_rasterInfo.bytesPerRawLine() * m_rasterInfo.rawLines();

#if 0 /* Please leave for debug. (drb) */
   rspfNotify(rspfNotifyLevel_DEBUG)
      << "\nDEBUG:"
      << "\norigin:                 " << origin
      << "\nSeek position:          " << startSeekPosition
      << "\nStarting line number:   " << origin.y
      << "\nbuffer_width:           " << buffer_width
      << "\nbytesPerRawLine():      "
      << m_rasterInfo.bytesPerRawLine()
      << "\nm_rasterInfo.offsetToFirstValidSample():  "
      << m_rasterInfo.offsetToFirstValidSample()
      << "\nbandOffset: " << bandOffset << std::endl;
#endif

   rspf_int32 num_bands = m_rasterInfo.numberOfBands();
   rspf_int32 height    = size.y;

   // Band loop:
   for (rspf_int32 band = 0; band < num_bands; ++band)
   {
      rspf_sint64 currentLine    = origin.y;
      rspf_sint64 linesProcessed = 0;

      std::streamoff offset = startSeekPosition + (band * bandOffset);

      // Line loop:
      while (currentLine <= m_rasterInfo.imageRect().lr().y &&
             linesProcessed < height)
      {
         // Seek to line.
         m_fileStrList[0]->seekg(offset, ios::beg);
         if (!m_fileStrList[0])
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:\n"
               << " Seek error!  Returning with error..." << std::endl;
            return false;
         }
         
         // Read the line of image data.   
         m_fileStrList[0]->read( (char*)buf, buffer_width );
         if ( m_fileStrList[0]->gcount() != buffer_width ) 
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << "\nERROR:  Reading image line."
               << "\ncurrentLine:  " << currentLine << std::endl;
            return false;
         }

         // Increment everybody accordingly.
         buf += buffer_width;
         offset += m_rasterInfo.bytesPerRawLine();
         ++linesProcessed;
         ++currentLine;

      } // End of line loop.

   } // End of band loop.
   
   return true;
}

//*******************************************************************
// Private Method:
//*******************************************************************
bool rspfGeneralRasterTileSource::fillBsqMultiFile(const rspfIpt& origin, const rspfIpt& size)
{
   static const char MODULE[] = "rspfGeneralRasterTileSource::fillBsqMultiFile";

   if (traceDebug()) CLOG << " Entered..." << std::endl;
   

   // This will fill a buffer the full width of valid samples * tileHeight().
   m_bufferRect.set_ul(origin);
   
   m_bufferRect.set_lry(min((origin.y + size.y -1),
                             m_rasterInfo.imageRect().lr().y));
   m_bufferRect.set_lrx(min((origin.x + size.x - 1),
                             m_rasterInfo.imageRect().lr().x));
   
   //---
   // Start seek position.
   //---
   std::streamoff startSeekPosition = m_rasterInfo.offsetToFirstValidSample() +
      origin.y * m_rasterInfo.bytesPerRawLine() +
      origin.x * m_rasterInfo.bytesPerPixel();
   
   //---
   // Loop through and process lines. 
   //---
   std::streamsize buffer_width = m_bufferRect.width() * m_rasterInfo.bytesPerPixel();
   
   rspf_uint8* buf = (rspf_uint8*)m_buffer;

#if 0
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nDEBUG:"
         << "\norigin:                 " << origin
         << "\nSeek position:          " << startSeekPosition
         << "\nStarting line number:   " << origin.y
         << "\nbuffer_width:           " << buffer_width
         << "\nbuffer_rect:            " << m_bufferRect
         << "\nbytesPerRawLine():      "
         << m_rasterInfo.bytesPerRawLine()
         << "\nm_rasterInfo.offsetToFirstValidSample():  "
         << m_rasterInfo.offsetToFirstValidSample() << std::endl;
   }
#endif

   // rspf_int32 num_bands = m_rasterInfo.numberOfBands();
   std::vector<rspf_uint32>::const_iterator bandIter = m_outputBandList.begin();
   while ( bandIter != m_outputBandList.end() )
   {
      rspf_int32 currentLine    = origin.y;
      rspf_int32 linesProcessed = 0;
      rspf_int64 offset         = startSeekPosition;
      
      while (currentLine <= m_rasterInfo.imageRect().lr().y && linesProcessed < size.y)
      {
         //---
         // Seek to line.
         //---
         m_fileStrList[ *bandIter ]->seekg(offset, ios::beg);

         if ( !m_fileStrList[ *bandIter ] )
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:\n"
               << " Seek error!  Returning with error..." << std::endl;
            return false;
         }
         
         //---
         // Read the line of image data.   
         //---
         m_fileStrList[ *bandIter ]->read((char*)buf, buffer_width);
         
         if ( m_fileStrList[ *bandIter ]->gcount() != buffer_width) 
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << "\nERROR:  Reading image line."
               << "\ncurrentLine:  " << currentLine << std::endl;
            return false;
         }

         // Increment everybody accordingly.
         buf += buffer_width;
         offset += m_rasterInfo.bytesPerRawLine();
         ++linesProcessed;
         ++currentLine;
         
      } // End of line loop.

      ++bandIter; // Next band...

   } // End: while ( bandIter ! = m_outputBandList.end() )
   
   return true;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfGeneralRasterTileSource::saveState(rspfKeywordlist& kwl,
                                             const char* prefix) const
{   
   // Our stuff:
   m_rasterInfo.saveState(kwl, prefix);

   // Base class:
   bool result = rspfImageHandler::saveState(kwl, prefix);
   
   if ( result && isBandSelector() && m_outputBandList.size() )
   {
      if ( isIdentityBandList( m_outputBandList ) == false )
      {
         // If we're not identity output the bands.
         rspfString bandsString;
         rspf::toSimpleStringList(bandsString, m_outputBandList);
         kwl.add(prefix,
                 rspfKeywordNames::BANDS_KW,
                 bandsString,
                 true);
      }
   }
   
   return result;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfGeneralRasterTileSource::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   bool result = false;
   m_outputBandList.clear();

   if ( rspfImageHandler::loadState(kwl, prefix) )
   {  
      // Set the band list if key is present.
      std::string pfx = ( prefix ? prefix : "" );
      std::string key = rspfKeywordNames::BANDS_KW;
      rspfString value;
      value.string() = kwl.findKey( pfx, key );
      if ( value.size() )
      {
         rspf::toSimpleVector( m_outputBandList, value );
      }
      result = open();
   }
   return result;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspfScalarType rspfGeneralRasterTileSource::getOutputScalarType() const
{
   return m_rasterInfo.getImageMetaData().getScalarType();
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfGeneralRasterTileSource::getTileWidth() const
{
   rspf_uint32 result = 0;
   if ( m_tile.valid() )
   {
      m_tile->getWidth();
   }
   else
   {
      rspfIpt tileSize;
      rspf::defaultTileSize(tileSize);
      result = tileSize.x;
   }
   return result;
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfGeneralRasterTileSource::getTileHeight() const
{
   rspf_uint32 result = 0;
   if ( m_tile.valid() )
   {
      m_tile->getHeight();
   }
   else
   {
      rspfIpt tileSize;
      rspf::defaultTileSize(tileSize);
      result = tileSize.y;
   }
   return result;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool
rspfGeneralRasterTileSource::isValidRLevel(rspf_uint32 reduced_res_level) const
{
   static const char MODULE[] = "rspfGeneralRasterTileSource::isValidRLevel";
   
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
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE
         << " Invalid reduced_res_level:  " << reduced_res_level
         << "\nHighest available:  " << (getNumberOfDecimationLevels() - 1)
         << std::endl;
      return false;
   }
}



//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32
rspfGeneralRasterTileSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      return m_rasterInfo.validLines();
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
rspf_uint32 rspfGeneralRasterTileSource::getNumberOfSamples(
   rspf_uint32 reduced_res_level) const
{
   if (reduced_res_level == 0)
   {
      return m_rasterInfo.validSamples();
   }
   else if (theOverview.valid())
   {
      return theOverview->getNumberOfSamples(reduced_res_level);
   }

   return 0;
}

double rspfGeneralRasterTileSource::getNullPixelValue(rspf_uint32 band) const
{
   return m_rasterInfo.getImageMetaData().getNullPix(band);
}

double rspfGeneralRasterTileSource::getMinPixelValue(rspf_uint32 band)const
{
   return m_rasterInfo.getImageMetaData().getMinPix(band);
}

double rspfGeneralRasterTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   return m_rasterInfo.getImageMetaData().getMaxPix(band);
}

bool rspfGeneralRasterTileSource::open()
{
   static const char MODULE[] = "rspfGeneralRasterTileSource::open";

   if (traceDebug()) CLOG << " Entered..." << std::endl;
   
   bool result = false;
   
   if(isOpen())
   {
      close();
   }
   
   //---
   // Find the header file:
   //
   // We need lines, samples, bands, scalar and interleave at a minimum:
   // 
   // A general raster image requires a keyword list to get essential image
   // information or meta data as its sometimes called.  The meta data file
   // can have four types of extensions: ".omd", ".hdr", ".kwl" and xml.
   // Look for them in that order.
   // Note that the ".omd" extension is for "Ossim Meta Data" and was made
   // up to avoid conflicting with other software packages ".hdr" files.
   //---
   if ( m_rasterInfo.open( theImageFile ) )
   {
      theMetaData = m_rasterInfo.getImageMetaData();
      
      result = initializeHandler();
      if ( result )
      {
         completeOpen();

         if ( isBandSelector() && m_outputBandList.size() && ( isIdentityBandList( m_outputBandList ) == false ) )
         {
            // This does range checking and will pass to overview if open.
            setOutputBandList( m_outputBandList );
         }
      }
   }

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " Exit status: " << (result?"true":"false") << std::endl;
   }
   return result;
}
                                             
bool rspfGeneralRasterTileSource::open( const rspfGeneralRasterInfo& info )
{
   if( isOpen() )
   {
      close();
   }
   
   m_rasterInfo = info;
   
   if( initializeHandler() )
   {
      completeOpen();  

       if ( isBandSelector() && m_outputBandList.size() && ( isIdentityBandList( m_outputBandList ) == false ) )
       { 
          // This does range checking and will pass to overview if open.
          setOutputBandList( m_outputBandList );
       }
   }
   else
   {
      return false;
   }
   
   return true;
}

bool rspfGeneralRasterTileSource::initializeHandler()
{
   //---
   // This private method assumes that "m_rasterInfo" object has been
   // initialized.  Note that "close() should have already been called if
   // there was an open file prior to this.
   //---
   std::vector<rspfFilename> aList = m_rasterInfo.getImageFileList();

   for (rspf_uint32 i=0; i<aList.size(); ++i)
   {
      rspfFilename f = aList[i];
      
      // open it...
      rspfRefPtr<rspfIFStream> is = rspfStreamFactoryRegistry::instance()->
         createNewIFStream(f, std::ios::in|std::ios::binary);

      // check the stream...
      if(is.valid())
      {
         // Check the file stream.
         if ( is->fail() )
         {
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfGeneralRasterTileSource::open" << " ERROR:\n"
               << "Cannot open:  " << f.c_str() << std::endl;
            is = 0;
            return false;
         }
      }

      // Check the file size (removed).

      m_fileStrList.push_back(is); // Add it to the list...
   }

   if ((aList.size()==1) && theImageFile.empty())
   {
      theImageFile = aList[0];
   }

   // Set the buffer interleave type.
   m_bufferInterleave = m_rasterInfo.interleaveType();
   if (m_bufferInterleave == RSPF_BSQ_MULTI_FILE)
   {
      m_bufferInterleave = RSPF_BSQ;
   }

   if ( m_outputBandList.empty() )
   {
      // Set starting output band list to identity.
      rspfImageSource::getOutputBandList( m_outputBandList );
   }

   //---
   // Get the byte order of the image data and host machine.  If different,
   // set the swap bytes flag...
   //---
   if (m_rasterInfo.getImageDataByteOrder() != rspf::byteOrder())
   {
      m_swapBytesFlag = true;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGeneralRasterTileSource::initializeHandler()" << " DEBUG:"
         << "\nScalar type:  "
         << rspfScalarTypeLut::instance()->
         getEntryString(m_rasterInfo.getImageMetaData().getScalarType())
         << "\nByte swapping is "
         << (m_swapBytesFlag?"enabled.":"not enabled.")
         // << "\nm_bufferSizeInPixels:  " << m_bufferSizeInPixels
         // << "\nbuffer size:  " << buffer_size
         << "\nRasterInfo:\n";
      m_rasterInfo.print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
   
   return true;
}

bool rspfGeneralRasterTileSource::isOpen() const
{
   bool result = false;
   if (m_fileStrList.size() > 0)
   {
      if(m_fileStrList[0].valid())
      {
         result = !(m_fileStrList[0]->fail());
      }
   }
   return result;
}

void rspfGeneralRasterTileSource::close()
{
   rspfImageHandler::close();  // base class

   m_tile = 0; // Not a leak, ref ptr.
   
   if ( m_buffer )
   {
      delete [] m_buffer;
      m_buffer = 0;
      m_bufferSizeInPixels = 0; // Must zero out for check in getTile method.
   }

   if ( m_lineBuffer )
   {
      delete [] m_lineBuffer;
      m_lineBuffer = 0;
   }

   std::vector<rspfRefPtr<rspfIFStream> >::iterator is = m_fileStrList.begin();
   while (is != m_fileStrList.end())
   {
      (*is)->close();
      // delete (*is);
      // (*is) = 0;
      ++is;
   }
   m_fileStrList.clear();
}

rspf_uint32 rspfGeneralRasterTileSource::getImageTileWidth() const
{
   return 0;
}

rspf_uint32 rspfGeneralRasterTileSource::getImageTileHeight() const
{
   return 0;
}

rspfString rspfGeneralRasterTileSource::getShortName()const
{
   return rspfString("ras");
}

rspfString rspfGeneralRasterTileSource::getLongName()const
{
   return rspfString("general raster reader");
}

rspf_uint32 rspfGeneralRasterTileSource::getNumberOfInputBands() const
{
   return m_rasterInfo.getImageMetaData().getNumberOfBands();
}

rspf_uint32 rspfGeneralRasterTileSource::getNumberOfOutputBands() const
{
   rspf_uint32 result = 0;
   if ( isBandSelector() && m_outputBandList.size() )
   {
      result = m_outputBandList.size();
   }
   else
   {
      result = m_rasterInfo.getImageMetaData().getNumberOfBands();
   }  
   return result;
}

rspfKeywordlist rspfGeneralRasterTileSource::getHdrInfo(rspfFilename hdrFile)
{
   rspfKeywordlist kwl;
   char delimeter = ' ';
   kwl.change_delimiter(delimeter);
   kwl.addFile(hdrFile);
   kwl.downcaseKeywords();

   rspfKeywordlist geoKwl;
   rspf_uint32 lines = 0;
   rspf_uint32 samples = 0;
   rspf_float32 noData = -9999;
   rspfString scalarType = "rspf_uint8";
   rspf_int32 numBands = 1;
   // rspf_int32 skipBytes = 0;
   rspf_int32 numBits = -1; 
   rspfString chPixelType = "N"; // not defined
   rspfString interleaveType = "BIL";
   rspfString byteOrder;
   bool noDataFound = false;

   const char* lookup = kwl.find("ncols");
   if (lookup)
   {
      samples = rspfString(lookup).toUInt32();
      geoKwl.add(rspfKeywordNames::NUMBER_SAMPLES_KW, samples);
   }

   lookup = kwl.find("nrows");
   if (lookup)
   {
      lines = rspfString(lookup).toUInt32();
      geoKwl.add(rspfKeywordNames::NUMBER_LINES_KW, lines);
   }

   // lookup = kwl.find("skipbytes");
   // if (lookup)
   // {
   //    skipBytes = rspfString(lookup).toInt();
   // }

   lookup = kwl.find("nbands");
   if (lookup)
   {
      numBands = rspfString(lookup).toInt();
   }

   lookup = kwl.find("nodata");
   if (lookup)
   {
      noData = rspfString(lookup).toFloat32();
      noDataFound = true;
   }
   lookup = kwl.find("nodata_value");
   if (lookup)
   {
      noData = rspfString(lookup).toFloat32();
      noDataFound = true;
   }

   lookup = kwl.find("nbits");
   if (lookup)
   {
      numBits = rspfString(lookup).toInt();
   }

   lookup = kwl.find("pixeltype");
   if (lookup)
   {
      chPixelType = rspfString(lookup);
   }

   lookup = kwl.find("layout");
   if (lookup)
   {
      interleaveType = rspfString(lookup);
   }

   lookup = kwl.find("byteorder");
   if (lookup)
   {
      byteOrder = rspfString(lookup);
   }

   if (numBits == -1)
   {
      FILE* fp;
      rspf_int64 size = 0;
      fp = fopen(theImageFile.c_str(), "r");
      if (fp != 0)
      {
         fseek(fp, 0, SEEK_END);
         size = ftell(fp);
      }
      fclose(fp);

      if (lines > 0 && samples > 0)
      {
         rspf_int32 numBytes = size/samples/lines/numBands;
         if( numBytes > 0 && numBytes != 3 )
         {
            numBits = numBytes*8;

            if( numBytes == 4 )
            {
               chPixelType = "F";
            }
         }
      }
   }

   if( numBits == 16 )
   {
      if (chPixelType == "S")
      {
         scalarType = "rspf_sint16";
      }
      else
      {
         scalarType = "rspf_uint16"; // default
      }
   }
   else if( numBits == 32 )
   {
      if( chPixelType == "S")
      {
         scalarType = "rspf_sint32";
      }
      else if( chPixelType == "F")
      {
         scalarType = "rspf_float32";
      }
      else
      {
         scalarType = "rspf_uint32"; // default 
      }
   }
   else if( numBits == 8 )
   {
      scalarType = "rspf_uint8";
      numBits = 8;
   }
   else if( numBits < 8 && numBits >= 1 )
   {
      scalarType = "rspf_uint8";
   }
   else if(numBits == -1)
   {
      if( chPixelType == "F")
      {
         scalarType = "rspf_float32";
         numBits = 32;
      }
      else
      {
         scalarType = "rspf_uint8";
         numBits = 8;
      }
   }

   if (noDataFound)
   {
      for (rspf_int32 i = 0; i < numBands; i++)
      {
         rspfString prefix = "band" + rspfString::toString(i+1) + ": ";
         geoKwl.add(prefix, rspfKeywordNames::NULL_VALUE_KW, noData);
      }
   }

   geoKwl.add(rspfKeywordNames::NUMBER_BANDS_KW, numBands);
   geoKwl.add(rspfKeywordNames::SCALAR_TYPE_KW, scalarType);
   geoKwl.add(rspfKeywordNames::INTERLEAVE_TYPE_KW, interleaveType);

   return geoKwl;
}

rspfKeywordlist rspfGeneralRasterTileSource::getXmlInfo(rspfFilename xmlFile)
{
   rspfKeywordlist kwl;
   rspfFgdcXmlDoc file;
   if (file.open(xmlFile))
   {

      rspfString scalarType = "rspf_uint8";
      rspf_int32 numBits = -1; 
      rspfString interleaveType = "BIL";

      rspfIpt size;
      rspf_int32 samples = 0;
      rspf_int32 lines = 0;
      if (file.getImageSize(size))
      {
         samples = size.x;
         lines = size.y;
      }
      if (samples > 0)
      {
         kwl.add(rspfKeywordNames::NUMBER_SAMPLES_KW, samples);
      }
      if (lines > 0)
      {
         kwl.add(rspfKeywordNames::NUMBER_LINES_KW, lines);
      }

      rspf_int32 bands = file.getNumberOfBands();
      if (bands > 0)
      {
         kwl.add(rspfKeywordNames::NUMBER_BANDS_KW, bands);
      }
      else
      {
         if (samples > 0 && lines > 0)//if there is no bands info but samples and lines info, default number of bands to 1
         {
            bands = 1;
            kwl.add(rspfKeywordNames::NUMBER_BANDS_KW, bands);
         }
      }

      rspfString eainfo;
      file.getPath("/metadata/eainfo/detailed/enttyp/enttypd", eainfo);

      if (numBits == -1)
      {
         if ( (lines > 0) && (samples > 0) && (bands > 0) )
         {
            rspf_int64 size = theImageFile.fileSize();            
            rspf_int32 numBytes = size/samples/lines/bands;
            if( numBytes > 0 && numBytes != 3 )
            {
               numBits = numBytes*8;
            }
         }
      }

      if( numBits == 16 )
      {
         scalarType = "rspf_uint16"; // default
      }
      else if( numBits == 32 )
      {
         if(eainfo.contains("float"))
         {
            scalarType = "rspf_float32";
         }
         else
         {
            scalarType = "rspf_uint32"; // default 
         }
      }
      else if( numBits == 8 )
      {
         scalarType = "rspf_uint8";
         numBits = 8;
      }
      else if( numBits < 8 && numBits >= 1 )
      {
         scalarType = "rspf_uint8";
      }

      kwl.add(rspfKeywordNames::SCALAR_TYPE_KW, scalarType);
      kwl.add(rspfKeywordNames::INTERLEAVE_TYPE_KW, interleaveType);
   }
   return kwl;
}

rspfRefPtr<rspfImageGeometry> rspfGeneralRasterTileSource::getImageGeometry()
{
   if ( !theGeometry.valid() )
   {
      // Check for external geom:
      theGeometry = getExternalImageGeometry();

      if ( !theGeometry.valid() )
      {
         theGeometry = new rspfImageGeometry();
         
         rspfString ext = theImageFile.ext();
         rspfFilename hdrFile = theImageFile;
         rspfFilename xmlFile = theImageFile;
         hdrFile = hdrFile.setExtension("hdr");
         xmlFile = xmlFile.setExtension("xml");
         if (hdrFile.exists())
         {
            rspfKeywordlist geoKwl;
            rspfKeywordlist kwl(hdrFile, ' ');
            kwl.downcaseKeywords();
            
            rspf_uint32 lines = 0;
            rspf_uint32 samples = 0;
            rspf_float32 ll_lon = 0.0;
            rspf_float32 ll_lat = 0.0;
            rspf_float32 xCellSize = 1.0;
            rspf_float32 yCellSize = 1.0;
            
            const char* lookup = kwl.find("ncols");
            if (lookup)
            {
               samples = rspfString(lookup).toUInt32();
               geoKwl.add(rspfKeywordNames::NUMBER_SAMPLES_KW, samples);
            }
            
            lookup = kwl.find("nrows");
            if (lookup)
            {
               lines = rspfString(lookup).toUInt32();
               geoKwl.add(rspfKeywordNames::NUMBER_LINES_KW, lines);
            }
            
            lookup = kwl.find("cellsize");
            if (lookup)
            {
               xCellSize = rspfString(lookup).toFloat32();
               yCellSize = xCellSize;
               geoKwl.add(rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT, yCellSize);
               geoKwl.add(rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON, xCellSize);
            }
            
            lookup = kwl.find("xdim");
            if (lookup)
            {
               xCellSize = rspfString(lookup).toFloat32();
               geoKwl.add(rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON, xCellSize);
            }
            
            lookup = kwl.find("ydim");
            if (lookup)
            {
               yCellSize = rspfString(lookup).toFloat32();
               geoKwl.add(rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT, yCellSize);
            }
            
            lookup = kwl.find("xllcenter");
            if (lookup)
            {
               rspf_float32 centerX = rspfString(lookup).toFloat32();
               ll_lon = centerX + xCellSize * 0.5;
               geoKwl.add(rspfKeywordNames::TIE_POINT_LON_KW, ll_lon);
            }
            
            lookup = kwl.find("yllcenter");
            if (lookup)
            {
               rspf_float32 centerY = rspfString(lookup).toFloat32();
               ll_lat = (centerY + (lines - 1) * yCellSize) + yCellSize * 0.5;
               geoKwl.add(rspfKeywordNames::TIE_POINT_LAT_KW, ll_lat);
            }
            
            lookup = kwl.find("xllcorner");
            if (lookup)
            {
               ll_lon = rspfString(lookup).toFloat32();
               geoKwl.add(rspfKeywordNames::TIE_POINT_LON_KW, ll_lon);
            }
            
            lookup = kwl.find("yllcorner");
            if (lookup)
            {
               rspf_uint32 centerY = rspfString(lookup).toFloat32();
               ll_lat = centerY + lines * yCellSize;
               geoKwl.add(rspfKeywordNames::TIE_POINT_LAT_KW, ll_lat);
            }
            
            lookup = kwl.find("ulxmap");
            if (lookup)
            {
               ll_lon = rspfString(lookup).toFloat32();
               geoKwl.add(rspfKeywordNames::TIE_POINT_LON_KW, ll_lon);
            }
            
            lookup = kwl.find("ulymap");
            if (lookup)
            {
               rspf_uint32 centerY = rspfString(lookup).toFloat32();
               ll_lat = centerY + lines * yCellSize;
               geoKwl.add(rspfKeywordNames::TIE_POINT_LAT_KW, ll_lat);
            }
            
            kwl.add(rspfKeywordNames::ORIGIN_LATITUDE_KW, ll_lat);
            
            geoKwl.add(rspfKeywordNames::TYPE_KW, "rspfEquDistCylProjection");
            
            geoKwl.add(rspfKeywordNames::DATUM_KW, rspfDatumFactory::instance()->wgs84()->
                       code());
            
            rspfRefPtr<rspfProjection> proj = rspfMapProjectionFactory::instance()->
               createProjection(geoKwl);

            if ( proj.valid() )
            {
               theGeometry->setProjection(proj.get());
            }
         }
         else if (xmlFile.exists())
         {
            rspfFgdcXmlDoc file;
            if ( file.open(xmlFile) )
            {
               rspfRefPtr<rspfProjection> proj = file.getProjection();
               if ( !proj.valid() )
               {
                  proj = file.getGridCoordSysProjection();
               }
               if ( proj.valid() )
               {
                  theGeometry->setProjection(proj.get());
               }
            }
            
         } // xml file exist...

      } // Matches second if ( !theGeometry.valid() )

      //---
      // WARNING:
      // Must have theGeometry at this point or the next call to
      // rspfImageGeometryRegistry::extendGeometry will put us in an infinite loop
      // as it does a recursive call back to rspfImageHandler::getImageGeometry().
      //---         
      
      // Check for set projection.
      if ( !theGeometry->getProjection() )
      {
         // Try factories for projection.
         rspfImageGeometryRegistry::instance()->extendGeometry(this);
      }

      // Set image things the geometry object should know about.
      initImageParameters( theGeometry.get() );
      
   } // Matches first if ( !theGeometry.valid() )

   return theGeometry;
}

bool rspfGeneralRasterTileSource::isBandSelector() const
{
   bool result = false;
   if ( m_rasterInfo.interleaveType() == RSPF_BSQ_MULTI_FILE )
   {
      result = true;
   }
   else if ( m_rasterInfo.interleaveType() == RSPF_BIP )
   {
      // Added for hyper spectral data with 256 bands.
      const rspf_int32 THRESHOLD = 4;
      if ( m_rasterInfo.numberOfBands() >= THRESHOLD )
      {
         result = true;
      }
   }
   if ( result && theOverview.valid() )
   {
      result = theOverview->isBandSelector();
   }   
   return result;
}

bool rspfGeneralRasterTileSource::setOutputBandList(const std::vector<rspf_uint32>& band_list)
{
   bool result = false;
   if ( isBandSelector() )
   {
      // Making a copy as passed in list could be our m_outputBandList.
      std::vector<rspf_uint32> inputList = band_list;
      result = rspfImageHandler::setOutputBandList( inputList, m_outputBandList );
      if ( result && m_tile.valid() )
      {
         if ( m_tile->getNumberOfBands() != m_outputBandList.size() )
         {
            m_tile = 0; // Force a reinitialize on next getTile.
         }
      }
   }
   return result;
}

void rspfGeneralRasterTileSource::getOutputBandList(std::vector<rspf_uint32>& bandList) const
{
   bandList = m_outputBandList;
}

void rspfGeneralRasterTileSource::allocateTile()
{
   m_tile = 0;
   rspf_uint32 bands = 0;
   if ( m_outputBandList.empty() )
   {
      bands = m_rasterInfo.numberOfBands();
   }
   else
   {
      bands = m_outputBandList.size();
   }
   
   if ( bands )
   {
      m_tile = rspfImageDataFactory::instance()->create(
         this, m_rasterInfo.getImageMetaData().getScalarType(), bands );
      
      if ( m_tile.valid() )
      {
         // These values can be overridden by loadState...
         for(rspf_uint32 band = 0; band < bands; ++ band)
         {
            m_tile->setNullPix(m_rasterInfo.getImageMetaData().getNullPix(band), band);
            m_tile->setMinPix(m_rasterInfo.getImageMetaData().getMinPix(band), band);
            m_tile->setMaxPix(m_rasterInfo.getImageMetaData().getMaxPix(band), band);
         }
         m_tile->initialize(); // This does a makeBlank().
      }
   }
}

void rspfGeneralRasterTileSource::allocateBuffer( const rspfImageData* tile )
{
   if( m_buffer )
   {
      delete [] m_buffer;
      m_buffer = 0;
      m_bufferSizeInPixels = 0; // Must zero out for check in getTile method.
   }
   if ( m_lineBuffer )
   {
      delete [] m_lineBuffer;
      m_lineBuffer = 0;
   }
   
   if ( tile )
   {
      // Store the size of the buffer in pixels for swapping bytes.
      m_bufferSizeInPixels = tile->getSize();
      if ( m_bufferSizeInPixels )
      {
         // Initialize buffer. This is bytes, not pixels.
         m_buffer = new rspf_uint8[ tile->getSizeInBytes() ];
         
         // Zero out the buffer rect.
         m_bufferRect = rspfIrect(0, 0, 0, 0);
      }

      if ( m_rasterInfo.interleaveType() == RSPF_BIP )
      {
         // Big enough to hold a whole line all bands.
         rspf_uint32 widthInBytes =
            tile->getWidth() * m_rasterInfo.getImageMetaData().getNumberOfBands() *
            m_rasterInfo.getImageMetaData().getBytesPerPixel();
         
         m_lineBuffer = new rspf_uint8[ widthInBytes ];
      }
   }
}
