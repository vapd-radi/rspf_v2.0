//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  Frank Warmerdam (warmerda@home.com)
//
//*******************************************************************
//  $Id: rspfTiffWriter.cpp 22232 2013-04-13 20:06:19Z dburken $

#include <rspf/rspfConfig.h>
#include <rspf/imaging/rspfTiffWriter.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfMapProjectionInfo.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfEpsgProjectionDatabase.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfStatePlaneProjectionInfo.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/base/rspfFilenameProperty.h>
#include <rspf/support_data/rspfGeoTiff.h>
#include <rspf/imaging/rspfMemoryImageSource.h>
#include <rspf/imaging/rspfScalarRemapper.h>

#include <tiffio.h>
#ifdef RSPF_HAS_GEOTIFF
#  if RSPF_HAS_GEOTIFF
#    include <xtiffio.h>
#    include <geotiff.h>
#    include <geo_normalize.h>
#    include <geovalues.h>
#  endif
#endif

#include <algorithm>
#include <sstream>

static rspfTrace traceDebug("rspfTiffWriter:debug");
static const char* TIFF_WRITER_OUTPUT_TILE_SIZE_X_KW = "output_tile_size_x";
static const char* TIFF_WRITER_OUTPUT_TILE_SIZE_Y_KW = "output_tile_size_y";
static const int   PCS_BRITISH_NATIONAL_GRID = 27700;
static const long  DEFAULT_JPEG_QUALITY = 75;

RTTI_DEF1(rspfTiffWriter, "rspfTiffWriter", rspfImageFileWriter);

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfTiffWriter.cpp 22232 2013-04-13 20:06:19Z dburken $";
#endif

rspfTiffWriter::rspfTiffWriter()
   :
      rspfImageFileWriter(),
      theTif(NULL),
      theCompressionType("none"),
      theJpegQuality(DEFAULT_JPEG_QUALITY),
      theOutputGeotiffTagsFlag(true),
      theColorLutFlag(false),
      theProjectionInfo(NULL),
      theOutputTileSize(RSPF_DEFAULT_TILE_WIDTH, RSPF_DEFAULT_TILE_HEIGHT),
      theForceBigTiffFlag(false),
      theBigTiffFlag(false)
{
   theColorLut = new rspfNBandLutDataObject();
   rspf::defaultTileSize(theOutputTileSize);
   theOutputImageType = "tiff_tiled_band_separate";

   
#ifdef RSPF_ID_ENABLED /* to quell unused variable warning. */
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)<< "RSPF_ID:  " << RSPF_ID << endl;
   }
#endif
}

rspfTiffWriter::~rspfTiffWriter()
{
   if(isOpen())
   {
      closeTiff();
   }
}

bool rspfTiffWriter::openTiff()
{
   static const char* MODULE = "rspfTiffWriter::openTiff()";

   if (theTif) // Close the existing file pointer.
   {
      
#ifdef RSPF_HAS_GEOTIFF
#  if RSPF_HAS_GEOTIFF 
      XTIFFClose( (TIFF*)theTif );
#  else
      TIFFClose( (TIFF*)theTif );
#  endif
#else
      TIFFClose( (TIFF*)theTif );   
#endif

   }

   // Check for empty file name.
   if (theFilename.empty())
   {
      return false;
   }

   rspfString openMode = "w";
   if(theBigTiffFlag||theForceBigTiffFlag)
   {
      openMode += "8";
   }
   // Open the new file.
#ifdef RSPF_HAS_GEOTIFF
#  if RSPF_HAS_GEOTIFF
   theTif = XTIFFOpen( theFilename.c_str(), openMode.c_str() );
#  else
   theTif = TIFFOpen( theFilename.c_str(), openMode.c_str() );
#  endif
#else
   theTif = TIFFOpen( theFilename.c_str(), openMode.c_str() );
#endif

   if (!theTif)
   {
      setErrorStatus(); // base class
      rspfSetError(getClassName().c_str(),
                    rspfErrorCodes::RSPF_ERROR,
                    "File %s line %d Module %s Error:\n\
Error opening file:  %s\n",
                    __FILE__,
                    __LINE__,
                    MODULE,
                    theFilename.c_str());

      return false;
   }
   return true;
}

bool rspfTiffWriter::closeTiff()
{
   if (theTif)
   {
#ifdef RSPF_HAS_GEOTIFF
#  if RSPF_HAS_GEOTIFF
      XTIFFClose( (TIFF*)theTif );
#  else
      TIFFClose( (TIFF*)theTif );
#  endif
#else
      TIFFClose( (TIFF*)theTif );
#endif
      theTif = NULL;
   }

   return true;
}

