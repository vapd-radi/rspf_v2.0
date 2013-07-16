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
// Contains class definition for TiffOverviewBuilder
// 
//*******************************************************************
//  $Id: rspfTiffOverviewBuilder.cpp 22232 2013-04-13 20:06:19Z dburken $

#include <rspf/imaging/rspfTiffOverviewBuilder.h>
#include <rspf/parallel/rspfMpi.h>
#include <rspf/parallel/rspfMpiMasterOverviewSequencer.h>
#include <rspf/parallel/rspfMpiSlaveOverviewSequencer.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfStdOutProgress.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfBitMaskTileSource.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/support_data/rspfGeoTiff.h>

#include <xtiffio.h>
#include <algorithm> /* for std::fill */
#include <sstream>
using namespace std;

RTTI_DEF1(rspfTiffOverviewBuilder,
          "rspfTiffOverviewBuilder",
          rspfOverviewBuilderBase)

static rspfTrace traceDebug("rspfTiffOverviewBuilder:degug");

// Property keywords.
static const char COPY_ALL_KW[]           = "copy_all_flag";
static const char INTERNAL_OVERVIEWS_KW[] = "internal_overviews_flag";

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfTiffOverviewBuilder.cpp 22232 2013-04-13 20:06:19Z dburken $";
#endif


//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfTiffOverviewBuilder::rspfTiffOverviewBuilder()
   :
      rspfOverviewBuilderBase(),
      m_nullDataBuffer(0),
      m_bytesPerPixel(1),
      m_bitsPerSample(8),
      m_tileWidth(0),
      m_tileHeight(0),
      m_tileSizeInBytes(0),
      m_sampleFormat(0),
      m_currentTiffDir(0),
      m_tiffCompressType(COMPRESSION_NONE),
      m_jpegCompressQuality(DEFAULT_COMPRESS_QUALITY),
      m_resampleType(rspfFilterResampler::rspfFilterResampler_BOX),
      m_nullPixelValues(),
      m_copyAllFlag(false),
      m_outputTileSizeSetFlag(false),
      m_internalOverviewsFlag(false)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiffOverviewBuilder::rspfTiffOverviewBuilder DEBUG:\n";
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  "
         << RSPF_ID
         << "\n";
#endif
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "overview stop dimension: " << m_overviewStopDimension
         << std::endl;
   }
}

rspfTiffOverviewBuilder::~rspfTiffOverviewBuilder()
{
}

void rspfTiffOverviewBuilder::setResampleType(
   rspfFilterResampler::rspfFilterResamplerType resampleType)
{
   m_resampleType = resampleType;
}

bool rspfTiffOverviewBuilder::buildOverview(const rspfFilename& overview_file, bool copy_all)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiffOverviewBuilder::buildOverview DEBUG:"
         << "\noverview file: " << overview_file.c_str()
         << "\ncopy_all flag: " << (copy_all?"true":"false")
         << std::endl;
   }


   m_outputFile  = overview_file;
   m_copyAllFlag = copy_all;

   return execute();
}


