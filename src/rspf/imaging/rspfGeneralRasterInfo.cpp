//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// Contains class definition for rspfGeneralRasterInfo
//*******************************************************************
// $Id: rspfGeneralRasterInfo.cpp 21745 2012-09-16 15:21:53Z dburken $

#include <rspf/imaging/rspfGeneralRasterInfo.h>
#include <rspf/base/rspfInterleaveTypeLut.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfScalarTypeLut.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageMetaData.h>
#include <rspf/support_data/rspfEnviHeader.h>
#include <rspf/support_data/rspfFgdcXmlDoc.h>

#include <iostream>
#include <iomanip>

static const rspfKeyword
NUMBER_LINES("number_lines",
             "Number of lines in the raster image.");

static const rspfKeyword
NUMBER_SAMPLES("number_samples",
               "Number of samples in the raster image.");

static const rspfKeyword
VALID_START_LINE("valid_start_line",
                 "First valid line of raster image(zero based).");

static const rspfKeyword
VALID_STOP_LINE("valid_stop_line",
                "Last valid line of raster image(zero based).");

static const rspfKeyword
VALID_START_SAMPLE("valid_start_sample",
                   "First valid sample of raster image(zero based).");

static const rspfKeyword
VALID_STOP_SAMPLE("valid_stop_sample",
                  "Last valid sample of raster image(zero based).");

static const rspfKeyword
SUB_IMAGE_OFFSET_LINE(
   "sub_image_offset_line",
   "Pixel line offset of sub-image in the full-image pixel space.");

static const rspfKeyword
SUB_IMAGE_OFFSET_SAMP(
   "sub_image_offset_samp",
   "Pixel sample offset of sub-image in the full-image pixel space."); 

static const rspfKeyword
HEADER_SIZE("header_size",
            "Header size in bytes.");

static const rspfKeyword
SET_NULLS("set_fill_to_nulls_mode",
          "0 = do nothing to pixels,\n1 = all zeroes to min values,\
\n2 = zeroes to null on edges only.");

static const rspfKeyword
PIXELS_TO_CHOP("pixels_to_chop",
               "Ammount of pixels to chop from edge.");

static const rspfInterleaveTypeLut INTERLEAVE_TYPE_LUT;

static rspfTrace traceDebug("rspfGeneralRasterInfo:debug");

rspfGeneralRasterInfo::rspfGeneralRasterInfo()
   :
   theMetaData(),
   theImageFileList(),
   theInterleaveType(RSPF_BIL),
   theRawImageRect(),
   theValidImageRect(),
   theImageRect(),
   theSubImageOffset(0,0),
   theHeaderSize(0),
   theSetNullsMode(NONE),
   thePixelsToChop(0),
   theImageDataByteOrder(RSPF_LITTLE_ENDIAN)
{
   theRawImageRect.makeNan();
   theValidImageRect.makeNan();
   theImageRect.makeNan();
}

rspfGeneralRasterInfo::rspfGeneralRasterInfo(const std::vector<rspfFilename>& imageFileList,
                                               rspfScalarType     pixelType,
                                               rspfInterleaveType il_type,
                                               rspf_int32         numberOfBands,
                                               rspf_int32         lines,
                                               rspf_int32         samples,
                                               rspf_int32         headerSize,
                                               rspfFillMode       nullsMode,
                                               rspf_int32         pixelsToChop)
   :
   theMetaData(pixelType, numberOfBands),
   theImageFileList(imageFileList),
   theInterleaveType(il_type),
   theRawImageRect(0,0,0,0),
   theValidImageRect(0,0,0,0),
   theImageRect(0,0,0,0),
   theSubImageOffset(0,0),
   theHeaderSize(headerSize),
   theSetNullsMode(nullsMode),
   thePixelsToChop(pixelsToChop),
   theImageDataByteOrder(RSPF_LITTLE_ENDIAN)
{
   theRawImageRect.set_lry(lines - 1);
   theRawImageRect.set_lrx(samples - 1);
   theValidImageRect = theRawImageRect;
   theImageRect = theRawImageRect;
}

