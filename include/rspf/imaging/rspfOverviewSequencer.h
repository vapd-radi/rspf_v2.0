//----------------------------------------------------------------------------
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
//
// Description: Class definition for sequencer for building overview files.
// 
//----------------------------------------------------------------------------
// $Id: rspfOverviewSequencer.h 22149 2013-02-11 21:36:10Z dburken $
#ifndef rspfOverviewSequencer_HEADER
#define rspfOverviewSequencer_HEADER 1

#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfMultiBandHistogram.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfFilterResampler.h>
#include <rspf/imaging/rspfBitMaskWriter.h>
#include <rspf/imaging/rspfMaskFilter.h>
#include <string>
#include <vector>

class rspfFilename;

/**
 * @class Sequencer for building overview files.
 */
class RSPF_DLL rspfOverviewSequencer
   : public rspfReferenced, public rspfErrorStatusInterface
   
{
public:

   /** default constructor */
   rspfOverviewSequencer();
   
   /**
    * @return The number of tiles within the area of interest.
    */
   rspf_uint32 getNumberOfTiles()const;

   /**
    * @return The number of tiles along the x or horizontal direction.
    */
   rspf_uint32 getNumberOfTilesHorizontal()const;

   /**
    * @return The number of tiles along the y or vertical direction.
    */
   rspf_uint32 getNumberOfTilesVertical()const;

   /**
    * @brief Gets the zero-based image rectangle for the output reduced
    * resolution data set (rrds).
    *
    * @param rect The rectangle to initialize.
    */
   virtual void getOutputImageRectangle(rspfIrect& rect) const; 

   /**
    * @brief Sets the input source or connection.
    * @param input Pointer to rspfImageHandler.
    * @note This object does not own the image handler.
    */
   void setImageHandler(rspfImageHandler* input);
   
   /**
    * @brief Enabled the generation of an alpha (bit) mask such that any full
    * or partial null pixels will be masked out. A mask file will be written to
    * the source image directory with the image file name and extension ".mask"
    */
   void setBitMaskObjects(rspfBitMaskWriter* mask_writer,
                          rspfMaskFilter* mask_filter);

   /**
    * @brief Sets the input source resolution to decimate from.
    * @param level The level to decimate.
    */
   void setSourceLevel(rspf_uint32 level);

   /**
    * @brief Gets the histogram accumulation mode.
    * @return mode UNKNOWN, NORMAL or FAST.
    */
   rspfHistogramMode getHistogramMode() const;

   /**
    * @brief Sets the histogram accumulation mode.
    * @param mode NONE, NORMAL or FAST.
    */
   void setHistogramMode(rspfHistogramMode mode);

   /**
    * @brief Write histogram method.
    */
   void writeHistogram();

   /**
    * @brief Write histogram method that takes a file name.
    * @param file File to write to.
    */
   void writeHistogram(const rspfFilename& file);

   /**
    * This must be called.  We can only initialize this
    * object completely if we know all connections
    * are valid.  Some other object drives this and so the
    * connection's initialize will be called after.  The job
    * of this connection is to set up the sequence.  It will
    * default to the bounding rect.  The area of interest can be
    * set to some other rectagle (use setAreaOfInterest).
    */
   virtual void initialize();

   /**
    * @brief Will set the internal pointers to the upperleft
    * tile number.  To go to the next tile in the sequence
    * just call getNextTile.
    */
   virtual void setToStartOfSequence();

   /**
    * Will allow you to get the next tile in the sequence.
    * Note the last tile returned will be an invalid
    * rspfRefPtr<rspfImageData>.  Callers should be able to do:
    * 
    * rspfRefPtr<rspfImageData> id = sequencer->getNextTile();
    * while (id.valid())
    * {
    *    doSomething;
    *    id = sequencer->getNextTile();
    * }
    * 
    */
   virtual rspfRefPtr<rspfImageData> getNextTile();

   /**
    * @brief This implementation does nothing.  Derived slave would use this
    * as a trigger to start resampling tiles.
    */
   virtual void slaveProcessTiles();

   /**
    * @return Always true for this implementation.
    */
   virtual bool isMaster()const;

   /** @return The tile size. */
   rspfIpt getTileSize() const;

   /**
    * @brief updated the tile size.  This will update the number of horizontal
    * and vertical tiles.
    * @param pt The tile size.  pt.x = width, pt.y = height
    * @param height The tile height.
    */
   void setTileSize(const rspfIpt& pt);

   /**
    * @brief Sets the resampling type.
    *
    * Supports BOX or NEAREST NEIGHBOR.
    * When indexed you should probably use nearest neighbor.
    * default = rspfFilterResampler::rspfFilterResampler_BOX
    *
    * @param resampleType The resampling method to use. 
    */ 
   void setResampleType(
      rspfFilterResampler::rspfFilterResamplerType resampleType);

   /**
    * @brief Turn on/off scan for min max flag.
    * This method assumes the null is known.
    * @param flag true turns scan on, false off. Default=off.
    */
   void setScanForMinMax(bool flag);
   
   /** @return scan for min max flag. */
   bool getScanForMinMax() const;
   
   /**
    * @brief Turn on/off scan for min, max, null flag.
    * Attempts to find null, min and max where null is the minimum value found,
    * min is the second most min and max is max.
    * @param flag true turns scan on, false off. Default=off.
    */
   void setScanForMinMaxNull(bool flag);

   /** @return scan for min max flag. */
   bool getScanForMinMaxNull() const;

   /**
    * @brief Writes an rspf metadata(omd) file with min, max, null values.
    *
    * Writes omd file to disk with min, max, null values.  If file existed
    * previously it will be ingested into keyword list prior to addition
    * of the min, max, nulls computed in this method.
    *
    * Note that prior to writing a sanity check is performed on the values as
    * a scan for null value might actually pick up the min if the image is
    * full.
    * 
    * @param file to write.
    */
   bool writeOmdFile(const std::string& file);   

protected:

   /** virtual destructor */
   virtual ~rspfOverviewSequencer();

   /**
    * @brief Gets the image rectangle for the input tile for
    * theCurrentTileNumber.
    * @param inputRect The rectangle to initialize.
    */
   void getInputTileRectangle(rspfIrect& inputRect) const;
   
   /**
    * @brief Gets the image rectangle for the output tile for
    * theCurrentTileNumber.
    * @param outputRect The rectangle to initialize.
    */
   void getOutputTileRectangle(rspfIrect& outputRect) const;

   /**
    * @brief Updates theNumberOfTilesHorizontal and theNumberOfTilesVertical.
    *
    * This required theAreaOfInterest and theTileSize to be set.
    */
   void updateTileDimensions();

   /**
    *  Resamples a patch of data.
    */
   void resampleTile(const rspfImageData* inputTile);

   template <class T> void resampleTile(const rspfImageData* inputTile,
                                        T dummy);

   /** @brief Clears out the arrays from a scan for min, max, nulls. */
   void clearMinMaxNullArrays();

   rspfRefPtr<rspfImageHandler>  m_imageHandler;
   rspfRefPtr<rspfBitMaskWriter> m_maskWriter;
   rspfRefPtr<rspfMaskFilter>    m_maskFilter;
   rspfRefPtr<rspfImageData>     m_tile;
   rspfIrect                      m_areaOfInterest;
   rspfIpt                        m_tileSize;
   rspf_uint32                    m_numberOfTilesHorizontal;
   rspf_uint32                    m_numberOfTilesVertical;
   rspf_uint32                    m_currentTileNumber;

   /** This is the resolution level to build overviews from. */
   rspf_uint32                   m_sourceResLevel;

   /** Dirty flag - if true, this object is not initialized. */
   bool                           m_dirtyFlag;

   /** TODO make this handle any decimation.  Right now hard coded to two. */
   rspf_int32                    m_decimationFactor;

   /** Currently only handles NEAREST_NEIGHBOR and BOX (default = BOX) */
   rspfFilterResampler::rspfFilterResamplerType m_resampleType;

   rspfRefPtr<rspfMultiBandHistogram> m_histogram;

   rspfHistogramMode m_histoMode;

   /**
    * Used to determine which tiles to accumulate a histogram from.  If set to
    * 1 every tile is accumulated, 2 every other tile, 3 every 3rd tile, and
    * so on.  Set in initialize method based on mode and image size.
    */
   rspf_uint32 m_histoTileIndex;

   /** Control flags for min, max, null scanning. */
   bool m_scanForMinMax;
   bool m_scanForMinMaxNull;
 
   /** Arrays o hold the min value for each band for scan min/max methods. */
   std::vector<rspf_float64> m_minValues; 
   std::vector<rspf_float64> m_maxValues; 
   std::vector<rspf_float64> m_nulValues;
};

#endif /* #ifndef rspfOverviewSequencer_HEADER */