bool rspfTiffOverviewBuilder::execute()
{
   static const char MODULE[] = "rspfTiffOverviewBuilder::execute";
   
   if (theErrorStatus == rspfErrorCodes::RSPF_ERROR)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nError status has been previously set!  Returning..."
         << std::endl;
      return false;
   }

   if ( !m_imageHandler || m_imageHandler->hasError() )
   {
      setErrorStatus();
      return false;
   }

   m_outputFile = getOutputFile();
   if (m_outputFile == rspfFilename::NIL)
   {
      return false;
   }

   // Check the file.  Disallow same file overview building.
   if ( !buildInternalOverviews() && (m_imageHandler->getFilename() == m_outputFile) )
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "Source image file and overview file cannot be the same!"
         << std::endl;
      return false;
   }

   rspfFilename outputFileTemp = m_outputFile;
   
   if ( !buildInternalOverviews() )
   {
      // Add .tmp in case process gets aborted to avoid leaving bad .ovr file.
      outputFileTemp += ".tmp";
   }

   // Required number of levels needed including r0.
   rspf_uint32 requiedResLevels = getRequiredResLevels(m_imageHandler.get());

   // Zero based starting resLevel.
   rspf_uint32 startingResLevel  = 0;
   if ( !copyR0() )
   {
      startingResLevel = m_imageHandler->getNumberOfDecimationLevels();
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE
         << "\nCurrent number of reduced res sets: "
         << m_imageHandler->getNumberOfDecimationLevels()
         << "\nNumber of required reduced res sets:  " << requiedResLevels
         << "\nStarting reduced res set:    " << startingResLevel
         << "\nResampling type:  " << getOverviewType().c_str()
         << std::endl;
   }

   if ( startingResLevel >= requiedResLevels )
   {
      rspfNotify(rspfNotifyLevel_INFO)
         << MODULE << " NOTICE:"
         << "\nImage has required reduced resolution data sets.\nReturning..."
         << std::endl;
      return true;
   }

   //---
   // If alpha bit mask generation was requested, then need to instantiate the mask writer object.
   // This is simply a "transparent" tile source placed after to the right of the image handler
   // that scans the pixels being pulled and accumulates alpha bit mask for writing at the end.
   //---
   if (m_bitMaskSpec.getSize() > 0)
   {
      m_maskWriter = new rspfBitMaskWriter();
      m_maskWriter->loadState(m_bitMaskSpec);
      m_maskWriter->setStartingResLevel(1);
      rspfRefPtr<rspfBitMaskTileSource> bmts = new rspfBitMaskTileSource;
      bmts->setAssociatedMaskWriter(m_maskWriter.get());
      m_maskFilter = new rspfMaskFilter();
      m_maskFilter->connectMyInputTo(m_imageHandler.get());
      m_maskFilter->setMaskSource((rspfImageSource*)bmts.get());
   }

   rspfStdOutProgress* progressListener = 0; // Only used on master.
   TIFF* tif = 0;                             // Only used on master.

   setProcessStatus(rspfProcessInterface::PROCESS_STATUS_EXECUTING);
   setPercentComplete(0.0);

   if (rspfMpi::instance()->getRank() == 0 )
   {
      //---
      // See if the file can be opened for writing.
      // Note:  If this file existed previously it will be overwritten.
      //---
      tif = openTiff(outputFileTemp);
      if (!tif)
      {
         // Set the error...
         setErrorStatus();
         rspfNotify(rspfNotifyLevel_WARN)
            << __FILE__ << " " << __LINE__ << " " << MODULE
            << "\nCannot open file: " << outputFileTemp << std::endl;
         return false;
      } 

      //---
      // Check for a listeners.  If the list is empty, add a standard out
      // listener so that command line apps like img2rr will get some progress.
      //---
      rspfStdOutProgress* progressListener = 0;
      if (theListenerList.empty())
      {
         progressListener = new rspfStdOutProgress(0, true);
         addListener(progressListener);
      }

      if (startingResLevel == 0)
      {       
         if (!writeR0(tif))
         {
            // Set the error...
            setErrorStatus();
            rspfNotify(rspfNotifyLevel_WARN)
               << __FILE__ << " " << __LINE__
               << "\nError copying image!" << std::endl;

            closeTiff(tif);
            if (progressListener)
            {
               removeListener(progressListener);
               delete progressListener;
               progressListener = 0;
            }
            
            if ( outputFileTemp.exists() && !buildInternalOverviews() )
            {
               rspfFilename::remove( outputFileTemp );
            }
            return false;
         }

         ++startingResLevel; // Go to r1.
      }

      if (needsAborting())
      {
         closeTiff(tif);
         if (progressListener)
         {
            removeListener(progressListener);
            delete progressListener;
            progressListener = 0;
         }
         return false;
      }
      
      TIFFFlush(tif);
      
   } // End of master only write of r0.
        
   for (rspf_uint32 i = startingResLevel; i < requiedResLevels; ++i)
   {
      if (rspfMpi::instance()->getRank() == 0 )
      {
         TIFFFlush(tif);
      }

      // Sync all processes...
      rspfMpi::instance()->barrier();
      
      rspfRefPtr<rspfImageHandler> ih = 0;

      //---
      // If we copied r0 to the overview file use it instead of the
      // original image handler as it is probably faster.
      //---
      if ( !copyR0() && (i <= m_imageHandler->getNumberOfDecimationLevels()) ) 
      {
         ih = m_imageHandler;
      }
      else
      {
         // We know we're a tiff so don't use the factory.
         ih = new rspfTiffTileSource;
         if ( ih->open(outputFileTemp) == false )
         {
            ih = 0;
            
            // Set the error...
            setErrorStatus();
            rspfNotify(rspfNotifyLevel_WARN)
               << __FILE__ << " " << __LINE__ << " " << MODULE
               << "\nCannot open file: " << outputFileTemp << std::endl;
            
            return false;
         }

         //---
         // Since the overview file is being opened here, need to set its handler's starting res
         // level where the original image file left off. This is usually R1 since the original
         // file only has R0, but the original file may have more than R0:
         //---
         if ( !copyR0() &&  !buildInternalOverviews() )
         {
            ih->setStartingResLevel( m_imageHandler->getNumberOfDecimationLevels());
         }
      }
      
      // If mask is to be generated, need to notify both the writer and the reader of new 
      // input source:
      if (m_bitMaskSpec.getSize() > 0)
      {
         m_maskFilter->connectMyInputTo(0, ih.get());
         m_maskWriter->connectMyInputTo(ih.get());
      }

      if ( !writeRn( ih.get(), tif, i, (i==startingResLevel) && !copyR0() ) )
      {
         // Set the error...
         setErrorStatus();
         rspfNotify(rspfNotifyLevel_WARN)
            << __FILE__ << " " << __LINE__ << " " << MODULE
            << "\nError creating reduced res set: " << i << std::endl;

         ih->disconnect();
         ih = 0;
         if (tif)
         {
            closeTiff(tif);
            tif = 0;
         }

         if (progressListener)
         {
            removeListener(progressListener);
            delete progressListener;
            progressListener = 0;
         }

         if ( outputFileTemp.exists() && !buildInternalOverviews() )
         {
            rspfFilename::remove( outputFileTemp );
         }

         return false;
      }
      
      if (needsAborting())
      {
         ih->disconnect();
         ih = 0;
         if (tif)
         {
            closeTiff(tif);
            tif = 0;
         }
         if (progressListener)
         {
            removeListener(progressListener);
            delete progressListener;
            progressListener = 0;
         }
         return false;
      }
      
      if (m_bitMaskSpec.getSize() > 0)
      {
         m_maskFilter->disconnectMyInput(0);
         m_maskWriter->disconnectAllInputs();
      }
      ih = 0;
   }

   if (rspfMpi::instance()->getRank() == 0 )
   {
      if (tif)
      {
         closeTiff(tif);
         tif = 0;
      }

      // Write out the alpha bit mask if one was enabled:
      if (m_maskWriter.valid())
      {
         rspfNotify(rspfNotifyLevel_INFO) << "Writing alpha bit mask file..." << std::endl;
         m_maskWriter->close();
      }

      // Remove the listener if we had one.
      if (progressListener)
      {
         removeListener(progressListener);
         delete progressListener;
         progressListener = 0;
      }

      if ( !buildInternalOverviews() )
      {
         outputFileTemp.rename(m_outputFile);
      }
      
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_INFO)
            << "Wrote file:  " << m_outputFile.c_str() << std::endl;
      }
     
      setCurrentMessage(rspfString("Finished..."));
   }
   
   finalize();  // Reset band list if a band selector.

   return true;
}

