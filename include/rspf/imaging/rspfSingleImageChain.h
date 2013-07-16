//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Utility class declaration for a single image chain.
// 
//----------------------------------------------------------------------------
// $Id$
#ifndef rspfSingleImageChain_HEADER
#define rspfSingleImageChain_HEADER 1

#include <vector>

#include <rspf/base/rspfConstants.h> /* RSPF_DLL */
#include <rspf/imaging/rspfBandSelector.h>
#include <rspf/imaging/rspfCacheTileSource.h>
#include <rspf/imaging/rspfHistogramRemapper.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageRenderer.h>
#include <rspf/imaging/rspfScalarRemapper.h>


// Forward class declarations:
class rspfFilename;
class rspfSrcRecord;

/**
 * @class rspfSingleImageChain
 * 
 * @brief Single image chain class.
 *
 * Convenience class for a single image chain.
 * 
 * For code example see: rspf/src/test/rspf-single-image-chain-test.cpp  
 *
 * Just a clarification on "start of chain" versus "end of chain" in this
 * file.
 *
 * Given chain of:
 * 1) image handler
 * 2) band selector (optional)
 * 3) histogram remapper(optional)
 * 4) scalar remapper (optional)
 * 5) resampler cache
 * 6) resampler
 * 7) band selector (optional when going one band to three)
 * 8) chain cache
 *
 * The "image handle" is the "start of chain".
 * The "chain cache" is the "end of chain".
 */
class RSPF_DLL rspfSingleImageChain : public rspfImageChain
{
public:

   /** default constructor */
   rspfSingleImageChain();

   /** Constructor that takes flags.*/
   rspfSingleImageChain(bool addHistogramFlag,
                         bool addResamplerCacheFlag,
                         bool addChainCacheFlag,
                         bool remapToEightBitFlag,
                         bool threeBandFlag,
                         bool threeBandReverseFlag);
   
   /** virtual destructor */
   virtual ~rspfSingleImageChain();

   /**
    * @brief reset method
    * This deletes all links in the chain, zero's out all data members, and
    * sets all flags back to default.
    */
   void reset();
   
   /**
    * @brief open method that takes an image file.
    *
    * Opens file and creates a simple chain with rspfImageHandler.
    *
    * @param file File to open.

    * @param openOverview If true image handler will attempt to open overview.
    * Note that if you are planning on doing a rendered chain or want to go
    * between res levels you should set this to true.  default = true
    *
    * @return true on success, false on error.
    *
    * @note This will close previous chain if one was opened.
    */
   bool open(const rspfFilename& file, bool openOverview=true);

   /**
    * @brief open method that takes an rspfSrcRecord.
    *
    * Opens file and creates a simple chain with rspfImageHandler.
    *
    * @return true on success, false on error.
    *
    * @note This will close previous chain if one was opened.
    */
   bool open(const rspfSrcRecord& src);
   
   /** @return true if image handler is opened. */
   bool isOpen() const;

   /** @brief close method to delete the image handler. */
   void close();

   /** @return The filename of the image. */
   rspfFilename getFilename() const;

   /**
    * @brief Create a rendered image chain.
    *
    * Typical usage is to call this after "open" method returns true like:
    * if ( myChain->open(myFile) == true )
    * {
    *    myChain->createRenderedChain();
    *    code-goes-here();
    * }
    *
    * Typical chain is:
    * 
    * 1) image handler
    * 2) band selector (optional)
    * 3) histogram remapper(optional)
    * 4) scalar remapper (optional)
    * 5) resampler cache
    * 6) resampler
    * 7) band selector (optional when going one band to three)
    * 8) chain cache
    *
    * NOTES:
    * 1) Cache on left hand side of resampler is critical to speed if you
    *    have the rspfImageRender enabled.
    *    
    * 2) If doing a sequential write where tiles to the right of the
    *    resampler will not be revisited the chain cache could be
    *    disabled to save memory.
    */
   void createRenderedChain();

   /**
    * @brief Create a rendered image chain that takes an rspfSrcRecord.
    */
   void createRenderedChain(const rspfSrcRecord& src);

