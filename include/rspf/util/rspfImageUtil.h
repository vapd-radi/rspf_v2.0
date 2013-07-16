//----------------------------------------------------------------------------
// File: rspfImageUtil.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: rspfImageUtil
//
// See class descriptions below for more.
// 
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfImageUtil_HEADER
#define rspfImageUtil_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfPropertyInterface.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfOverviewBuilderBase.h>
#include <OpenThreads/Mutex>
#include <ostream>

class rspfArgumentParser;
class rspfFileWalker;
class rspfGpt;

/**
 * @brief rspfImageUtil class.
 *
 * Utility class for processing image recursively.  This is for doing things like:
 * 
 * building overview, histograms, compute min/max, extract vertices.
 */
class RSPF_DLL rspfImageUtil : public rspfReferenced
{
public:

   /** default constructor */
   rspfImageUtil();

   /** virtual destructor */
   virtual ~rspfImageUtil();

   /**
    * @brief Adds application arguments to the argument parser.
    * @param ap Parser to add to.
    */
   void addArguments(rspfArgumentParser& ap);

   /**
    * @brief Initial method.
    *
    * Typically called from application prior to execute.  This parses
    * all options and put in keyword list m_kwl.
    * 
    * @param ap Arg parser to initialize from.
    *
    * @return true, indicating process should continue with execute.
    */
   bool initialize(rspfArgumentParser& ap);

   /**
    * @brief Execute method.
    *
    * This launches file walking mechanism.
    *
    * @return int, 0 = good, non-zero something happened.  Because this can
    * process multiple files a non-zero return may indicate just one file
    * did not complete, e.g. building overviews.
    * 
    * @note Throws rspfException on error.
    */
   rspf_int32 execute();

   /**
    * @brief ProcessFile method.
    *
    * This method is linked to the rspfFileWalker::walk method via a callback
    * mechanism.  It is called by the rspfFileWalk (caller).  In turn this
    * class (callee) calls rspfFileWalker::setRecurseFlag and
    * rspfFileWalker::setAbortFlag to control the waking process.
    * 
    * @param file to process.
    */
   void processFile(const rspfFilename& file);

   /**
    * @brief Sets create overviews flag keyword CREATE_OVERVIEWS_KW used by
    * processFile method.
    *
    * @param flag If true overview will be created if image does not already
    * have the required or if the REBUILD_OVERVIEWS_KW is set.
    *
    * @note Number of required overviews is controlled by the rspf preferences
    * keyword overview_stop_dimension.
    */
   void setCreateOverviewsFlag( bool flag );

   /** @return true if CREATE_OVERVIEWS_KW is found and set to true. */
   bool createOverviews() const;
   
   /**
    * @brief Sets the rebuild overview flag keyword REBUILD_OVERVIEWS_KW used by
    * processFile method.
    *
    * @param flag If true forces a rebuild of overviews even if image has
    * required number of reduced resolution data sets.
    *
    * @note Number of required overviews is controlled by the rspf preferences
    * keyword overview_stop_dimension.
    */
   void setRebuildOverviewsFlag( bool flag );

   /** @return true if REBUILD_OVERVIEWS_KW is found and set to true. */
   bool rebuildOverviews() const;

   /**
    * @brief Sets the rebuild histogram flag keyword REBUILD_HISTOGRAM_KW used by
    * processFile method.
    *
    * @param flag If true forces a rebuild of histogram even if image has one already.
    */
   void setRebuildHistogramFlag( bool flag );

   /** @return true if REBUILD_HISTOGRAM_KW is found and set to true. */
   bool rebuildHistogram() const;

   /**
    * @brief Sets key OVERVIEW_TYPE_KW.
    *
    * Available types depens on plugins.  Know types:
    * rspf_tiff_box ( defualt )
    * rspf_tiff_nearest
    * rspf_kakadu_nitf_j2k ( kakadu plugin )
    * gdal_tiff_nearest	    ( gdal plugin )
    * gdal_tiff_average	    ( gdal plugin )
    * gdal_hfa_nearest      ( gdal plugin )	
    * gdal_hfa_average      ( gdal plugin )	
    * 
    * @param type One of the above.
    */
   void setOverviewType( const std::string& type );
   