bool rspfTiffWriter::writeTiffTags()
{
   static const char MODULE[] = "rspfTiffWriter::writeTiffTags";
   TIFF* tiffPtr = (TIFF*)theTif;
   if (!tiffPtr)
   {
      setErrorStatus(); // base class
      rspfSetError(getClassName().c_str(),
                    rspfErrorCodes::RSPF_ERROR,
                    "File %s line %d %s\nError:  Tiff pointer is null!\n\
Call setFilename method.\n",
                    __FILE__,
                    __LINE__,
                    MODULE);
      return false;
   }

   //---
   // NOTE:
   // Since the tiff library uses the variable argument list function "va_arg"
   // it is important to use the correct data type.  If in doubt see the
   // code for libtiff's _TIFFVSetField in "tif_dir.c" in the libtiff package.
   //---

   int bitsPerSample  = 0;
   int sampleFormat   = 0;
   switch( theInputConnection->getOutputScalarType() )
   {
   case RSPF_UINT8:
      bitsPerSample = 8;
      sampleFormat = SAMPLEFORMAT_UINT;
      break;

   case RSPF_UINT16:
   case RSPF_USHORT11:
      bitsPerSample = 16;
      sampleFormat = SAMPLEFORMAT_UINT;
      break;

   case RSPF_SINT16:
      bitsPerSample = 16;
      sampleFormat = SAMPLEFORMAT_INT;
      break;

   case RSPF_FLOAT32:
   case RSPF_NORMALIZED_FLOAT:
      bitsPerSample = 32;
      sampleFormat = SAMPLEFORMAT_IEEEFP;
      break;

   case RSPF_NORMALIZED_DOUBLE:
   case RSPF_FLOAT64:
      bitsPerSample = 64;
      sampleFormat = SAMPLEFORMAT_IEEEFP;
      break;

   default:
      return false;
   }

   // Set the pixel type.
   TIFFSetField( (TIFF*)tiffPtr, TIFFTAG_BITSPERSAMPLE, bitsPerSample );
   TIFFSetField( (TIFF*)tiffPtr, TIFFTAG_SAMPLEFORMAT, sampleFormat );

   // Set the image dimensions.
   rspf_uint32  width  = theAreaOfInterest.width();
   rspf_uint32  height = theAreaOfInterest.height();
   TIFFSetField( tiffPtr, TIFFTAG_IMAGEWIDTH, width);
   TIFFSetField( tiffPtr, TIFFTAG_IMAGELENGTH, height);
   if (isTiled())
   {
      rspf_uint32 tileXSize = theOutputTileSize.x;
      rspf_uint32 tileYSize = theOutputTileSize.y;
      TIFFSetField(tiffPtr, TIFFTAG_TILEWIDTH,  tileXSize);
      TIFFSetField(tiffPtr, TIFFTAG_TILELENGTH, tileYSize);
   }
   else
   {
      TIFFSetField(tiffPtr, TIFFTAG_ROWSPERSTRIP, rspf_uint32(1));
   }

   rspf_uint32 numberOfBands = theInputConnection->getNumberOfOutputBands();

   // Set the min/max values.
   std::vector<rspf_float64> minBand(numberOfBands);
   std::vector<rspf_float64> maxBand(numberOfBands);
   for(rspf_uint32 idx = 0; idx < numberOfBands; ++idx)
   {
      maxBand[idx] = theInputConnection->getMaxPixelValue(idx);
      minBand[idx] = theInputConnection->getMinPixelValue(idx);
   }
   
   writeMinMaxTags(minBand, maxBand);
   
   // Set the planar configuration.
   if ( (theOutputImageType == "tiff_strip") ||
        (theOutputImageType == "tiff_tiled") ||
        (theOutputImageType == "image/tiff") ||
        (theOutputImageType == "image/tif") ||
        (theOutputImageType == "image/gtif") ||
        (theOutputImageType == "image/gtiff") )
   {
      TIFFSetField( tiffPtr, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
   }
   else
   {
      TIFFSetField( tiffPtr, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);
   }

   // Set the compression type:
   uint16 tiffCompressType = COMPRESSION_NONE;
   theCompressionType.downcase();
   if( theCompressionType == "jpeg")
   {
      tiffCompressType  = COMPRESSION_JPEG;

      // Set the jpeg quality.
      TIFFSetField( tiffPtr, TIFFTAG_JPEGQUALITY,  theJpegQuality);
   }
   else if(theCompressionType == "packbits")
   {
      tiffCompressType = COMPRESSION_PACKBITS;
   }
   else if((theCompressionType == "deflate") ||
           (theCompressionType == "zip"))
   {
      tiffCompressType  = COMPRESSION_DEFLATE;
   }
   TIFFSetField( tiffPtr, TIFFTAG_COMPRESSION, tiffCompressType);
   TIFFSetField(tiffPtr, TIFFTAG_SAMPLESPERPIXEL, (int)theInputConnection->getNumberOfOutputBands());

   rspfScalarType scalarType = theInputConnection->getOutputScalarType();
   bool lutEnabled = (theColorLutFlag&&
                      ((scalarType == RSPF_UINT8)||
                       (scalarType == RSPF_UINT16)||
                       (scalarType == RSPF_USHORT11))&&
                      (theColorLut->getNumberOfEntries() > 0)&&
                      (theInputConnection->getNumberOfOutputBands() == 1));
   if(lutEnabled)
   {
      TIFFSetField( tiffPtr, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE );
      TIFFSetField( tiffPtr, TIFFTAG_INDEXED, (rspf_uint16)1);

      if(scalarType == RSPF_UINT8)
      {
         rspf_uint16 r[256], g[256], b[256];
         
         memset(r, '\0', sizeof(rspf_uint16)*256);
         memset(g, '\0', sizeof(rspf_uint16)*256);
         memset(b, '\0', sizeof(rspf_uint16)*256);
         
         for(rspf_uint32 i = 0; i < theColorLut->getNumberOfEntries(); i++)
         {
            r[i] = (rspf_uint16) (((*theColorLut)[i][0]/255.0)*65535);
            g[i] = (rspf_uint16) (((*theColorLut)[i][1]/255.0)*65535);
            b[i] = (rspf_uint16) (((*theColorLut)[i][2]/255.0)*65535);
         }
         TIFFSetField(tiffPtr, TIFFTAG_COLORMAP, r, g ,b);
      }
      else
      {
         rspf_uint16 r[65536], g[65536], b[65536];
         memset(r, '\0', sizeof(rspf_uint16)*65536);
         memset(g, '\0', sizeof(rspf_uint16)*65536);
         memset(b, '\0', sizeof(rspf_uint16)*65536);
         
         for(rspf_uint32 i = 0; i < theColorLut->getNumberOfEntries(); i++)
         {
            r[i] = (rspf_uint16) ((*theColorLut)[i][0]);
            g[i] = (rspf_uint16) ((*theColorLut)[i][1]);
            b[i] = (rspf_uint16) ((*theColorLut)[i][2]);
         }
         TIFFSetField(tiffPtr, TIFFTAG_COLORMAP, r, g ,b);
      }
   }
   else if( (theInputConnection->getNumberOfOutputBands() == 3 ||
             theInputConnection->getNumberOfOutputBands() == 4 ||
             (thePhotoMetric == "rgb"))&&
            (scalarType == RSPF_UCHAR))
   {
      TIFFSetField( tiffPtr, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );
   }
   else
   {
      TIFFSetField( tiffPtr, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK );
   }

   return true;
}

bool rspfTiffWriter::writeGeotiffTags(rspfRefPtr<rspfMapProjectionInfo> projectionInfo)
{
   TIFF* tiffPtr = (TIFF*)theTif;
   bool result = false;
   if ( tiffPtr )
   {
      if ( projectionInfo.valid() )
      {
         result = rspfGeoTiff::writeTags(tiffPtr, projectionInfo);
      }
   }
   return result;
}

void rspfTiffWriter::checkColorLut()
{
   
   // this code appears to be wrong.  We can only do an outo lut if the immediate input to the sequencer is 
   // a handler with a lut or some kind of lut source.
   // 
   // I think that we should add a flag to enable auto setting of the lut instead of just doing it. This code causes core
   // dumps if one is to replicate bands with a band selector where a lut is a single band output like a CIB
   //
   // I currenlty have to disable
   //
   
   
#if 0
   bool needColorLut = false;
   bool needLoop = true;
   rspfRefPtr<rspfNBandLutDataObject> colorLut = 0;
   rspfConnectableObject::ConnectableObjectList imageInputs = theInputConnection->getInputList();
   if (imageInputs.size() > 0)
   {
      for (rspf_uint32 i = 0; i < imageInputs.size(); i++)
      {
         if (needLoop == false)
         {
            break;
         }
         rspfImageChain* source = PTR_CAST(rspfImageChain, imageInputs[i].get());
         if (source)
         {
            rspfConnectableObject::ConnectableObjectList imageChains = source->getInputList();
            for (rspf_uint32 j = 0; j < imageChains.size(); j++)
            {
               if (needLoop == false)
               {
                  break;
               }
               rspfImageChain* imageChain = PTR_CAST(rspfImageChain, imageChains[j].get());
               if (imageChain)
               {
                  rspfConnectableObject::ConnectableObjectList imageHandlers =
                     imageChain->findAllObjectsOfType(STATIC_TYPE_INFO(rspfImageHandler), false);
                  
                  for (rspf_uint32 h= 0; h < imageHandlers.size(); h++)
                  {
                     rspfImageHandler* handler =
                        PTR_CAST(rspfImageHandler, imageHandlers[h].get());
                     if (handler)
                     {
                        if (handler->getLut() != 0) //
                        {
                           colorLut = handler->getLut();
                           needColorLut = true;
                        }
                        else //if not all handlers have color luts, ignore the color lut.
                        {
                           needColorLut = false;
                           needLoop = false;
                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   if (needColorLut && colorLut != 0)
   {
      setLut(*colorLut.get());
   }
#endif
}

bool rspfTiffWriter::writeFile()

{
   static const char MODULE[] = "rspfTiffWriter::writeFile";

   if (traceDebug()) CLOG << "Entered..." << std::endl;

   //checkColorLut();

   if(isLutEnabled())
   {
      theNBandToIndexFilter = new rspfNBandToIndexFilter;
      theNBandToIndexFilter->connectMyInputTo(0, theInputConnection->getInput());
      theNBandToIndexFilter->setLut(*theColorLut.get());
      theNBandToIndexFilter->initialize();
      theInputConnection->disconnect();
      theInputConnection->connectMyInputTo(0, theNBandToIndexFilter.get());
      theInputConnection->initialize();
   }
   else
   {
      theNBandToIndexFilter = 0;
   }
   
   if (traceDebug() && theInputConnection.get())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << "DEBUG:"
         << "\nnull:  " << theInputConnection->getNullPixelValue()
         << "\nmin:   " << theInputConnection->getMinPixelValue()
         << "\nmax:   " << theInputConnection->getMaxPixelValue()
         << std::endl;
   }
   
   if (isTiled())
   {
      if ( (theInputConnection->getTileWidth()  !=
            static_cast<rspf_uint32>(theOutputTileSize.x)) ||
           (theInputConnection->getTileHeight() !=
            static_cast<rspf_uint32>(theOutputTileSize.y)) )
      {
         theInputConnection->setTileSize(theOutputTileSize);
      }
   }
   
   if(!theInputConnection->isMaster())
   {
      theInputConnection->slaveProcessTiles();

      if(theNBandToIndexFilter.valid())
      {
         theInputConnection->connectMyInputTo(0, theNBandToIndexFilter->getInput());
         theNBandToIndexFilter = 0;
      }
      
      return true;
   }

   // this might be called from writeFile(projection infoamrtion) method
   // we will check to see if the tiff is open.  If not then call the open.
   //
   if(isOpen())
   {
      close();
   }
   rspf_uint64 threeGigs = (static_cast<rspf_uint64>(1024)*
                            static_cast<rspf_uint64>(1024)*
                            static_cast<rspf_uint64>(1024)*
                            static_cast<rspf_uint64>(3));
   rspfIrect bounds = theInputConnection->getBoundingRect();
   rspf_uint64 byteCheck = (static_cast<rspf_uint64>(bounds.width())*
                             static_cast<rspf_uint64>(bounds.height())*
                             static_cast<rspf_uint64>(theInputConnection->getNumberOfOutputBands())*
                             static_cast<rspf_uint64>(rspf::scalarSizeInBytes(theInputConnection->getOutputScalarType())));
	
   if(byteCheck > threeGigs)
   {
      theBigTiffFlag = true;
   }
   else
   {
      theBigTiffFlag = false;
   }
   open();

   if (!isOpen())
   {
      if (traceDebug())
      {
         CLOG << " ERROR:  Could not open!  Returning..." << std::endl;
      }

      return false;
   }

   if(!theInputConnection)
   {
      if (traceDebug())
      {
         CLOG << " ERROR:  No input connection!  Returning..." << std::endl;
      }

      return false;
   }

   // First write the tiff tags.
   if (writeTiffTags() == false)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " ERROR:"
            << "\nError detected writing tiff tags.  Returning..." << std::endl;
         return false;
      }
   }

   // Write the geotiff tags.
   if (theOutputGeotiffTagsFlag)
   {
      if(theViewController.get()) // let this override
      {                     // found in rspfImageWriter base
         rspfMapProjection* proj = PTR_CAST(rspfMapProjection,
                                             theViewController->getView());
         if(proj)
         {
            rspfRefPtr<rspfMapProjectionInfo> projectionInfo
               = new rspfMapProjectionInfo(proj, theAreaOfInterest);
            projectionInfo->setPixelType(thePixelType);

            if (writeGeotiffTags(projectionInfo) == false)
            {
               if(traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_DEBUG)
                     << MODULE << " ERROR:"
                     << "\nError detected writing geotiff tags.  Returning..."
                     << std::endl;
               }
               return false;
            }
         }
      }
      else if(theProjectionInfo.valid())
      {
         theProjectionInfo->setPixelType(thePixelType);
         if (writeGeotiffTags(theProjectionInfo) == false)
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << MODULE << " ERROR:"
                  << "\nError detected writing geotiff tags.  Returning..."
                  << std::endl;
            }
            return false;
         }
      }
      else
      {
         // Fetch the map projection of the input image if it exists:
         rspfMapProjection* mapProj = 0;
         rspfRefPtr<rspfImageGeometry> imgGeom = theInputConnection->getImageGeometry();
         if ( imgGeom.valid() )
         {
            const rspfProjection* proj = imgGeom->getProjection();
            mapProj = PTR_CAST(rspfMapProjection, proj);
         }
         if(mapProj)
         {
            rspfRefPtr<rspfMapProjectionInfo> projectionInfo
               = new rspfMapProjectionInfo(mapProj, theAreaOfInterest);

            projectionInfo->setPixelType(thePixelType);
            if (writeGeotiffTags(projectionInfo) == false)
            {
               if(traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_DEBUG)
                     << MODULE << " ERROR:"
                     << "\nError detected writing geotiff tags.  Returning..."
                     << std::endl;
               }
               return false;
            }
         }
      }

   } // End of "if (theOutputGeotiffTagsFlag)"

   // Write the file with the image data.
   bool status = false;
   if(theOutputImageType == "tiff_strip")
   {
      status = writeToStrips();
   }
   else if(theOutputImageType == "tiff_strip_band_separate")
   {
      status = writeToStripsBandSep();
   }
   else if((theOutputImageType == "tiff_tiled")||
           (rspfString::downcase(theOutputImageType) == "image/tiff")||
           (rspfString::downcase(theOutputImageType) == "image/tif")||
           (rspfString::downcase(theOutputImageType) == "image/gtif")||
           (rspfString::downcase(theOutputImageType) == "image/gtiff"))
   {
      status = writeToTiles();
   }
   else if(theOutputImageType == "tiff_tiled_band_separate")
   {
      status = writeToTilesBandSep();
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " ERROR:"
            << "\nUnsupported output type:  " << theOutputImageType
            << std::endl;
      }
   }

   close();

   if (traceDebug()) CLOG << " Exited..." << std::endl;

   if(theNBandToIndexFilter.valid())
   {
      theInputConnection->connectMyInputTo(0, theNBandToIndexFilter->getInput());
      theNBandToIndexFilter = 0;
   }
   
   return status;
}

void rspfTiffWriter::setLut(const rspfNBandLutDataObject& lut)
{
   theColorLutFlag = true;
   theColorLut = (rspfNBandLutDataObject*)lut.dup();
}

bool rspfTiffWriter::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   kwl.add(prefix,
           "output_geotiff_flag",
           (int)theOutputGeotiffTagsFlag,
           true);

   kwl.add(prefix,
           TIFF_WRITER_OUTPUT_TILE_SIZE_X_KW,
           theOutputTileSize.x,
           true);

   kwl.add(prefix,
           TIFF_WRITER_OUTPUT_TILE_SIZE_Y_KW,
           theOutputTileSize.y,
           true);

   kwl.add(prefix,
           rspfKeywordNames::COMPRESSION_QUALITY_KW,
           theJpegQuality,
           true);

   kwl.add(prefix,
           rspfKeywordNames::COMPRESSION_TYPE_KW,
           theCompressionType,
           true);

   kwl.add(prefix,
           "color_lut_flag",
           (rspf_uint32)theColorLutFlag,
           true);

   if(theColorLutFlag)
   {
      if(theLutFilename != "")
      {
         kwl.add(prefix,
                 "lut_filename",
                 theLutFilename.c_str(),
                 true);
      }
      else
      {
         rspfString newPrefix = rspfString(prefix) + "lut.";
         theColorLut->saveState(kwl, newPrefix.c_str());
      }
   }


   return rspfImageFileWriter::saveState(kwl,
                                          prefix);
}

