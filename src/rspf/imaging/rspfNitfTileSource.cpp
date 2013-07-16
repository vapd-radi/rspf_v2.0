//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  Contains class definition for rspfNitfTileSource.
// 
//*******************************************************************
//  $Id: rspfNitfTileSource.cpp 22149 2013-02-11 21:36:10Z dburken $

#include <rspf/imaging/rspfNitfTileSource.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfInterleaveTypeLut.h>
#include <rspf/base/rspfPackedBits.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/imaging/rspfJpegMemSrc.h>
#include <rspf/imaging/rspfTiffTileSource.h>
#include <rspf/imaging/rspfJpegDefaultTable.h>
#include <rspf/base/rspf2dTo2dShiftTransform.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/support_data/rspfNitfIchipbTag.h>
#include <rspf/support_data/rspfNitfImageHeaderV2_0.h>
#include <rspf/support_data/rspfNitfImageHeaderV2_1.h>
#include <rspf/support_data/rspfNitfStdidcTag.h>
#include <rspf/support_data/rspfNitfVqCompressionHeader.h>

#if defined(JPEG_DUAL_MODE_8_12)
#include <rspf/imaging/rspfNitfTileSource_12.h>
#endif

#include <jerror.h>
#include <fstream>
#include <algorithm> /* for std::fill */

RTTI_DEF1_INST(rspfNitfTileSource, "rspfNitfTileSource", rspfImageHandler)

#ifdef RSPF_ID_ENABLED
   static const char RSPF_ID[] = "$Id: rspfNitfTileSource.cpp 22149 2013-02-11 21:36:10Z dburken $";
#endif
   
//---
// NOTE:  This should match the enumerations for ReadMode.
//---
static const char* READ_MODE[] = { "READ_MODE_UNKNOWN",
                                   "READ_BIB_BLOCK",
                                   "READ_BIP_BLOCK",
                                   "READ_BIR_BLOCK",
                                   "READ_BSQ_BLOCK",
                                   "READ_BIB",
                                   "READ_BIP",
                                   "READ_BIR",
                                   "READ_JPEG_BLOCK" };

//***
// Static trace for debugging
//***
static rspfTrace traceDebug("rspfNitfTileSource:debug");

// 64x64*12bits
// divide by 8 bits to get bytes gives you 6144 bytes
static const rspf_uint32   RSPF_NITF_VQ_BLOCKSIZE = 6144;

rspfNitfTileSource::rspfNitfTileSource()
   :
      rspfImageHandler(),
      theTile(0),
      theCacheTile(0),
      theNitfFile(new rspfNitfFile()),
      theNitfImageHeader(0),
      theReadMode(READ_MODE_UNKNOWN),
      theScalarType(RSPF_SCALAR_UNKNOWN),
      theSwapBytesFlag(false),
      theNumberOfInputBands(0),
      theNumberOfOutputBands(0),
      theBlockSizeInBytes(0),
      theReadBlockSizeInBytes(0),
      theNumberOfImages(0),
      theCurrentEntry(0),
      theImageRect(0,0,0,0),
      theFileStr(),
      theOutputBandList(),
      theCacheSize(0, 0),
      theCacheTileInterLeaveType(RSPF_INTERLEAVE_UNKNOWN),
      theCacheEnabledFlag(false),
      theCacheId(-1),
      thePackedBitsFlag(false),
      theCompressedBuf(0),
      theNitfBlockOffset(0),
      theNitfBlockSize(0),
      m_isJpeg12Bit(false),
      m_jpegOffsetsDirty(false)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::rspfNitfTileSource entered..." << endl;
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)<< "RSPF_ID:  " << RSPF_ID << endl;
#endif
   }

}

rspfNitfTileSource::~rspfNitfTileSource()
{
   destroy();
}

void rspfNitfTileSource::destroy()
{
   if (theCacheId != -1)
   {
      rspfAppFixedTileCache::instance()->deleteCache(theCacheId);
      theCacheId = -1;
   }

   // Delete the list of image headers.
   theNitfImageHeader.clear();

   if(theFileStr.is_open())
   {
      theFileStr.close();
   }

   theCacheTile = 0;
   theTile      = 0;
   theOverview  = 0;
 }

bool rspfNitfTileSource::isOpen()const
{
   return (theNitfImageHeader.size() > 0);
}

bool rspfNitfTileSource::open()
{
   bool result = false;
   
   if(isOpen())
   {
      close();
   }
   
   theErrorStatus = rspfErrorCodes::RSPF_OK;

   if ( parseFile() )
   {
      result = allocate();
   }
   if (result)
   {
      completeOpen();
   }
   
   return result;
}

void rspfNitfTileSource::close()
{
   destroy();
}

bool rspfNitfTileSource::parseFile()
{
   static const char MODULE[] = "rspfNitfTileSource::parseFile";
   
   rspfFilename file = getFilename();

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG: Nitf file =  " << file << endl;
   }

   if (file.empty())
   {
      setErrorStatus();
      return false;
   }

   if ( !theNitfFile )  // A close deletes "theNitfFile".
   {
      theNitfFile = new rspfNitfFile();
   }
   
   if ( !theNitfFile->parseFile(file) )
   {
      setErrorStatus();
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << "DEBUG:" << "\nError parsing file!" << endl;
      }

      return false;
   }

   // Get the number of images within the file.
   theNumberOfImages = theNitfFile->getHeader()->getNumberOfImages();

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << "DEBUG:\nNumber of images "
         <<theNumberOfImages << std::endl; 
   }
   
   if ( theNumberOfImages == 0 )
   {
      setErrorStatus();
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << "DEBUG:\nNo images in file!" << endl;
      }
      
      return false;
   }
   theEntryList.clear();
   //---
   // Get image header pointers.  Note there can be multiple images in one
   // image file.
   //---
   for (rspf_uint32 i = 0; i < theNumberOfImages; ++i)
   {
      rspfRefPtr<rspfNitfImageHeader> hdr = theNitfFile->getNewImageHeader(i);
      if (!hdr)
      {
         setErrorStatus();
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " ERROR:\nNull image header!" << endl;
         }
         
         return false;
      }
      if (traceDebug())
      {
         if(hdr.valid())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << MODULE << "DEBUG:"
               << "\nImage header[" << i << "]:\n" << *hdr
               << endl;
         }
      }

      if( !hdr->isCompressed() )
      {
         // Skip cloud mask images??? (drb)
         if (hdr->getRepresentation() != "NODISPLY")
         {
            theEntryList.push_back(i);
            theNitfImageHeader.push_back(hdr);
         }
         else 
         {
            rspfString cat = hdr->getCategory().trim().downcase();
            // this is an NGA Highr Resoluion Digital Terrain Model NITF format
            if(cat == "dtem")
            {
               theEntryList.push_back(i);
               theNitfImageHeader.push_back(hdr);
            }
         }

      }
      else if ( canUncompress(hdr.get()) )
      {
         theEntryList.push_back(i);
         theCacheEnabledFlag = true;
         theNitfImageHeader.push_back(hdr);

         if (hdr->getBitsPerPixelPerBand() == 8)
         {
            m_isJpeg12Bit = false;
         }
         else if (hdr->getBitsPerPixelPerBand() == 12)
         {
           m_isJpeg12Bit = true;
         }
      }
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "Entry " << i
               <<" has an unsupported compression code = "
               << hdr->getCompressionCode() << std::endl;
         }
         return false;
      }
   }

   if(theEntryList.size()<1)
   {
      return false;
   }
   
   //### WHY IS THIS HERE? THIS CAUSED A BUG BECAUSE theCurrentEntry was previously initialized 
   //### in loadState() according to a KWL. Any entry index in the KWL was being ignored.
   //if(theEntryList.size()>0)
   //{
   //   theCurrentEntry = theEntryList[0];
   //}

   theNumberOfImages = (rspf_uint32)theNitfImageHeader.size();
   
   if (theNitfImageHeader.size() != theNumberOfImages)
   {
      setErrorStatus();
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE
            << "DEBUG:\nNumber of header not equal number of images!"
            << endl;
      }
      
      return false;
   }

   // Check the current entry range.
   if ( theCurrentEntry >= theNumberOfImages )
   {
      if(theEntryList.size())
      {
         theCurrentEntry = theEntryList[0];
      }
   }
   
   // Initialize the lut to the current entry if the current entry has a lut.
   initializeLut();
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG:"
         << "\nCurrent entry:  " << theCurrentEntry
         << endl;
   }


   // Open up a stream to the file.
   theFileStr.open(file.c_str(), ios::in | ios::binary);
   if (!theFileStr)
   {
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << " ERROR:"
            << "\nCannot open:  " << file.c_str() << endl;
      }
      return false;
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " leaving with true..." << endl;
      
   }
   
   return true;
}

bool rspfNitfTileSource::allocate()
{
   // Clear out the cache if there was any.
   if (theCacheId != -1)
   {
      rspfAppFixedTileCache::instance()->deleteCache(theCacheId);
      theCacheId = -1;
   }

   // Clear buffers:
   theTile = 0;
   theCacheTile = 0;
   theCompressedBuf.clear();

   // Set the scalar type.
   initializeScalarType();
   if (theScalarType == RSPF_SCALAR_UNKNOWN)
   {
      return false;
   }

   // Set the swap bytes flag.
   initializeSwapBytesFlag();
   
   // Set the read mode.
   initializeReadMode();
   if (theReadMode == READ_MODE_UNKNOWN)
   {
      return false;
   }
   
   // Set the number of bands.
   initializeBandCount();
   if (theNumberOfInputBands == 0)
   {
      return false;
   }
   
   // Initialize the image rectangle. before the cache size is done
   if (initializeImageRect() == false)
   {
      return false;
   }
   
   // Initialize the cache size.  Must be done before
   // setting the blocksize.  Since bit encoded data may very
   // and we need to know if the nitf file needs to be accessed
   // like a general raster.
   //
   initializeCacheSize();
   if ( (theCacheSize.x == 0) || (theCacheSize.y == 0) )
   {
      return false;
   }
   
   // Initialize the block size.
   if (initializeBlockSize() == false)
   {
      return false;
   }

   // Initialize the cache tile interleave type.
   initializeCacheTileInterLeaveType();
   if (theCacheTileInterLeaveType == RSPF_INTERLEAVE_UNKNOWN)
   {
      return false;
   }

   return true;
}

bool rspfNitfTileSource::allocateBuffers()
{
   //---
   // Initialize the cache tile.  This will be used for a block buffer even
   // if the cache is disabled.
   //---
   initializeCacheTile();
   if (!theCacheTile.valid())
   {
      return false;
   }

   // Initialize the cache if enabled.
   if (theCacheEnabledFlag)
   {
      theCacheId = rspfAppFixedTileCache::instance()->
         newTileCache(theBlockImageRect, theCacheSize);
   }

   //---
   // Initialize the compressed buffer if needed.
   //---
   initializeCompressedBuf();

   //---
   // Make the output tile.
   //---
   initializeOutputTile();

   return true;
}

