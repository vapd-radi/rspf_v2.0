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
//
// Contains class declaration for rspfGeneralRasterInfo
//*******************************************************************
//  $Id: rspfGeneralRasterInfo.h 22197 2013-03-12 02:00:55Z dburken $

#ifndef rspfGeneralRasterInfo_HEADER
#define rspfGeneralRasterInfo_HEADER 1

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageMetaData.h>
#include <vector>

//---
// Forward class declarations.
//---
class rspfEnviHeader;
class rspfKeywordlist;

/**
 * @class rspfGeneralRasterInfo
 *
 */
class RSPFDLLEXPORT rspfGeneralRasterInfo : public rspfObject
{
public:

   enum rspfFillMode
   {
      NONE,
      ZEROES_TO_MIN,
      ZEROES_TO_NULL_EDGES_ONLY
   };

   enum
   {
      UNKNOWN = -1
   };

   /* @brief default constructor */
   rspfGeneralRasterInfo();
   
   rspfGeneralRasterInfo(const std::vector<rspfFilename>& imageFileList,
                          rspfScalarType      pixelType,
                          rspfInterleaveType  il_type,
                          rspf_int32          numberOfBands,
                          rspf_int32          lines,
                          rspf_int32          samples,
                          rspf_int32          headerSize,
                          rspfFillMode        nullsMode,
                          rspf_int32          pixelsToChop);
   
   rspfGeneralRasterInfo(const rspfKeywordlist& kwl, const char* prefix=0);

   /* @brief copy constructor */
   rspfGeneralRasterInfo( const rspfGeneralRasterInfo& obj );

   /** @brief Assignment operator=. */
   const rspfGeneralRasterInfo& operator=( const rspfGeneralRasterInfo& rhs );
   
   virtual ~rspfGeneralRasterInfo ();

   /**
    *  Zero based rectangle of the valid image.
    */
   const rspfIrect& imageRect() const;

   /**
    *  Returns the rectangle of where the image is within the raster file.
    *  In most cases this will be the same as the rawImageRect. (zero based)
    */
   const rspfIrect& validImageRect() const;

   /**
    *  Zero based rectangle of the raw image.
    */
   const rspfIrect& rawImageRect() const;

   /**
    * Returns the offset of this image's origin to a full-image's origin, in
    * the case where this image is a sub-image of a larger full-image.
    */
   const rspfIpt& subImageOffset() const;

   /**
    *  Returns the size of header in bytes.
    */
   rspf_int32 headerSize() const;

   /**
    *  Returns the fill mode.  This is used to flip values on ingest.
    *  Valid Mode Enumerations:
    *  - 0  Do nothing to the pixels on ingest.
    *  - 1  Flip all zeroes to the minimum pixel value.
    *  - 2  Flip zeroes within the image line to minimum pixel value.
    *       This is used by edge walking code.
    */
   rspf_uint32 fillToNullsMode() const;

   /**
    *  Sets theSetNullsMode.
    */
   void setFillToNullsMode(rspf_uint32 mode);

   /**
    * @brief Number of bands. Inlined below.
    * @return Number of bands from theMetaData class member cast to an int32.
    */
   inline rspf_int32 numberOfBands() const;
   
   /**
    * @brief Bytes per pixel.  Inlined below.
    * @return Bytes per pixel from theMetaData class member cast to an int32.
    */   
   inline rspf_int32 bytesPerPixel() const;
   
   /**
    *  Returns the number of pixels from the edge of a line
    *  to set to the fill value.  This can be used to eliminate
    *  resampling error on the edge of an image and still keep
    *  the image demensions relative to the geometry file.
    */
   rspf_int32 pixelsToChop() const;
   
   /**
    *  Enumerated in InterleaveTypeLUT. Should be either bip,
    *  bil or bsq.  Defaulted to bip.
    */
   rspfInterleaveType interleaveType() const;

   /**
    *  Returns the number of lines within "theValidImageRect".
    */
   inline rspf_int32 validLines() const;

   /**
    *  Returns the number of lines within "theRawImageRect".
    */
   inline rspf_int32 rawLines() const;

   /**
    *  Returns the number of bytes in a raw line.
    */
   inline rspf_int32 bytesPerRawLine() const;

   /**
    *  Returns the number of bytes in a valid line.
    */
   inline rspf_int32 bytesPerValidLine() const;

   /**
    *  Returns the number of samples within "theValidImageRect".
    */
   inline rspf_int32 validSamples() const;

   /**
    *  Returns the number of samples within "theRawImageRect".
    */
   inline rspf_int32 rawSamples() const;

   /**
    *  Returns the offset in bytes to the first valid sample in the image.
    */
   inline std::streamoff offsetToFirstValidSample() const;
   
   const std::vector<rspfFilename>& getImageFileList() const;

   void setImageFileList(const std::vector<rspfFilename>& list);

   /**
    * @brief Sets the image file list to file.  Performs a clear, prior to
    * setting.
    * @param file Image file.
    */
   void setImageFile(const rspfFilename& file);

   void setHeaderSize(rspf_int32 headerSize);
   
   void setInterleaveType(rspfInterleaveType il_type);

   virtual std::ostream& print(std::ostream& out) const;

   void setImageRect(const rspfIrect& imageRect);
   
   void setValidImageRect(const rspfIrect &imageRect);

   void setRawImageRect(const rspfIrect &imageRect);

   void setSubImageOffset(const rspfIpt& d);

   rspfByteOrder getImageDataByteOrder() const;

   void setImageDataByteOrder(rspfByteOrder byteOrder);
   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * @brief Takes image file and attempts to derive/find header file to
    * parse for general raster data, i.e. lines, samples, bands and so on.
    * @param image file.
    * @return true on success, false on error.
    */
   bool open( const rspfFilename& imageFile );