bool rspfTiffWriter::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   const char *value;

   rspfIpt defaultTileSize;
   rspf::defaultTileSize(defaultTileSize);

   value = kwl.find(prefix,
                    TIFF_WRITER_OUTPUT_TILE_SIZE_X_KW);
   if(value)
   {
      theOutputTileSize.x = rspfString(value).toLong();
      if(theOutputTileSize.x<1)
      {
         theOutputTileSize.x = defaultTileSize.x;
      }
   }

   value = kwl.find(prefix,
                    TIFF_WRITER_OUTPUT_TILE_SIZE_Y_KW);
   if(value)
   {
      theOutputTileSize.y = rspfString(value).toLong();
      if(theOutputTileSize.y<1)
      {
         theOutputTileSize.y = defaultTileSize.y;
      }
   }


   value = kwl.find(prefix, rspfKeywordNames::COMPRESSION_TYPE_KW);
   if(value)
   {
      theCompressionType = rspfString(value).downcase();
   }
   else
   {
      theCompressionType = "none";
   }

   value = kwl.find(prefix, rspfKeywordNames::COMPRESSION_QUALITY_KW);
   if(value)
   {
      setJpegQuality(rspfString(value).toLong());
   }

   value = kwl.find(prefix, rspfKeywordNames::PHOTOMETRIC_KW);
   if(value)
   {
      thePhotoMetric = rspfString(value).downcase();
   }

   value = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if(value)
   {
      setFilename(rspfFilename(value));
   }

   const char* flag = kwl.find(prefix, "output_geotiff_flag");
   if(flag)
   {
      theOutputGeotiffTagsFlag = rspfString(flag).toBool();
   }

   rspfString newPrefix = rspfString(prefix) + "lut.";

   const char* colorLutFlag = kwl.find(prefix, "color_lut_flag");
   if(colorLutFlag)
   {
      theColorLutFlag = rspfString(colorLutFlag).toBool();
   }
   else
   {
      theColorLutFlag = false;
   }
   theLutFilename = rspfFilename(kwl.find(prefix, "lut_filename"));

   theLutFilename = rspfFilename(theLutFilename.trim());
   if(theLutFilename != "")
   {
      theColorLut->open(theLutFilename);
   }
   else
   {
      theColorLut->loadState(kwl, newPrefix.c_str());
   }

   if(rspfImageFileWriter::loadState(kwl,
                                      prefix))
   {
      if((theOutputImageType!="tiff_tiled") &&
         (theOutputImageType!="tiff_tiled_band_separate") &&
         (theOutputImageType!="tiff_strip") &&
         (theOutputImageType!="tiff_strip_band_separate")&&
         (theOutputImageType!="image/tiff")&&
         (theOutputImageType!="image/tif")&&
         (theOutputImageType!="image/gtif")&&
         (theOutputImageType!="image/gtiff"))
      {

         theOutputImageType = "tiff_tiled_band_separate";;
      }
   }
   else
   {
      return false;
   }

   return true;
}