   /**
    * @brief Adds an image handler for file.
    * 
    * @param file File to open.
    *
    * @param openOverview If true image handler will attempt to open overview.
    * Note that if you are planning on doing a rendered chain or want to go
    * between res levels you should set this to true. default = true
    *
    * @return true on success, false on error.
    */
   bool addImageHandler(const rspfFilename& file,
                        bool openOverview=true);

   /**
    * @brief Adds an image handler from src record.
    *
    * This take an rspfSrcRecord which can contain a supplemental directory
    * to look for overviews.
    * 
    * @param rec Record to open.
    * @return true on success, false on error.
    */
   bool addImageHandler(const rspfSrcRecord& src);
   
   /** @brief Adds a band selector to the end of the chain. */
   void addBandSelector();

   /**
    * @brief Adds a band selector.
    *
    * This takes an rspfSrcRecord which can contain a band selection list.
    * 
    * @param src Record to initialize band selector from.
    * 
    */
   void addBandSelector(const rspfSrcRecord& src);

   /** @brief Adds histogram remapper to the chain. */
   void addHistogramRemapper();

   /**
    * @brief Adds a band selector.
    *
    * This takes an rspfSrcRecord which can contain a histogram
    * operation to be performed.
    * 
    * @param src Record to initialize band selector from.
    */
   void addHistogramRemapper(const rspfSrcRecord& src);

   /**
    * @brief Adds a new cache to the current end of the chain.
    * @return Pointer to cache.
    */
   rspfRefPtr<rspfCacheTileSource> addCache();

   /** @brief Adds a resampler to the end of the chain. */ 
   void addResampler();

   /**
    * @brief Adds scalar remapper either to the left of the resampler cache
    * or at the end of the chain if not present.
    */
   void addScalarRemapper();

   /**
    * @return rspfRefPtr containing the image handler.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfImageHandler> getImageHandler() const;

   /**
    * @return rspfRefPtr containing the image handler.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfImageHandler> getImageHandler();

   /**
    * @return rspfRefPtr containing  the band selector.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfBandSelector> getBandSelector() const;

   /**
    * @return rspfRefPtr containing  the band selector.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfBandSelector> getBandSelector();

   /**
    * @return rspfRefPtr containing the histogram remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfHistogramRemapper> getHistogramRemapper() const;

   /**
    * @return rspfRefPtr containing the histogram remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfHistogramRemapper> getHistogramRemapper();

   /**
    * @return  rspfRefPtr containing the resampler cache.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfCacheTileSource> getResamplerCache() const;

   /**
    * @return  rspfRefPtr containing the resampler cache.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfCacheTileSource> getResamplerCache();

   /**
    * @return rspfRefPtr containing the resampler.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfImageRenderer> getImageRenderer() const;

   /**
    * @return rspfRefPtr containing the resampler.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfImageRenderer> getImageRenderer();

   /**
    * @return rspfRefPtr containing the scalar remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfScalarRemapper> getScalarRemapper() const;

   /**
    * @return rspfRefPtr containing the scalar remapper.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfScalarRemapper> getScalarRemapper();

   /**
    * @return rspfRefPtr containing the chain cache.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfCacheTileSource> getChainCache() const;

   /**
    * @return rspfRefPtr containing the chain cache.
    * @note Can contain a null pointer so callers should validate.
    */
   rspfRefPtr<rspfCacheTileSource> getChainCache();

   /**
    * @brief If flag is true a histogram will be added to the chain at create time.
    * @param flag
    */
   void setAddHistogramFlag(bool flag);

   /**
    * @brief Gets the add histogram flag.
    * @return true or false.
    */
   bool getAddHistogramFlag() const;

   /**
    * @brief If flag is true a resampler cache will be added to the chain at create time.
    * This is a cache to the left of the resampler.
    * @param flag
    */
   void setAddResamplerCacheFlag(bool flag);

   /**
    * @brief Gets the add resampler cache flag.
    * @return true or false.
    */
   bool getAddResamplerCacheFlag() const;

   /**
    * @brief If flag is true a chain cache will be added to the chain at create time.
    * This is a cache at the end of the chain.
    * @param flag
    */
   void setAddChainCacheFlag(bool flag);

   /**
    * @brief Gets the add chain cache flag.
    * @return true or false.
    */
   bool getAddChainCacheFlag() const;

