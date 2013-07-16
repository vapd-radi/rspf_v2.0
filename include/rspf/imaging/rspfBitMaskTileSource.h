//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
//*******************************************************************
//  $Id: rspfBitMaskTileSource.h 2644 2011-05-26 15:20:11Z oscar.kramer $

#ifndef rspfBitMaskTileSource_HEADER
#define rspfBitMaskTileSource_HEADER

#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfBitMaskWriter.h>

class rspfImageGeometry;

//*************************************************************************************************
//! This is the image handler for providing mask pixels to an rspfMaskFilter object.
//!
//! This class takes care of reading the custom, bit-compressed RSPF bit mask file format. 
//! 
//! When the developer needs to mask an image, the following needs to be done:
//!
//!   Creating the Mask:
//!   1. A mask file first needs to be created using rspfBitMaskWriter and a source image. The app
//!      "img2rr --create-mask" directs that program to use rspfBitMaskWriter to generate a mask  
//!      during overview generation. There is also create-bitmask app that will generate the mask.
//!   2. An RSPF ".mask" file should exist on disk after step 0. This is typically the same base 
//!      name as the source image file being masked.
//!   
//!   Using the Mask:
//!   1. An rspfBitMaskTileSource (derived from rspfImageHandler) needs to be instantiated and
//!      the mask file name used to open it. 
//!   3. An rspfMaskFilter (derived from rspfImageCombiner) is the actual filter that combines
//!      the source image handler and the rspfBitMaskTileSource object to provide the resultant 
//!      masked tiles.
//!      
//! The RSPF bit mask file contains a small header that consists of:
//!    magic number,
//!    starting res level,
//!    number of res levels
//!    num columns at starting res level,
//!    num rows at starting res level, 
//!    num columns at 2nd res level,
//!    num rows at 2nd res level, 
//!    etc., for all res levels
//!    <space> 
//!    <compressed binary mask data>
//!
//! The number of rows and columns represents the number of bytes in the mask, with each byte
//! containing 8 alpha mask bits representing 8 pixels in the original image.
//!
//! This class functions as an image handler, returning 255 for unmasked pixels, and 0 for masked 
//! pixels. 
//! 
//! For debug purposes, this code initializes the tile to all 128's so that the 
//! rspfMaskFilter can recognize uninitialized mask data.
//!
//*************************************************************************************************

class RSPF_DLL rspfBitMaskTileSource : public rspfImageHandler
{
public:

   rspfBitMaskTileSource();

   //! Return a tile of masks. These are "bit masks", i.e., either 0 (hide) or 1 (show).
   //! The pixels' remaining 7 bits are cleared and should not be used.
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tile_rect, rspf_uint32 rLevel=0);
   
   virtual bool         isValidRLevel     (rspf_uint32 res_level)   const;
   virtual rspf_uint32 getNumberOfLines  (rspf_uint32 res_level=0) const;
   virtual rspf_uint32 getNumberOfSamples(rspf_uint32 res_level=0) const;

   virtual rspf_uint32    getNumberOfInputBands()  const { return 1; }
   virtual rspf_uint32    getNumberOfOutputBands() const { return 1; }
   virtual rspfScalarType getOutputScalarType()    const { return RSPF_UINT8; }
   virtual rspf_uint32    getImageTileWidth()      const { return 0; }
   virtual rspf_uint32    getImageTileHeight()     const { return 0; }   

   virtual bool isOpen() const;
   virtual bool open();
   virtual void close();

   virtual double getNullPixelValue(rspf_uint32 /*band=0*/) const { return 0; }
   virtual double getMinPixelValue (rspf_uint32 /*band=0*/) const { return 0; }
   virtual double getMaxPixelValue (rspf_uint32 /*band=0*/) const { return 1.0; }

   //! There is no geometry associated with a mask file -- it is pixel-for-pixel mapped to
   //! an image file. The image file must have the valid geometry.
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   //! Overrides base class implementation.
   virtual rspf_uint32 getNumberOfDecimationLevels() const;

   //! For backward compatibility with older mask format ("Ming Mask"), provide for associating
   //! the mask with an image handler:
   void setAssociatedImageHandler(rspfImageHandler* handler);

   //! This class can be used during overview generation, in which case there will be a simultaneous
   //! mask writing. Instead of accessing a mask file (which may not yet exist), the mask buffer
   //! of the writer will be accessed directly instead of our own buffer.
   void setAssociatedMaskWriter(rspfBitMaskWriter* maskWriter);

protected:
   virtual ~rspfBitMaskTileSource();
   
   //! Returns true if this handler recognizes the file name extension as an RSPF raster mask file.
   bool isSupportedExtension();

   virtual void establishDecimationFactors();

   rspfRefPtr<rspfImageData>  m_tile;
   vector<rspf_uint8*>         m_buffers;
   vector<rspfIpt>             m_bufferSizes;
   
   //! For backward compatibility with older mask format ("Ming Mask"), provide for associating
   //! the mask with an image handler:
   rspfRefPtr<rspfImageHandler>  m_handler;

   //! This class can be used during overview generation, in which case there will be a simultaneous
   //! mask writing. Instead of accessing a mask file (which may not yet exist), the mask buffer
   //! of the writer will be accessed directly instead of our own buffer.
   rspfRefPtr<rspfBitMaskWriter>  m_maskWriter;

   TYPE_DATA
};

#endif