bool rspfTiffOverviewBuilder::writeR0(TIFF* tif)
{
   static const char MODULE[] = "rspfTiffOverviewBuilder::writeR0";

   rspfIrect rect = m_imageHandler->getImageRectangle();

   if (!setTags(tif, rect, 0))
   {
      closeTiff(tif);
      rspfNotify(rspfNotifyLevel_WARN) << MODULE << " Error writing tags!" << std::endl;
      return false;
   }
   
   // Set the geotiff tags.
   if ( setGeotiffTags(m_imageHandler->getImageGeometry().get(),
                       m_imageHandler->getBoundingRect(),
                       0,
                       tif) == false )
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_NOTICE)
            << MODULE << " NOTICE: geotiff tags not set." << std::endl;
      } 
   }
   
   rspf_int32 samples         = m_imageHandler->getNumberOfSamples();
   rspf_int32 lines           = m_imageHandler->getNumberOfLines();
   rspf_int32 tilesWide       = samples % m_tileWidth ?
                           samples / m_tileWidth + 1 : samples / m_tileWidth;
   rspf_int32 tilesHigh       = lines % m_tileHeight ?
                           lines / m_tileHeight + 1 : lines / m_tileHeight;
   rspf_int32 numberOfTiles   = tilesWide * tilesHigh;

   int tileNumber = 0;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiffOverviewBuilder::writeR0 DEBUG:"
         << "\nsamples:        " << samples
         << "\nlines:          " << lines
         << "\ntilesWide:      " << tilesWide
         << "\ntilesHigh:      " << tilesHigh
         << "\nnumberOfTiles:  " << numberOfTiles
         << std::endl;
   }

   setCurrentMessage(rspfString("Copying r0..."));
   
   //***
   // Tile loop in the line direction.
   //***
   for(int i = 0; i < tilesHigh; ++i)
   {
      rspfIpt origin(0, 0);
      origin.y = i * m_tileHeight;
      
      //***
      // Tile loop in the sample (width) direction.
      //***
      for(int j = 0; j < tilesWide; ++j)
      {
         origin.x = j * m_tileWidth;

         rspfRefPtr<rspfImageData> t =
            m_imageHandler->getTile(rspfIrect(origin.x,
                                                origin.y,
                                                origin.x +(m_tileWidth-1),
                                                origin.y +(m_tileHeight-1)));

         // Check for errors reading tile:
         if ( m_imageHandler->hasError() )
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR: reading tile:  " << i << std::endl;
            return false;
         }

         // If masking was enabled, pass the tile onto that object for processing:
         if (m_maskWriter.valid())
            m_maskWriter->generateMask(t, 0);

         //***
         // Band loop.
         //***
         for (uint32 band=0;
              band<m_imageHandler->getNumberOfInputBands();
              ++band)
         {
            tdata_t data;
            
            if ( t.valid() && (t->getDataObjectStatus() != RSPF_NULL) )
            {
               // Grab a pointer to the tile for the band.
               data = static_cast<tdata_t>(t->getBuf(band));
            }
            else
            {
               data = static_cast<tdata_t>(&(m_nullDataBuffer.front()));
            }

            // Write the tile.
            int bytesWritten = 0;
            bytesWritten = TIFFWriteTile(tif,
                                         data,
                                         origin.x,
                                         origin.y,
                                         0,        // z
                                         band);    // sample

            if (bytesWritten != m_tileSizeInBytes)
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << MODULE << " ERROR:"
                  << "Error returned writing tiff tile:  " << i
                  << "\nExpected bytes written:  " << m_tileSizeInBytes
                  << "\nBytes written:  " << bytesWritten
                  << std::endl;
               theErrorStatus = rspfErrorCodes::RSPF_ERROR;
               return false;
            }

         } // End of band loop.  
        
         ++tileNumber;

      } // End of tile loop in the sample (width) direction.

      if (needsAborting())
      {
         setPercentComplete(100.0);
         break;
      }
      else
      {
         double tile = tileNumber;
         double numTiles = numberOfTiles;
         setPercentComplete(tile / numTiles * 100.0);
      }

   } // End of tile loop in the line (height) direction.

   //***
   // Write the current dirctory.
   //***
   if (!TIFFWriteDirectory(tif))
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Error writing directory!" << std::endl;
      return false;
   }

   ++m_currentTiffDir;

   return true;
}