bool rspfTiffWriter::isTiled() const
{
   return ( theOutputImageType == "tiff_tiled" ||
            theOutputImageType == "image/tiff" ||
            theOutputImageType == "image/tif" ||
            theOutputImageType == "image/gtif" ||
            theOutputImageType == "image/gtiff" ||
            theOutputImageType == "tiff_tiled_band_separate" );
}

bool rspfTiffWriter::writeToTiles()
{
   static const char* const MODULE = "rspfTiffWriter::writeToTiles";
   TIFF* tiffPtr = (TIFF*)theTif;

   if (traceDebug()) CLOG << " Entered." << std::endl;

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   rspfRefPtr<rspfImageData> tempTile = 0;

   if(theColorLutFlag)
   {
      tempTile = rspfImageDataFactory::instance()->create(this, 1, theInputConnection.get());
   }
   else
   {
      tempTile = rspfImageDataFactory::instance()->create(this, theInputConnection.get());
   }
   if(tempTile.valid())
   {
      tempTile->initialize();
   }
   rspf_uint32 tilesWide       = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint32 tilesHigh       = theInputConnection->getNumberOfTilesVertical();
   rspf_uint32 tileWidth       = theInputConnection->getTileWidth();
   rspf_uint32 tileHeight      = theInputConnection->getTileHeight();
   rspf_uint32 numberOfTiles   = theInputConnection->getNumberOfTiles();

   // Tile loop in the height direction.
   rspf_uint32 tileNumber = 0;
   vector<rspf_float64> minBands;
   vector<rspf_float64> maxBands;
   for(rspf_uint32 i = 0; ((i < tilesHigh)&&!needsAborting()); i++)
   {
      rspfIpt origin(0,0);
      origin.y = i * tileHeight;

      // Tile loop in the sample (width) direction.
      for(rspf_uint32 j = 0; ((j < tilesWide)&&!needsAborting()); j++)
      {
         origin.x = j * tileWidth;

         // Grab the tile.
         rspfRefPtr<rspfImageData> id = theInputConnection->getNextTile();
         if (!id)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "Error returned writing tiff tile:  " << tileNumber
               << "\nNULL Tile encountered"
               << std::endl;
            return false;
         }

         rspfDataObjectStatus  tileStatus      = id->getDataObjectStatus();
         rspf_uint32           tileSizeInBytes = id->getSizeInBytes();
         if (tileStatus != RSPF_FULL)
         {
            // Clear out the buffer since it won't be filled all the way.
            tempTile->setImageRectangle(id->getImageRectangle());
            tempTile->makeBlank();
         }

         if ((tileStatus == RSPF_PARTIAL || tileStatus == RSPF_FULL))
         {
            // Stuff the tile into the tileBuffer.
            id->unloadTile(tempTile->getBuf(),
                           id->getImageRectangle(),
                           RSPF_BIP);
            tempTile->setDataObjectStatus(id->getDataObjectStatus());
            if(!theColorLutFlag&&!needsAborting())
            {
               id->computeMinMaxPix(minBands, maxBands);
            }
         }

         //---
         // Write the tile to disk.
         //---
         rspf_uint32 bytesWritten = 0;
         bytesWritten = TIFFWriteTile(tiffPtr,
                                      tempTile->getBuf(),
                                      origin.x,
                                      origin.y,
                                      0,            // z
                                      0);           // s

         if (bytesWritten != tileSizeInBytes)
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << MODULE << " ERROR:"
                  << "Error returned writing tiff tile:  " << i
                  << "\nExpected bytes written:  " << tileSizeInBytes
                  << "\nBytes written:  " << bytesWritten
                  << std::endl;
            }
            setErrorStatus();
            return false;
         }

         ++tileNumber;

      } // End of tile loop in the sample (width) direction.

      double tile = tileNumber;
      double numTiles = numberOfTiles;
      setPercentComplete(tile / numTiles * 100);

   } // End of tile loop in the line (height) direction.

   if(!theColorLutFlag&&!needsAborting())
   {
      writeMinMaxTags(minBands, maxBands);
   }

   if (traceDebug()) CLOG << " Exited." << std::endl;

   return true;
}