bool rspfNitfTileSource::canUncompress(const rspfNitfImageHeader* hdr) const
{

   bool result = false;
   if (hdr)
   {
      rspfString code = hdr->getCompressionCode();

      if (code == "C3") // jpeg
      {
         if (hdr->getBitsPerPixelPerBand() == 8)
         {
            result = true;
         }
         else if (hdr->getBitsPerPixelPerBand() == 12)
         {
           result = true;
         }
         else
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "Entry with jpeg compression (C3) has an unsupported "
                  << "JPEG data precision: " << hdr->getBitsPerPixelPerBand()
                  << std::endl;
            }
         }
      }
      else if(isVqCompressed( code ) &&
              (hdr->getCompressionHeader().valid()) )
      {
         // we will only support single band vq compressed NITFS
         // basically CIB and CADRG products are single band code words.
         //
         if(hdr->getNumberOfBands() == 1)
         {
            result = true;
         }
      }
   }
   return result;
}

void rspfNitfTileSource::initializeReadMode()
{
   // Initialize the read mode.
   theReadMode = READ_MODE_UNKNOWN;
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return;
   }

   rspf_uint32 numberOfBlocks = getNumberOfBlocks();
   rspfString imode           = hdr->getIMode();
   rspfString compressionCode = hdr->getCompressionCode();

   if ( (compressionCode == "C3") && ((imode == "B")||(imode == "P")) )
   {
      theReadMode = READ_JPEG_BLOCK; 
   }
   else if (numberOfBlocks > 1)
   {
      if (imode == "B")
      {
         theReadMode = READ_BIB_BLOCK;
      }
      else if (imode == "P")
      {
         theReadMode = READ_BIP_BLOCK;
      }
      else if (imode == "R")
      {
         theReadMode = READ_BIR_BLOCK;
      }
      else if (imode == "S")
      {
         theReadMode = READ_BSQ_BLOCK;
      }
   }
   else // The entire image comprises one block.
   {
      if (imode == "B")
      {
         theReadMode = READ_BIB;
      }
      else if (imode == "P")
      {
         theReadMode = READ_BIP;
      }
      else if (imode == "R")
      {
         theReadMode = READ_BIR;
      }
      else if (imode == "S")
      {
         theReadMode = READ_BSQ_BLOCK;
      }
   }        
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::initializeReadMode DEBUG:"
         << "\nnumberOfBlocks:  " << numberOfBlocks
         << "\nIMODE:           " << imode
         << "\nRead Mode:       " << READ_MODE[theReadMode]
         << endl;
   }
}

void rspfNitfTileSource::initializeScalarType()
{
   thePackedBitsFlag = false;
   // Initialize the read mode.
   theScalarType = RSPF_SCALAR_UNKNOWN;
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return;
   }

   rspf_int32 bitsPerPixel = hdr->getActualBitsPerPixelPerBand();
   if (bitsPerPixel < 1)
   {
      bitsPerPixel = hdr->getBitsPerPixelPerBand();
   }

   rspfString pixelValueType = hdr->getPixelValueType().upcase();
   
   switch (bitsPerPixel)
   {
      case 8:
      {
         theScalarType = RSPF_UINT8;
         break;
      }
      case 11:
      {
         theScalarType = RSPF_USHORT11;
         break;
      }
      case 12:
      {
         theScalarType = RSPF_UINT16;
         break;
      }
      case 16:
      {
         if(pixelValueType == "SI")
         {
            theScalarType = RSPF_SINT16;
         }
         else
         {
            theScalarType = RSPF_UINT16;
         }
         break;
      }
      case 32:
      {
         if(pixelValueType == "SI")
         {
            theScalarType = RSPF_SINT32;
         }
         else if(pixelValueType == "R")
         {
            theScalarType = RSPF_FLOAT32;
         }
         break;
      }
      case 64:
      {
         if(pixelValueType == "R")
         {
            theScalarType = RSPF_FLOAT64;
         }
         
         break;
      }
      default:
      {
         if(hdr->isCompressed())
         {
            thePackedBitsFlag = true;
            if(bitsPerPixel < 8)
            {
               theScalarType = RSPF_UINT8;
            }
            else if(bitsPerPixel < 16)
            {
               theScalarType = RSPF_UINT16;
            }
            else if(bitsPerPixel < 32)
            {
               theScalarType = RSPF_FLOAT32;
            }
         }
         else
         {
            if(bitsPerPixel<8)
            {
               theScalarType = RSPF_UINT8;
            }
         }
         break;
      }
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::initializeScalarType DEBUG:"
         << "\nScalar type:  "
         << (rspfScalarTypeLut::instance()->getEntryString(theScalarType))
         << "\nPacked bits:  " << (thePackedBitsFlag?"true":"false")
         << endl;
   }
}

void rspfNitfTileSource::initializeSwapBytesFlag()
{
   if ( (theScalarType != RSPF_UINT8) &&
        (rspf::byteOrder() == RSPF_LITTLE_ENDIAN) )
   {
      theSwapBytesFlag = true;
   }
   else
   {
     theSwapBytesFlag = false;
   }
}

void rspfNitfTileSource::initializeBandCount()
{
   // Initialize the read mode.
   theNumberOfInputBands = 0;
   theNumberOfOutputBands = 0;
   theOutputBandList.clear();
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return;
   }

   if(!isVqCompressed(hdr->getCompressionCode()))
   {
      theNumberOfInputBands = hdr->getNumberOfBands();
      theNumberOfOutputBands = hdr->getNumberOfBands();
      if(hdr->getRepresentation().contains("LUT")&&(theNumberOfInputBands == 1))
      {
         theNumberOfOutputBands = 3;
      }
   }
   else 
   {
      rspfRefPtr<rspfNitfImageBand> bandInfo = hdr->getBandInformation(0);
      if ( bandInfo.valid() )
      {
         theNumberOfInputBands = 1;
         theNumberOfOutputBands = bandInfo->getNumberOfLuts();
      }
   }
   
   theOutputBandList.resize(theNumberOfOutputBands);
   
   for (rspf_uint32 i=0; i < theNumberOfOutputBands; ++i)
   {
      theOutputBandList[i] = i; // One to one for initial setup.
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::initializeBandCount DEBUG:"
         << "\nInput Band count:  " << theNumberOfInputBands
         << "\nOutput Band count:  " << theNumberOfOutputBands
         << endl;
   }
}

bool rspfNitfTileSource::initializeBlockSize()
{
   theBlockSizeInBytes     = 0;
   theReadBlockSizeInBytes = 0;
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return false;
   }

   rspf_uint32 bytesRowCol = 0;
   rspf_uint32 bytesRowColCacheTile = 0;

   if(isVqCompressed(hdr->getCompressionCode()))
   {
      bytesRowCol = RSPF_NITF_VQ_BLOCKSIZE;
   }
   else
   {
      bytesRowCol = (hdr->getNumberOfPixelsPerBlockHoriz()*
                     hdr->getNumberOfPixelsPerBlockVert()*
                     hdr->getBitsPerPixelPerBand()) / 8;
   }
   
   bytesRowColCacheTile = (theCacheSize.x*
                           theCacheSize.y*
                           hdr->getBitsPerPixelPerBand())/8;

#if 0
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG:"
         << "\ncompressionHeader:  " << compressionHeader
         << "\ngetNumberOfPixelsPerBlockHoriz():  "
         << hdr->getNumberOfPixelsPerBlockHoriz()
         << "\ngetNumberOfPixelsPerBlockVert():  "
         << hdr->getNumberOfPixelsPerBlockVert()
         << "\ngetBitsPerPixelPerBand():  "
         << hdr->getBitsPerPixelPerBand()
         << "\nbytesRowCol:  " << bytesRowCol
         << "\nbytesRowColCacheTile:  " << bytesRowColCacheTile
         << endl;
   }
#endif
   
   theBlockSizeInBytes = bytesRowCol;
   theReadBlockSizeInBytes = theBlockSizeInBytes;
   switch (theReadMode)
   {
      case READ_BSQ_BLOCK:
      case READ_BIB_BLOCK:
      {
         break;
      }
      case READ_BIB:
      {
         theReadBlockSizeInBytes = bytesRowColCacheTile;
         break;
      }
      
      case READ_BIP_BLOCK:
      case READ_BIR_BLOCK:
      {
         theBlockSizeInBytes     *= theNumberOfInputBands;
         theReadBlockSizeInBytes *= theNumberOfInputBands;
         break;
      }
      case READ_BIP:
      case READ_BIR:   
      {
         theBlockSizeInBytes *= theNumberOfInputBands;
         theReadBlockSizeInBytes = bytesRowColCacheTile*theNumberOfInputBands;
         break;
      }
      case READ_JPEG_BLOCK:
      {
         theBlockSizeInBytes *= theNumberOfInputBands;
         rspfString code = hdr->getCompressionCode();
         if (code == "C3") // jpeg
         {
            m_jpegOffsetsDirty  = true;
         }
         break;
      }
      default:
      {
         return false;
      }
   }

//#if 0
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::initializeBlockSize DEBUG:"
         << "\nNumber of input bands:          " << theNumberOfInputBands
         << "\nNumber of output bands:          " << theNumberOfOutputBands
         << "\nBlock size in bytes:      " << theBlockSizeInBytes
         << "\nRead block size in bytes: " << theReadBlockSizeInBytes
         << endl;
   }
//#endif

   return true;
}

//*************************************************************************************************
// Virtual method determines the decimation factors at each resolution level. 
// This implementation derives the R0 decimation from the image metadata if available, then hands
// off the computation of remaining R-levels to the base class implementation.
//*************************************************************************************************
void rspfNitfTileSource::establishDecimationFactors()
{
   theDecimationFactors.clear();
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (hdr)
   {
      double decimation;
      hdr->getDecimationFactor(decimation);
      if ((decimation != 0.0) && !rspf::isnan(decimation))
      {
         //---
         // Note: Commented out as other code is picking up the resolution and then we're applying
         // a decimation on top of that. (drb Aug. 2011)
         // rspfDpt dec_2d (decimation, decimation);
         //---
         rspfDpt dec_2d (1.0, 1.0);
         theDecimationFactors.push_back(dec_2d);
      }
   }

   // Just needed to set the first R level here, the base class can do the rest:
   rspfImageHandler::establishDecimationFactors();
}

