//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
//*******************************************************************
//  $Id: rspfGeneralRasterWriter.cpp 21962 2012-11-30 15:44:32Z dburken $

#include <rspf/imaging/rspfGeneralRasterWriter.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfEnviHeaderFileWriter.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfEndian.h>

#include <cstdlib>
#include <fstream>

static rspfTrace traceDebug("rspfGeneralRasterWriter:debug");

RTTI_DEF1(rspfGeneralRasterWriter,
          "rspfGeneralRasterWriter",
          rspfImageFileWriter)

static const char DEFAULT_FILE_NAME[] = "output.ras";

rspfGeneralRasterWriter::rspfGeneralRasterWriter()
   :
      rspfImageFileWriter(),
      theOutputStream(0),
      theOwnsStreamFlag(false),
      theRlevel(0),
      theOutputByteOrder(RSPF_LITTLE_ENDIAN),
      theMinPerBand(0),
      theMaxPerBand(0)
{
  setOutputImageType(RSPF_GENERAL_RASTER_BSQ);

  // Since there is no internal geometry set the flag to write out one.
  setWriteExternalGeometryFlag(true);
  theOutputByteOrder = rspfEndian().getSystemEndianType();
}

rspfGeneralRasterWriter::~rspfGeneralRasterWriter()
{
   if(isOpen())
   {
     close();
   }
}

bool rspfGeneralRasterWriter::isOpen()const
{
   return ( theOutputStream ? true : false );
}

bool rspfGeneralRasterWriter::open()
{
   bool result = false;
   
   close();

   // Check for empty filenames.
   if ( theFilename.size() )
   {
      std::ofstream* os = new std::ofstream();
      os->open(theFilename.c_str(), ios::out | ios::binary);
      
      if(os->is_open())
      {
         theOutputStream = os;
         theOwnsStreamFlag = true;
         result = true;
      }
      else
      {
         delete os;
         os = 0;
      }
   }

   return result;
}

void rspfGeneralRasterWriter::close()
{
   if (theOutputStream)      
   {
      theOutputStream->flush();
      if (theOwnsStreamFlag)
      {
         delete theOutputStream;
         theOwnsStreamFlag = false;
      }
      theOutputStream = 0;
   }
}

bool rspfGeneralRasterWriter::writeFile()
{
   bool result = false;

   if( theInputConnection.valid() && ( getErrorStatus() == rspfErrorCodes::RSPF_OK ) )
   {
      //---
      // Make sure we can open the file.  Note only the master process is used for
      // writing...
      //---
      if(theInputConnection->isMaster())
      {
         if (!isOpen())
         {
            open();
         }
      }

      result = writeStream();

      if ( result )
      {
         // Do this only on the master process. Note left to right precedence!
         if (getSequencer() && getSequencer()->isMaster())
         {
            //---
            // Write the header out.  We do this last since we must
            // compute min max pixel while we are writting the image.
            // since the header is an external text file this is Ok
            // to do.
            //---
            writeHeader();
            
            if (theOutputImageType.contains("envi"))
            {
               writeEnviHeader();
            }
         }
      } 

      close();
   }

   return result;
   
} // End: rspfGeneralRasterWriter::writeFile()

bool rspfGeneralRasterWriter::writeStream()
{
   static const char MODULE[] = "rspfGeneralRasterWriter::writeStream";

   bool result = false;
   
   if( theInputConnection.valid() && theOutputStream &&
       ( getErrorStatus() == rspfErrorCodes::RSPF_OK ) )
   {
      if ( theInputConnection->isMaster() )
      {
         // Write the file with the image data.
         if ( (theOutputImageType == "general_raster_bip") ||
              (theOutputImageType == "general_raster_bip_envi") )
         {
            result = writeToBip();
         }
         else if ( (theOutputImageType == "general_raster_bil") ||
                   (theOutputImageType == "general_raster_bil_envi") )
         {
            result = writeToBil();
         }
         else if ( (theOutputImageType == "general_raster_bsq") ||
                   (theOutputImageType == "general_raster_bsq_envi") )
         {
            result = writeToBsq();
         }
         else
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << MODULE << " ERROR:"
               << "\nUnsupported output type:  " << theOutputImageType << std::endl;
            result = false;
         }

         if ( result )
         {
            // Flush the stream to disk...
            theOutputStream->flush();
         }
      }
      else // Matching else: if ( theInputConnection->isMaster() )
      {
         // Slave process:
         theInputConnection->slaveProcessTiles();
         result = true;
      }
   }

   return result;
   
} // End: rspfGeneralRasterWriter::writeStream()

