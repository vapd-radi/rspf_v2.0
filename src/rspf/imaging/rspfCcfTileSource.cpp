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
// Contains class definition for rspfCcfTileSource.
//*******************************************************************
//  $Id: rspfCcfTileSource.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <algorithm>
#include <rspf/imaging/rspfCcfTileSource.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfInterleaveTypeLut.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>

using namespace std;

RTTI_DEF1(rspfCcfTileSource, "rspfCcfTileSource", rspfImageHandler);

//***
// Define Trace flags for use within this file:
//***

static rspfTrace traceExec  ("rspfCcfTileSource:exec");
static rspfTrace traceDebug ("rspfCcfTileSource:debug");

// For interleave type enum to string conversions.
static const rspfInterleaveTypeLut ILUT;

rspfCcfTileSource::rspfCcfTileSource()
   :
      rspfImageHandler(),
      theCcfHead(),
      theTile(NULL),
      theChipBuffer(NULL),
      theFileStr(NULL),
      theOutputBandList(1),
      theByteOrder(rspf::byteOrder())
{}

rspfCcfTileSource::~rspfCcfTileSource()
{
  close();
}

rspfRefPtr<rspfImageData> rspfCcfTileSource::getTile(
   const  rspfIrect& rect, rspf_uint32 resLevel)
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