#if 0
rspfImageGeometry* rspfNitfTileSource::getImageGeometry()
{
   //---
   // Call base class getImageGeometry which will check for external geometry
   // or an already set geometry.
   //---
   rspfImageGeometry* result = rspfImageHandler::getImageGeometry();

   if (result)
   {
      if ( !result->getTransform() )
      {
         rspfRefPtr<rspf2dTo2dTransform> transform = 0;
         
         const rspfNitfImageHeader* hdr = getCurrentImageHeader();
         if (hdr)
         {
            //---
            // Test for the ichipb tag and set the sub image if needed.
            // 
            // NOTE # 1:
            // 
            // There are nitf writers that set the ichipb offsets and only have
            // IGEOLO field present.  For these it has been determined
            // (but still in question) that we should not apply the sub image
            // offset.
            //
            // See trac # 1578
            // http://trac.osgeo.org/rspf/ticket/1578
            //
            // NOTE # 2:
            //
            // Let the ICHIPB have precedence over the STDIDC tag as we could
            // have a chip of a segment.
            //---
            rspfRefPtr<rspfNitfRegisteredTag> tag =
               hdr->getTagData(rspfString("ICHIPB"));
            if (tag.valid())
            {
               rspfNitfIchipbTag* ichipb =
                  PTR_CAST(rspfNitfIchipbTag, tag.get());
               if (ichipb)
               {
//                  const rspfRefPtr<rspfNitfRegisteredTag> blocka =
//                     hdr->getTagData(rspfString("BLOCKA"));
//                  const rspfRefPtr<rspfNitfRegisteredTag> rpc00a =
//                     hdr->getTagData(rspfString("RPC00A"));              
//                  const rspfRefPtr<rspfNitfRegisteredTag> rpc00b =
//                     hdr->getTagData(rspfString("RPC00B"));
                  
                  //---
                  // If any of these tags are present we will use the sub
                  // image from the ichipb tag.
                  //---
//                  if ( blocka.get() || rpc00a.get() || rpc00b.get() )
                  // ************************* THERE ARE PROBLEMS NOT SETTING THIS AT SITE.  GO AHEAD AND ALWAYS INIT THE SHIFT
                  {
                     transform = ichipb->newTransform();
                  }
               }
            }
   
            if ( !transform)
            {
               //---
               // Look for the STDIDC tag for a sub image (segment) offset.
               //
               // See: STDI-002 Table 7.3 for documentation.
               //---
               tag = hdr->getTagData(rspfString("STDIDC"));
               if (tag.valid() && (hdr->getIMode() == "B") )
               {
                  rspfDpt shift;
                  rspfNitfStdidcTag* stdidc =
                     PTR_CAST(rspfNitfStdidcTag, tag.get());
                  if (stdidc)
                  {
                     rspf_int32 startCol = stdidc->getStartColumn().toInt32();
                     rspf_int32 startRow = stdidc->getStartRow().toInt32();
                     if ( (startCol > 0) && (startRow > 0) )
                     {
                        
                        // field are one based; hence, the - 1.
                        shift.x = (startCol-1) *
                           hdr->getNumberOfPixelsPerBlockHoriz();
                        shift.y = (startRow-1) *
                           hdr->getNumberOfPixelsPerBlockVert();
                     }
                     if(shift.x > 0 ||
                        shift.y > 0)
                     {
                        transform = new rspf2dTo2dShiftTransform(shift);
                     }
                  }
               }
            }
            
         } // matches: if (hdr)

         if ( transform.valid() )
         {
            result->setTransform( transform.get() );
         }
         //else
         //{
         //   rspfImageGeometryRegistry::instance()->createTransform(this);
         //}
         
         
      } // matches: if ( !result->getTransform() )

      if ( !result->getProjection() )
      {
         rspfRefPtr<rspfProjection> proj =
            rspfProjectionFactoryRegistry::instance()->
               createProjection(this);
         if ( proj.valid() )
         {
            result->setProjection( proj.get() );
         }
         //else
         //{
         //   rspfImageGeometryRegistry::instance()->createProjection(this);
         //}
         
      }
      
   } // matches: if (result)

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::createImageGeometry DEBUG:\n";

      if (result)
      {
         result->print(rspfNotify(rspfNotifyLevel_DEBUG)) << "\n";
      }
   }

   return result;
}
#endif

bool rspfNitfTileSource::initializeImageRect()
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      theImageRect.makeNan();
      return false;
   }
   
   theBlockImageRect = hdr->getBlockImageRect();
   theImageRect      = hdr->getImageRect();

   if (traceDebug())
   {
      rspfIpt iloc;
      hdr->getImageLocation(iloc); // for temp debug (drb)
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::initializeImageRect DEBUG:"
         << "\noffset from ILOC field:  " << iloc
         << "\nImage Rect:              " << theImageRect
         << "\nBlock rect:              " << theBlockImageRect
         << endl;
   }
   return true;
}

void rspfNitfTileSource::initializeCacheSize()
{
   theCacheSize.x = 0;
   theCacheSize.y = 0;
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return;
   }
   switch (theReadMode)
   {
      case READ_BIB_BLOCK:
      case READ_BIP_BLOCK:
      case READ_BIR_BLOCK:
      case READ_BSQ_BLOCK:
      case READ_JPEG_BLOCK:
         theCacheSize.x = hdr->getNumberOfPixelsPerBlockHoriz();
         theCacheSize.y = hdr->getNumberOfPixelsPerBlockVert();
         break;

      case READ_BIB:
      case READ_BIP:
      case READ_BIR:
         theCacheSize.x = hdr->getNumberOfPixelsPerBlockHoriz();
         theCacheSize.y = hdr->getNumberOfPixelsPerBlockVert();
//          theCacheSize.x = getNumberOfSamples(0);
//          theCacheSize.y = getTileHeight();
//          if(theCacheSize.y > hdr->getNumberOfPixelsPerBlockVert())
//          {
//             theCacheSize.y = hdr->getNumberOfPixelsPerBlockVert();
//          }
         break;

      default:
         break;
   }

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::initializeCacheSize DEBUG:"
         << "\nCache size:  " << theCacheSize
         << endl;
   }
}

void rspfNitfTileSource::initializeCacheTileInterLeaveType()
{
   theCacheTileInterLeaveType = RSPF_INTERLEAVE_UNKNOWN;
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return;
   }

   switch (theReadMode)
   {
      case READ_BIB_BLOCK:
      case READ_BSQ_BLOCK:
      case READ_BIB:
      case READ_JPEG_BLOCK:  
         theCacheTileInterLeaveType = RSPF_BSQ;
         break;

      case READ_BIP_BLOCK:
      case READ_BIP:
         theCacheTileInterLeaveType = RSPF_BIP;
         break;

      case READ_BIR_BLOCK:
      case READ_BIR:
         theCacheTileInterLeaveType = RSPF_BIL;
         break;

      default:
         break;
   }

   if (traceDebug())
   {
      rspfInterleaveTypeLut lut;
      
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::initializeCacheTileInterLeaveType DEBUG:"
         << "\nCache tile interleave type:  "
         << lut.getEntryString(theCacheTileInterLeaveType)
         << endl;
   }
}

void rspfNitfTileSource::initializeCacheTile()
{
   theCacheTile = rspfImageDataFactory::instance()->create(
      this,
      theScalarType,
      theNumberOfOutputBands,
      theCacheSize.x,
      theCacheSize.y);

   theCacheTile->initialize();
}

void rspfNitfTileSource::initializeCompressedBuf()
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return;
   }

   if( (hdr->getRepresentation().upcase().contains("LUT")) ||
       ( isVqCompressed(hdr->getCompressionCode()) ) )
   {
      theCompressedBuf.resize(theReadBlockSizeInBytes);
      std::fill(theCompressedBuf.begin(), theCompressedBuf.end(), '\0');
   }
}

void rspfNitfTileSource::initializeOutputTile()
{
   //---
   // Make the output tile.  This implementation will use default tile size.
   rspfImageDataFactory* idf = rspfImageDataFactory::instance();
   theTile = idf->create(this, this);
   theTile->initialize();
}

void rspfNitfTileSource::initializeLut()
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (hdr)
   {
      if ( hdr->hasLut() )
      {
         //---
         // NOTE: Only band 0 ??? (drb)
         //---
         theLut = theNitfImageHeader[theCurrentEntry]->createLut(0);
      }
   }
}

rspfRefPtr<rspfImageData> rspfNitfTileSource::getTile(
   const  rspfIrect& tileRect, rspf_uint32 resLevel)
{
   // This tile source bypassed, or invalid res level, return a blank tile.
   if(!isSourceEnabled() || !isOpen() || !isValidRLevel(resLevel))
   {
      return rspfRefPtr<rspfImageData>();
   }

   if ( !theTile.valid() )
   {
      // First call to getTile:
      allocateBuffers();
      if ( !theTile.valid() )
      {
         return theTile;
      }
   }

   // Rectangle must be set prior to getOverviewTile call.
   theTile->setImageRectangle(tileRect);

   if (resLevel)
   {
      if ( getOverviewTile(resLevel, theTile.get() ) )
      {
         return theTile;
      }
   }
   
   rspf_uint32 level = resLevel;
   if (theStartingResLevel)  // Used as overview.
   {
      if (theStartingResLevel <= resLevel)
      {
         //---
         // Adjust the level to be relative to the reader using this as
         // overview.
         //---
         level -= theStartingResLevel; 
      }
   }

   //---
   // See if the whole tile is going to be filled, if not, start out with
   // a blank tile so data from a previous load gets wiped out.
   //---
   if ( !tileRect.completely_within(theImageRect) )
   {
      // Start with a blank tile.
      theTile->makeBlank();
   }

   //---
   // See if any point of the requested tile is in the image.
   //---
   if ( tileRect.intersects(theBlockImageRect) )
   {
      rspfIrect clipRect = tileRect.clipToRect(theImageRect);
            
      // See if the requested clip rect is already in the cache tile.
      if ( (clipRect.completely_within(theCacheTile->getImageRectangle()))&&
           (theCacheTile->getDataObjectStatus() != RSPF_EMPTY)&&
           (theCacheTile->getBuf()))
      {
         //---
         // Note: Clip the cache tile(nitf block) to the image clipRect since
         // there are nitf blocks that go beyond the image dimensions, i.e.,
         // edge blocks.
         //---
         rspfIrect cr =
               theCacheTile->getImageRectangle().clipToRect(clipRect);
         theTile->loadTile(theCacheTile->getBuf(),
                           theCacheTile->getImageRectangle(),
                           cr,
                           theCacheTileInterLeaveType);
         //---
         // Validate the tile.  This will set the status to full, partial
         // or empty.  Must be performed if any type of combining is to be
         // performed down the chain.
         //---
         theTile->validate();
      }
      else
      {
         if (loadTile(clipRect) == true)
         {
            //---
            // Validate the tile.  This will set the status to full, partial
            // or empty.  Must be performed if any type of combining is to be
            // performed down the chain.
            //---
            theTile->validate();
         }
         else
         {
            // Flag an error for callers:
            setErrorStatus();
            
            rspfNotify(rspfNotifyLevel_WARN)
               << __FILE__ << " " << __LINE__
               << " loadTile failed!"
               << std::endl;

            theTile->makeBlank(); // loadTile failed...
         }
      }
   } // End of if ( tileRect.intersects(image_rect) )

   return theTile;   
}