bool rspfTiffOverviewBuilder::writeRn( rspfImageHandler* imageHandler,
                                        TIFF* tif,
                                        rspf_uint32 resLevel,
                                        bool firstResLevel )
{
   if ( tif ) //  && buildInternalOverviews() )
   {
      // Create an empty directory to start with.
      TIFFCreateDirectory( tif );
   }
   else
   {
      return false;
   }
   
   //---
   // Set up the sequencer.  This will be one of three depending on if we're
   // running mpi and if we are a master process or a slave process.
   //---
   rspfRefPtr<rspfOverviewSequencer> sequencer;
   
   if(rspfMpi::instance()->getNumberOfProcessors() > 1)
   {
      if ( rspfMpi::instance()->getRank() == 0 )
      {
         sequencer = new rspfMpiMasterOverviewSequencer();
      }
      else
      {
         sequencer = new rspfMpiSlaveOverviewSequencer();
      }
   }
   else
   {
      sequencer = new rspfOverviewSequencer();
   }
   
   sequencer->setImageHandler(imageHandler);

   if (m_maskWriter.valid() && m_maskFilter.valid())
   {
      sequencer->setBitMaskObjects(m_maskWriter.get(), m_maskFilter.get());
   }

   // sourceResLevel: This is the res level to pull data from:
   rspf_uint32 sourceResLevel = imageHandler->getNumberOfDecimationLevels() +
      imageHandler->getStartingResLevel() - 1;

   sequencer->setSourceLevel(sourceResLevel);
   sequencer->setResampleType(m_resampleType);
   sequencer->setTileSize( rspfIpt(m_tileWidth, m_tileHeight) );
   
   if ( firstResLevel )
   {
      // Set up things that are only performed on first scan through tiles.
      
      if ( getHistogramMode() != RSPF_HISTO_MODE_UNKNOWN )
      {
         // Accumulate a histogram.  Can't do with mpi/multi-process.
         if(rspfMpi::instance()->getNumberOfProcessors() == 1)
         {
            sequencer->setHistogramMode(getHistogramMode());
         }
         //---
         // else{} Not sure if we want an error thrown here.  For now will handle at the
         // application level.
         //---
      }
      if ( getScanForMinMaxNull() == true )
      {
         sequencer->setScanForMinMaxNull(true);
      }
      else if ( getScanForMinMax() == true )
      {
         sequencer->setScanForMinMax(true);
      }
   }

   // Note sequence setup must be performed before intialize. 
   sequencer->initialize();

   // If we are a slave process start the resampling of tiles.
   if (rspfMpi::instance()->getRank() != 0 )
   {
      sequencer->slaveProcessTiles();
      return true;
   }

   //---
   // The rest of the method on master node only.
   //---
   static const char MODULE[] = "rspfTiffOverviewBuilder::writeRn";

   ostringstream os;
   os << "creating r" << resLevel << "...";
   setCurrentMessage(os.str());

   if (resLevel == 0)
   {
      return false;
   }

   rspfIrect rect;
   sequencer->getOutputImageRectangle(rect);

   if (!setTags(tif, rect, resLevel))
   {
      setErrorStatus();
      closeTiff(tif);
      tif = 0;
      rspfNotify(rspfNotifyLevel_WARN) << MODULE << " Error writing tags!" << std::endl;
      return false;
   }

   if ( !buildInternalOverviews() && !copyR0() && (resLevel == 1) )
   {
      //---
      // Set the geotif tags for the first layer.
      // Note this is done in writeR0 method if m_copyAllFlag is set.
      //---
      if ( setGeotiffTags(m_imageHandler->getImageGeometry().get(),
                          rspfDrect(rect),
                          resLevel,
                          tif) == false )
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_NOTICE)
               << MODULE << " NOTICE: geotiff tags not set." << std::endl;
         } 
      }
   }

   rspf_uint32 outputTilesWide = sequencer->getNumberOfTilesHorizontal();
   rspf_uint32 outputTilesHigh = sequencer->getNumberOfTilesVertical();
   rspf_uint32 numberOfTiles   = sequencer->getNumberOfTiles();
   rspf_uint32 tileNumber      = 0;

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiffOverviewBuilder::writeRn DEBUG:"
         << "\noutputTilesWide:  " << outputTilesWide
         << "\noutputTilesHigh:  " << outputTilesHigh
         << "\nnumberOfTiles:    " << numberOfTiles
         << std::endl;
   }
 
   // Tile loop in the line direction.
   rspf_uint32 y = 0;

   for(rspf_uint32 i = 0; i < outputTilesHigh; ++i)
   {
      // Tile loop in the sample (width) direction.
      rspf_uint32 x = 0;
      for(rspf_uint32 j = 0; j < outputTilesWide; ++j)
      {
         // Grab the resampled tile.
         rspfRefPtr<rspfImageData> t = sequencer->getNextTile();

         // Check for errors reading tile:
         if ( sequencer->hasError() )
         {
            setErrorStatus();
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR: reading tile:  " << i << std::endl;
            return false;
         }
         
         if ( t.valid() && ( t->getDataObjectStatus() != RSPF_NULL ) )
         {
            // Write it to the tiff.
            for (rspf_uint32 band = 0; band < t->getNumberOfBands(); ++band)
            {
               // Write the tile.
               int bytesWritten = 0;
               bytesWritten = TIFFWriteTile(tif,
                                            t->getBuf(band),
                                            x,
                                            y,
                                            0,        // z
                                            band);    // sample
               
               if (bytesWritten != m_tileSizeInBytes)
               {
                  rspfNotify(rspfNotifyLevel_WARN)
                     << MODULE << " ERROR:"
                     << "Error returned writing tiff tile:  " << i
                     << "\nExpected bytes written:  " << m_tileSizeInBytes
                     << "\nBytes written:  " << bytesWritten
                     << std::endl;
                  theErrorStatus = rspfErrorCodes::RSPF_ERROR;
                  
                  return false;
               }
            }
         }
         x += m_tileWidth; // Increment x for next TIFFWriteTile.
         ++tileNumber;      // Increment tile number for percent complete.

      } // End of tile loop in the sample (width) direction.

      if (needsAborting())
      {
         setPercentComplete(100.0);
         break;
      }
      else
      {
         double tile = tileNumber;
         double numTiles = numberOfTiles;
         setPercentComplete(tile / numTiles * 100.0);
      }

      y += m_tileHeight; // Increment y for next TIFFWriteTile.

   } // End of tile loop in the line (height) direction.

   //---
   // Write the current dirctory.
   //---
   if (!TIFFFlush(tif))
   {
      setErrorStatus();
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " Error writing to TIF file!" << std::endl;
      return false;
   }

   if ( firstResLevel )
   {
      if ( rspfMpi::instance()->getNumberOfProcessors() == 1 )
      {
         if ( getHistogramMode() != RSPF_HISTO_MODE_UNKNOWN )
         {
            // Write the histogram.
            rspfFilename histoFilename = getOutputFile();
            histoFilename.setExtension("his");
            sequencer->writeHistogram(histoFilename);
         }

         if ( ( getScanForMinMaxNull() == true ) || ( getScanForMinMax() == true ) )
         {
            // Write the omd file:
            rspfFilename file = getOutputFile();
            file = file.setExtension("omd");
            sequencer->writeOmdFile(file);
         }
      }
   }

   ++m_currentTiffDir;

   return true;
}