   /**
    * @brief sets the overview stop dimension.
    *
    * The overview builder will decimate the image until both dimensions are
    * at or below this dimension.
    *
    * @param dimension
    *
    * @note Recommend a power of 2 value, i.e. 8, 16, 32 and so on.
    */
   void setOverviewStopDimension( rspf_uint32 dimension );
   void setOverviewStopDimension( const std::string& dimension );

   /**
    * @brief Sets the tile size.
    *
    * @param tileSize
    *
    * @note Must be a multiple of 16, i.e. 64, 128, 256 and so on.
    */
   void setTileSize( rspf_uint32 tileSize );

   /**
    * @return Overview stop dimension or 0 if OVERVIEW_STOP_DIM_KW is not
    * found.
    */
   rspf_uint32 getOverviewStopDimension() const;

   /**
    * @brief Sets create histogram flag keyword CREATE_HISTOGRAM_KW used by
    * processFile method.
    *
    * @param flag If true a full histogram will be created.
    */
   void setCreateHistogramFlag( bool flag );
   
   /** @return true if CREATE_HISTOGRAM_KW is found and set to true. */
   bool createHistogram() const;

   /**
    * @brief Sets create histogram flag keyword CREATE_HISTOGRAM_FAST_KW used by
    * processFile method.
    *
    * @param flag If true a histogram will be created in fast mode.
    */
   void setCreateHistogramFastFlag( bool flag );

   /** @return true if CREATE_HISTOGRAM_FAST_KW is found and set to true. */
   bool createHistogramFast() const;

   /**
    * @brief Sets create histogram "R0" flag keyword CREATE_HISTOGRAM_R0_KW used by
    * processFile method.
    *
    * @param flag If true a histogram will be created from R0.
    */
   void setCreateHistogramR0Flag( bool flag );

   /** @return true if CREATE_HISTOGRAM_R0_KW is found and set to true. */
   bool createHistogramR0() const;

   /** @return true if any of the histogram options are set. */
   bool hasHistogramOption() const;

   /** @return Histogram mode or RSPF_HISTO_MODE_UNKNOWN if not set. */
   rspfHistogramMode getHistogramMode() const;

   /**
    * @brief Sets scan for min/max flag keyword SCAN_MIN_MAX_KW used by
    * processFile method.
    *
    * @param flag If true a file will be scanned for min/max and a file.omd
    * will be written out.
    */
   void setScanForMinMax( bool flag );
   
   /** @return true if SCAN_MIN_MAX_KW is found and set to true. */
   bool scanForMinMax() const;

   /**
    * @brief Sets scan for min/max/null flag keyword SCAN_MIN_MAX_KW used by
    * processFile method.
    *
    * @param flag If true a file will be scanned for min/max/null and a file.omd
    * will be written out.
    */
   void setScanForMinMaxNull( bool flag );

   /** @return true if SCAN_MIN_MAX_NULL_KW is found and set to true. */
   bool scanForMinMaxNull() const;

   /**
    * @brief Sets the writer property for compression quality.
    *
    * @param quality For TIFF JPEG takes values from 1
    * to 100, where 100 is best.  For J2K plugin (if available),
    * numerically_lossless, visually_lossless, lossy.
    */
   void setCompressionQuality( const std::string& quality );

   /**
    * @brief Sets the compression type to use when building overviews.
    *  
    * @param compression_type Current supported types:
    * - deflate 
    * - jpeg
    * - lzw
    * - none
    * - packbits
    */
   void setCompressionType( const std::string& type );

   /**
    * @brief Sets the overview builder copy all flag.
    * @param flag
    */
   void setCopyAllFlag( bool flag );