bool rspfTiffWriter::writeToTilesBandSep()
{
   static const char* const MODULE = "rspfTiffWriter::writeToTilesBandSep";
   TIFF* tiffPtr = (TIFF*)theTif;
   if (traceDebug()) CLOG << " Entered." << std::endl;

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   rspf_uint32 bands     = theInputConnection->getNumberOfOutputBands();
   rspf_uint32 tilesWide = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint32 tilesHigh = theInputConnection->getNumberOfTilesVertical();
   rspf_uint32 tileWidth     = theInputConnection->getTileWidth();
   rspf_uint32 tileHeight    = theInputConnection->getTileHeight();
   rspf_uint32 numberOfTiles = theInputConnection->getNumberOfTiles();

#if 0
   if(traceDebug())
   {
      rspfIrect   boundingRect  = theInputConnection->getBoundingRect();
      rspfNotify(rspfNotifyLevel_NOTICE)
         << "Bounding rect = " << boundingRect
         << "\nBands         = " << bands
         << "\ntilesWide     = " << tilesWide
         << "\ntilesHigh     = " << tilesHigh
         << "\ntileWidth     = " << tileWidth
         << "\ntileHeight    = " << tileHeight << std::endl;
   }
#endif

   rspf_uint32 tileNumber = 0;
   vector<rspf_float64> minBands;
   vector<rspf_float64> maxBands;
   for(rspf_uint32 i = 0; ((i < tilesHigh)&&!needsAborting()); ++i)
   {
      rspfIpt origin;
      origin.y = i * tileHeight;

      //---
      // Tile loop in the sample (width) direction.
      //---
      for(rspf_uint32 j = 0; ((j < tilesWide)&&!needsAborting()); ++j)
      {
         origin.x = j * tileWidth;

         rspfRefPtr<rspfImageData> id = theInputConnection->getNextTile();
	 if(!id)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "Error returned writing tiff tile:  " << i
               << "\nNULL Tile encountered"
               << std::endl;
            return false;
         }
	 rspf_int32 tileSizeInBytes = id->getSizePerBandInBytes();

         if(!theColorLutFlag)
         {
            id->computeMinMaxPix(minBands, maxBands);
         }

         //---
         // Band loop.
         //---
         for (rspf_uint32 band=0; ((band<bands)&&(!needsAborting())); ++band)
         {
            // Grab a pointer to the tile for the band.
            tdata_t* data = (tdata_t*)id->getBuf(band);
            // Write the tile.
            tsize_t bytesWritten = 0;
            if(data)
            {
               bytesWritten = TIFFWriteTile(tiffPtr,
                                            data,
                                            (rspf_uint32)origin.x,
                                            (rspf_uint32)origin.y,
                                            (rspf_uint32)0,        // z
                                            (tsample_t)band);    // sample
            }
            if ( ( bytesWritten != tileSizeInBytes ) && !needsAborting() )
            {
               if(traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_DEBUG)
                     << MODULE << " ERROR:"
                     << "Error returned writing tiff tile:  " << i
                     << "\nExpected bytes written:  " << tileSizeInBytes
                     << "\nBytes written:  " << bytesWritten
                     << std::endl;
               }
               setErrorStatus();
               return false;
            }
            
         } // End of band loop.

         ++tileNumber;

      } // End of tile loop in the sample (width) direction.

      double tile = tileNumber;
      double numTiles = numberOfTiles;
      setPercentComplete(tile / numTiles * 100);
      if(needsAborting())
      {
         setPercentComplete(100);
      }

   } // End of tile loop in the line (height) direction.

   if(!theColorLutFlag&&!needsAborting())
   {
      writeMinMaxTags(minBands, maxBands);
   }
    
   if (traceDebug()) CLOG << " Exited." << std::endl;

   return true;
}