rspfGeneralRasterInfo::rspfGeneralRasterInfo(const rspfKeywordlist& kwl,
                     const char* prefix)
   :
   theImageFileList(),
   theInterleaveType(RSPF_BIL),
   theRawImageRect(0,0,0,0),
   theValidImageRect(0,0,0,0),
   theImageRect(0,0,0,0),
   theSubImageOffset(0,0),
   theHeaderSize(0),
   theSetNullsMode(NONE),
   thePixelsToChop(0),
   theImageDataByteOrder(RSPF_LITTLE_ENDIAN)
{
   theRawImageRect.makeNan();
   theValidImageRect.makeNan();
   theImageRect.makeNan();
   
   loadState(kwl, prefix);
}

rspfGeneralRasterInfo::rspfGeneralRasterInfo( const rspfGeneralRasterInfo& obj )
   :
   theMetaData           ( obj.theMetaData ),
   theImageFileList      ( obj.theImageFileList ),
   theInterleaveType     ( obj.theInterleaveType ),
   theRawImageRect       ( obj.theRawImageRect ),
   theValidImageRect     ( obj.theValidImageRect ),
   theImageRect          ( obj.theImageRect ),
   theSubImageOffset     ( obj.theSubImageOffset ),
   theHeaderSize         ( obj.theHeaderSize ),
   theSetNullsMode       ( obj.theSetNullsMode ),
   thePixelsToChop       ( obj.thePixelsToChop ),
   theImageDataByteOrder ( obj.theImageDataByteOrder )
{
}

const rspfGeneralRasterInfo& rspfGeneralRasterInfo::operator=(
   const rspfGeneralRasterInfo& rhs )
{
   if ( this != &rhs )
   {
      theMetaData           = rhs.theMetaData;
      theImageFileList      = rhs.theImageFileList;
      theInterleaveType     = rhs.theInterleaveType;
      theRawImageRect       = rhs.theRawImageRect;
      theValidImageRect     = rhs.theValidImageRect;
      theImageRect          = rhs.theImageRect;
      theSubImageOffset     = rhs.theSubImageOffset;
      theHeaderSize         = rhs.theHeaderSize;
      theSetNullsMode       = rhs.theSetNullsMode;
      thePixelsToChop       = rhs.thePixelsToChop;
      theImageDataByteOrder = rhs.theImageDataByteOrder;
   }
   return *this;
}

rspfGeneralRasterInfo::~rspfGeneralRasterInfo()
{
}

void rspfGeneralRasterInfo::clear()
{
   theMetaData.clear();
   theImageFileList.clear();
   theInterleaveType = RSPF_BIL;
   theRawImageRect.makeNan();
   theValidImageRect.makeNan();
   theImageRect.makeNan();
   theSubImageOffset.x = 0;
   theSubImageOffset.y = 0;
   theHeaderSize = 0;
   theSetNullsMode = NONE;
   thePixelsToChop = 0;
   theImageDataByteOrder = RSPF_LITTLE_ENDIAN;
}

const rspfIrect& rspfGeneralRasterInfo::imageRect() const
{
   return theImageRect;
}

const rspfIrect& rspfGeneralRasterInfo::validImageRect() const
{
   return theValidImageRect;
}

const rspfIrect& rspfGeneralRasterInfo::rawImageRect() const
{
   return theRawImageRect;
}

const rspfIpt& rspfGeneralRasterInfo::subImageOffset() const
{
   return theSubImageOffset;
}

rspf_int32 rspfGeneralRasterInfo::headerSize() const
{
   return theHeaderSize;
}

rspf_uint32 rspfGeneralRasterInfo::fillToNullsMode() const
{
   return theSetNullsMode;
}
  
std::ostream& rspfGeneralRasterInfo::print(std::ostream& out) const
{
   //---
   // This will print in a keyword format that can be read by the constructor.
   // that takes a keyword list.
   //---
   rspfKeywordlist kwl;
   saveState( kwl, 0 );
   out << kwl << std::endl;
   return out;
}

void rspfGeneralRasterInfo::setFillToNullsMode(rspf_uint32 mode)
{
   static const char MODULE[] = "rspfGeneralRasterInfo::setFillToNullMode";
   if(mode < 3)
   {
      theSetNullsMode = (rspfFillMode)mode;
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << MODULE << " ERROR:"
         << "\nmode out of bounds(0 - 2):  " << mode << std::endl
         << "\nmode has not been changed." << std::endl;
   }
}

rspf_int32 rspfGeneralRasterInfo::pixelsToChop() const
{
   return thePixelsToChop;
} 

rspfInterleaveType rspfGeneralRasterInfo::interleaveType() const
{
   return theInterleaveType;
}

const std::vector<rspfFilename>& rspfGeneralRasterInfo::getImageFileList() const
{
   return theImageFileList;
}