bool rspfNitfTileSource::loadTile(const rspfIrect& clipRect)
{
   rspfIrect zbClipRect  = clipRect;

   const rspf_uint32 BLOCK_HEIGHT = theCacheSize.y;
   const rspf_uint32 BLOCK_WIDTH  = theCacheSize.x;

   zbClipRect.stretchToTileBoundary(rspfIpt(BLOCK_WIDTH, BLOCK_HEIGHT));
   
   //---
   // Shift the upper left corner of the "clip_rect" to the an even nitf
   // block boundry.  
   //---
   rspfIpt nitfBlockOrigin = zbClipRect.ul();

   // Vertical block loop.
   rspf_int32 y = nitfBlockOrigin.y;
   while (y < zbClipRect.lr().y)
   {
      // Horizontal block loop.
      rspf_int32 x = nitfBlockOrigin.x;
      while (x < zbClipRect.lr().x)
      {
         if ( loadBlockFromCache(x, y, clipRect) == false )
         {
            if ( loadBlock(x, y) )
            {
               //---
               // Note: Clip the cache tile(nitf block) to the image clipRect
               // since there are nitf blocks that go beyond the image
               // dimensions, i.e., edge blocks.
               //---    
               rspfIrect cr =
                  theCacheTile->getImageRectangle().clipToRect(clipRect);
               
               theTile->loadTile(theCacheTile->getBuf(),
                                 theCacheTile->getImageRectangle(),
                                 cr,
                                 theCacheTileInterLeaveType);
            }
            else
            {
               // Error loading...
               return false;
            }
         }
         
         x += BLOCK_WIDTH; // Go to next block.
      }
      
      y += BLOCK_HEIGHT; // Go to next row of blocks.
   }

   return true;
}

bool rspfNitfTileSource::loadBlockFromCache(rspf_uint32 x, rspf_uint32 y,
                                             const rspfIrect& clipRect)
{
   bool result = false;
   
   if (theCacheEnabledFlag)
   {
      //---
      // The origin set in the cache tile must have the sub image offset in it
      // since "theTile" is relative to any sub image offset.  This is so that
      // "theTile->loadTile(theCacheTile)" will work.
      //---
      rspfIpt origin(x, y);

      rspfRefPtr<rspfImageData> tempTile =
         rspfAppFixedTileCache::instance()->getTile(theCacheId, origin);
      if (tempTile.valid())
      {
         //---
         // Note: Clip the cache tile(nitf block) to the image clipRect since
         // there are nitf blocks that go beyond the image dimensions, i.e.,
         // edge blocks.
         //---    
         rspfIrect cr =
            tempTile->getImageRectangle().clipToRect(clipRect);

         theTile->loadTile(tempTile.get()->getBuf(),
                           tempTile->getImageRectangle(),
                           cr,
                           theCacheTileInterLeaveType);
         result = true;
      }
   }
   
   return result;
}

bool rspfNitfTileSource::loadBlock(rspf_uint32 x, rspf_uint32 y)
{
#if 0
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::loadBlock DEBUG:"
         << "  x:  " << x << " y:  " << y << endl;
   }
#endif
   
   //---
   // The origin set in the cache tile must have the sub image offset in it
   // since "theTile" is relative to any sub image offset.  This is so that
   // "theTile->loadTile(theCacheTile)" will work.
   //---
   rspfIpt origin(x, y);
    
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   theCacheTile->setOrigin(origin);
   rspf_uint32 readSize = theReadBlockSizeInBytes;
   if(!theCacheTile->getImageRectangle().completely_within(theBlockImageRect))
   {
      readSize = getPartialReadSize(origin);
   }
   if((hdr->hasBlockMaskRecords())||
      (readSize != theReadBlockSizeInBytes))
   {
      theCacheTile->makeBlank();
   }

   switch (theReadMode)
   {
      case READ_BIR:  
      case READ_BIR_BLOCK:
      case READ_BIP:  
      case READ_BIP_BLOCK:
      {
         std::streamoff p;
         if(getPosition(p, x, y, 0))
         {
            theFileStr.seekg(p, ios::beg);
            char* buf = (char*)(theCacheTile->getBuf());
            if (!theFileStr.read(buf, readSize))
            {
               theFileStr.clear();
               rspfNotify(rspfNotifyLevel_FATAL)
                  << "rspfNitfTileSource::loadBlock BIP Read Error!"
                  << "\nReturning error..." << endl;
               theErrorStatus = rspfErrorCodes::RSPF_ERROR;
               
               return false;
            }
         }
         break;
      }
      case READ_BSQ_BLOCK:
      case READ_BIB_BLOCK:
      case READ_BIB:
      {
         //---
         // NOTE:
         // With some of these types we could do one read and get all bands.
         // The reads are done per for future enabling on band selection
         // at the image handler level.
         //---
         for (rspf_uint32 band = 0; band < theNumberOfInputBands; ++band)
         {
            rspf_uint8* buf =0;
            if(isVqCompressed(hdr->getCompressionCode())||
               hdr->getRepresentation().upcase().contains("LUT"))
            {
               buf = (rspf_uint8*)&(theCompressedBuf.front());
            }
            else
            {
               buf = (rspf_uint8*)(theCacheTile->getBuf(band));
            }
            std::streamoff p;
            if(getPosition(p, x, y, band))
            {
               theFileStr.seekg(p, ios::beg);
               if (!theFileStr.read((char*)buf, readSize))
               {
                  theFileStr.clear();
                  rspfNotify(rspfNotifyLevel_FATAL)
                     << "rspfNitfTileSource::loadBlock Read Error!"
                     << "\nReturning error..." << endl;
                  theErrorStatus = rspfErrorCodes::RSPF_ERROR;
                  return false;
               }
               else if(hdr->getCompressionCode() == "C4")
               {
                  vqUncompressC4(theCacheTile,
                                 (rspf_uint8*)&(theCompressedBuf.front()));
               }

               else if(hdr->getCompressionCode() == "M4")
               {
                  vqUncompressM4(theCacheTile,
                                 (rspf_uint8*)&(theCompressedBuf.front()));
               }
               else if(hdr->getRepresentation().upcase().contains("LUT"))
               {
                  lutUncompress(theCacheTile,
                                (rspf_uint8*)&(theCompressedBuf.front()));
               }
            }
         }
         break;
      }
      case READ_JPEG_BLOCK:
      {
         if (uncompressJpegBlock(x, y) == false)
         {
            theFileStr.clear();
            rspfNotify(rspfNotifyLevel_FATAL)
               << "rspfNitfTileSource::loadBlock Read Error!"
               << "\nReturning error..." << endl;
            theErrorStatus = rspfErrorCodes::RSPF_ERROR;
            return false;
         }
         break;
      }
      default:
         break;
   }
   
   if(thePackedBitsFlag)
   {
      explodePackedBits(theCacheTile);
   }
   // Check for swap bytes.
   if (theSwapBytesFlag)
   {
      rspfEndian swapper;
      swapper.swap(theScalarType,
                   theCacheTile->getBuf(),
                   theCacheTile->getSize());
   }

   if ( !isVqCompressed(hdr->getCompressionCode()) )
   {
      convertTransparentToNull(theCacheTile);
   }

   // Set the origin of the cache tile.
   theCacheTile->validate();
   if (theCacheEnabledFlag)
   {
      // Add it to the cache for the next time.
      rspfAppFixedTileCache::instance()->addTile(theCacheId, theCacheTile);
   }
   
   return true;
}

void rspfNitfTileSource::explodePackedBits(rspfRefPtr<rspfImageData> packedBuffer)const
{
   rspf_uint8* tempBuf = new rspf_uint8[packedBuffer->getSizePerBandInBytes()];
   rspf_uint32 idx      = 0;
   rspf_uint32 bandIdx  = 0;
   rspf_uint32 h = packedBuffer->getHeight();
   rspf_uint32 w = packedBuffer->getWidth();
   rspf_uint32 maxIdx = w*h;
   rspf_uint32 bandCount = packedBuffer->getNumberOfBands();
   switch(packedBuffer->getScalarType())
   {
      case RSPF_UINT8:
      {
         
         rspf_uint8* outputBuf = (rspf_uint8*)tempBuf;
         for(bandIdx = 0; bandIdx < bandCount; ++bandIdx)
         {
            rspfPackedBits packedBits((rspf_uint8*)packedBuffer->getBuf(bandIdx),
                                           getCurrentImageHeader()->getBitsPerPixelPerBand());
            for(idx = 0; idx < maxIdx; ++idx)
            {
               *outputBuf = (rspf_uint8)packedBits.getValueAsUint32(idx);
               ++outputBuf;
            }
            
            memcpy((char*)packedBuffer->getBuf(bandIdx),
                   (char*)tempBuf,
                   theCacheTile->getSizePerBandInBytes()*bandCount);
         }
         break;
      }
      case RSPF_UINT16:
      {
         
         rspf_uint16* outputBuf = (rspf_uint16*)tempBuf;
         for(bandIdx = 0; bandIdx < bandCount; ++bandIdx)
         {
            rspfPackedBits packedBits((rspf_uint8*)packedBuffer->getBuf(bandIdx),
                                           getCurrentImageHeader()->getBitsPerPixelPerBand());
            for(idx = 0; idx < maxIdx; ++idx)
            {
               *outputBuf = (rspf_uint16)packedBits.getValueAsUint32(idx);
               ++outputBuf;
            }
            
            memcpy((char*)packedBuffer->getBuf(bandIdx),
                   (char*)tempBuf,
                   theCacheTile->getSizePerBandInBytes()*bandCount);
         }
         break;
      }
      case RSPF_FLOAT:
      {
         rspf_float32* outputBuf = (rspf_float32*)tempBuf;
         for(bandIdx = 0; bandIdx < bandCount; ++bandIdx)
         {
            rspfPackedBits packedBits((rspf_uint8*)packedBuffer->getBuf(bandIdx),
                                           getCurrentImageHeader()->getBitsPerPixelPerBand());
            for(idx = 0; idx < maxIdx; ++idx)
            {
               *outputBuf = (rspf_float32)packedBits.getValueAsUint32(idx);
               ++outputBuf;
            }
            
            memcpy((char*)packedBuffer->getBuf(bandIdx),
                   (char*)tempBuf,
                   theCacheTile->getSizePerBandInBytes()*bandCount);
         }
         break;
      }
      default:
      {
         break;
      }
   }
   delete [] tempBuf;
}

