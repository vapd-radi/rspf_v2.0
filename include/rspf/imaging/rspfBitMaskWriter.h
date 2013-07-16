//*************************************************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Oscar Kramer
//
//*************************************************************************************************
//  $Id: rspfBitMaskWriter.h 2644 2011-05-26 15:20:11Z oscar.kramer $

#ifndef rspfBitMaskWriter_HEADER
#define rspfBitMaskWriter_HEADER

#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfOutputSource.h>
#include <rspf/imaging/rspfPixelFlipper.h>
#include <vector>

class rspfFilename;
class rspfKeywordlist;
class rspfImageHandler;
class rspfBitMaskTileSource;

//*************************************************************************************************
//! 
//! Class for computing a mask from an input image source and writing the mask file to disk.
//!
//! This class takes care of all details associated with computing, writing and reading the custom,
//! bit-compressed RSPF bit mask file format. 
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
//! The mask is computed here using the specified values for bogus pixels. If a pixel in the source
//! image is in the range of bogus values, a mask of 0 is assigned to that pixel, otherwise 1. 
//!
//*************************************************************************************************
class RSPF_DLL rspfBitMaskWriter : public rspfOutputSource
{
public:
   static const char* BM_STARTING_RLEVEL_KW;
   static const char* MASK_FILE_MAGIC_NUMBER;

   //! Default constructor typically used when reading a mask from disk.
   rspfBitMaskWriter();

   ~rspfBitMaskWriter();

   //! Sets the NULL pixel value to consider when computing mask:
   void setBogusPixel(double pixel_value);

   //! Sets the range of pixels (inclusive) to be regarded as NULL pixels when computing mask
   void setBogusPixelRange(double min, double max);

   //! Sets the starting resolution level for the mask.
   void setStartingResLevel(rspf_uint32 res_level) { m_startingResLevel = res_level; }

   //! Given a source's tile, derives the alpha mask and saves it in buffer for later writing to 
   //! disk.
   void generateMask(rspfRefPtr<rspfImageData> tile, rspf_uint32 rLevel);

   //! For imagery that already has overviews built, but with artifact edge pixels (such as JP2-
   //! compressed data), it is preferred to build the mask overviews directly from the R0 mask.
   //! This method will build the remaining number of R-levels from the last defined mask buffer
   //! such that the total number of R-levels (counting R0) are represented. Returns TRUE if successful.
   bool buildOverviews(rspf_uint32 total_num_rlevels);

   //! Computes and writes the mask file according to the specification in the KWL.
   //! Returns TRUE if successful.
   virtual bool loadState(const rspfKeywordlist& spec, const char* prefix=0);

   virtual bool isOpen() const;
   virtual bool open();
   virtual void close();

   virtual bool canConnectMyInputTo(rspf_int32 myInputIndex, 
                                    const rspfConnectableObject* object) const;
   virtual rspf_int32 connectMyInputTo (rspfConnectableObject *inputObject, 
                                         bool makeOutputConnection=true, 
                                         bool createEventFlag=true);

protected:
   friend class rspfBitMaskTileSource;

   //! Initializes the flipper (used for identifying pixels for masking) to default values
   void initializeFlipper();

   //! Deletes allocated buffers and resets all values to defaults.
   void reset();

   //! Since overviews may not yet exist when the mask is being written, we must compute the
   //! size of the source image based on the original R0 image size.
   rspfIpt computeImageSize(rspf_uint32 rlevel, rspfImageData* tile) const;

   rspfRefPtr<rspfPixelFlipper>  m_flipper;
   vector<rspf_uint8*>            m_buffers;
   vector<rspfIpt>                m_bufferSizes;
   rspf_uint32                    m_startingResLevel;
   rspfIpt                        m_imageSize; //!< Size of full res source image
};

#endif