bool rspfCcfTileSource::getTile(rspfImageData* result,
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
         
         rspfIrect image_rect = theCcfHead.imageRect(resLevel);
   
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
            
            // Load the tile buffer with data from the ccf.
            status = fillBuffer(tile_rect,
                                clip_rect,
                                image_rect,
                                resLevel,
                                result);
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
bool rspfCcfTileSource::fillBuffer(const rspfIrect& tile_rect,
                                    const rspfIrect& clip_rect,
                                    const rspfIrect& image_rect,
                                    rspf_uint32 reduced_res_level,
                                    rspfImageData* tile)
{
   bool status = false;
   
   //***
   // Determine the pixel type and make the appropriate tiles.
   //***
   switch (theCcfHead.pixelType())
   {
      case RSPF_UCHAR:
         status = fillUcharBuffer(tile_rect,
                                  clip_rect,
                                  image_rect,
                                  reduced_res_level,
                                  tile);
         break;
      case RSPF_USHORT11:
      case RSPF_USHORT16:
         status = fillUshortBuffer(tile_rect,
                                   clip_rect,
                                   image_rect,
                                   reduced_res_level,
                                   tile);
         break;
      default:
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfCcfTileSource fillBuffer ERROR:"
            << "\nUnsupported pixel type!"
            << "\nBuffer not filled.  Returning error." << endl;
         break;
   }

   return status;
}

//*******************************************************************
// Private Method:
//*******************************************************************
bool rspfCcfTileSource::fillUcharBuffer(const rspfIrect& tile_rect,
                                         const rspfIrect& clip_rect,
                                         const rspfIrect& /* image_rect */,
                                         rspf_uint32 reduced_res_level,
                                         rspfImageData* tile)
{
   if(!isOpen()) return false;
      
   const rspf_uint32 TILE_SIZE_PER_BAND     = tile->getSizePerBand();
   const rspf_uint32 CHIP_SIZE_PER_BAND     = theCcfHead.chipSizePerBand();
   const rspf_uint32 SAMPLES_PER_CHIP       = theCcfHead.samplesPerChip();
   const rspf_uint32 LINES_PER_CHIP         = theCcfHead.linesPerChip();
   const rspf_uint32 BYTES_PER_CHUNK        = theCcfHead.bytesPerChunk();
   const rspf_uint32 BYTES_PER_CHIP         = theCcfHead.bytesPerChip();
   const rspf_uint32 CHIPS_IN_SAMPLE_DIR    = theCcfHead.sampleChipsPerChunk();
   const rspf_uint32 CHIPS_IN_LINE_DIR      = theCcfHead.lineChipsPerChunk();
   const rspf_uint32 NUMBER_OF_INPUT_BANDS  = theCcfHead.numberOfBands();
   const rspf_uint32 CHUNKS_IN_SAMPLE_DIR   = theCcfHead.chunksInSampleDir(reduced_res_level);
   const rspf_uint32 CHUNK_SIDE_SIZE        = SAMPLES_PER_CHIP *
                                        CHIPS_IN_SAMPLE_DIR;
   const rspf_uint32 CHIP_OFFSET_TO_END     = SAMPLES_PER_CHIP - 1;

   //***
   // Shift the upper left corner of the "clip_rect" to the an even chunk
   // boundry.
   //***
   rspfIpt chunkOrigin = clip_rect.ul();
   adjustToStartOfChunk(chunkOrigin);

   //***
   // Calculate the number of chunks needed in the line/sample directions.
   //***
   rspf_uint32 size_in_x
      = static_cast<rspf_uint32>(clip_rect.lr().x - chunkOrigin.x + 1);
   rspf_uint32 size_in_y
      = static_cast<rspf_uint32>(clip_rect.lr().y - chunkOrigin.y + 1);
   rspf_uint32 w = (CHIPS_IN_SAMPLE_DIR*SAMPLES_PER_CHIP);
   rspf_uint32 h = (CHIPS_IN_LINE_DIR*LINES_PER_CHIP);
   rspf_uint32 chunks_in_x_dir = size_in_x / w  + (size_in_x % w  ? 1 : 0);
   rspf_uint32 chunks_in_y_dir = size_in_y / h + (size_in_y % h ? 1 : 0);
   
   
   //***
   // Get the start of data for the "reduced_res_level" passed in.
   //***
   streampos startOfData = theCcfHead.startOfData(reduced_res_level);
   rspfIpt ulChunkPt = chunkOrigin;

   // Chunk loop in line direction.
   for (rspf_uint32 y=0; y<chunks_in_y_dir; ++y)
   {
      ulChunkPt.x = chunkOrigin.x;

      // Chunk loop in sample direction.
      for (rspf_uint32 x=0; x<chunks_in_x_dir; ++x)
      {
         //***
         // Sequence through the chips in the chunk.
         // Grab the ccf data and stuff the tile buffer.
         //***
         rspfIpt chip_pt = ulChunkPt;

         // Chip loop in line direction.
         for (rspf_uint32 y_chip=0; y_chip<CHIPS_IN_LINE_DIR; ++y_chip) 
         {
            chip_pt.x = ulChunkPt.x;  // Reset x back to front of chunk.

            // Chip loop in sample direction.
            for (rspf_uint32 x_chip=0; x_chip<CHIPS_IN_SAMPLE_DIR; ++x_chip)
            {
               rspfIrect chip_rect(chip_pt.x,
                               chip_pt.y,
                               chip_pt.x + CHIP_OFFSET_TO_END,
                               chip_pt.y + CHIP_OFFSET_TO_END);
               
               if (chip_rect.intersects(clip_rect))
               {
                  //***
                  // Some point in the chip intersect the tile so grab the
                  // data.
                  //***

                  //***
                  // Get the seek position for the chunk.
                  //***
                  streampos tmp_offset
                     = ( ulChunkPt.y / CHUNK_SIDE_SIZE * BYTES_PER_CHUNK *
                         CHUNKS_IN_SAMPLE_DIR ) +
                     ( ulChunkPt.x / CHUNK_SIDE_SIZE * BYTES_PER_CHUNK );

                  streampos seek_position = startOfData + tmp_offset;

                  // Now move it to the chip.
                  tmp_offset
                     = ( y_chip *  BYTES_PER_CHIP * CHIPS_IN_SAMPLE_DIR ) +
                     (x_chip * BYTES_PER_CHIP);

                  seek_position += tmp_offset;

                  // Seek to the chip
                  theFileStr->seekg(seek_position, ios::beg);

                  //***
                  // Read the chip from the ccf file into the chip buffer.
                  // This will get all the bands.  Bands are interleaved by
                  // chip.
                  //***
                  if (!theFileStr->read((char*)theChipBuffer, BYTES_PER_CHIP))
                  {
                     theErrorStatus = rspfErrorCodes::RSPF_ERROR;
                     cerr << "rspfCcfTileSource fillUchBuffer READ ERROR!"
                          << "\nReturning from method." << endl;
                     return false;
                  }
                  
                  rspfIrect chip_clip_rect = clip_rect.clipToRect(chip_rect);

                  //***
                  // Band loop in sample direction.  Bands are interleaved by
                  // chip.
                  //***
                  for (rspf_uint32 band=0; band<NUMBER_OF_INPUT_BANDS; ++band)
                  {
                     //***
                     // Get a pointer positioned at the first valid pixel in
                     // the chip.
                     //***
                     rspf_uint8* s
                        = theChipBuffer +
                        band * CHIP_SIZE_PER_BAND + 
                        (chip_clip_rect.ul().y - chip_rect.ul().y) *
                        SAMPLES_PER_CHIP + chip_clip_rect.ul().x -
                        chip_rect.ul().x;
                                
                     //***
                     // Get a pointer positioned at the first valid pixel in
                     // the tile.
                     //***
                     rspf_uint8* d
                        = (rspf_uint8*)tile->getBuf() +
                        band * TILE_SIZE_PER_BAND +
                        (chip_clip_rect.ul().y - tile_rect.ul().y) *
                        tile->getWidth() + chip_clip_rect.ul().x -
                        tile_rect.ul().x;      

                     // Line loop through a chip.
                     for (rspf_int32 chip_line = chip_clip_rect.ul().y;
                          chip_line <= chip_clip_rect.lr().y;
                          chip_line++)
                     {
                        // Sample loop through a chip.
                        rspf_uint32 i=0;
                        for (rspf_int32 chip_sample = chip_clip_rect.ul().x;
                             chip_sample <= chip_clip_rect.lr().x;
                             chip_sample++)
                        {
                           d[i] = s[i];
                           i++;
                        }

                        // Increment the pointers by one line.
                        s += SAMPLES_PER_CHIP;
                        d += tile->getWidth();
                     }
                     
                  } // End of band loop.
                  
               } // End of if (chip_rect.intersects(clip_rect))
               
               chip_pt.x += SAMPLES_PER_CHIP;
               
            } // End of chip loop in the sample direction.

           chip_pt.y += LINES_PER_CHIP; 
            
         } // End of chip loop in the line direction.
         
         ulChunkPt.x += CHUNK_SIDE_SIZE;
         
      }  // End of chunk loop in the sample direction.

      ulChunkPt.y += CHUNK_SIDE_SIZE;
      
   }  // End of chunk loop in the line direction.

   return true;
}

//*******************************************************************
// Private Method:
//*******************************************************************
bool rspfCcfTileSource::fillUshortBuffer(const rspfIrect& tile_rect,
                                          const rspfIrect& clip_rect,
                                          const rspfIrect& /* image_rect */,
                                          rspf_uint32 reduced_res_level,
                                          rspfImageData* tile)
{
   const rspf_uint32 TILE_SIZE_PER_BAND     = tile->getSizePerBand();
   const rspf_uint32 CHIP_SIZE_PER_BAND     = theCcfHead.chipSizePerBand();
   const rspf_uint32 SAMPLES_PER_CHIP       = theCcfHead.samplesPerChip();
   const rspf_uint32 LINES_PER_CHIP         = theCcfHead.linesPerChip();
   const rspf_uint32 BYTES_PER_CHUNK        = theCcfHead.bytesPerChunk();
   const rspf_uint32 BYTES_PER_CHIP         = theCcfHead.bytesPerChip();
   const rspf_uint32 CHIPS_IN_SAMPLE_DIR    = theCcfHead.sampleChipsPerChunk();
   const rspf_uint32 CHIPS_IN_LINE_DIR      = theCcfHead.lineChipsPerChunk();
   const rspf_uint32 NUMBER_OF_INPUT_BANDS  = theCcfHead.numberOfBands();
   const rspf_uint32 CHUNKS_IN_SAMPLE_DIR   = theCcfHead.
                                        chunksInSampleDir(reduced_res_level);
   const rspf_uint32 CHUNK_SIDE_SIZE        = SAMPLES_PER_CHIP *
                                        CHIPS_IN_SAMPLE_DIR;
   const rspf_uint32 CHIP_OFFSET_TO_END     = SAMPLES_PER_CHIP - 1;
   
   //***
   // Shift the upper left corner of the "clip_rect" to the an even chunk
   // boundry.
   //***
   rspfIpt chunkOrigin = clip_rect.ul();
   adjustToStartOfChunk(chunkOrigin);

   //***
   // Calculate the number of chunks needed in the line/sample directions.
   //***
   
   rspf_uint32 size_in_x = clip_rect.lr().x - chunkOrigin.x + 1;
   rspf_uint32 size_in_y = clip_rect.lr().y - chunkOrigin.y + 1;
   rspf_uint32 w = (CHIPS_IN_SAMPLE_DIR*SAMPLES_PER_CHIP);
   rspf_uint32 h = (CHIPS_IN_LINE_DIR*LINES_PER_CHIP);
   rspf_uint32 chunks_in_x_dir = size_in_x / w  + (size_in_x % w  ? 1 : 0);
   rspf_uint32 chunks_in_y_dir = size_in_y / h + (size_in_y % h ? 1 : 0);
   //***
   // Get the start of data for the "reduced_res_level" passed in.
   //***
   streampos startOfData = theCcfHead.startOfData(reduced_res_level);
   rspfIpt ulChunkPt = chunkOrigin;

   // Chunk loop in line direction.
   for (rspf_uint32 y=0; y<chunks_in_y_dir; ++y)
   {
      ulChunkPt.x = chunkOrigin.x;

      // Chunk loop in sample direction.
      for (rspf_uint32 x=0; x<chunks_in_x_dir; ++x)
      {
         //***
         // Sequence through the chips in the chunk.
         // Grab the ccf data and stuff the tile buffer.
         //***
         rspfIpt chip_pt = ulChunkPt;

         // Chip loop in line direction.
         for (rspf_uint32 y_chip=0; y_chip<CHIPS_IN_LINE_DIR; ++y_chip) 
         {
            chip_pt.x = ulChunkPt.x;  // Reset x back to front of chunk.

            // Chip loop in sample direction.
            for (rspf_uint32 x_chip=0; x_chip<CHIPS_IN_SAMPLE_DIR; ++x_chip)
            {
               rspfIrect chip_rect(chip_pt.x,
                               chip_pt.y,
                               chip_pt.x + CHIP_OFFSET_TO_END,
                               chip_pt.y + CHIP_OFFSET_TO_END);
               
               if (chip_rect.intersects(clip_rect))
               {
                  //***
                  // Some point in the chip intersect the tile so grab the
                  // data.
                  //***

                  //***
                  // Get the seek position for the chunk.
                  //***
                  streampos tmp_offset
                     = ( ulChunkPt.y / CHUNK_SIDE_SIZE * BYTES_PER_CHUNK *
                         CHUNKS_IN_SAMPLE_DIR ) +
                     ( ulChunkPt.x / CHUNK_SIDE_SIZE * BYTES_PER_CHUNK );

                  streampos seek_position = startOfData + tmp_offset;

                  // Now move it to the chip.
                  tmp_offset
                     = ( y_chip *  BYTES_PER_CHIP * CHIPS_IN_SAMPLE_DIR ) +
                     (x_chip * BYTES_PER_CHIP);

                  seek_position += tmp_offset;

                  // Seek to the chip
                  theFileStr->seekg(seek_position, ios::beg);

                  //***
                  // Read the chip from the ccf file into the chip buffer.
                  // This will get all the bands.  Bands are interleaved by
                  // chip.
                  //***
                  if (!theFileStr->read((char*)theChipBuffer, BYTES_PER_CHIP))
                  {
                     theErrorStatus = rspfErrorCodes::RSPF_ERROR;
                     cerr << "rspfCcfTileSource fillUshortBuffer READ ERROR!"
                          << "Returning from method." << endl;
                     return false;
                  }
                  
                  rspfIrect chip_clip_rect = clip_rect.clipToRect(chip_rect);

                  //***
                  // Band loop in sample direction.  Bands are interleaved by
                  // chip.
                  //***
                  for (rspf_uint32 band=0; band<NUMBER_OF_INPUT_BANDS; band++)
                  {
                     //***
                     // Get a pointer positioned at the first valid pixel in
                     // the chip.
                     //***
                     rspf_uint16* s
                        = (rspf_uint16*)theChipBuffer +
                        band * CHIP_SIZE_PER_BAND + 
                        (chip_clip_rect.ul().y - chip_rect.ul().y) *
                        SAMPLES_PER_CHIP + chip_clip_rect.ul().x -
                        chip_rect.ul().x;       
                     
                     //***
                     // Get a pointer positioned at the first valid pixel in
                     // the tile.
                     //***
                     rspf_uint16* d
                        = (rspf_uint16*)tile->getBuf() +
                        band * TILE_SIZE_PER_BAND +
                        (chip_clip_rect.ul().y - tile_rect.ul().y) *
                        tile->getWidth() + chip_clip_rect.ul().x -
                        tile_rect.ul().x;
                     
                     // Line loop through a chip.
                     for (rspf_int32 chip_line = chip_clip_rect.ul().y;
                          chip_line <= chip_clip_rect.lr().y;
                          ++chip_line)
                     {
                        // Sample loop through a chip.
                        rspf_uint32 i=0;
                        for (rspf_int32 chip_sample = chip_clip_rect.ul().x;
                             chip_sample <= chip_clip_rect.lr().x;
                             ++chip_sample)
                        {
                           if (theByteOrder == RSPF_LITTLE_ENDIAN)
                           {
                              //***
                              // CCF data alway stored in big endian byte
                              // order so swap the bytes.
                              //***
                              d[i] = (s[i] << 8) | (s[i] >> 8);
                           }
                           else
                           {
                              d[i] = s[i];
                           }
                           
                           ++i;
                        }

                        // Increment the pointers by one line.
                        s += SAMPLES_PER_CHIP;
                        d += tile->getWidth();
                     }
                     
                  } // End of band loop.
                  
               } // End of if (chip_rect.intersects(clip_rect))
               
               chip_pt.x += SAMPLES_PER_CHIP;
               
            } // End of chip loop in the sample direction.

           chip_pt.y += LINES_PER_CHIP; 
            
         } // End of chip loop in the line direction.
         
         ulChunkPt.x += CHUNK_SIDE_SIZE;
         
      }  // End of chunk loop in the sample direction.

      ulChunkPt.y += CHUNK_SIDE_SIZE;
      
   }  // End of chunk loop in the line direction.

   return true;
}
   
            
//*******************************************************************
// Public Method:
//*******************************************************************
rspfIrect
rspfCcfTileSource::getImageRectangle(rspf_uint32 reduced_res_level) const
{
   return theCcfHead.imageRect(reduced_res_level);
}

//*******************************************************************
// Public Method:
//*******************************************************************
bool rspfCcfTileSource::setOutputBandList(const vector<rspf_uint32>& outputBandList)
{
   static const char MODULE[] = "rspfCcfTileSource::setOutputBandList";
   if (traceExec())  CLOG << "entering..." << endl;
   
   if (outputBandList.size() != theOutputBandList.size())
   {
      //***
      // Verify that each individual band does not bust the range of input
      // bands.
      //***
      for (rspf_uint32 i=0; i<outputBandList.size(); i++)
      {
         if (outputBandList[i] > (getNumberOfInputBands() - 1))
         {
            setErrorStatus();

            cerr << MODULE << " ERROR:"
                 << "\nOutput band number in list is greater than the "
                 << "number of bands in the image source!"
                 << "\noutputBandList[" << i << "]:  "
                 << "\nHighest availabe band:  "
                 << (getNumberOfInputBands() - 1)
                 << "\nError status has been set!  Returning..."
                 << endl;
            return false;            
         }
      }

      // Remove the old tiles and reallocate with the new size.
      theTile = NULL;
   }

   theOutputBandList = outputBandList;  // Assign the new list.

   if (traceExec())  CLOG << "returning..." << endl;
   return true;
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfCcfTileSource::saveState(rspfKeywordlist& kwl,
                                   const char* prefix) const
{
   // Currently nothing to do here.
   return rspfImageHandler::saveState(kwl, prefix);
}

//*******************************************************************
// Public method:
//*******************************************************************
bool rspfCcfTileSource::loadState(const rspfKeywordlist& kwl,
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
bool rspfCcfTileSource::open()
{
   static const char MODULE[] = "rspfCcfTileSource::open";
   if (traceExec())  CLOG << "entering..." << endl;

   if(isOpen())
   {
      close();
   }
   
   if (traceDebug())
   {
      CLOG << "DEBUG -- "
           << "\n\t theImageFile: " << theImageFile << endl;
   }
   
   if (theCcfHead.parseCcfHeader(theImageFile.c_str()) == false)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      if (traceExec())  CLOG << "returning with error..." << endl;
      return false;
   }

   if (traceDebug())
   {
      CLOG << "DEBUG -- "
           << "\n\t theImageFile: " << theImageFile
           << "\n\t theCcfHead: " << theCcfHead
           << endl;
   }

   theFileStr = new std::ifstream;
   // Open up the file for reading.
   theFileStr->open(theImageFile.c_str(), ios::in | ios::binary);
   
   if (!(*theFileStr))
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      
      cerr << MODULE << " ERROR!"
           << "\nCould not open file:  " << theImageFile.c_str()
           << "\nReturning..." << endl;
      if (traceExec())  CLOG << "returning with error..." << endl;

      delete theFileStr;
      theFileStr = NULL;
      return false;
   }

   theTile = rspfImageDataFactory::instance()->create(this, this);
   theTile->initialize();
     
   theChipBuffer  = new rspf_uint8[theCcfHead.bytesPerChip()];
   
   // Initialize the output band list.
   if (getNumberOfInputBands() > theOutputBandList.size())
   {
      theOutputBandList.resize(getNumberOfInputBands());
   }
   
   for (rspf_uint32 i=0; i<getNumberOfInputBands(); ++i)
   {
      theOutputBandList[i] = i; // One to one for initial setup.
   }
   
   completeOpen();
   
   if (traceDebug() && theTile.valid())
   {
      CLOG << "DEBUG:"
           << "\ntheTile:\n" << *theTile << endl;
   }

   if (traceExec())  CLOG << "returning..." << endl;
   return true;
}

void rspfCcfTileSource::close()
{
   theTile = NULL;
   if (theChipBuffer)
   {
      delete [] theChipBuffer;
      theChipBuffer = NULL;
   }
   if(theFileStr)
   {
      theFileStr->close();
      theFileStr->clear();
      delete theFileStr;
      theFileStr = NULL;
   }
}

//*******************************************************************
// Public method:
//*******************************************************************
rspfScalarType rspfCcfTileSource::getOutputScalarType() const
{
   return theCcfHead.pixelType();
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfCcfTileSource::getTileWidth() const
{
   return ( theTile.valid() ? theTile->getWidth() : 0 );
}

//*******************************************************************
// Public method:
//*******************************************************************
rspf_uint32 rspfCcfTileSource::getTileHeight() const
{
   return ( theTile.valid() ? theTile->getHeight() : 0 );
}

rspf_uint32 rspfCcfTileSource::getImageTileWidth() const
{
   return 32;
}

rspf_uint32 rspfCcfTileSource::getImageTileHeight() const
{
   return 32;
}

void rspfCcfTileSource::initVerticesFromHeader()
{
   theValidImageVertices.clear();
   
   const vector<rspfIpt>& validImageVertices
      = theCcfHead.getValidImageVertices();
   
   rspf_uint32 upper = (rspf_uint32)validImageVertices.size();
   if(!upper) return;
   for(rspf_uint32 index = 0; index < upper; ++index)
   {
      theValidImageVertices.push_back(validImageVertices[index]);
   }

   if (traceDebug())
   {
      cout << "rspfCcfTileSource::initVerticesFromHeader DEBUG:"
           << "\nValid image vertices:"
           << endl;
      for (rspf_uint32 i=0; i<upper; ++i)
      {
         cout << "\npoint[" << i << "]:  " << theValidImageVertices[i];
      }
      cout << endl;
   }
}

bool rspfCcfTileSource::isEcgGeom(const rspfFilename& filename)const
{
   rspfFilename file = filename;
   file.setExtension("geom");
   
   if(file.exists())
   {
      char bytes[3];
      ifstream in(file.c_str());
      
      in.read((char*)bytes, 3);
      rspfString testEcg(bytes,
                          bytes + 3);
      testEcg = testEcg.downcase();
      if(in&&(testEcg == "ecg"))
      {
         return true;
      }
   }
   return false;
}

rspfString rspfCcfTileSource::getShortName()const
{
   return rspfString("ccf");
}

rspfString rspfCcfTileSource::getLongName()const
{
   return rspfString("ccf reader");
}

rspf_uint32 rspfCcfTileSource::getNumberOfInputBands() const
{
   return theCcfHead.numberOfBands();
}

rspf_uint32 rspfCcfTileSource::getNumberOfOutputBands()const
{
   return getNumberOfInputBands();
}

rspf_uint32 rspfCcfTileSource::getNumberOfLines(rspf_uint32 reduced_res_level) const
{
   return theCcfHead.numberOfLines(reduced_res_level);
}

rspf_uint32 rspfCcfTileSource::getNumberOfSamples(rspf_uint32 reduced_res_level) const
{
   return theCcfHead.numberOfSamples(reduced_res_level);
}

rspf_uint32 rspfCcfTileSource::getNumberOfDecimationLevels() const
{
   return theCcfHead.numberOfReducedResSets();
}

bool rspfCcfTileSource::isOpen()const
{
   return (theFileStr != NULL);
}

rspfRefPtr<rspfProperty> rspfCcfTileSource::getProperty(const rspfString& name)const
{
	if(name == "file_type")
	{
		
		return new rspfStringProperty(name, "CCF");
	}
	return rspfImageHandler::getProperty(name);
}

void rspfCcfTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
	rspfImageHandler::getPropertyNames(propertyNames);
	propertyNames.push_back("file_type");
}

void rspfCcfTileSource::adjustToStartOfChunk(rspfIpt& pt) const
{
   pt.x &= 0xffffff00;
   pt.y &= 0xffffff00;
}

void rspfCcfTileSource::adjustToStartOfChip(rspfIpt& pt) const
{
   pt.x &= 0xffffffe0;
   pt.y &= 0xffffffe0;
}