bool rspfGeneralRasterWriter::writeToBip()
{
   rspfEndian endian;
   static const char* const MODULE = "rspfGeneralRasterWriter::writeToBip";
   
   if (traceDebug()) CLOG << " Entered." << std::endl;
   
   
   //---
   // Get an arbitrary tile just to get the size in bytes!
   // This should be changed later... An rspfImageSource should know
   // this.
   //---
   rspfRefPtr<rspfImageData> id;
   
   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();
   
   rspf_uint64 bands     = theInputConnection->getNumberOfOutputBands();
   rspf_uint64 tilesWide =  theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint64 tilesHigh     = theInputConnection->getNumberOfTilesVertical();
   rspf_uint64 tileHeight    = theInputConnection->getTileHeight();
   rspf_uint64 numberOfTiles = theInputConnection->getNumberOfTiles();
   rspf_uint64 width         = theAreaOfInterest.width();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\nrspfGeneralRasterWriter::writeToBip DEBUG:"
         << "\nbands:          " << bands
         << "\ntilesWide:      " << tilesWide
         << "\ntilesHigh:      " << tilesHigh
         << "\ntileHeight:     " << tileHeight
         << "\nnumberOfTiles:  " << numberOfTiles
         << "\nwidth:          " << width
         << std::endl;
   }
   
   //---
   // Buffer to hold one line x tileHeight
   //---
   rspf_uint64 bufferSizeInBytes = 0;
   rspf_uint64 bytesInLine       = 0;
   unsigned char* buffer = NULL;
   
   theMinPerBand.clear();
   theMaxPerBand.clear();
   rspf_uint64 tileNumber = 0;
   bool wroteSomethingOut = false;
   rspfScalarType scalarType = theInputConnection->getOutputScalarType();
   for(rspf_uint64 i = 0; ((i < tilesHigh)&&(!needsAborting())); ++i)
   {
      // Clear the buffer.
      if(buffer)
      {
	 memset(buffer, 0, bufferSizeInBytes);
      }
      
      rspfIrect bufferRect(theAreaOfInterest.ul().x,
			    theAreaOfInterest.ul().y + i*tileHeight,
			    theAreaOfInterest.ul().x + (width - 1),
			    theAreaOfInterest.ul().y + i*tileHeight + (tileHeight - 1));
      // Tile loop in the sample (width) direction.
      for(rspf_uint64 j = 0; ((j < tilesWide)&&(!needsAborting())); ++j)
      {
         // Get the tile and copy it to the buffer.
         id = theInputConnection->getNextTile();
	 if(id.valid())
         {
            id->computeMinMaxPix(theMinPerBand, theMaxPerBand);
            if(!buffer)
            {
               bytesInLine     = id->getScalarSizeInBytes() * width * bands;
               
               //---
               // Buffer to hold one line x tileHeight
               //---
               bufferSizeInBytes = bytesInLine * tileHeight;
               buffer = new unsigned char[bufferSizeInBytes];
               memset(buffer, 0, bufferSizeInBytes);
            }
            id->unloadTile(buffer,
                           bufferRect,
                           RSPF_BIP);
         }
         ++tileNumber;
      }
      
      // Get the number of lines to write from the buffer.
      rspf_uint64 linesToWrite =
         min(tileHeight,
             static_cast<rspf_uint64>(theAreaOfInterest.lr().y -
                                       bufferRect.ul().y + 1));
      // Write the buffer out to disk.  
      rspf_uint8* buf = buffer;
      if(buf)
      {
         for (rspf_uint64 ii=0; ((ii<linesToWrite)&&(!needsAborting())); ++ii)
         {
            std::streamsize lineBytes = bytesInLine;
            wroteSomethingOut = true;

            if(endian.getSystemEndianType() != theOutputByteOrder)
            {
               endian.swap(scalarType,
                           buf,
                           lineBytes/rspf::scalarSizeInBytes(scalarType));
            }
            theOutputStream->write((char*)buf, lineBytes);
            if (theOutputStream->fail())
            {
               rspfNotify(rspfNotifyLevel_FATAL)
                  << MODULE << " ERROR:"
                  << "Error returned writing line!" << std::endl;
               setErrorStatus();
               if(buffer)
               {
                  // Free the memory.
                  delete [] buffer;
               }
               return false;
            }
	    
            buf += bytesInLine;
	    
         } // End of loop to write lines from buffer to tiff file.
      }
      double tile = tileNumber;
      double numTiles = numberOfTiles;
      setPercentComplete(tile / numTiles * 100);
      if(needsAborting())
      {
         setPercentComplete(100.0);
      }
      
   } // End of loop in the line (height) direction.
   if(buffer)
   {
      // Free the memory.
      delete [] buffer;
   }
   
   if (traceDebug()) CLOG << " Exited." << std::endl;
   
   return wroteSomethingOut;
}