bool rspfTiffWriter::writeToStrips()
{
   static const char* const MODULE = "rspfTiffWriter::writeToStrips";
   TIFF* tiffPtr = (TIFF*)theTif;

   if (traceDebug()) CLOG << " Entered." << std::endl;

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   rspf_uint32 bands = theInputConnection->getNumberOfOutputBands();
   rspf_uint32 tilesWide = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint32 tilesHigh = theInputConnection->getNumberOfTilesVertical();
   rspf_uint32 tileHeight = theInputConnection->getTileHeight();
   rspf_uint32 numberOfTiles = theInputConnection->getNumberOfTiles();
   rspf_uint32 width = theAreaOfInterest.width();
   rspf_uint32 bytesInLine =
      rspf::scalarSizeInBytes(theInputConnection->getOutputScalarType()) *
      width * bands;

   //---
   // Buffer to hold one line x tileHeight
   //---
   rspf_uint32 bufferSizeInBytes = bytesInLine * tileHeight;
   unsigned char* buffer = new unsigned char[bufferSizeInBytes];

   int tileNumber = 0;
   vector<rspf_float64> minBands;
   vector<rspf_float64> maxBands;
   for(rspf_uint32 i = 0; ((i < tilesHigh)&&(!needsAborting())); ++i)
   {
      // Clear the buffer.
      memset(buffer, 0, bufferSizeInBytes);

      // Set the buffer rectangle.
      rspfIrect bufferRect(theAreaOfInterest.ul().x,
                            theAreaOfInterest.ul().y + i * tileHeight,
                            theAreaOfInterest.ul().x + width - 1,
                            theAreaOfInterest.ul().y + i * tileHeight +
                            tileHeight - 1);

      // Tile loop in the sample (width) direction.
      for(rspf_uint32 j = 0; ((j < tilesWide)&&(!needsAborting())); ++j)
      {
         // Get the tile and copy it to the buffer.
         rspfRefPtr<rspfImageData> id = theInputConnection->getNextTile();
         if (!id)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "Error returned writing tiff tile:  " << tileNumber
               << "\nNULL Tile encountered"
               << std::endl;
            delete [] buffer;
            return false;
         }
         id->unloadTile(buffer, bufferRect, RSPF_BIP);
         if(!theColorLutFlag&&!needsAborting())
         {
            id->computeMinMaxPix(minBands, maxBands);
         }
         ++tileNumber;
      }

      // Get the number of lines to write from the buffer.
      rspf_uint32 linesToWrite = min(tileHeight, static_cast<rspf_uint32>(theAreaOfInterest.lr().y - bufferRect.ul().y + 1));

      // Write the buffer out to disk.
      rspf_uint32 row = static_cast<rspf_uint32>(bufferRect.ul().y -
                                                   theAreaOfInterest.ul().y);
      rspf_uint8* buf = buffer;
      for (rspf_uint32 ii=0; ((ii<linesToWrite)&&(!needsAborting())); ++ii)
      {
         rspf_int32 status = TIFFWriteScanline(tiffPtr,
                                                buf,
                                                row,
                                                0);
         if (status == -1)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "Error returned writing tiff scanline:  " << row
               << std::endl;
            setErrorStatus();
            delete [] buffer;
            return false;
         }

         ++row;  // Increment the line number.
         buf += bytesInLine;

      } // End of loop to write lines from buffer to tiff file.

      double tile = tileNumber;
      double numTiles = numberOfTiles;
      setPercentComplete(tile / numTiles * 100);
      if(needsAborting())
      {
         setPercentComplete(100);
      }

   } // End of loop in the line (height) direction.

   if(!theColorLutFlag)
   {
      writeMinMaxTags(minBands, maxBands);
   }
   
   // Free the memory.
   delete [] buffer;

   if (traceDebug()) CLOG << " Exited." << std::endl;

   return true;
}

bool rspfTiffWriter::writeToStripsBandSep()
{
   static const char* const MODULE = "rspfTiffWriter::writeToStripsBandSep";
   TIFF* tiffPtr = (TIFF*)theTif;

   if (traceDebug()) CLOG << " Entered." << std::endl;

   // Start the sequence at the first tile.
   theInputConnection->setToStartOfSequence();

   rspf_uint32 bands = theInputConnection->getNumberOfOutputBands();
   rspf_uint32 tilesWide = theInputConnection->getNumberOfTilesHorizontal();
   rspf_uint32 tilesHigh = theInputConnection->getNumberOfTilesVertical();
   rspf_uint32 tileHeight      = theInputConnection->getTileHeight();
   rspf_uint32 numberOfTiles   = theInputConnection->getNumberOfTiles();
   rspf_uint32 width           = theAreaOfInterest.width();
   rspf_uint32 bytesInLine     =
      rspf::scalarSizeInBytes(theInputConnection->getOutputScalarType()) *
      width;

   //---
   // Buffer to hold one line x tileHeight
   //---
   rspf_uint32 bufferSizeInBytes = bytesInLine * tileHeight * bands;

   unsigned char* buffer = new unsigned char[bufferSizeInBytes];

   // Tile loop in height direction.
   rspf_uint32 tileNumber = 0;
   vector<rspf_float64> minBands;
   vector<rspf_float64> maxBands;
   for(rspf_uint32 i = 0; ((i < tilesHigh)&&(!needsAborting())); ++i)
   {
      // Clear the buffer.
      memset(buffer, 0, bufferSizeInBytes);

      // Set the buffer rectangle.
      rspfIrect bufferRect(theAreaOfInterest.ul().x,
                            theAreaOfInterest.ul().y + i * tileHeight,
                            theAreaOfInterest.ul().x + width - 1,
                            theAreaOfInterest.ul().y + i * tileHeight +
                            tileHeight - 1);

      // Tile loop in the sample (width) direction.
      for(rspf_uint32 j = 0; ((j < tilesWide)&&(!needsAborting())); ++j)
      {
         // Get the tile and copy it to the buffer.
         rspfRefPtr<rspfImageData> id = theInputConnection->getNextTile();
         if (!id)
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << MODULE << " ERROR:"
               << "Error returned writing tiff tile:  " << tileNumber
               << "\nNULL Tile encountered"
               << std::endl;
            delete [] buffer;
            return false;
         }
         id->unloadTile(buffer, bufferRect, RSPF_BIL);
         if(!theColorLutFlag)
         {
            id->computeMinMaxPix(minBands, maxBands);
         }
         ++tileNumber;
      }

      // Get the number of lines to write from the buffer.
      rspf_uint32 linesToWrite = min(tileHeight, static_cast<rspf_uint32>(theAreaOfInterest.lr().y - bufferRect.ul().y + 1));

      // Write the buffer out to disk.
      rspf_uint32 row = static_cast<rspf_uint32>(bufferRect.ul().y -
                                       theAreaOfInterest.ul().y);
      rspf_uint8* buf = buffer;
      for (rspf_uint32 ii=0; ((ii<linesToWrite)&&(!needsAborting())); ++ii)
      {
         for (rspf_uint32 band =0; ((band<bands)&&(!needsAborting())); ++band)
         {
            rspf_int32 status = TIFFWriteScanline(tiffPtr,
                                                   buf,
                                                   row,
                                                   band);
            if (status == -1)
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << MODULE << " ERROR:"
                  << "Error returned writing tiff scanline:  " << row
                  << std::endl;
               delete [] buffer;
               return false;
            }
            buf += bytesInLine;
         }

         ++row;  // Increment the line number.

      } // End of loop to write lines from buffer to tiff file.

      double tile = tileNumber;
      double numTiles = numberOfTiles;
      setPercentComplete(tile / numTiles * 100);
      if(needsAborting())
      {
         setPercentComplete(100);
      }
   } // End of loop in the line (height) direction.

   if(!theColorLutFlag)
   {
      writeMinMaxTags(minBands, maxBands);
   }

   // Free the memory.
   delete [] buffer;

   if (traceDebug()) CLOG << " Exited." << std::endl;

   return true;
}