void rspfNitfTileSource::convertTransparentToNull(rspfRefPtr<rspfImageData> tile)const
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();

   if(!hdr||!tile) return;

   if(!tile->getBuf()) return;
   rspfIpt tempOrigin = tile->getOrigin();
   rspf_uint32 blockNumber = getBlockNumber(tempOrigin);
   rspf_uint32 numberOfBands = tile->getNumberOfBands();
   rspf_uint32 band = 0;

   if(hdr->hasPadPixelMaskRecords())
   {
      if(hdr->hasTransparentCode())
      {
         rspf_uint32 idx = 0;
         rspf_uint32 maxIdx = tile->getWidth()*tile->getHeight();
         
         for (band = 0; band < numberOfBands; ++band)
         {
            if(hdr->getPadPixelMaskRecordOffset(blockNumber,
                                                band)!=0xffffffff)
            {
               switch(tile->getScalarType())
               {
                  case RSPF_UINT8:
                  {
                     rspf_uint8 transparentValue = hdr->getTransparentCode();
                     rspf_uint8* buf = (rspf_uint8*)tile->getBuf(band);
                     rspf_uint8 nullPix = (rspf_uint8)tile->getNullPix(band);
                     for(idx = 0; idx < maxIdx; ++idx)
                     {
                        if(*buf == transparentValue)
                        {
                           *buf = nullPix;
                        }
                        ++buf;
                     }
                     break;
                  }
                  case RSPF_USHORT11:
                  case RSPF_UINT16:
                  {
                     rspf_uint16 transparentValue = hdr->getTransparentCode();
                     rspf_uint16* buf = (rspf_uint16*)tile->getBuf(band);
                     rspf_uint16 nullPix = (rspf_uint16)tile->getNullPix(band);
                     for(idx = 0; idx < maxIdx; ++idx)
                     {
                        if(*buf == transparentValue)
                        {
                           *buf = nullPix;
                        }
                        ++buf;
                     }
                     break;
                  }
                  case RSPF_SINT16:
                  {
                     rspf_sint16 transparentValue = hdr->getTransparentCode();
                     rspf_sint16* buf = (rspf_sint16*)tile->getBuf(band);
                     rspf_sint16 nullPix = (rspf_sint16)tile->getNullPix(band);
                     for(idx = 0; idx < maxIdx; ++idx)
                     {
                        if(*buf == transparentValue)
                        {
                           *buf = nullPix;
                        }
                        ++buf;
                     }
                     break;
                  }
                  default:
                  {
                     break;
                  }
               }
            }
         }
      }
   }
}


double rspfNitfTileSource::getMinPixelValue(rspf_uint32 band)const
{
   double result = rspfImageHandler::getMinPixelValue(band);

   if(thePackedBitsFlag)
   {
      if(result < 1.0) result = 1.0;
   }
   
   return result;
}

double rspfNitfTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   double result = rspfImageHandler::getMaxPixelValue(band);

   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if(hdr)
   {
      if(thePackedBitsFlag)
      {
         double test = 1<<(hdr->getBitsPerPixelPerBand());
         
         if(result > test) result = test;
      }
      else
      {
         rspf_int32 bitsPerPixel = hdr->getActualBitsPerPixelPerBand();
         switch (bitsPerPixel)
         {
            case 11:
            {
               if (result > 2047.0)
               {
                  result = 2047.0;
               }
               break;
            }
            case 12:
            {
               if (result > 4095.0)
               {
                  result = 4095.0;
               }
               break;
            }
            default:
               break;
         }
      }
   }

   return result;
}

double rspfNitfTileSource::getNullPixelValue(rspf_uint32 band)const
{
   double result = rspfImageHandler::getNullPixelValue(band);

   if(thePackedBitsFlag)
   {
      if((result < 0) ||
         (result > getMaxPixelValue(band)))
         {
            result = 0.0;
         }
   }


   return result;
}


bool rspfNitfTileSource::getPosition(std::streamoff& streamPosition,
                                      rspf_uint32 x,
                                      rspf_uint32 y,
                                      rspf_uint32 band) const
{
   //
   // NOTE:  "theCacheSize is always relative to a block size except in
   // the case where a block is the entire image.
   //
   streamPosition = 0;
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return streamPosition;
   }

   rspf_uint64 blockNumber = getBlockNumber(rspfIpt(x,y));
   
#if 0
   cout << "rspfNitfTileSource::getPosition blockNumber:  "
        << blockNumber << endl;
#endif
   
   streamPosition = (std::streamoff)hdr->getDataLocation(); // Position to first block.
   if(hdr->hasBlockMaskRecords())
   {
      rspf_uint64 blockOffset = hdr->getBlockMaskRecordOffset(blockNumber,
                                                               band);
      if(blockOffset == 0xffffffff)
      {
         return false;
      }
      streamPosition += blockOffset;
   }
   
   switch (theReadMode)
   {
      case READ_BIB_BLOCK:
      {
         if(!hdr->hasBlockMaskRecords())
         {
            streamPosition +=
            (std::streamoff)((rspf_uint64)blockNumber * 
                (rspf_uint64)getBlockOffset()) +
               ((rspf_uint64)getBandOffset() * band);
         }
         else
         {
            streamPosition += (std::streamoff)((rspf_uint64)getBandOffset() * (rspf_uint64)band);
            
         }
         break;
      }
      
      case READ_BIB:
      {
         streamPosition +=
         (std::streamoff) ((rspf_uint64)blockNumber * (rspf_uint64)theReadBlockSizeInBytes)+
            ((rspf_uint64)getBandOffset() * (rspf_uint64)band);
         break;
      }
      
      case READ_BSQ_BLOCK:
      {
         
         if(!hdr->hasBlockMaskRecords())
         {
            streamPosition += (std::streamoff)((rspf_uint64)blockNumber * 
                                               (rspf_uint64)getBlockOffset()) +
                                              ((rspf_uint64)getBandOffset() * 
                                               (rspf_uint64)band);
         }
         
         break;
      }
      case READ_JPEG_BLOCK:
      {
         streamPosition += (std::streamoff)((rspf_uint64)blockNumber * (rspf_uint64)theReadBlockSizeInBytes);
         break;
      }
      default:
      {
         if(!hdr->hasBlockMaskRecords())
         {
            streamPosition += (std::streamoff)((rspf_uint64)blockNumber*(rspf_uint64)getBlockOffset());
         }
         
         break;
      }
   }

   return true;
}

std::streampos rspfNitfTileSource::getBandOffset() const
{
   std::streampos bandOffset = 0;

   switch (theReadMode)
   {
      case READ_BIB_BLOCK:
      case READ_BIP_BLOCK:
      case READ_BIR_BLOCK:
      case READ_BIB:
      case READ_BIP:
      case READ_BIR:
         bandOffset = theBlockSizeInBytes;
         break;
         
      case READ_BSQ_BLOCK:
         bandOffset = getNumberOfBlocks() * theBlockSizeInBytes;
         break;

      default:
         break;
   }

   return bandOffset;
}

std::streampos rspfNitfTileSource::getBlockOffset() const
{
   std::streampos blockOffset = 0;
   std::streampos blockSizeInBytes = 0;
   if (getNumberOfBlocks() == 1)
   {
      blockSizeInBytes = theReadBlockSizeInBytes;
   }
   else
   {
      blockSizeInBytes = theBlockSizeInBytes;
   }
   
   switch (theReadMode)
   {
      case READ_BIB_BLOCK:
      case READ_BIB:
         // Band interleaved by block.
         blockOffset = blockSizeInBytes * theNumberOfInputBands;
         break;
         
      case READ_BIR_BLOCK:
      case READ_BSQ_BLOCK:
      case READ_BIP_BLOCK:
      case READ_BIP:
      case READ_BIR:
         // Blocks side by side.
         blockOffset = blockSizeInBytes;
         break;
      case READ_JPEG_BLOCK:
        blockSizeInBytes = theReadBlockSizeInBytes;
        break;
   
      default:
         break;
   }

   return blockOffset;
}
   
rspf_uint32 rspfNitfTileSource::getNumberOfBlocks() const
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return 0;
   }

   return static_cast<rspf_uint32>( hdr->getNumberOfBlocksPerRow() *
                                     hdr->getNumberOfBlocksPerCol() );
}

bool rspfNitfTileSource::loadState(const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   if ( !rspfImageHandler::loadState(kwl, prefix) )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfNitfTileSource::loadState(kwl, prefix) DEBUG:"
            << "\nUnable to load, exiting..." << std::endl;
      }
      return false;
   }
   
   const char* lookup = kwl.find(prefix, "entry");
   if (lookup)
   {
      rspfString s(lookup);
      theCurrentEntry = s.toUInt32();
   }

   lookup = kwl.find(prefix,rspfKeywordNames::ENABLE_CACHE_KW);
   if (lookup)
   {
      rspfString s(lookup);
      theCacheEnabledFlag = s.toBool();
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::loadState(kwl, prefix) DEBUG:"
         << "\nCurrent entry:      " << theCurrentEntry
         << "\nCache enable flag:  " << theCacheEnabledFlag
         << std::endl;
   }
   
   return open();
}

bool rspfNitfTileSource::saveState(rspfKeywordlist& kwl,
                                    const char* prefix) const
{
   // Add the entry number.
   kwl.add(prefix, "entry", theCurrentEntry, true);

   // Add the cache_enable flag.
   kwl.add(prefix, rspfKeywordNames::ENABLE_CACHE_KW, theCacheEnabledFlag, true);

   // Call the base class save state.
   return rspfImageHandler::saveState(kwl, prefix);
}

rspfScalarType rspfNitfTileSource::getOutputScalarType() const
{
   return theScalarType;
}

rspf_uint32 rspfNitfTileSource::getTileWidth() const
{
   rspf_uint32 result = 0;
   if(!theCacheSize.hasNans()&& theCacheSize.x > 0)
   {
      result = theCacheSize.x;
   }
   else
   {
      rspfIpt tileSize;
      rspf::defaultTileSize(tileSize);
      result = static_cast<rspf_uint32>(tileSize.x);
   }
   return result;
}

rspf_uint32 rspfNitfTileSource::getTileHeight() const
{
   rspf_uint32 result = 0;
   if(!theCacheSize.hasNans()&& theCacheSize.y > 0)
   {
      result = theCacheSize.y;
   }
   else
   {
      rspfIpt tileSize;
      rspf::defaultTileSize(tileSize);
      result = static_cast<rspf_uint32>(tileSize.y);
   }
   return result;
}

rspf_uint32 rspfNitfTileSource::getNumberOfInputBands() const
{
   return theNumberOfInputBands;
}

rspf_uint32 rspfNitfTileSource::getNumberOfOutputBands() const
{
   return theNumberOfOutputBands;
}

rspf_uint32 rspfNitfTileSource::getNumberOfLines(rspf_uint32 resLevel) const
{
   rspf_uint32 result = 0;
   if (resLevel == 0)
   {
      const rspfNitfImageHeader* hdr = getCurrentImageHeader();
      if (hdr)
      {
         result = hdr->getNumberOfRows();
      }
   }
   else if (theOverview.valid())
   {
      result = theOverview->getNumberOfLines(resLevel);
   }
   return result;
}

rspf_uint32 rspfNitfTileSource::getNumberOfSamples(rspf_uint32 resLevel) const
{
   rspf_uint32 result = 0;
   if (resLevel == 0)
   {
      const rspfNitfImageHeader* hdr = getCurrentImageHeader();
      if (hdr)
      {
         result = hdr->getNumberOfCols();
      }
   }
   else if (theOverview.valid())
   {
      result = theOverview->getNumberOfSamples(resLevel);
   }
   return result;
}