bool rspfGeneralRasterWriter::writeToBil()
{
   rspfEndian endian;
   static const char* const MODULE = "rspfGeneralRasterWriter::writeToBil";

   if (traceDebug()) CLOG << " Entered." << std::endl;

   //***
   // Get an arbitrary tile just to get the size in bytes!
   // This should be changed later... An rspfImageSource should know
   // this.
   //***
   rspfRefPtr<rspfImageData> id;
   theInputConnection->setToStartOfSequence();

   rspf_uint64 bands     = theInputConnection->getNumberOfOutputBands();
   rspf_uint64 tilesWide = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint64 tilesHigh = theInputConnection->getNumberOfTilesVertical();
   rspf_uint64 tileHeight        = theInputConnection->getTileHeight();
   rspf_uint64 numberOfTiles     = theInputConnection->getNumberOfTiles();
   rspf_uint64 width             = theAreaOfInterest.width();
   rspf_uint64 bufferSizeInBytes = 0;
   rspf_uint64 bytesInLine       = 0;
   unsigned char* buffer    = NULL;

   // Start with a clean min/max.
   theMinPerBand.clear();
   theMaxPerBand.clear();

   rspf_uint64 tileNumber = 0;
   bool wroteSomethingOut = false;
   rspfScalarType scalarType = theInputConnection->getOutputScalarType();
   for(rspf_uint64 i = 0; ((i < tilesHigh)&&(!needsAborting())); ++i)
   {
      // Clear the buffer.
      // Clear the buffer.
      if(buffer)
      {
         memset(buffer, 0, bufferSizeInBytes);
      }
      
      rspfIrect bufferRect(theAreaOfInterest.ul().x,
                            theAreaOfInterest.ul().y + i*tileHeight,
                            theAreaOfInterest.ul().x + (width - 1),
                            theAreaOfInterest.ul().y + i *
                            tileHeight + (tileHeight - 1));
      
      // Tile loop in the sample (width) direction.
      for(rspf_uint64 j = 0; ((j < tilesWide)&&(!needsAborting())); ++j)
      {
         // Get the tile and copy it to the buffer.
         id = theInputConnection->getNextTile();
	 if(id.valid())
         {
            id->computeMinMaxPix(theMinPerBand, theMaxPerBand);
            
            if(!buffer)
            {
               bytesInLine     = id->getScalarSizeInBytes() * width;
               
               // Buffer to hold one line x tileHeight
               bufferSizeInBytes = bytesInLine * tileHeight * bands;
               buffer = new unsigned char[bufferSizeInBytes];
               memset(buffer, 0, bufferSizeInBytes);
            }
            id->unloadTile(buffer,
                           bufferRect,
                           RSPF_BIL);
         }
         ++tileNumber;
      }
      
      // Get the number of lines to write from the buffer.
      rspf_uint64 linesToWrite =
         min(tileHeight,
             static_cast<rspf_uint64>(theAreaOfInterest.lr().y -
                                       bufferRect.ul().y + 1));
      
      // Write the buffer out to disk.  
      rspf_uint8* buf = buffer;
      for (rspf_uint64 ii=0; ((ii<linesToWrite)&(!needsAborting())); ++ii)
      {
         for (rspf_uint64 band = 0;
              ((band < bands)&&(!needsAborting()));
              ++band)
         {
            wroteSomethingOut = true;
            if(endian.getSystemEndianType() != theOutputByteOrder)
            {
               endian.swap(scalarType,
                           buf,
                           bytesInLine/rspf::scalarSizeInBytes(scalarType));
            }
            theOutputStream->write((char*)buf, bytesInLine);
            if (theOutputStream->fail())
            {
               rspfNotify(rspfNotifyLevel_FATAL)
                  << MODULE << " ERROR:"
                  << "Error returned writing line!" << std::endl;
               setErrorStatus();
	       if(buffer)
               {
                  // Free the memory.
                  delete [] buffer;
               }
               return false;
            }

            buf += bytesInLine;
         }
         
      } // End of loop to write lines from buffer to tiff file.

      double tile = tileNumber;
      double numTiles = numberOfTiles;
      setPercentComplete(tile / numTiles * 100);

      if(needsAborting())
      {
         setPercentComplete(100.0);
      }

   } // End of loop in the line (height) direction.

   if(buffer)
   {
      // Free the memory.
      delete [] buffer;
   }
   
   if (traceDebug()) CLOG << " Exited." << std::endl;

   return wroteSomethingOut;
}