void rspfGeneralRasterInfo::setImageFileList(const std::vector<rspfFilename>& list)
{
   theImageFileList = list;
}

void rspfGeneralRasterInfo::setImageFile(const rspfFilename& file)
{
   theImageFileList.clear();
   theImageFileList.push_back( file );
}   

void rspfGeneralRasterInfo::setImageDataByteOrder(rspfByteOrder byteOrder)
{
   theImageDataByteOrder = byteOrder;
}

void rspfGeneralRasterInfo::setHeaderSize(rspf_int32 headerSize)
{
   theHeaderSize = headerSize;
}

void rspfGeneralRasterInfo::setInterleaveType(rspfInterleaveType il_type)
{
   theInterleaveType = il_type;
}

void rspfGeneralRasterInfo::setImageRect(const rspfIrect& imageRect)
{
   theImageRect = imageRect;
}

void rspfGeneralRasterInfo::setValidImageRect(const rspfIrect &imageRect)
{
   theValidImageRect = imageRect;
}

void rspfGeneralRasterInfo::setRawImageRect(const rspfIrect &imageRect)
{
   theRawImageRect = imageRect;
}

void rspfGeneralRasterInfo::setSubImageOffset(const rspfIpt& d)
{
   theSubImageOffset = d;
}

rspfByteOrder rspfGeneralRasterInfo::getImageDataByteOrder() const
{
   return theImageDataByteOrder;
}  

bool rspfGeneralRasterInfo::saveState(rspfKeywordlist& kwl,
                                       const char* prefix) const
{
   for (rspf_uint32 i=0; i<theImageFileList.size(); ++i)
   {
      rspfString kw = rspfKeywordNames::FILENAME_KW;
      kw += rspfString::toString(i+1);
      kwl.add(prefix, theImageFileList[i].c_str());
   }

   theMetaData.saveState(kwl, prefix);

   kwl.add(prefix,
           NUMBER_LINES,
           rspfString::toString( rawLines() ), true);
   kwl.add(prefix,
           NUMBER_SAMPLES,
           rspfString::toString( rawSamples() ),
           true);
   kwl.add(prefix,
           HEADER_SIZE,
           rspfString::toString(theHeaderSize),
           true);
   kwl.add(prefix,
           SUB_IMAGE_OFFSET_LINE,
           theSubImageOffset.line,
           true);
   kwl.add(prefix,
           SUB_IMAGE_OFFSET_SAMP,
           theSubImageOffset.samp,
           true);
   kwl.add(prefix,
           VALID_START_LINE,
           theValidImageRect.ul().y,
           true);
   kwl.add(prefix,
           VALID_STOP_LINE,
           theValidImageRect.lr().y,
           true);
   kwl.add(prefix,
           VALID_START_SAMPLE,
           theValidImageRect.ul().x,
           true);
   kwl.add(prefix,
           VALID_STOP_SAMPLE,
           theValidImageRect.ur().x,
           true);
   kwl.add(prefix,
           rspfKeywordNames::INTERLEAVE_TYPE_KW,
           INTERLEAVE_TYPE_LUT.getEntryString(theInterleaveType),
           true);

   kwl.add(prefix,
           PIXELS_TO_CHOP,
           thePixelsToChop,
           true);

   kwl.add(prefix,
           SET_NULLS,
           theSetNullsMode,
           true);


   if (bytesPerPixel() > 1)
   {
      kwl.add(prefix,
              rspfKeywordNames::BYTE_ORDER_KW,
              (theImageDataByteOrder == RSPF_LITTLE_ENDIAN ? "little_endian" :
               "big_endian"),
              true);
   }

   return true;
}