rspf_uint32 rspfNitfTileSource::getBlockNumber(const rspfIpt& block_origin) const
{
   rspf_uint32 blockNumber = 0;
   
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return blockNumber; 
   }

   rspf_uint32 blockY;
   rspf_uint32 blockX;
   blockX  = (block_origin.x /
              theCacheSize.x);
   blockY= (block_origin.y /
            theCacheSize.y);

   switch (theReadMode)
   {
      case READ_BIB_BLOCK:
      case READ_BIP_BLOCK:
      case READ_BIR_BLOCK:
      case READ_BSQ_BLOCK:
      case READ_JPEG_BLOCK:
      {
         blockNumber = ((blockY*hdr->getNumberOfBlocksPerRow()) +
                        blockX);
         break;
      }
      case READ_BIB:
      case READ_BIP:
      case READ_BIR:
         //---
         // These read modes are for a single block image.  The cache size will
         // be set to the width of the image (block) by the height of one tile.
         //
         // This is to avoid reading an entire large image with a single block
         // into memory.
         //---
         blockNumber = blockY;
         break;

      default:
         break;
   }
   return blockNumber;
}

rspf_uint32 rspfNitfTileSource::getPartialReadSize(const rspfIpt& /* blockOrigin */)const
{
   rspf_uint32 result = 0;
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return result;
   }
   
   if(theCacheTile->getImageRectangle().completely_within(theBlockImageRect))
   {
      return theReadBlockSizeInBytes;
   }
   rspfIrect clipRect = theCacheTile->getImageRectangle().clipToRect(theBlockImageRect);
   
   result = (theCacheSize.x*
             clipRect.height()*
             hdr->getBitsPerPixelPerBand())/8;
   
   switch (theReadMode)
   {
      case READ_BSQ_BLOCK:
      case READ_BIB_BLOCK:
      case READ_BIB:
      {
         // purposely left blank.  only hear for clarity.
         break;
      }

      case READ_BIP_BLOCK:
      case READ_BIR_BLOCK:
      case READ_BIP:
      case READ_BIR:   
      {
         result *= theNumberOfInputBands;
         break;
      }
      default:
      {
         break;
      }
   }
   return result;
}

bool rspfNitfTileSource::isVqCompressed(const rspfString& compressionCode)const
{
   return((compressionCode == "C4")||
          (compressionCode == "M4"));
}


rspf_uint32 rspfNitfTileSource::getImageTileWidth() const
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return 0;
   }
   return hdr->getNumberOfPixelsPerBlockHoriz();
}

rspf_uint32 rspfNitfTileSource::getImageTileHeight() const
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return 0;
   }
   return hdr->getNumberOfPixelsPerBlockVert();
}

rspfString rspfNitfTileSource::getShortName()const
{
   return rspfString("nitf");
}

rspfString rspfNitfTileSource::getLongName()const
{
   return rspfString("nitf reader");
}

rspf_uint32 rspfNitfTileSource::getCurrentEntry() const
{
   return theCurrentEntry;
}

rspf_uint32 rspfNitfTileSource::getNumberOfEntries() const
{
   return (rspf_uint32)theEntryList.size();
}

void rspfNitfTileSource::getEntryList(std::vector<rspf_uint32>& entryList)const
{
   entryList = theEntryList;
//    entryList.resize(theNumberOfImages);
//    for (rspf_uint32 i = 0; i < theNumberOfImages; ++i)
//    {
//       entryList[i] = i;
//    }
}

bool rspfNitfTileSource::setCurrentEntry(rspf_uint32 entryIdx)
{
   bool result = true;
   if (theCurrentEntry != entryIdx)
   {
      if ( isOpen() )
      {
         if ( entryIdx < theNumberOfImages )
         {
            // Clear the geometry.
            theGeometry = 0;
            
            // Must clear or openOverview will use last entries.
            theOverviewFile.clear();
            
            theCurrentEntry = entryIdx;
            
            //---
            // Since we were previously open and the the entry has changed we
            // need to reinitialize some things.
            //---
            result = allocate();
            if (result)
            {
               completeOpen();
            }
         }
         else
         {
            result = false; // Entry index out of range.
         }
      }
      else
      {
         //---
         // Not open.
         // Allow this knowing that the parseFile will check for out of range.
         //---
         theCurrentEntry = entryIdx;
      }
   }
   
   if(result)
   {
      if(theNitfImageHeader[theCurrentEntry]->getRepresentation().contains("LUT"))
      {
         theLut = theNitfImageHeader[theCurrentEntry]->createLut(0);
      }
      
      
   }
   return result;
}

bool rspfNitfTileSource::getCacheEnabledFlag() const
{
   return theCacheEnabledFlag;
}

void rspfNitfTileSource::setCacheEnabledFlag(bool flag)
{
   if (flag != theCacheEnabledFlag)
   {
      // State of caching has changed...

      theCacheEnabledFlag = flag;

      if ( theCacheEnabledFlag) // Cache enabled.
      {
         theCacheId = rspfAppFixedTileCache::instance()->
            newTileCache(theBlockImageRect, theCacheSize);
      }
      else // Cache disabled...
      {
         // Clean out the cache if there was one.
         if (theCacheId != -1)
         {
            rspfAppFixedTileCache::instance()->deleteCache(theCacheId);
            theCacheId = -1;
         }
      }
   }
}

const rspfNitfFileHeader* rspfNitfTileSource::getFileHeader()const
{
   if(theNitfFile.valid())
   {
      return theNitfFile->getHeader();
   }
   
   return 0;
}

rspfNitfFileHeader* rspfNitfTileSource::getFileHeader()
{
   if(theNitfFile.valid())
   {
      return theNitfFile->getHeader();
   }
   
   return 0;
}

const rspfNitfImageHeader* rspfNitfTileSource::getCurrentImageHeader() const
{
   if(theNitfImageHeader.size())
   {
      return theNitfImageHeader[theCurrentEntry].get();
   }
   
   return 0;
}

rspfNitfImageHeader* rspfNitfTileSource::getCurrentImageHeader()
{
   if(theNitfImageHeader.size())
   {
      return theNitfImageHeader[theCurrentEntry].get();
   }
   
   return 0;
}

void rspfNitfTileSource::setBoundingRectangle(const rspfIrect& imageRect)
{
   theImageRect = imageRect;
   // now shift the internal block rect as well
   theBlockImageRect = (theBlockImageRect - theBlockImageRect.ul());
}

rspfRefPtr<rspfProperty> rspfNitfTileSource::getProperty(const rspfString& name)const
{
   if (name == rspfKeywordNames::ENABLE_CACHE_KW)
   {
      rspfProperty* p = new rspfBooleanProperty(name, theCacheEnabledFlag);
      return rspfRefPtr<rspfProperty>(p);
   }
   else 
   {
      if(theNitfFile.valid())
      {
         if(theNitfFile->getHeader())
         {
            rspfRefPtr<rspfProperty> p = theNitfFile->getHeader()->getProperty(name);
            if(p.valid())
            {
               p->setReadOnlyFlag(true);
               return p;
            }
         }
      }
      const rspfNitfImageHeader* imageHeader = getCurrentImageHeader();
      if(imageHeader)
      {
         rspfRefPtr<rspfProperty> p = imageHeader->getProperty(name);
         if(p.valid())
         {
            p->setReadOnlyFlag(true);
            return p;
         }
      }
   }

   return rspfImageHandler::getProperty(name);
}

void rspfNitfTileSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if (!property) return;
   
   rspfString name = property->getName();

   if (name == rspfKeywordNames::ENABLE_CACHE_KW)
   {
      rspfBooleanProperty* obj = PTR_CAST(rspfBooleanProperty,
                                           property.get());
      if (obj)
      {
         setCacheEnabledFlag(obj->getBoolean());
      }
   }
   else
   {
      rspfImageHandler::setProperty(property);
   }
}

void rspfNitfTileSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageHandler::getPropertyNames(propertyNames);
   propertyNames.push_back(rspfKeywordNames::ENABLE_CACHE_KW);
   if(theNitfFile->getHeader())
   {
      theNitfFile->getHeader()->getPropertyNames(propertyNames);
   }
   const rspfNitfImageHeader* imageHeader = getCurrentImageHeader();
   if(imageHeader)
   {
      imageHeader->getPropertyNames(propertyNames);
   }
}

rspfString rspfNitfTileSource::getSecurityClassification() const
{
   if(getCurrentImageHeader())
   {
      return getCurrentImageHeader()->getSecurityClassification();
   }
   
   return "U";
}

void rspfNitfTileSource::lutUncompress(rspfRefPtr<rspfImageData> destination, rspf_uint8* source)
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr||!destination)
   {
      return;
   }
   if((destination->getNumberOfBands()<3)||
      (!destination->getBuf())||
      (destination->getScalarType()!=RSPF_UINT8)||
      (!theLut.valid()))
   {
      return;
   }

   if(destination->getNumberOfBands()!=theLut->getNumberOfBands())
   {
      return;
   }
   
   rspf_uint8* tempRows[3];
   tempRows[0] = (rspf_uint8*)destination->getBuf(0);
   tempRows[1] = (rspf_uint8*)destination->getBuf(1);
   tempRows[2] = (rspf_uint8*)destination->getBuf(2);
   
   rspf_uint8* srcPtr = source;
   rspf_uint32 compressionYidx   = 0;
   rspf_uint32 compressionXidx   = 0;
   rspf_uint32 uncompressIdx     = 0;
   rspf_uint32 h = destination->getHeight();
   rspf_uint32 w = destination->getWidth();
   
   for(compressionYidx = 0; compressionYidx < h; ++compressionYidx)
   {
      for(compressionXidx = 0; compressionXidx < w; ++compressionXidx)
      {
         tempRows[0][uncompressIdx] = (*theLut)[*srcPtr][0];
         tempRows[1][uncompressIdx] = (*theLut)[*srcPtr][1];
         tempRows[2][uncompressIdx] = (*theLut)[*srcPtr][2];
         ++srcPtr;
         ++uncompressIdx;
      }
   }
}