bool rspfGeneralRasterWriter::writeToBsq()
{
   rspfEndian endian;
   static const char* const MODULE = "rspfGeneralRasterWriter::writeToBsq";

   if (traceDebug()) CLOG << " Entered." << std::endl;

   //***
   // Get an arbitrary tile just to get the size in bytes!
   // This should be changed later... An rspfImageSource should know
   // this.
   //***
   rspfRefPtr<rspfImageData> id;

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   rspf_uint64 bands     = theInputConnection->getNumberOfOutputBands();
   rspf_uint64 tilesWide = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint64 tilesHigh = theInputConnection->getNumberOfTilesVertical();
   rspf_uint64 tileHeight      = theInputConnection->getTileHeight();
   rspf_uint64 numberOfTiles   = theInputConnection->getNumberOfTiles();
   rspf_uint64 width           = theAreaOfInterest.width();
   rspf_uint64 height          = theAreaOfInterest.height();

   rspf_uint64 bytesInLine     = 0;
   rspf_uint64 buf_band_offset = 0;
   
   // Use the system "streampos" typedef for future 64 bit seeks (long long).
   streampos file_band_offset = 0;
   
   //***
   // Buffer to hold one line x tileHeight
   //***
   rspf_uint64 bufferSizeInBytes = 0;
   unsigned char* buffer = NULL;
      
   theMinPerBand.clear();
   theMaxPerBand.clear();

   rspf_uint64 tileNumber = 0;
   bool wroteSomethingOut = false;
   rspfScalarType scalarType = theInputConnection->getOutputScalarType();
   for(rspf_uint64 i = 0; ((i < tilesHigh)&&(!needsAborting())); ++i)
   {
      if(buffer)
      {
	 // Clear the buffer.
	 memset(buffer, 0, bufferSizeInBytes);
      }
      
      rspfIrect bufferRect(theAreaOfInterest.ul().x,
			    theAreaOfInterest.ul().y + i*tileHeight,
			    theAreaOfInterest.ul().x + (width - 1),
			    theAreaOfInterest.ul().y + i *
                            tileHeight + (tileHeight - 1));
      
      // Tile loop in the sample (width) direction.
      for(rspf_uint64 j = 0; ((j < tilesWide)&&(!needsAborting())); ++j)
      {
         // Get the tile and copy it to the buffer.
         id = theInputConnection->getNextTile();
	 if(id.valid())
         {
            id->computeMinMaxPix(theMinPerBand, theMaxPerBand);
            if(!buffer)
            {
               bytesInLine     = id->getScalarSizeInBytes() * width;
               buf_band_offset = bytesInLine * tileHeight;
               file_band_offset = height * bytesInLine;
               bufferSizeInBytes = bytesInLine * tileHeight * bands;
               buffer = new unsigned char[bufferSizeInBytes];
               memset(buffer, 0, bufferSizeInBytes);
            }
            id->unloadTile(buffer,
                           bufferRect,
                           RSPF_BSQ);
         }
         ++tileNumber;
      }
      
      // Get the number of lines to write from the buffer.
      rspf_uint64 linesToWrite =
         min(tileHeight,
             static_cast<rspf_uint64>(theAreaOfInterest.lr().y -
                                       bufferRect.ul().y + 1));
      
      // Write the buffer out to disk.  
      rspf_uint64 start_line =
         static_cast<rspf_uint64>(bufferRect.ul().y -
                                   theAreaOfInterest.ul().y);
      for (rspf_uint64 band = 0; ((band < bands)&&(!needsAborting())); ++band)
      {
         rspf_uint8* buf = buffer;
         buf += buf_band_offset * band;
         
         // Put the file pointer in the right spot.
         streampos pos = file_band_offset * band + start_line * bytesInLine;
         theOutputStream->seekp(pos, ios::beg);
         if (theOutputStream->fail())
         {
            rspfNotify(rspfNotifyLevel_FATAL) << MODULE << " ERROR:"
                 << "Error returned seeking to image data position!" << std::endl;
            setErrorStatus();
            return false;
         }
         
         for (rspf_uint64 ii=0; ((ii<linesToWrite)&&(!needsAborting())); ++ii)
         {
            wroteSomethingOut = true;
            if(endian.getSystemEndianType() != theOutputByteOrder)
            {
               endian.swap(scalarType,
                           buf,
                           bytesInLine/rspf::scalarSizeInBytes(scalarType));
            }

            theOutputStream->write((char*)buf, bytesInLine);
            
            if (theOutputStream->fail())
            {
               rspfNotify(rspfNotifyLevel_FATAL) << MODULE << " ERROR:"
                    << "Error returned writing line!" << std::endl;
               setErrorStatus();
               return false;
            }
            
            buf += bytesInLine;
         }
         
      } // End of loop to write lines from buffer to tiff file.
      
      double tile = tileNumber;
      double numTiles = numberOfTiles;
      setPercentComplete(tile / numTiles * 100);

      if(needsAborting())
      {
         setPercentComplete(100.0);
      }
      
   } // End of loop in the line (height) direction.
   
   // Free the memory.
   delete [] buffer;
   
   if (traceDebug()) CLOG << " Exited." << std::endl;
   
   return wroteSomethingOut;
}