   /**
    * @return true if COPY_ALL_FLAG_KW key is found and value is true; else,
    * false.
    */
   bool getCopyAllFlag() const;

   /**
    * @brief Sets the overview builder internal overviews flag.
    * @param flag
    */
   void setInternalOverviewsFlag( bool flag );
   
   /**
    * @return true if INTERNAL_OVERVIEWS_FLAG_KW key is found and value is true; else,
    * false.
    */
   bool getInternalOverviewsFlag() const;
   
   /**
    * @brief Sets the output directory.  Typically overviews and histograms
    * are placed parallel to image file.  This overrides.
    *  
    * @param directory
    */
   void setOutputDirectory( const std::string& directory );
 
   /**
    * @brief Sets the output file name flag OUTPUT_FILENAMES_KW.
    *
    * If set to true all files that we can successfully open will be output.
    *  
    * @param flag
    */
   void setOutputFileNamesFlag( bool flag );

   /**
    * @return true if OUTPUT_FILENAMES_KW key is found and value is true; else,
    * false.
    */
   bool getOutputFileNamesFlag() const;
 
   /**
    * @brief Set number of threads to use.
    *
    * This is only used in execute method if a directory is given to
    * application to walk.
    *
    * @param threads Defaults to 1 if THREADS_KW is not found.
    */
   void setNumberOfThreads( rspf_uint32 threads );
   void setNumberOfThreads( const std::string& threads );

private:

   void createOverview(rspfRefPtr<rspfImageHandler>& ih,
                       bool& consumedHistogramOptions,
                       bool& consumedCmmOptions);

   void createOverview(rspfRefPtr<rspfImageHandler>& ih,
                       rspfRefPtr<rspfOverviewBuilderBase>& ob,
                       rspf_uint32 entry,
                       bool useEntryIndex,
                       bool& consumedHistogramOptions);

   /** @return true if entry has required overviews. */
   bool hasRequiredOverview( rspfRefPtr<rspfImageHandler>& ih,
                             rspfRefPtr<rspfOverviewBuilderBase>& ob );

   void createHistogram(rspfRefPtr<rspfImageHandler>& ih);

   void createHistogram(rspfRefPtr<rspfImageHandler>& ih,
                       rspf_uint32 entry,
                       bool useEntryIndex);

   
   /** @brief Initializes arg parser and outputs usage. */
   void usage(rspfArgumentParser& ap);

   void outputOverviewWriterTypes() const;

   /**
    * @return true if file is a directory based image and the stager should go
    * on to next directory; false if stager should continue with directory.
    */
   bool isDirectoryBasedImage(const rspfImageHandler* ih) const;


   /**
    * @brief Initializes type from OVERVIEW_TYPE_KW or sets to default
    * rspf_tiff_box if not found.
    */
   void getOverviewType(std::string& type) const;

   /** @brief set reader or writer properties based on cast of pi. */
   void setProps(rspfPropertyInterface* pi) const;
   
   /**
    * @return Threads to use.  Defaults to 1 if THREADS_KW is not found.
    */
   rspf_uint32 getNumberOfThreads() const;

   /** @return the next writer prop index. */
   rspf_uint32 getNextWriterPropIndex() const;

   /** @return the next reader prop index. */   
   rspf_uint32 getNextReaderPropIndex() const;

   /**
    * @brief Adds option to m_kwl with mutex lock.
    * @param key
    * @param value
    */
   void addOption( const std::string& key, rspf_uint32 value );
   void addOption( const std::string& key, const std::string& value );

   /**
    * @brief Sets the m_errorStatus for return on execute.
    */
   void setErrorStatus( rspf_int32 status );

   
   /** Holds all options passed into intialize except writer props. */
   rspfRefPtr<rspfKeywordlist> m_kwl;

   rspfFileWalker*   m_fileWalker;
   OpenThreads::Mutex m_mutex;

   rspf_int32 m_errorStatus;
};

#endif /* #ifndef rspfImageUtil_HEADER */