//*******************************************************************
// Private Method:
//*******************************************************************
bool rspfTiffOverviewBuilder::setTags(TIFF* tif,
                                       const rspfIrect& outputRect,
                                       rspf_int32 resLevel) const
{
   if (outputRect.hasNans())
   {
      return false;
   }
   
   rspf_int32   imageWidth      = outputRect.width();
   rspf_int32   imageHeight     = outputRect.height();
   int16         samplesPerPixel = m_imageHandler->getNumberOfOutputBands();
   rspf_float64 minSampleValue  = m_imageHandler->getMinPixelValue();
   rspf_float64 maxSampleValue  = m_imageHandler->getMaxPixelValue();

   if (resLevel)
   {
      TIFFSetField( tif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE );
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTiffOverviewBuilder::setTags DEBUG:"
         << "\nrrds_level:      " << resLevel
         << "\nimageWidth:      " << imageWidth
         << "\nimageHeight:     " << imageHeight
         << "\nminSampleValue:  " << minSampleValue
         << "\nmaxSampleValue:  " << maxSampleValue
         << std::endl;
   }
   TIFFSetField( tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE );
   TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, imageWidth);
   TIFFSetField( tif, TIFFTAG_IMAGELENGTH, imageHeight);
   TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, m_bitsPerSample );
   TIFFSetField( tif, TIFFTAG_SAMPLEFORMAT, m_sampleFormat );
   TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, samplesPerPixel );

   if( m_imageHandler->getNumberOfInputBands() == 3 )
      TIFFSetField( tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
   else
      TIFFSetField( tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );
   
   TIFFSetField( tif, TIFFTAG_TILEWIDTH,  m_tileWidth  );
   TIFFSetField( tif, TIFFTAG_TILELENGTH, m_tileHeight );

   //---
   // Only turn on compression for 8 bit, one or three band data.  Not sure what compression
   // types can handle what but this was crashing rspf-prepoc on a directory walk with jpeg
   // compression.
   //---
   if ( ( m_imageHandler->getOutputScalarType() == RSPF_UINT8 ) &&
        ( ( m_imageHandler->getNumberOfInputBands() == 3 ) ||
          ( m_imageHandler->getNumberOfInputBands() == 1 ) ) )
   {
      // Set the compression related tags...
      TIFFSetField( tif, TIFFTAG_COMPRESSION, m_tiffCompressType );
      if (m_tiffCompressType == COMPRESSION_JPEG)
      {
         TIFFSetField( tif, TIFFTAG_JPEGQUALITY,  m_jpegCompressQuality);
      }
   }
   else
   {
      if ( traceDebug() && (m_tiffCompressType != COMPRESSION_NONE ) )
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfTiffOverviewBuilder::setTags WARNING:\n"
            << "Compression not set for this data type:\n"
            << "scalar type: "
            << rspfScalarTypeLut::instance()->getEntryString(
               m_imageHandler->getOutputScalarType())
            << "\nband count: " << m_imageHandler->getNumberOfInputBands()
            << std::endl;
      }
      TIFFSetField( tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE );
   }
   
   // Set the min/max values.
   switch( m_imageHandler->getOutputScalarType() )
   {
      case RSPF_SINT16:
      case RSPF_FLOAT32:
      case RSPF_FLOAT64:
      case RSPF_NORMALIZED_DOUBLE:
         TIFFSetField( tif, TIFFTAG_SMINSAMPLEVALUE, minSampleValue );
         TIFFSetField( tif, TIFFTAG_SMAXSAMPLEVALUE, maxSampleValue );
         break;

      case RSPF_UINT8:
      case RSPF_USHORT11:
      case RSPF_UINT16:
      case RSPF_UINT32:
      default:
         TIFFSetField( tif, TIFFTAG_MINSAMPLEVALUE,
                       static_cast<int>(minSampleValue) );
         TIFFSetField( tif, TIFFTAG_MAXSAMPLEVALUE,
                       static_cast<int>(maxSampleValue) );
         break;
   }
   
    return true;
}