bool rspfGeneralRasterWriter::saveState(rspfKeywordlist& kwl,
                                         const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::BYTE_ORDER_KW,
           ((theOutputByteOrder==RSPF_LITTLE_ENDIAN)?"little_endian":"big_endian"),
           true);
   return rspfImageFileWriter::saveState(kwl,
                                      prefix);
}

bool rspfGeneralRasterWriter::loadState(const rspfKeywordlist& kwl,
                                         const char* prefix)
{
   const char* value;
   
   value = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if(value)
   {
      setFilename(rspfFilename(value));
   }
   
   value = kwl.find(prefix, rspfKeywordNames::INPUT_RR_LEVEL_KW);
   if(value)
   {
      theRlevel = atoi(value);
   }

   if(rspfImageFileWriter::loadState(kwl, prefix))
   {
      if( (theOutputImageType!="general_raster_bip")      &&
          (theOutputImageType!="general_raster_bil")      &&
          (theOutputImageType!="general_raster_bsq")      &&
          (theOutputImageType!="general_raster_bip_envi") &&
          (theOutputImageType!="general_raster_bil_envi") &&
          (theOutputImageType!="general_raster_bsq_envi")
         )
      {
         theOutputImageType = "general_raster_bsq";
      }
   }
   else
   {
      return false;
   }
   const char* outputByteOrder = kwl.find(prefix, rspfKeywordNames::BYTE_ORDER_KW);
   theOutputByteOrder = rspfEndian().getSystemEndianType();
   if(outputByteOrder)
   {
      rspfString byteOrder = outputByteOrder;
      byteOrder = byteOrder.downcase();
      if(byteOrder.contains("little"))
      {
         theOutputByteOrder = RSPF_LITTLE_ENDIAN;
      }
      else if(byteOrder.contains("big"))
      {
         theOutputByteOrder = RSPF_BIG_ENDIAN;
      }
   }
   
   return true;
}