void rspfTiffWriter::setTileSize(const rspfIpt& tileSize)
{
   if ( (tileSize.x % 16) || (tileSize.y % 16) )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfTiffWriter::changeTileSize ERROR:"
            << "\nTile size must be a multiple of 32!"
            << "\nSize remains:  " << theOutputTileSize
            << std::endl;
      }
      return;
   }

   theOutputTileSize = tileSize;
}

void rspfTiffWriter::writeMinMaxTags(const vector<rspf_float64>& minBand,
                                      const vector<rspf_float64>& maxBand)
{
   TIFF* tiffPtr = (TIFF*)theTif;
   if(minBand.size() && maxBand.size())
   {
      rspf_float64 minValue =
         *std::min_element(minBand.begin(), minBand.end());
      rspf_float64 maxValue =
         *std::max_element(maxBand.begin(), maxBand.end());

      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfTiffWriter::writeMinMaxTags DEBUG:"
            << "\nminValue:  " << minValue
            << "\nmaxValue:  " << maxValue
            << std::endl;
      }

      switch( theInputConnection->getOutputScalarType() )
      {
         case RSPF_USHORT11:
         {
            TIFFSetField( tiffPtr, TIFFTAG_MINSAMPLEVALUE,
                          static_cast<rspf_sint16>(0) );
            TIFFSetField( tiffPtr, TIFFTAG_MAXSAMPLEVALUE,
                          static_cast<rspf_sint16>(2047) );
            break;
         }
         case RSPF_UINT8:
         case RSPF_UINT16:
         {
            TIFFSetField( tiffPtr, TIFFTAG_MINSAMPLEVALUE,
                          static_cast<rspf_sint16>(minValue) );
            TIFFSetField( tiffPtr, TIFFTAG_MAXSAMPLEVALUE,
                          static_cast<rspf_sint16>(maxValue) );
            break;
         }
         
         case RSPF_SINT16: 
         case RSPF_UINT32:
         case RSPF_FLOAT32:
         case RSPF_FLOAT64:
         case RSPF_NORMALIZED_FLOAT:
         case RSPF_NORMALIZED_DOUBLE:
         {
            TIFFSetField( tiffPtr, TIFFTAG_SMINSAMPLEVALUE,
                          static_cast<rspf_float32>(minValue) );
            TIFFSetField( tiffPtr, TIFFTAG_SMAXSAMPLEVALUE,
                          static_cast<rspf_float32>(maxValue) );
            break;
         }
         default:
         {
            break;
         }
      }
   }
}

void rspfTiffWriter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property)
   {
      return;
   }

   if(property->getName() == rspfKeywordNames::COMPRESSION_QUALITY_KW)
   {
//       rspfNumericProperty* numericProperty = PTR_CAST(rspfNumericProperty,
//                                                        property.get());
//       if (numericProperty)
//       {
      setJpegQuality( property->valueToString().toInt32() );
//       }
   }
   else if (property->getName() == rspfKeywordNames::COMPRESSION_TYPE_KW)
   {
      rspfStringProperty* stringProperty = PTR_CAST(rspfStringProperty,
                                                     property.get());
      if (stringProperty)
      {
         rspfString s;
         stringProperty->valueToString(s);
         setCompressionType(s);
      } 
   }
   else if(property->getName() == "lut_file")
   {
      theLutFilename = rspfFilename(property->valueToString());
      theColorLut->open(theLutFilename);
   }
   else if(property->getName() == "color_lut_flag")
   {
      theColorLutFlag = property->valueToString().toBool();
   }
   else if(property->getName() == "big_tiff_flag")
   {
      theForceBigTiffFlag = property->valueToString().toBool();
   }
   else if(property->getName() == rspfKeywordNames::OUTPUT_TILE_SIZE_KW)
   {
      theOutputTileSize.x = property->valueToString().toInt32();
      theOutputTileSize.y =  theOutputTileSize.x;
   }
   else
   {
      rspfImageFileWriter::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfTiffWriter::getProperty(const rspfString& name)const
{
   if(name == "Filename")
   {
      rspfRefPtr<rspfProperty> tempProp = rspfImageFileWriter::getProperty(name);
      if(tempProp.valid())
      {
         rspfFilenameProperty* filenameProp = PTR_CAST(rspfFilenameProperty,
                                                        tempProp.get());

         if(filenameProp)
         {
            filenameProp->addFilter("*.tif");
         }

         return tempProp;
      }
   }
   else if (name == rspfKeywordNames::COMPRESSION_QUALITY_KW)
   {
      rspfNumericProperty* numericProp =
         new rspfNumericProperty(name,
                                  rspfString::toString(theJpegQuality),
                                  1.0,
                                  100.0);
      numericProp->
         setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
      return numericProp;
   }
   else if (name == rspfKeywordNames::COMPRESSION_TYPE_KW)
   {
      rspfStringProperty* stringProp =
         new rspfStringProperty(name,
                                 getCompressionType(),
                                 false); // editable flag
      stringProp->addConstraint(rspfString("none"));
      stringProp->addConstraint(rspfString("jpeg"));
      stringProp->addConstraint(rspfString("packbits"));
      stringProp->addConstraint(rspfString("deflate"));
      stringProp->addConstraint(rspfString("zip"));      
      return stringProp;
   }
   else if (name == "lut_file")
   {
      rspfFilenameProperty* property = new rspfFilenameProperty(name, theLutFilename);
      property->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_INPUT);

      return property;
   }
   else if (name == "color_lut_flag")
   {
      rspfBooleanProperty* boolProperty = new rspfBooleanProperty(name,
                                                                    theColorLutFlag);
      return boolProperty;
   }
   else if(name == "big_tiff_flag")
   {
       rspfBooleanProperty* boolProperty = new rspfBooleanProperty(name,
                                                                    theForceBigTiffFlag);
      return boolProperty;     
   }
   else if( name == rspfKeywordNames::OUTPUT_TILE_SIZE_KW )
   {
      rspfStringProperty* stringProp =
         new rspfStringProperty(name,
                                 rspfString::toString(theOutputTileSize.x),
                                 false); // editable flag
      stringProp->addConstraint(rspfString("16"));
      stringProp->addConstraint(rspfString("32"));
      stringProp->addConstraint(rspfString("64"));
      stringProp->addConstraint(rspfString("128"));
      stringProp->addConstraint(rspfString("256"));      
      stringProp->addConstraint(rspfString("512"));      
      stringProp->addConstraint(rspfString("1024"));      
      stringProp->addConstraint(rspfString("2048"));      
      return stringProp;
     
   }
   return rspfImageFileWriter::getProperty(name);
}