bool rspfTiffOverviewBuilder::setGeotiffTags(const rspfImageGeometry* geom,
                                              const rspfDrect& boundingRect,
                                              rspf_uint32 resLevel,
                                              TIFF* tif)
{
   bool result = false;

   if ( geom && tif )
   {
      const rspfProjection* proj = geom->getProjection();
      if (proj)
      {
         // Must duplicate if changing scale.
         rspfObject* obj = proj->dup();
         rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, obj);
         if ( mapProj )
         {
            if ( mapProj->hasModelTransform() == false )
            {
               if (resLevel)
               {
                  rspf_float64 factor = (rspf_float64)(1 << resLevel);
                  mapProj->applyScale(rspfDpt(factor, factor), true);
               }
               rspfRefPtr<rspfMapProjectionInfo> projInfo =
                  new rspfMapProjectionInfo(mapProj, boundingRect);
               result = rspfGeoTiff::writeTags(tif, projInfo);
            }
         }
         delete obj; // Cleanup from dup.
         obj = 0;
      }
   }
   
   return result;
}

TIFF* rspfTiffOverviewBuilder::openTiff(const rspfString& filename) const
{
   rspfString openMode;
   if ( !buildInternalOverviews() )
   {
      openMode = "w";

      rspf_uint64 fourGigs = (static_cast<rspf_uint64>(1024)*
                               static_cast<rspf_uint64>(1024)*
                               static_cast<rspf_uint64>(1024)*
                               static_cast<rspf_uint64>(4));
      rspfIrect bounds = m_imageHandler->getBoundingRect();
      rspf_uint64 byteCheck =
         (static_cast<rspf_uint64>(bounds.width())*
          static_cast<rspf_uint64>(bounds.height())*
          static_cast<rspf_uint64>(m_imageHandler->getNumberOfOutputBands())*
          static_cast<rspf_uint64>(rspf::scalarSizeInBytes(m_imageHandler->
                                                             getOutputScalarType())));
         
      if((byteCheck*static_cast<rspf_uint64>(2))>fourGigs)
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << " Big tiff activated\n";
         }
         openMode += "8";
      }
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG) << " No big tiff activated\n";
         }
      }
   }
   else
   {
      openMode = "r+"; // Append to existing file...
   }

   // Open:
   return XTIFFOpen( filename.c_str(), openMode.c_str() );
}

void rspfTiffOverviewBuilder::closeTiff(TIFF* tif)
{
   XTIFFClose( tif );
}

void rspfTiffOverviewBuilder::setCompressionType(rspf_uint16 compression_type)
{
   switch (compression_type)
   {
   case COMPRESSION_JPEG:
   case COMPRESSION_LZW:
   case COMPRESSION_DEFLATE:
   case COMPRESSION_PACKBITS:
      m_tiffCompressType = compression_type;
      break;
   default:
      m_tiffCompressType = COMPRESSION_NONE;
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << __FILE__ << " " << __LINE__
            << "\nrspfTiffOverviewBuilder::setCompressionType Unsupported compression type: "
            << compression_type << "\nDefaulting to none."
            << std::endl;
      }
      break;
   }
}