bool rspfGeneralRasterInfo::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   static const char MODULE[] = "rspfGeneralRasterInfo::loadState";
   if ( traceDebug() )
   {
      CLOG << "DEBUG: entered..."
           << "\nprefix:  " << (prefix ? prefix : "")
           << "\nInput keyword list:\n"
           << kwl
           << std::endl;
   }   

   bool result = false;

   //---
   // Look for required and option keyword.  Break from loop if required
   // keyword is not found.
   //---
   while( 1 )
   {
      // Check for errors in the rspfKeywordlist.
      if(kwl.getErrorStatus() == rspfErrorCodes::RSPF_ERROR)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << MODULE << " ERROR:\n"
            << "Detected an error in the keywordlist:  " << kwl
            << std::endl;
         break;
      }

      std::string key;
      rspfString value; // Use for keyword list lookups.
      rspf_int32 lines = 0;
      rspf_int32 samples = 0;

      // Lines (required):
      key = NUMBER_LINES;
      value.string() = kwl.findKey( key );  // Required to have this.
      if ( value.size() )
      {
         lines = value.toInt32();
         if ( !lines )
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << " ERROR:\n"
                  << "Required number of lines is 0!" << std::endl;
            }
            break;
         } 
      }
      else
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << " ERROR:\n"
               << "Required keyword not found:  " << key << std::endl;
         }
         break;
      }

      // Samples (required):
      key = NUMBER_SAMPLES;
      value.string() = kwl.findKey( key );  // Required to have this.
      if ( value.size() )
      {
         samples = value.toInt32();
         if ( !samples )
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << " ERROR:\n"
                  << "Required number of samples is 0!" << std::endl;
            }
            break;
         }          
      }
      else
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << " ERROR:\n"
               << "Required keyword not found:  " << key << std::endl;
         }
         break;
      }
      
      // Bands rspfImageMetaData::loadState checks for required bands:
      if(!theMetaData.loadState(kwl, prefix))
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << " Error loading meta data!\n" << std::endl;
         }
         break;
      }

      // If we get here assign the rectangles:
      theRawImageRect   = rspfIrect( 0, 0, samples - 1, lines - 1 );
      theValidImageRect = theRawImageRect;
      theImageRect      = theRawImageRect;

      int tmp = INTERLEAVE_TYPE_LUT.getEntryNumber(kwl);
      if (tmp == rspfLookUpTable::NOT_FOUND)
      {
         theInterleaveType = RSPF_BIL;
      }
      else
      {
         theInterleaveType = static_cast<rspfInterleaveType>(tmp);
      }
      
      // Get the image files.
      if (theInterleaveType != RSPF_BSQ_MULTI_FILE)
      {
         // Look for "filename" first, then deprecated "image_file".
         key = rspfKeywordNames::FILENAME_KW;
         value.string() = kwl.findKey( key );
         if ( value.empty() )
         {
            // deprecated keyword...
            key = rspfKeywordNames::IMAGE_FILE_KW;
            value.string() = kwl.findKey( key );
         }
         
         if ( value.size() )
         {
            //---
            // omd (rspf metadata) files can have just the base filename, e.g. image.ras,
            // in which case open will fail if not in the image dir.  So only put it in
            // the list if it doesn't exits.
            //---
            rspfFilename f = value;
            if ( f.exists() )
            {
               theImageFileList.clear();
               theImageFileList.push_back(rspfFilename(value));
            }
         }

         if ( theImageFileList.empty() )
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "ERROR:\n"
                  << "Required keyword not found:  "
                  << rspfKeywordNames::FILENAME_KW << std::endl;
            }
            break;
         }
      }
      else
      {
         // multiple file names.
         rspf_int32 count = 0;
         
         // look for image file key word with no number.
         // Required to have this.
         key = rspfKeywordNames::FILENAME_KW;
         value.string() = kwl.findKey( key );
         if ( value.empty() )
         {
            // deprecated keyword...
            key = rspfKeywordNames::IMAGE_FILE_KW;
            value.string() = kwl.findKey( key );
         }
         
         if ( value.size() )
         {
            theImageFileList.push_back(rspfFilename(value));
            ++count;
         }
         
         rspf_int32 i = 0;
         while ( (count < numberOfBands()) && (i < 1000) )
         {
            key = rspfKeywordNames::FILENAME_KW;
            key += rspfString::toString(i).string();
            value.string() = kwl.findKey( key );
            if ( value.empty() )
            {
               // Lookup for deprecated keyword.
               key = rspfKeywordNames::IMAGE_FILE_KW;
               key += rspfString::toString(i).string();
               value.string() = kwl.findKey( key );
            }
            
            if ( value.size() )
            {
               theImageFileList.push_back(rspfFilename(value));
               ++count;
            }
            ++i;
         }
         
         if (count != numberOfBands())  // Error, count should equal bands!
         {
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << " ERROR:\n"
                  << "Required keyword not found:  "
                  << rspfKeywordNames::FILENAME_KW
                  << "\nInterleave type is multi file; however,"
                  << " not enough were pick up!"  << std::endl;
            }
            break;
         }
      }

      key = VALID_START_LINE;
      value.string() = kwl.findKey( key ); // Default is zero.
      if ( value.size() )
      {
         theValidImageRect.set_uly( value.toInt32() );
      }

      key = VALID_STOP_LINE;
      value.string() = kwl.findKey( key ); // Default is last line.
      if ( value.size() )
      {
         theValidImageRect.set_lry( value.toInt32() );
      }
      
      if (theValidImageRect.lr().y < theValidImageRect.ul().y)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << " ERROR:"
            << "\nValid stop line < start line."
            << "\nValid start line:  " << theValidImageRect.ul().y
            << "\nValid stop line:   " << theValidImageRect.lr().y
            << "\nError status has been set.  Returning." << std::endl;
         break;
      }

      key = VALID_START_SAMPLE;
      value.string() = kwl.findKey( key ); // Default is zero.
      if ( value.size() )
      {
         theValidImageRect.set_ulx( value.toInt32() );
      }

      key = VALID_STOP_SAMPLE;
      value.string() = kwl.findKey( key ); // Default is last sample.
      if ( value.size() ) 
      {
         theValidImageRect.set_lrx( value.toInt32() );
      }
      
      if (theValidImageRect.lr().x < theValidImageRect.ul().x)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << " ERROR:"
            << "\nValid stop samp < start samp."
            << "\nValid start samp:  " << theValidImageRect.ul().x
            << "\nValid stop samp:   " << theValidImageRect.lr().x
            << "\nError status has been set.  Returning." << std::endl;
         break;
      }
      
      theImageRect.set_lry(theValidImageRect.lr().y -
                           theValidImageRect.ul().y);
      theImageRect.set_lrx(theValidImageRect.lr().x -
                           theValidImageRect.ul().x);
      
      key = SUB_IMAGE_OFFSET_LINE;
      value.string() = kwl.findKey( key ); // Default is zero.
      if ( value.size() )
      {
         theSubImageOffset.line = value.toInt32();
      }

      key = SUB_IMAGE_OFFSET_SAMP;
      value.string() = kwl.findKey( key ); // Default is zero.
      if ( value.size() )
      {
         theSubImageOffset.samp = atoi(value);
      }

      key = HEADER_SIZE;
      value.string() = kwl.findKey( key ); // Default is zero.
      if ( value.size() )
      {
         theHeaderSize = value.toInt32();
      }

      key = SET_NULLS;
      value.string() = kwl.findKey( key ); // Default is 2.
      if ( value.size() )
      {
         int tmp;
         tmp = atoi(value);
         if ((tmp < 3) && (tmp > -1))
         {
            theSetNullsMode = (rspfFillMode)tmp;
         }
         else
         {
            theSetNullsMode = ZEROES_TO_NULL_EDGES_ONLY;  // 2
            rspfNotify(rspfNotifyLevel_WARN)
               << " WARNING:"
               << "\nset_fill_to_nulls_mode value out of range."
               << "\nDefaulted to 2" << std::endl;
         }
      }

      key = PIXELS_TO_CHOP;
      value.string() = kwl.findKey( key ); // Default is zero.
      if ( value.size() )
      {
         thePixelsToChop = value.toInt32();
      }
      
      if (bytesPerPixel() > 1)
      {
         // get the byte order of the data.
         key = rspfKeywordNames::BYTE_ORDER_KW;
         value.string() = kwl.findKey( key );
         if ( value.size() )
         {
            rspfString s(value);
            if (s.trim() != "")  // Check for empty string.
            {
               s.downcase();
               if (s.contains("big"))
               {
                  theImageDataByteOrder = RSPF_BIG_ENDIAN;
               }
               else if(s.contains("little"))
               {
                  theImageDataByteOrder = RSPF_LITTLE_ENDIAN;
               }
            }
         }
      }

      // End of while forever loop.
      result = true;
      break;

   } // Matches: while (1)

   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE  << " Exit status: " << (result?"true":"false") << std::endl;
   }      
   
   return result;
   
} // End: bool rspfGeneralRasterInfo::loadState