void rspfNitfTileSource::vqUncompressC4(
   rspfRefPtr<rspfImageData> destination, rspf_uint8* source)
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr||!destination)
   {
      return;
   }

   const rspf_uint32 BANDS = destination->getNumberOfBands();

   if( ( (BANDS != 1) && (BANDS!=3) ) ||
       (!destination->getBuf()) ||
       (destination->getScalarType()!=RSPF_UINT8) ||
       !theLut.valid() ||
       (theLut->getNumberOfBands() != BANDS) )
   {
      return;
   }
   
   rspfNitfVqCompressionHeader* compressionHeader =
      PTR_CAST(rspfNitfVqCompressionHeader,
               hdr->getCompressionHeader().get());

   if(!compressionHeader)
   {
      return;
   }
   
   const std::vector<rspfNitfVqCompressionOffsetTableData>& table =
      compressionHeader->getTable();

   rspfRefPtr<rspfNitfImageBand> bandInfo = hdr->getBandInformation(0);
   
   if(!bandInfo.valid()) return;
   
   std::vector<rspfRefPtr<rspfNitfImageLut> > luts(BANDS);
   std::vector<rspf_uint8*> tempRows(BANDS);
   
   rspf_uint32 band;
   for (band =0; band<BANDS; ++band)
   {
      luts[band] = bandInfo->getLut(band);
      if ( luts[band].valid() )
      {
         tempRows[band] = (rspf_uint8*)destination->getBuf(band);
      }
      else
      {
         return;
      }
   }

   rspfPackedBits bits(source, compressionHeader->getImageCodeBitLength());


   const rspf_uint32 ROWS = static_cast<rspf_uint32>(table.size());
   const rspf_uint32 COLS =
      static_cast<rspf_uint32>(table[0].
                                theNumberOfValuesPerCompressionLookup);
   const rspf_uint32 COMPRESSION_HEIGHT =
      compressionHeader->getNumberOfImageRows();
   const rspf_uint32 COMPRESSION_WIDTH  =
      compressionHeader->getNumberOfImageCodesPerRow();
   rspf_uint32 DEST_WIDTH  = destination->getWidth();
   
   rspf_uint32 compressionIdx = 0;
   rspf_uint32 uncompressIdx  = 0;
   rspf_uint32 uncompressYidx = 0;
   rspf_uint8  lutValue = 0;
   rspf_uint8* data     = 0;
   rspf_uint32 codeWord = 0;
   
   for(rspf_uint32 compressionYidx = 0;
       compressionYidx < COMPRESSION_HEIGHT;
       ++compressionYidx)
   {
      uncompressYidx = compressionYidx * ROWS * DEST_WIDTH;
      
      for(rspf_uint32 compressionXidx = 0;
          compressionXidx < COMPRESSION_WIDTH;
          ++compressionXidx)
      {
         uncompressIdx = uncompressYidx + COLS * compressionXidx;

         codeWord = bits.getValueAsUint32(compressionIdx++);
         codeWord *= COLS;

         for(rspf_uint32 rowIdx = 0; rowIdx < ROWS; ++rowIdx)
         {
            data = &(table[rowIdx].theData[codeWord]);
            
            for(rspf_uint32 colIdx = 0; colIdx < COLS; ++colIdx)
            {
               lutValue = (*data)&0xff;

               for (band = 0; band < BANDS; ++band)
               {
                  rspf_uint8 p = (*theLut.get())[lutValue][band];
                  tempRows[band][uncompressIdx+colIdx] = p;
               }
               ++data;
               
            } // column loop

            uncompressIdx += DEST_WIDTH;
            
         } // row loop

      } // x compression loop
      
   } // y compression loop
}

void rspfNitfTileSource::vqUncompressM4(
   rspfRefPtr<rspfImageData> destination, rspf_uint8* source)
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr||!destination)
   {
      return;
   }

   const rspf_uint32 BANDS = destination->getNumberOfBands();

   if(( (BANDS != 1)&&(BANDS!=3) ) ||
      (!destination->getBuf())||
      (destination->getScalarType()!=RSPF_UINT8))
   {
      return;
   }
   
   rspfNitfVqCompressionHeader* compressionHeader =
      PTR_CAST(rspfNitfVqCompressionHeader,
               hdr->getCompressionHeader().get());

   if(!compressionHeader)
   {
      return;
   }

   const std::vector<rspfNitfVqCompressionOffsetTableData>& table =
      compressionHeader->getTable();

   rspfRefPtr<rspfNitfImageBand> bandInfo = hdr->getBandInformation(0);
   
   if(!bandInfo.valid()) return;
   
   std::vector<rspfRefPtr<rspfNitfImageLut> > luts(BANDS);
   std::vector<rspf_uint8*> tempRows(BANDS);

   rspf_uint32 band;
   for (band =0; band<BANDS; ++band)
   {
      luts[band] = bandInfo->getLut(band);
      if ( luts[band].valid() )
      {
         tempRows[band] = (rspf_uint8*)destination->getBuf(band);
      }
      else
      {
         return;
      }
   }

   const rspf_uint8 NI = 216; // null index (transparency index).
   const rspf_uint8 NP = 0;   // null pixel

   rspf_uint32 destWidth  = destination->getWidth();
   rspfPackedBits bits(source, compressionHeader->getImageCodeBitLength());

   rspf_uint32 compressionYidx   = 0;
   rspf_uint32 compressionXidx   = 0;
   rspf_uint32 compressionIdx    = 0;
   rspf_uint32 uncompressIdx     = 0;
   rspf_uint32 uncompressYidx    = 0;
   rspf_uint32 rows   = (rspf_uint32)table.size();
   rspf_uint32 cols   = 0;
   rspf_uint32 rowIdx = 0;
   rspf_uint32 colIdx = 0;
   if(rows)
   {
      cols = table[0].theNumberOfValuesPerCompressionLookup;
   }
   rspf_uint32 compressionHeight = compressionHeader->getNumberOfImageRows();
   rspf_uint32 compressionWidth  =
      compressionHeader->getNumberOfImageCodesPerRow();
   rspf_uint8 lutValue = 0;
   rspf_uint8* data=0;

   for(compressionYidx = 0;
       compressionYidx < compressionHeight;
       ++compressionYidx)
   {
      uncompressYidx = compressionYidx*rows*destWidth;

      for(compressionXidx = 0;
          compressionXidx < compressionWidth;
          ++compressionXidx)
      {
         uncompressIdx = uncompressYidx + cols*compressionXidx;
         rspf_uint32 codeWord = bits.getValueAsUint32(compressionIdx);
         
         bool transparent = false;
         if (codeWord == 4095)
         {
            //---
            // Check to see if the whole kernel is transparent.  If no, the
            // null index '216' could be used for valid pixels.
            //
            // For more see docs:
            // MIL-PRF-89041A 3.13.1.2 Transparent pixels
            // MIL-STD-2411
            //---
            codeWord *= cols;
            transparent = true;
            for(rowIdx = 0; rowIdx < rows; ++rowIdx)
            {
               data = &table[rowIdx].theData[codeWord];
               
               for(colIdx = 0; colIdx < cols; ++colIdx)
               {
                  lutValue = (*data)&0xff;
                  if (lutValue != NI)
                  {
                     // Not a kernel full of transparent pixels.
                     transparent = false;
                     break;
                  }
                  ++data;
               }
               if (!transparent)
               {
                  break;
               }
               uncompressIdx += destWidth;
            }
         }

         // Reset everyone for loop to copy pixel data from lut.
         uncompressIdx = uncompressYidx + cols*compressionXidx;
         codeWord = bits.getValueAsUint32(compressionIdx);
         codeWord *= cols;

         for(rowIdx = 0; rowIdx < rows; ++rowIdx)
         {
            data = &table[rowIdx].theData[codeWord];
            
            for(colIdx = 0; colIdx < cols; ++colIdx)
            {
               lutValue = (*data)&0xff;
               
               for (band = 0; band < BANDS; ++band)
               {
                  rspf_uint8 p = luts[band]->getValue(lutValue);
                  tempRows[band][uncompressIdx+colIdx] = (!transparent?p:NP);
               }
               ++data;
            }

            uncompressIdx += destWidth;
         }
         ++compressionIdx;
         
      } // x loop
      
   } // y loop
}

bool rspfNitfTileSource::scanForJpegBlockOffsets()
{
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();

   if ( !hdr || (theReadMode != READ_JPEG_BLOCK) || !theFileStr )
   {
      return false;
   }

   theNitfBlockOffset.clear();
   theNitfBlockSize.clear();

   //---
   // Get the totol blocks.
   // Note:  There can be more than one jpeg image in the nitf.  So after blocks
   // found equals total_blocks get out.
   //---
   rspf_uint32 total_blocks = hdr->getNumberOfBlocksPerRow()*hdr->getNumberOfBlocksPerCol();
   
   //---
   // NOTE:
   // SOI = 0xffd8 Start of image
   // EOI = 0xffd9 End of image
   // DHT = 0xffc4 Define Huffman Table(s)
   // DQT = 0xffdb Define Quantization Table(s)
   //---
   char c;

   // Seek to the first block.
   theFileStr.seekg(hdr->getDataLocation(), ios::beg);
   if (theFileStr.fail())
   {
      return false;
   }
   
   // Read the first two bytes and verify it is SOI; if not, get out.
   theFileStr.get( c );
   if (static_cast<rspf_uint8>(c) != 0xff)
   {
      return false;
   }
   theFileStr.get(c);
   if (static_cast<rspf_uint8>(c) != 0xd8)
   {
      return false;
   }

   rspf_uint32 blockSize = 2;  // Read two bytes...

   // Add the first offset.
   theNitfBlockOffset.push_back(hdr->getDataLocation());

   // Find all the SOI markers.
   while ( theFileStr.get(c) ) 
   {
      ++blockSize;
      if (static_cast<rspf_uint8>(c) == 0xff) // Found FF byte.
      {
         //---
         // Loop to skip multiple 0xff's in cases like FF FF D8
         //---
         while ( theFileStr.get(c) )
         {
            ++blockSize;
            if (static_cast<rspf_uint8>(c) != 0xff)
            {
               break;
            }
         }
         
         if (static_cast<rspf_uint8>(c) == 0xd8) 
         {
            // At SOI 0xFFD8 marker...
            std::streamoff pos = theFileStr.tellg();
            theNitfBlockOffset.push_back(pos-2);
         }
         else if (static_cast<rspf_uint8>(c) == 0xd9)
         {
            // At EOI 0xD9marker...
            theNitfBlockSize.push_back(blockSize);
            blockSize = 0;
         }

         //---
         // Since there can be more than one jpeg entry in a file, breeak out of
         // loop when we hit block size.
         //---
         if ( (theNitfBlockOffset.size() == total_blocks) &&
              (theNitfBlockSize.size()   == total_blocks) )
         {
            break;
         }
      }
   }

   theFileStr.seekg(0, ios::beg);
   theFileStr.clear();
   
   if (theNitfBlockOffset.size() != total_blocks)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG:"
            << "\nBlock offset count wrong!"
            << "\nblocks:  " << total_blocks
            << "\noffsets:  " << theNitfBlockOffset.size()
            << std::endl;
      }
      
      return false;
   }
   if (theNitfBlockSize.size() != total_blocks)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "DEBUG:"
            << "\nBlock size count wrong!"
            << "\nblocks:  " << total_blocks
            << "\nblock size array:  " << theNitfBlockSize.size()
            << std::endl;
      }

      return false;
   }

#if 0 /* Please leave for debug. (drb) */
   rspfNotify(rspfNotifyLevel_WARN) << "current entry: " << theCurrentEntry << "\n";
   for (rspf_uint32 i = 0; i < total_blocks; ++i)
   {
      cout << "theNitfBlockOffset[" << i << "]: " << theNitfBlockOffset[i]
           << "\ntheNitfBlockSize[" << i << "]: " << theNitfBlockSize[i]
           << "\n";
   }
#endif

   return true;
}