   /** @return Const reference to theMetaData class member. */
   inline const rspfImageMetaData& getImageMetaData() const;

   /** @return Reference to theMetaData class member. */
   inline rspfImageMetaData& getImageMetaData();

   /**
    * @brief Initialize from envi header file.
    * @param enviHeader
    * @return true on success, false on error.
    */  
   bool initializeFromEnviHdr( const rspfEnviHeader& enviHdr );

private:

   /**
    * @brief Initialize from header file.
    * @param headerFile
    * @return true on success, false on error.
    */
   bool initializeFromHdr( const rspfFilename& imageFile,
                           const rspfFilename& headerFile );

   /**
    * @brief Initialize from envi header file.
    * @param headerFile
    * @return true on success, false on error.
    */  
   bool initializeFromEnviHdr( const rspfFilename& headerFile );

   /**
    * @brief Initialize from omd file.
    * @param file
    * @return true on success, false on error.
    */  
   bool initializeFromXml( const rspfFilename& imageFile,
                           const rspfFilename& headerFile );

   /**
    * @brief Private method to get bits per pixel from file size,
    * lines, samples and bands.  Method created to avoid duplicate code
    * in "initizeFrom" methods.
    *
    * This assumes that "theRawImageRect" and "theNumberOfBands" has been
    * set.
    *
    * @param imageFile This image file.
    * @param Calculated bits per pixel.
    * @return bits per pixel or 0 on error.
    */
   rspf_int32 getBitsPerPixel( const rspfFilename& imageFile ) const;

   /** @brief Clears data members. */
   void clear();
   
   rspfImageMetaData          theMetaData;
   std::vector<rspfFilename>  theImageFileList;
   rspfInterleaveType         theInterleaveType;

   /**
    *  The zero based rectangle of the entire image not including any header.
    */
   rspfIrect theRawImageRect;

   /**
    *  The rectangle representing where the valid part of the image lies
    *  within "theRawImageRect".
    */
   rspfIrect theValidImageRect;

   /**
    *  The valid image rectangle normalized to be zero based.
    */
   rspfIrect theImageRect;

   /**
    *  The offset from the full-image origin to this image's origin
    */
   rspfIpt theSubImageOffset;

   /**
    *  Size of header in bytes.
    */
   rspf_int32 theHeaderSize;

   /**
    *  Mode of how to handle pixels on import.
    *   0 = Do nothing the pixels.
    *   1 = Set any zeroes to min value of radiometry.
    *   2 = Set zeroes on line edges only to nulls; any other zeroes are
    *       set to min value of radiometry.
    */
   rspfFillMode theSetNullsMode;

   /**
    *  Amount of pixels to chop for each line edge.
    */
   rspf_int32 thePixelsToChop;

   /**
    *  The byte order the image data is stored in.
    */
  rspfByteOrder theImageDataByteOrder;
};


//---
// Inlines:
// 
// These methods are used throughout raster read/write code; hence, inlines.
// Note that most of these are used in calculating std::streamoff which is
// a signed 64 bit integer on most machines; hence, the cast to an int from
// an unsigned int for convenience.
//---
inline const rspfImageMetaData& rspfGeneralRasterInfo::getImageMetaData() const
{
   return theMetaData;
}

inline rspfImageMetaData& rspfGeneralRasterInfo::getImageMetaData()
{
   return theMetaData;
}

inline rspf_int32 rspfGeneralRasterInfo::numberOfBands() const
{
   return static_cast<rspf_int32>( theMetaData.getNumberOfBands() );
}

inline rspf_int32 rspfGeneralRasterInfo::bytesPerPixel() const
{
   return static_cast<rspf_int32>( theMetaData.getBytesPerPixel() );
}

inline rspf_int32 rspfGeneralRasterInfo::validLines() const
{
   return static_cast<rspf_int32>(theValidImageRect.height());
}

inline rspf_int32 rspfGeneralRasterInfo::rawLines() const
{
   return static_cast<rspf_int32>(theRawImageRect.height());
}

inline rspf_int32 rspfGeneralRasterInfo::bytesPerRawLine() const
{
   if (interleaveType() == RSPF_BIP)
   {
      return rawSamples() * bytesPerPixel() * numberOfBands();
   }
   else
   {
      return rawSamples() * bytesPerPixel();
   }
}

inline rspf_int32 rspfGeneralRasterInfo::bytesPerValidLine() const
{
   if (interleaveType() == RSPF_BIP)
   {
      return validSamples() * bytesPerPixel() * numberOfBands();
   }
   else
   {
      return validSamples() * bytesPerPixel();
   }
}

inline rspf_int32 rspfGeneralRasterInfo::validSamples() const
{
   return static_cast<rspf_int32>(theValidImageRect.width());
}

inline rspf_int32 rspfGeneralRasterInfo::rawSamples() const
{
   return static_cast<rspf_int32>(theRawImageRect.width());
}

inline std::streamoff rspfGeneralRasterInfo::offsetToFirstValidSample() const
{
   if (interleaveType() == RSPF_BIP)
   {
      return headerSize() +
         ( bytesPerRawLine() * validImageRect().ul().y ) +
         ( validImageRect().ul().x * numberOfBands() * bytesPerPixel() );
   }
   else if (interleaveType() == RSPF_BIL)
   {
      return headerSize() +
         ( bytesPerRawLine() * validImageRect().ul().y * numberOfBands() ) +
         ( validImageRect().ul().x * bytesPerPixel() );
   }
   else // BSQ
   {
      return headerSize() +
         ( bytesPerRawLine() * validImageRect().ul().y ) +
         ( validImageRect().ul().x * bytesPerPixel() );
   }
}

#endif