bool rspfGeneralRasterInfo::open( const rspfFilename& imageFile )
{
   static const char MODULE[] = "rspfGeneralRasterInfo::open";
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " entered..." << "\nimageFile: " << imageFile << std::endl;
   }

   bool result = false;

   // Wipe any previous state slick.
   clear();

   rspfFilename copyFile = imageFile;
   if ( !imageFile.exists() )
   {
      copyFile = imageFile.expand();
   }

   // Look for the headrer of omd file as they are written out by img2rr.
   rspfFilename hdr = copyFile;
   hdr.setExtension("hdr"); // image.hdr
   if ( !hdr.exists() )
   {
      hdr = imageFile;
      hdr.string() += ".hdr"; // image.ras.hdr
      if ( ! hdr.exists() )
      {
         hdr = imageFile;
         hdr.setExtension("xml"); // image.xml
      }
   }

   if ( hdr.exists() )
   {
      if ( traceDebug() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "header file: " << hdr << std::endl;
      }
      
      rspfString ext = hdr.ext().downcase();
      
      if ( ext == "hdr" )
      {
         if ( rspfEnviHeader::isEnviHeader( hdr ) )
         {
            result = initializeFromEnviHdr( hdr );
         }
         else
         {
            result = initializeFromHdr( imageFile, hdr );
         }

         if ( !result )
         {
            // Could be an rspf meta data file:
            rspfKeywordlist kwl( hdr );
            result = loadState( kwl, 0 );
         }
      }
      else if ( ext == "xml" )
      {
         result = initializeFromXml( imageFile, hdr );
      }
   }
   
   //---
   // Set the file name.  Needed for rspfGeneralRasterTileSource::open.
   // Note set here above loadState call to stop loadState from returning
   // false if no image file found.
   //---
   if ( theImageFileList.empty() )
   {
      setImageFile( imageFile );
   }
   
   rspfFilename omd = imageFile;
   omd.setExtension("omd"); // image.omd
   if ( !omd.exists() )
   {
      omd.setExtension("kwl"); // image.kwl
   }

   if ( omd.exists() )
   {
      if ( traceDebug() )
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "omd file: " << omd << std::endl;
      }

      rspfKeywordlist kwl( omd );

      if ( result && theMetaData.getNumberOfBands() )
      {
         //---
         // Just update the band info in case it has min/max values from
         // a compute min/max scan.
         //---
         theMetaData.updateMetaData( kwl, std::string("") );
      }
      else
      {
         // We're not initialized yet so do a loadState:
         result = loadState( kwl, 0 );
      }
   }
   
   if ( traceDebug() )
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE  << " Exit status: " << (result?"true":"false") << std::endl;
   }
   
   return result;
}