void rspfTiffOverviewBuilder::setJpegCompressionQuality(rspf_int32 quality)
{
   if (quality > 1 && quality < 101)
   {
      m_jpegCompressQuality = quality;
   }
   else
   {
      m_jpegCompressQuality = DEFAULT_COMPRESS_QUALITY;

      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfTiffOverviewBuilder::setJpegCompressionQuality\n"
         << "\nCompression quality of " << quality << " is out of range!"
         << "\nRange is 100 to 1.  Current quality set to default of 75."
         << std::endl;
   }
}

bool rspfTiffOverviewBuilder::getCopyAllFlag() const
{
   return m_copyAllFlag;
}

void rspfTiffOverviewBuilder::setCopyAllFlag(bool flag)
{
   m_copyAllFlag = flag;
}

void rspfTiffOverviewBuilder::setInternalOverviewsFlag( bool flag )
{
   m_internalOverviewsFlag = flag;
}

bool rspfTiffOverviewBuilder::getInternalOverviewsFlag() const
{
   return m_internalOverviewsFlag;
}

rspfObject* rspfTiffOverviewBuilder::getObject()
{
   return this;
}

const rspfObject* rspfTiffOverviewBuilder::getObject() const
{
   return this;
}

void rspfTiffOverviewBuilder::setOutputFile(const rspfFilename& file)
{
   m_outputFile = file;
}

rspfFilename rspfTiffOverviewBuilder::getOutputFile() const
{
   rspfFilename result;

   if ( buildInternalOverviews() )
   {
      // m_imageHandler pointer good if buildInternalOverviews() returns true.
      result = m_imageHandler->getFilename();
   }
   else
   {
      result = m_outputFile;
   }

   if (m_outputFile == rspfFilename::NIL)
   {
      if ( m_imageHandler.valid() )
      {
         bool usePrefix = (m_imageHandler->getNumberOfEntries()>1?true:false);
         result = m_imageHandler->
            getFilenameWithThisExtension(rspfString("ovr"), usePrefix);
      }
   }
   
   return result;
}

void rspfTiffOverviewBuilder::setOutputTileSize(const rspfIpt& tileSize)
{
   m_tileWidth  = tileSize.x;
   m_tileHeight = tileSize.y;
   m_outputTileSizeSetFlag = true;
}

bool rspfTiffOverviewBuilder::setInputSource(rspfImageHandler* imageSource)
{
   static const char MODULE[] = "rspfTiffOverviewBuilder::initializeFromHandler";

   bool result = rspfOverviewBuilderBase::setInputSource( imageSource );
   
   if ( result )
   {
      if(!m_outputTileSizeSetFlag)
      {
         rspfIpt tileSize;
         rspf::defaultTileSize(tileSize);
         m_tileWidth  = tileSize.x;
         m_tileHeight = tileSize.y;
      }
      
      if (traceDebug())
      {
         CLOG << "DEBUG:"
              << "\nm_tileWidth:   " << m_tileWidth
              << "\nm_tileHeight:  " << m_tileHeight
              << "\nSource image is tiled:  "
              << (m_imageHandler->isImageTiled()?"true":"false")
              << "\nm_imageHandler->getTileWidth():  "
              << m_imageHandler->getTileWidth()
              << "\nm_imageHandler->getTileHeight():  "
              << m_imageHandler->getTileHeight()
              << "\nm_imageHandler->getImageTileWidth():  "
              << m_imageHandler->getImageTileWidth()
              << "\nm_imageHandler->getImageTileHeight():  "
              << m_imageHandler->getImageTileHeight()
              << std::endl;
      }

      switch(m_imageHandler->getOutputScalarType())
      {
         case RSPF_UINT8:
            m_bitsPerSample = 8;
            m_bytesPerPixel = 1;
            m_sampleFormat  = SAMPLEFORMAT_UINT;
            break;
            
         case RSPF_USHORT11:
         case RSPF_UINT16:
            m_bitsPerSample = 16;
            m_bytesPerPixel = 2;
            m_sampleFormat  = SAMPLEFORMAT_UINT;
            break;
            
         case RSPF_SINT16:
            m_bitsPerSample = 16;
            m_bytesPerPixel = 2;
            m_sampleFormat  = SAMPLEFORMAT_INT;
            break;
            
         case RSPF_SINT32:
            m_bitsPerSample = 32;
            m_bytesPerPixel = 4;
            m_sampleFormat  = SAMPLEFORMAT_INT;
            break;
            
         case RSPF_UINT32:
            m_bitsPerSample = 32;
            m_bytesPerPixel = 4;
            m_sampleFormat  = SAMPLEFORMAT_UINT;
            break;
            
         case RSPF_FLOAT32:
            m_bitsPerSample = 32;
            m_bytesPerPixel = 4;
            m_sampleFormat  = SAMPLEFORMAT_IEEEFP;
            break;
            
         case RSPF_NORMALIZED_DOUBLE:
         case RSPF_FLOAT64:
            m_bitsPerSample = 64;
            m_bytesPerPixel = 8;
            m_sampleFormat  = SAMPLEFORMAT_IEEEFP;
            break;
            
         default:
            // Set the error...
            setErrorStatus();
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "\nUnknow pixel type:  "
               << (rspfScalarTypeLut::instance()->
                   getEntryString(m_imageHandler->getOutputScalarType()))
               << std::endl;
            result = false;
      }

      if ( result )
      {
         m_tileSizeInBytes = m_tileWidth * m_tileHeight * m_bytesPerPixel;
         
         //---
         // Make a buffer to pass to pass to the write tile methods when an image
         // handler returns a null tile.
         //---
         m_nullDataBuffer.resize(m_tileSizeInBytes);
      
         // Fill it with zeroes.
         std::fill(m_nullDataBuffer.begin(), m_nullDataBuffer.end(), 0);
      }
   }
   else
   {
      // Set the error...
      setErrorStatus();
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nSetting image handler as input failed!"
         << std::endl;
   }

   return result;

} // End: rspfTiffOverviewBuilder::setInputSource(rspfImageHandler* imageSource)