   /**
    * @brief Sets remap to eigth bit flag.
    * @param flag
    */
   void setRemapToEightBitFlag(bool flag);

   /**
    * @brief Get the remap to eight bit flag.
    * @return true or false.
    */
   bool getRemapToEightBitFlag() const;
   
   /**
    * @brief Sets the three band flag.
    *
    * If set will for a three band output.  So if one band it will duplicate
    * so that rgb = b1,b1,b1.
    * 
    * @param flag
    */
   void setThreeBandFlag(bool flag);

   /**
    * @brief Get the three band flag.
    * @return true or false.
    */
   bool getThreeBandFlag() const;
   
   /**
    * @brief Sets the three band reverse flag.
    *
    * @param flag
    */
   void setThreeBandReverseFlag(bool flag);

   /**
    * @brief Get the three band reverse flag.
    * @return true or false.
    */
   bool getThreeBandReverseFlag() const;

   /**
    * @brief Utility method to force 3 band output.
    *
    * Set band selector to a three band (rgb) output. If image has less than
    * three bands it will set to rgb = b1,b1,b1.  If image has three or
    * more bands the band selector will be see to rgb = b1, b2, b3.
    *
    * @note This will not work unless the image handler is initialized.
    */
   void setToThreeBands();
   
   /**
    * @brief Utility method to set to 3 bandsand reverse them.  This is
    * mainly used by NITF and Lndsat color data where the bands are in bgr
    * format and you want it in rgb combination.  If image has less than
    * three bands it will set to rgb = b1,b1,b1.  If image has three or
    * more bands the band selector will be see to rgb = b3, b2, b1.
    *
    * @note This will not work unless the image handler is initialized.
    */
   void setToThreeBandsReverse();

   /**
    * @brief method to set band selector.
    *
    * This will set the band selection to bandList.  If a band selector is
    * not in the chain yet it will be added.
    *
    * @param bandList The list of bands.
    */
   void setBandSelection(const std::vector<rspf_uint32>& bandList);

   /**
    * @brief Convenience method to return the scalar type of the image handler.
    * 
    * @return Scalar type of the image handler.
    *
    * This can return RSPF_SCALAR_UNKNOWN if the image handler has not been
    * set yet.  Also, this is NOT the same as calling getOutputScalarType
    * which could have a different scalar type than the image if the
    * m_remapToEightBitFlag has been set.
    */
   rspfScalarType getImageHandlerScalarType() const;

   /**
    * @brief Convenience method to open the histogram and apply a default
    * stretch.
    *
    * This will only work if the image is open, there is a histogram remapper
    * in the chain, and there was a histogram created.
    * 
    * Valid stretches (from rspfHistogramRemapper.h):
    *
    @verbatim
      rspfHistogramRemapper::LINEAR_ONE_PIECE
      rspfHistogramRemapper::LINEAR_1STD_FROM_MEAN
      rspfHistogramRemapper::LINEAR_2STD_FROM_MEAN
      rspfHistogramRemapper::LINEAR_3STD_FROM_MEAN
      rspfHistogramRemapper::LINEAR_AUTO_MIN_MAX
    @endverbatim
    *
    * @return true on success, false on error.
    */
   bool openHistogram( rspfHistogramRemapper::StretchMode mode );

private:

   /**  Pointers to links in chain. */
   rspfRefPtr<rspfImageHandler>      m_handler;
   rspfRefPtr<rspfBandSelector>      m_bandSelector;
   rspfRefPtr<rspfHistogramRemapper> m_histogramRemapper;
   rspfRefPtr<rspfCacheTileSource>   m_resamplerCache;
   rspfRefPtr<rspfImageRenderer>     m_resampler;
   rspfRefPtr<rspfScalarRemapper>    m_scalarRemapper;
   rspfRefPtr<rspfCacheTileSource>   m_chainCache;

   /** control flags */
   bool m_addHistogramFlag;
   bool m_addResamplerCacheFlag;
   bool m_addChainCacheFlag;
   bool m_remapToEightBitFlag;
   bool m_threeBandFlag;
   bool m_threeBandReverseFlag;

};

#endif /* #ifndef rspfSingleImageChain_HEADER */