bool rspfGeneralRasterInfo::initializeFromHdr( const rspfFilename& imageFile,
                                                const rspfFilename& headerFile )
{
   bool result = false;

   rspfKeywordlist kwl;
   char delimeter = ' ';
   kwl.change_delimiter(delimeter);

   if ( kwl.addFile(headerFile) )
   {
      kwl.downcaseKeywords();
      rspfString value;
   
      while( 1 ) 
      {
         //---
         // Go through the data members in order.
         // If a required item is not found break from loop.
         //--
         theMetaData.clear();

         // scalar ( default ) - adjusted below :
         theMetaData.setScalarType( RSPF_UINT8 );

         // Image file name:
         theImageFileList.clear();
         theImageFileList.push_back( imageFile );
         
         // interleave ( not required - default=BIL)
         theInterleaveType = RSPF_BIL;
         value.string() = kwl.findKey( std::string("layout") );
         if ( value.size() )
         {
            rspfInterleaveTypeLut lut;
            rspf_int32 intrlv = lut.getEntryNumber( value.string().c_str(), true );
            if ( intrlv != rspfLookUpTable::NOT_FOUND )
            {
               theInterleaveType = static_cast<rspfInterleaveType>(intrlv);
            }
         }

         // bands ( required ):
         rspf_uint32 bands = 0;
         value.string() = kwl.findKey( std::string("nbands") );
         if ( value.size() )
         {
            bands = value.toUInt32();
         }
         if ( !bands )
         {
            break;
         }
         theMetaData.setNumberOfBands( bands );

         // lines ( required ):
         rspf_int32 lines = 0;
         value.string() = kwl.findKey( std::string("nrows") );
         if ( value.size() )
         {
            lines = value.toInt32();
         }
         if ( !lines )
         {
            break;
         }
         
         // samples ( required ):
         rspf_int32 samples = 0;
         value.string() = kwl.findKey( std::string("ncols") );
         if ( value.size() )
         {
            samples = value.toInt32();
         }
         if ( !samples )
         {
            break;
         }

         // nodata or null value ( not required )
         value.string() = kwl.findKey( std::string("nodata") );
         if ( value.empty() )
         {
            value.string() = kwl.findKey( std::string("nodata_value") );
         }
         if ( value.size() )
         {
            rspf_float64 nullValue = value.toUInt32();
            for ( rspf_uint32 band = 0; band < theMetaData.getNumberOfBands(); ++band )
            {
               theMetaData.setNullPix( band, nullValue );
            }
            theMetaData.setNullValuesValid(true);
         }

         // Set the rectangles:
         theRawImageRect   = rspfIrect( 0, 0, samples - 1, lines - 1 );
         theValidImageRect = theRawImageRect;
         theImageRect      = theRawImageRect;         

         // sample start ( not required ):
         theSubImageOffset.x = 0;

         // line start ( not required ):
         theSubImageOffset.y = 0;

         // header offset ( not required ):
         theHeaderSize = 0;

         // null mode:
         theSetNullsMode = rspfGeneralRasterInfo::NONE;

         // pixels to chop:
         thePixelsToChop = 0; 

         // Byte order, ( not required - defaulted to system if not found.
         theImageDataByteOrder = rspf::byteOrder();
         value.string() = kwl.findKey( std::string("byteorder") );
         if ( value.size() )
         {
            rspf_uint32 i = value.toUInt32();
            if ( i == 0 )
            {
               theImageDataByteOrder = RSPF_LITTLE_ENDIAN;
            }
            else
            {
               theImageDataByteOrder = RSPF_BIG_ENDIAN;
            }
         }

         // Pixel type used for scalar below:
         std::string pixelType = "N"; // not defined
         value.string() = kwl.findKey( std::string("pixeltype") );
         if ( value.size() )
         {
            pixelType = value.string();
         }
         
         rspf_int32 nbits = -1;
         value.string() = kwl.findKey( std::string("nbits") );
         if ( value.size() )
         {
            nbits = value.toInt32();
         }
         else
         {
            nbits = getBitsPerPixel( imageFile );
         }

         switch( nbits )
         {
            case 8:
            {
               theMetaData.setScalarType( RSPF_UINT8 );
               break;
            }
            case 16:
            {
               if (pixelType == "S")
               {
                  theMetaData.setScalarType( RSPF_SINT16 );
               }
               else
               {
                  theMetaData.setScalarType( RSPF_UINT16 );
               }
               break;
            }
            case 32:
            {
               if( pixelType == "S")
               {
                  theMetaData.setScalarType( RSPF_SINT32 );
               }
               else if( pixelType == "F")
               {
                  theMetaData.setScalarType( RSPF_FLOAT32 );
               }
               else
               {
                  theMetaData.setScalarType( RSPF_UINT32 );
               }
               break;
            }
            default:
            {
               if( (nbits < 8) && (nbits >= 1 ) )
               {
                  theMetaData.setScalarType( RSPF_UINT8 );
               }
               break;
            }
         }

         result = true;
         break; // Trailing break to get out.
      }
   }

   return result;
   
} // End: rspfGeneralRasterInfo::initializeFromHdr