bool rspfNitfTileSource::uncompressJpegBlock(rspf_uint32 x, rspf_uint32 y)
{
   rspf_uint32 blockNumber = getBlockNumber( rspfIpt(x,y) );

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfNitfTileSource::uncompressJpegBlock DEBUG:"
         << "\nblockNumber:  " << blockNumber
         << std::endl;
   }

   //---
   // Logic to hold off on scanning for offsets until a block is actually needed
   // to speed up loads for things like rspf-info that don't actually read
   // pixel data.
   //---
   if ( m_jpegOffsetsDirty )
   {
      if ( scanForJpegBlockOffsets() )
      {
         m_jpegOffsetsDirty = false;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_FATAL)
            << "rspfNitfTileSource::uncompressJpegBlock scan for offsets error!"
            << "\nReturning error..." << endl;
         theErrorStatus = rspfErrorCodes::RSPF_ERROR;
         return false;
      }
   }
   
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "\noffset to block: " << theNitfBlockOffset[blockNumber]
         << "\nblock size: " << theNitfBlockSize[blockNumber]
         << std::endl;
   }
   
   // Seek to the block.
   theFileStr.seekg(theNitfBlockOffset[blockNumber], ios::beg);
   
   // Read the block into memory.
   std::vector<rspf_uint8> compressedBuf(theNitfBlockSize[blockNumber]);
   if (!theFileStr.read((char*)&(compressedBuf.front()),
                        theNitfBlockSize[blockNumber]))
   {
      theFileStr.clear();
      rspfNotify(rspfNotifyLevel_FATAL)
         << "rspfNitfTileSource::uncompressJpegBlock Read Error!"
         << "\nReturning error..." << endl;
      theErrorStatus = rspfErrorCodes::RSPF_ERROR;
      return false;
   }
   
   if (m_isJpeg12Bit)
   {
#if defined(JPEG_DUAL_MODE_8_12)
      return rspfNitfTileSource_12::uncompressJpeg12Block(x,y,theCacheTile, 
       getCurrentImageHeader(), theCacheSize, compressedBuf, theReadBlockSizeInBytes, 
       theNumberOfOutputBands);
#endif  
   }

   //---
   // Most of comments below from jpeg-6b "example.c" file.
   //---
   
   /* This struct contains the JPEG decompression parameters and pointers
    * to working space (which is allocated as needed by the JPEG library).
    */
   jpeg_decompress_struct cinfo;
   
   /* We use our private extension JPEG error handler.
    * Note that this struct must live as long as the main JPEG parameter
    * struct, to avoid dangling-pointer problems.
    */
   rspfJpegErrorMgr jerr;
   
   /* Step 1: allocate and initialize JPEG decompression object */
   
   /* We set up the normal JPEG error routines, then override error_exit. */
   cinfo.err = jpeg_std_error(&jerr.pub);
 
   jerr.pub.error_exit = rspfJpegErrorExit;

   /* Establish the setjmp return context for my_error_exit to use. */
   if (setjmp(jerr.setjmp_buffer))
   {
      /* If we get here, the JPEG code has signaled an error.
       * We need to clean up the JPEG object, close the input file, and return.
       */
     jpeg_destroy_decompress(&cinfo);
     return false;
   }

   /* Now we can initialize the JPEG decompression object. */
   jpeg_CreateDecompress(&cinfo, JPEG_LIB_VERSION, sizeof(cinfo));
   
   //---
   // Step 2: specify data source.  In this case we will uncompress from
   // memory so we will use "rspfJpegMemorySrc" in place of " jpeg_stdio_src".
   //---
   rspfJpegMemorySrc (&cinfo,
                       &(compressedBuf.front()),
                       static_cast<size_t>(theReadBlockSizeInBytes));

   /* Step 3: read file parameters with jpeg_read_header() */
   jpeg_read_header(&cinfo, TRUE);
  
   // Check for Quantization tables.
   if (cinfo.quant_tbl_ptrs[0] == NULL)
   {
      // This will load table specified in COMRAT field.
      if (loadJpegQuantizationTables(cinfo) == false)
      {
        jpeg_destroy_decompress(&cinfo);
        return false;
      }
   }

   // Check for huffman tables.
   if (cinfo.ac_huff_tbl_ptrs[0] == NULL)
   {
      // This will load default huffman tables into .
      if (loadJpegHuffmanTables(cinfo) == false)
      {
        jpeg_destroy_decompress(&cinfo);
        return false;
      }
   }

   /* Step 4: set parameters for decompression */
   
   /* In this example, we don't need to change any of the defaults set by
    * jpeg_read_header(), so we do nothing here.
    */

   /* Step 5: Start decompressor */
   jpeg_start_decompress(&cinfo);

#if 0 /* Please leave for debug. (drb) */
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "jpeg cinfo.output_width:  " << cinfo.output_width
         << "\njpeg cinfo.output_height: " << cinfo.output_height
         << "\n";
   }
#endif
   
   const rspf_uint32 SAMPLES = cinfo.output_width;

   //---
   // Note: Some nitf will be tagged with a given number of lines but the last
   // jpeg block may go beyond that to a complete block.  So it you clamp to
   // last line of the nitf you will get a libjpeg error:
   // 
   // "Application transferred too few scanlines"
   //
   // So here we will always read the full jpeg block even if it is beyond the
   // last line of the nitf.
   //---
   const rspf_uint32 LINES_TO_READ =
      min(static_cast<rspf_uint32>(theCacheSize.y), cinfo.output_height);

   /* JSAMPLEs per row in output buffer */
   const rspf_uint32 ROW_STRIDE = SAMPLES * cinfo.output_components;

   if ( (SAMPLES < theCacheTile->getWidth() ) ||
        (LINES_TO_READ < theCacheTile->getHeight()) )
   {
      theCacheTile->makeBlank();
   }

   if ( (SAMPLES > theCacheTile->getWidth()) ||
        (LINES_TO_READ > theCacheTile->getHeight()) )
   {
     // Error...
     jpeg_finish_decompress(&cinfo);
     jpeg_destroy_decompress(&cinfo);
     return false;
   }

   // Get pointers to the cache tile buffers.
   std::vector<rspf_uint8*> destinationBuffer(theNumberOfInputBands);
   for (rspf_uint32 band = 0; band < theNumberOfInputBands; ++band)
   {
     destinationBuffer[band] = theCacheTile->getUcharBuf(band);
   }

   std::vector<rspf_uint8> lineBuffer(ROW_STRIDE);
   JSAMPROW jbuf[1];
   jbuf[0] = (JSAMPROW) &(lineBuffer.front());

   while (cinfo.output_scanline < LINES_TO_READ)
   {
     // Read a line from the jpeg file.
     jpeg_read_scanlines(&cinfo, jbuf, 1);

     //---
     // Copy the line which if band interleaved by pixel the the band
     // separate buffers.
     //
     // Note:
     // Not sure if IMODE of 'B' is interleaved the same as image with
     // IMODE of 'P'.
     //
     // This works with all samples with IMODE of B and P but I only have
     // one band 'B' and three band 'P'.  So if we ever get a three band
     // 'B' it could be wrong here. (drb - 20090615)
     //---
     rspf_uint32 index = 0;
     for (rspf_uint32 sample = 0; sample < SAMPLES; ++sample)         
     {
       for (rspf_uint32 band = 0; band < theNumberOfInputBands; ++band)
       {
         destinationBuffer[band][sample] = lineBuffer[index];
         ++index;
       }
     }

     for (rspf_uint32 band = 0; band < theNumberOfInputBands; ++band)
     {
       destinationBuffer[band] += theCacheSize.x;         
     }
   }

   // clean up...

   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   return true;
}

//---
// Default JPEG quantization tables
// Values from: MIL-STD-188-198, APPENDIX A
//---
bool rspfNitfTileSource::loadJpegQuantizationTables(
   jpeg_decompress_struct& cinfo) const
{
   //---
   // Check to see if table is present.  We will only look at the first table
   // in the array of arrays.
   // 
   // NOTE:  There are four tables in the array "cinfo.quant_tbl_ptrs".  It
   // looks like the standard is to use the first table. (not sure though)
   //---
   if (cinfo.quant_tbl_ptrs[0] != NULL)
   {
      return false;
   }

   // Get the COMRAT (compression rate code) from the header:
   const rspfNitfImageHeader* hdr = getCurrentImageHeader();
   if (!hdr)
   {
      return false;
   }
   
   rspfString comrat = hdr->getCompressionRateCode();
   rspf_uint32 tableIndex = 0;
   if (comrat.size() >= 4)
   {
      // COMRAT string like: "00.2" = use table 2. (between 1 and 5).
      rspfString s;
      s.push_back(comrat[static_cast<std::string::size_type>(3)]);
      rspf_int32 comTbl = s.toInt32();
      if ( (comTbl > 0) && (comTbl < 6) )
      {
         tableIndex = comTbl-1;
      }
      else
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfNitfTileSource::loadJpegQuantizationTables WARNING\n"
            << "\nNo quantization tables specified!"
            << endl;
         return false;  
      }
   }

   cinfo.quant_tbl_ptrs[0] = jpeg_alloc_quant_table((j_common_ptr) &cinfo);
 
   JQUANT_TBL* quant_ptr = cinfo.quant_tbl_ptrs[0]; // quant_ptr is JQUANT_TBL*

   for (rspf_int32 i = 0; i < 64; ++i)
   {
      /* Qtable[] is desired quantization table, in natural array order */
      quant_ptr->quantval[i] = QTABLE_ARRAY[tableIndex][i];
   }
   return true;
}

//---
// Default JPEG Huffman tables
// Values from: MIL-STD-188-198, APPENDIX B
//---
bool rspfNitfTileSource::loadJpegHuffmanTables(
   jpeg_decompress_struct& cinfo) const
{
   if ( (cinfo.ac_huff_tbl_ptrs[0] != NULL) &&
        (cinfo.dc_huff_tbl_ptrs[0] != NULL) )
   {
      return false;
   }

   cinfo.ac_huff_tbl_ptrs[0] = jpeg_alloc_huff_table((j_common_ptr)&cinfo);
   cinfo.dc_huff_tbl_ptrs[0] = jpeg_alloc_huff_table((j_common_ptr)&cinfo);

   rspf_int32 i;
   JHUFF_TBL* huff_ptr;
   
   // Copy the ac tables.
   huff_ptr = cinfo.ac_huff_tbl_ptrs[0]; /* huff_ptr is JHUFF_TBL* */     
   for (i = 0; i < 16; ++i) 
   {
      // huff_ptr->bits is array of 17 bits[0] is unused; hence, the i+1
      huff_ptr->bits[i+1] = AC_BITS[i]; 
   }
   
   for (i = 0; i < 256; ++i)
   {
      huff_ptr->huffval[i] = AC_HUFFVAL[i];
   }
   
   // Copy the dc tables.
   huff_ptr = cinfo.dc_huff_tbl_ptrs[0]; /* huff_ptr is JHUFF_TBL* */
   for (i = 0; i < 16; ++i)
   {
      // huff_ptr->bits is array of 17 bits[0] is unused; hence, the i+1
      huff_ptr->bits[i+1] = DC_BITS[i];
   }
   
   for (i = 0; i < 256; i++)
   {
      /* symbols[] is the list of Huffman symbols, in code-length order */
      huff_ptr->huffval[i] = DC_HUFFVAL[i];
   }
   return true;
}

// Protected to disallow use...
rspfNitfTileSource::rspfNitfTileSource(const rspfNitfTileSource& /* obj */)
{
}

// Protected to disallow use...
rspfNitfTileSource& rspfNitfTileSource::operator=(
   const rspfNitfTileSource& /* rhs */)
{
   return *this;
}