void rspfTiffWriter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back(rspfString(
                              rspfKeywordNames::COMPRESSION_QUALITY_KW));
   propertyNames.push_back(rspfString(
                              rspfKeywordNames::COMPRESSION_TYPE_KW));
   propertyNames.push_back(rspfString("lut_file"));
   propertyNames.push_back(rspfString("color_lut_flag"));
   propertyNames.push_back(rspfString("big_tiff_flag"));
   propertyNames.push_back(rspfString(rspfKeywordNames::OUTPUT_TILE_SIZE_KW));
  
   rspfImageFileWriter::getPropertyNames(propertyNames);
}

bool rspfTiffWriter::isOpen()const
{
   return (theTif!=NULL);
}

bool rspfTiffWriter::open()
{
   if(theTif)
   {
      closeTiff();
   }
   return openTiff();
}

void rspfTiffWriter::close()
{
   closeTiff();
}

void rspfTiffWriter::setJpegQuality(rspf_int32 quality)
{
   // Range 1 to 100 with 100 being best.
   if (quality > 0 && quality < 101)
   {
      theJpegQuality = quality;
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfTiffWriter::setJpegQuality DEBUG:"
            << "\nquality out of range:  " << quality
            << "\nquality has been set to default:  " 
            << DEFAULT_JPEG_QUALITY
            << "\nvalid range:  1 to 100 with 100 being best."
            << std::endl;
      }
      
      theJpegQuality = DEFAULT_JPEG_QUALITY;
   }
}

rspf_int32 rspfTiffWriter::getJpegQuality()const
{
   
   return theJpegQuality;
}

void rspfTiffWriter::setCompressionType(const rspfString& type)
{
   theCompressionType = type;
}

rspfString rspfTiffWriter::getCompressionType()const
{
   return theCompressionType;
}

bool rspfTiffWriter::getGeotiffFlag()const
{
   return theOutputGeotiffTagsFlag;
}

void rspfTiffWriter::setGeotiffFlag(bool flag)
{
   theOutputGeotiffTagsFlag = flag;
}

rspfIpt rspfTiffWriter::getOutputTileSize()const
{
   return theOutputTileSize;
}

rspf_int32 rspfTiffWriter::setProjectionInfo(const rspfMapProjectionInfo& proj)
{
   theProjectionInfo = new rspfMapProjectionInfo(proj);

   return rspfErrorCodes::RSPF_OK;
}


void rspfTiffWriter::getImageTypeList(std::vector<rspfString>& imageTypeList)const
{
   imageTypeList.push_back(rspfString("tiff_strip"));
   imageTypeList.push_back(rspfString("tiff_strip_band_separate"));
   imageTypeList.push_back(rspfString("tiff_tiled"));
   imageTypeList.push_back(rspfString("tiff_tiled_band_separate"));
}

rspfString rspfTiffWriter::getExtension() const
{
   return rspfString("tif");
}

bool rspfTiffWriter::hasImageType(const rspfString& imageType) const
{
   // check for non image type list types
   // We will support mime type
   //
   if((imageType == "image/tiff")||
      (imageType == "image/gtiff")||
      (imageType == "image/tif")||
      (imageType == "image/gtif"))
   {
      return true;
   }

   return rspfImageFileWriter::hasImageType(imageType);
}

bool rspfTiffWriter::isLutEnabled()const
{
   return (theColorLutFlag);
}

rspfTiffWriter::UnitType rspfTiffWriter::getUnitType(
   rspf_int32 pcsCode,
   const rspfString& projName) const
{


   if ( ( projName == "rspfCylEquAreaProjection" ) ||
        ( projName == "rspfEquDistCylProjection" ) ||
        ( projName == "rspfLlxyProjection" ) )
   {
      return ANGULAR_DEGREES;
   } 

   UnitType pcsUnits = getPcsUnitType(pcsCode);

   UnitType type = UNDEFINED;
   
   switch (theLinearUnits)
   {
      case RSPF_METERS:
      {
         type = LINEAR_METER;
         break;
      }
      
      case RSPF_FEET:
      {
         type = LINEAR_FOOT;
         break;
      }
      
      case RSPF_US_SURVEY_FEET:
      {
         type = LINEAR_FOOT_US_SURVEY;
         break;
      }
      default:
      {
         break;
      }
   }
   if (type == UNDEFINED)
   {
      return pcsUnits;
   }
   return type;
}

rspfTiffWriter::UnitType rspfTiffWriter::getPcsUnitType(rspf_int32 pcsCode) const
{
   UnitType pcsUnits = UNDEFINED;
   
   rspfRefPtr<rspfMapProjection> proj = PTR_CAST(rspfMapProjection, 
      rspfEpsgProjectionDatabase::instance()->findProjection((rspf_uint32) pcsCode));
   
   if (proj.valid())
   {
      rspfUnitType type = proj->getProjectionUnits();
      if (type == RSPF_METERS)
      {
         pcsUnits = LINEAR_METER;
      }
      else
      {
         pcsUnits = LINEAR_FOOT_US_SURVEY;
      }
   }
   return pcsUnits;
}

//*************************************************************************************************
// Will take an rspfIMageData tile and write it to disk as a general raster file.
//*************************************************************************************************
void rspfTiffWriter::dumpTileToFile(rspfRefPtr<rspfImageData> t,  const rspfFilename& f)
{
   rspfRefPtr<rspfMemoryImageSource> tile = new rspfMemoryImageSource;
   tile->setImage(t);
   rspfRefPtr<rspfScalarRemapper> remapper = new rspfScalarRemapper(tile.get(), RSPF_UINT8);
   rspfRefPtr<rspfTiffWriter> writer = new rspfTiffWriter();
   writer->connectMyInputTo(0, remapper.get());
   writer->setFilename(f);
   writer->setGeotiffFlag(false);
   writer->execute();
   writer=0;
   tile=0;
}