bool rspfGeneralRasterInfo::initializeFromEnviHdr( const rspfFilename& headerFile )
{
   bool result = false;
   rspfEnviHeader hdr;
   if( hdr.open( headerFile ) )
   {
      result = initializeFromEnviHdr( hdr );
   }
   return result;
}

bool rspfGeneralRasterInfo::initializeFromEnviHdr( const rspfEnviHeader& enviHdr )
{
   bool result = false;

   while( 1 ) 
   {
      //---
      // Go through the data members in order.
      // If a required item is not found break from loop.
      //--
      theMetaData.clear();
      
      // scalar ( required ) :
      if( enviHdr.getOssimScalarType() != RSPF_SCALAR_UNKNOWN )
      {
         theMetaData.setScalarType( enviHdr.getOssimScalarType() );
      }
      else
      {
         break;
      }
      
      theImageFileList.clear();
      
      // interleave ( required ):
      theInterleaveType = enviHdr.getOssimInterleaveType();
      if ( theInterleaveType == RSPF_INTERLEAVE_UNKNOWN )
      {
         break;
      }
      
      // bands ( required ):
      if ( !enviHdr.getBands() )
      {
         break;
      }
      theMetaData.setNumberOfBands( enviHdr.getBands() );
      
      // lines ( required ):
      rspf_uint32 lines = enviHdr.getLines();
      if ( !lines )
      {
         break;
      }
      
      // samples ( required ):
      rspf_uint32 samples = enviHdr.getSamples();
      if ( !samples )
      {
         break;
      }
      
      // Set the rectangles:
      theRawImageRect   = rspfIrect( 0, 0, samples - 1, lines - 1 );
      theValidImageRect = theRawImageRect;
      theImageRect      = theRawImageRect;         

      // sample start ( not required ):
      theSubImageOffset.x = enviHdr.getXStart();
      
      // line start ( not required ):
      theSubImageOffset.y = enviHdr.getYStart();
      
      // header offset ( not required ):
      theHeaderSize = enviHdr.getHeaderOffset();
      
      // null mode:
      theSetNullsMode = rspfGeneralRasterInfo::NONE;
      
      // pixels to chop:
      thePixelsToChop = 0; 
      
      // Byte order, this will be system if not found.
      theImageDataByteOrder = enviHdr.getByteOrder();
      
      result = true;
      break; // Trailing break to get out.
   }
   return result;
   
} // End: rspfGeneralRasterInfo::initializeFromEnviHdr( const rspfEnviHeader& )