bool rspfTiffOverviewBuilder::setOverviewType(const rspfString& type)
{
   bool result = true;
   if (type == "rspf_tiff_nearest")
   {
      m_resampleType =
         rspfFilterResampler::rspfFilterResampler_NEAREST_NEIGHBOR;
   }
   else if (type == "rspf_tiff_box")
   {
      m_resampleType = rspfFilterResampler::rspfFilterResampler_BOX;
   }
   else
   {
      result = false;
   }
   return result;
}

rspfString rspfTiffOverviewBuilder::getOverviewType() const
{
   rspfString type;
   if (m_resampleType == rspfFilterResampler::rspfFilterResampler_NEAREST_NEIGHBOR)
   {
      type = "rspf_tiff_nearest";
   }
   else
   {
      type = "rspf_tiff_box"; // This is default...
   }
   return type;
}

void rspfTiffOverviewBuilder::getTypeNameList(
   std::vector<rspfString>& typeList)const
{
   typeList.push_back(rspfString("rspf_tiff_box"));
   typeList.push_back(rspfString("rspf_tiff_nearest"));
}

void rspfTiffOverviewBuilder::setProperty(rspfRefPtr<rspfProperty> property)
{
   if ( property.valid() )
   {
      if(property->getName() == rspfKeywordNames::COMPRESSION_QUALITY_KW)
      {
         m_jpegCompressQuality = property->valueToString().toInt32();
      }
      else if(property->getName() == rspfKeywordNames::COMPRESSION_TYPE_KW)
      {
         rspfString value = property->valueToString();
         value = value.downcase();
         if(value == "jpeg")
         {
            m_tiffCompressType =  COMPRESSION_JPEG;
         }
         else if(value == "lzw")
         {
            m_tiffCompressType =  COMPRESSION_LZW;
            
         }
         else if(value == "deflate")
         {
            m_tiffCompressType =  COMPRESSION_DEFLATE;
         }
         else if(value == "packbits")
         {
            m_tiffCompressType =  COMPRESSION_PACKBITS;
         }
         else
         {
            m_tiffCompressType = COMPRESSION_NONE;
         }
      }
      else if(property->getName() == COPY_ALL_KW)
      {
         m_copyAllFlag = property->valueToString().toBool();
      }
      else if( property->getName() == INTERNAL_OVERVIEWS_KW )
      {
         m_internalOverviewsFlag = property->valueToString().toBool();
      }
      else if(property->getName() == rspfKeywordNames::OVERVIEW_STOP_DIMENSION_KW)
      {
         m_overviewStopDimension = property->valueToString().toUInt32();
      }
      else if(property->getName() == rspfKeywordNames::OUTPUT_TILE_SIZE_KW)
      {
         rspfIpt ipt;
         
         ipt.toPoint(property->valueToString());
         
         setOutputTileSize(ipt);
      }
   }
}

void rspfTiffOverviewBuilder::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(rspfKeywordNames::COMPRESSION_QUALITY_KW);
   propertyNames.push_back(rspfKeywordNames::COMPRESSION_TYPE_KW);
   propertyNames.push_back(COPY_ALL_KW);
   propertyNames.push_back(INTERNAL_OVERVIEWS_KW);
   propertyNames.push_back(rspfKeywordNames::OVERVIEW_STOP_DIMENSION_KW);
}

bool rspfTiffOverviewBuilder::canConnectMyInputTo(
   rspf_int32 index,
   const rspfConnectableObject* obj) const
{
   if ( (index == 0) &&
        PTR_CAST(rspfImageHandler, obj) )
   {
      return true;
   }

   return false;
}

bool rspfTiffOverviewBuilder::buildInternalOverviews() const
{
   bool result = false;
   if ( m_internalOverviewsFlag &&  m_imageHandler.valid() )
   {
      if ( m_imageHandler->getClassName() == "rspfTiffTileSource" )
      {
         result = true;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_NOTICE)
            << "Internal overviews opton not supported for class: "
            << m_imageHandler->getClassName()
            << std::endl;
      }
   }
   return result;
}

bool rspfTiffOverviewBuilder::copyR0() const
{
   return ( m_copyAllFlag && !buildInternalOverviews());
}