void rspfGeneralRasterWriter::writeHeader() const
{
   static const char MODULE[] = "rspfGeneralRasterWriter::writeHeader";

   if (traceDebug()) CLOG << " Entered..." << std::endl;

   // Make a header file name from the image file.
   rspfFilename headerFile = theFilename;
   headerFile.setExtension(".omd"); // rspf meta data

   std::ofstream os;
   os.open(headerFile.c_str(), ios::out);
   if (!os)
   {
      rspfNotify(rspfNotifyLevel_FATAL) << MODULE << " Error:\n"
           << "Could not open:  " << headerFile << std::endl;
      return;
   }

   rspfString interleaveType = getInterleaveString();

   rspfString scalar =
      rspfScalarTypeLut::instance()->getEntryString(theInputConnection->
                                                     getOutputScalarType());
   
   os << "// *** rspf meta data general raster header file ***\n"
      << rspfKeywordNames::FILENAME_KW << ": " 
      << theFilename.file().c_str() << "\n"
      << rspfKeywordNames::IMAGE_TYPE_KW << ": "
      << getOutputImageTypeString() << "\n"
      << rspfKeywordNames::INTERLEAVE_TYPE_KW << ": "
      << interleaveType.c_str() << "\n"
      << rspfKeywordNames::NUMBER_BANDS_KW << ":  "
      << theInputConnection->getNumberOfOutputBands() << "\n"
      << rspfKeywordNames::NUMBER_LINES_KW << ": "
      << (theAreaOfInterest.lr().y - theAreaOfInterest.ul().y + 1) << "\n"
      << rspfKeywordNames::NUMBER_SAMPLES_KW << ": "
      << (theAreaOfInterest.lr().x - theAreaOfInterest.ul().x + 1) << "\n"
      << rspfKeywordNames::SCALAR_TYPE_KW << ": "
      << scalar.c_str() << "\n"
      << rspfKeywordNames::BYTE_ORDER_KW <<": "
      << ((theOutputByteOrder==RSPF_BIG_ENDIAN)?"big_endian":"little_endian")
      << "\n"
      << std::endl;

   // Output the null/min/max for each band.
   os << "\n// NOTE:  Bands are one based, band1 is the first band."
      << std::endl;

   for (rspf_uint32 i=0; i<theInputConnection->getNumberOfOutputBands(); ++i)
   {
      rspfString prefix = rspfKeywordNames::BAND_KW +
                           rspfString::toString(i+1) + ".";
      
      rspfString null_pix = rspfString::toString(theInputConnection->
                                                   getNullPixelValue(i));
      rspfString min_pix;
      rspfString max_pix;

      if(!theMinPerBand.size()||!theMaxPerBand.size())
      {
         min_pix  = rspfString::toString(theInputConnection->
					  getMinPixelValue(i));
         max_pix  =  rspfString::toString(theInputConnection->
                                           getMaxPixelValue(i));
      }
      else
      {
         min_pix   = rspfString::toString(theMinPerBand[i]);
         max_pix  = rspfString::toString(theMaxPerBand[i]);
      }
      
      os << prefix.c_str() << rspfKeywordNames::NULL_VALUE_KW << ":  "
         << null_pix.c_str() << "\n"
         << prefix << rspfKeywordNames::MIN_VALUE_KW << ":  "
         << min_pix.c_str() << "\n"
         << prefix << rspfKeywordNames::MAX_VALUE_KW << ":  "
         << max_pix.c_str() << std::endl;
   }
   
   os.close();
   
   if (traceDebug()) CLOG << " Exited..." << endl;
}