bool rspfGeneralRasterInfo::initializeFromXml( const rspfFilename& imageFile,
                                                const rspfFilename& headerFile )
{
   bool result = false;

   rspfFgdcXmlDoc file;
   if (file.open( headerFile ))
   {
      while( 1 ) 
      {
         //---
         // Go through the data members in order.
         // If a required item is not found break from loop.
         //--
         theMetaData.clear();

         // scalar ( default ) - adjusted below :
         theMetaData.setScalarType( RSPF_UINT8 );

         // Image file name:
         theImageFileList.clear();
         theImageFileList.push_back( imageFile );

         // interleave ( defaulted ):
         theInterleaveType = RSPF_BIL;

         // bands ( required ):
         if ( !file.getNumberOfBands() )
         {
            break;
         }
         theMetaData.setNumberOfBands( file.getNumberOfBands() );
         
         rspfIpt size;
         if ( file.getImageSize(size) ) // Lines, samples not image file size.
         {
            // lines, samples ( required ):
            if ( !size.x || !size.y )
            {
               break;
            }
         }
         else
         {
            break;
         }

         // Set the rectangles:
         theRawImageRect   = rspfIrect( 0, 0, size.x - 1, size.y - 1 );
         theValidImageRect = theRawImageRect;
         theImageRect      = theRawImageRect;         

         // sample start ( not required ):
         theSubImageOffset.x = 0;

         // line start ( not required ):
         theSubImageOffset.y = 0;

         // header offset ( not required ):
         theHeaderSize = 0;

         // null mode:
         theSetNullsMode = rspfGeneralRasterInfo::NONE;

         // pixels to chop:
         thePixelsToChop = 0; 

         // Byte order *** need this ***, defaulting to system for now:
         theImageDataByteOrder = rspf::byteOrder();

         // Adjust scalar if needed, note defaulted to 8 bit above:
         rspfString eainfo;
         file.getPath("/metadata/eainfo/detailed/enttyp/enttypd", eainfo);
         rspf_int32 numBits = 0;
         rspf_int64 fileSize = imageFile.fileSize(); // Image file size.
         rspf_int32 numBytes = fileSize / size.x / size.y / numberOfBands();
         if( numBytes > 0 && numBytes != 3 )
         {
            numBits = numBytes*8;
         }
         if( numBits == 16 )
         {
            theMetaData.setScalarType( RSPF_UINT16 );
         }
         else if( numBits == 32 )
         {
            if(eainfo.contains("float"))
            {
               theMetaData.setScalarType( RSPF_FLOAT32 );
            }
            else
            {
               theMetaData.setScalarType( RSPF_UINT32 );
            }
         }

         result = true;
         break; // Trailing break to get out.
      }
   }

   return result;
   
} // End: rspfGeneralRasterInfo::initializeFromXml

rspf_int32 rspfGeneralRasterInfo::getBitsPerPixel( const rspfFilename& imageFile ) const
{
   // Note currently does not consider header size.
   rspf_int32 result = 0;
   
   rspf_int64 fileSize = imageFile.size();
   rspfIpt rectSize = theRawImageRect.size();
   if ( fileSize && rectSize.x && rectSize.y && numberOfBands() )
   {
      result = ( fileSize / rectSize.x / rectSize.y / numberOfBands() ) * 8;
   }
   return result;
}