void rspfGeneralRasterWriter::writeEnviHeader() const
{
   static const char MODULE[] = "rspfGeneralRasterWriter::writeEnviHeader";

   if (traceDebug()) CLOG << " Entered..." << endl;

   if (!theInputConnection)
   {
      return;
   }
   
   // Make a header file name from the image file.
   rspfFilename headerFile = theFilename;
   headerFile.setExtension(".hdr"); // rspf meta data

   rspfString interleaveType = getInterleaveString();
   rspfKeywordlist kwl;
   kwl.add(rspfKeywordNames::INTERLEAVE_TYPE_KW, interleaveType.c_str());

   rspfRefPtr<rspfEnviHeaderFileWriter> hdr = new rspfEnviHeaderFileWriter;
   hdr->connectMyInputTo(0, theInputConnection.get());
   hdr->initialize();
   hdr->setFilename(headerFile);
   hdr->loadState(kwl);
   hdr->setAreaOfInterest(theAreaOfInterest);
   hdr->execute();
   
   if (traceDebug()) CLOG << " Exited..." << endl;
}

rspfString rspfGeneralRasterWriter::getExtension() const
{
   return "ras";
   //return getInterleaveString();
}

void rspfGeneralRasterWriter::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   imageTypeList.push_back(rspfString("general_raster_bip"));
   imageTypeList.push_back(rspfString("general_raster_bil"));
   imageTypeList.push_back(rspfString("general_raster_bsq"));
   imageTypeList.push_back(rspfString("general_raster_bip_envi"));
   imageTypeList.push_back(rspfString("general_raster_bil_envi"));
   imageTypeList.push_back(rspfString("general_raster_bsq_envi"));
}

rspfString rspfGeneralRasterWriter::getInterleaveString() const
{
   rspfString interleaveType = "unknown";
   if ( (theOutputImageType == "general_raster_bip") ||
        (theOutputImageType == "general_raster_bip_envi") )
   {
      interleaveType = "bip";
   }
   else if ( (theOutputImageType == "general_raster_bil") ||
             (theOutputImageType == "general_raster_bil_envi") )
   {
      interleaveType = "bil";
   }
   else if ( (theOutputImageType == "general_raster_bsq") ||
             (theOutputImageType == "general_raster_bsq_envi") )
   {
      interleaveType = "bsq";
   }
   return interleaveType;
}

bool rspfGeneralRasterWriter::setOutputStream(std::ostream& stream)
{
   if (theOwnsStreamFlag && theOutputStream)
   {
      delete theOutputStream;
   }
   theOutputStream = &stream;
   theOwnsStreamFlag = false;
   return true;
}
