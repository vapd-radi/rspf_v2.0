//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Class declaration for ortho-image generator.
//
//*************************************************************************
// $Id: rspfOrthoIgen.h 20936 2012-05-18 14:45:00Z oscarkramer $

#ifndef rspfOrthoIgen_HEADER
#define rspfOrthoIgen_HEADER

#include <rspf/parallel/rspfIgen.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfConnectableContainer.h>
#include <rspf/base/rspfArgumentParser.h>
#include <rspf/base/rspfApplicationUsage.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageChain.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/support_data/rspfSrcRecord.h>
#include <map>

class rspfConnectableObject;
class rspfMapProjection;
class rspfImageSource;
class rspfImageHandler;
class rspfImageCombiner;

class RSPF_DLL rspfOrthoIgen : public rspfIgen
{
public:
   enum rspfOrthoIgenProjectionType
   {
      RSPF_UNKNOWN_PROJECTION = 0,
      RSPF_UTM_PROJECTION,
      RSPF_SRS_PROJECTION,
      RSPF_GEO_PROJECTION,
      RSPF_INPUT_PROJECTION,
      RSPF_EXTERNAL_PROJECTION
   };
   enum OriginType
   {
      RSPF_CENTER_ORIGIN     = 0,
      RSPF_UPPER_LEFT_ORIGIN = 1
   };
   typedef std::map<rspfString,rspfString, rspfStringLtstr> PropertyMap;
   
   rspfOrthoIgen();

   /**
    * Will add command line arguments to initialize this object from a command line application
    *
    * <pre>
    * Command Line Arguments:
    *
    * -t or --thumbnail  Takes an integer argument that is used to set both width
    *                    and height.
    *                    example: -t 1024 will produce an image that is 1024x1024
    * --meters           Takes a double precision argument that is used for meters per pixel
    *                    override.
    *                    example: --meters 5 will create a 5 meter product.
    * --slave-buffers    Takes an integer as an argument that specifies the number of slave tile buffers per
    *                    slave node.  The slave nodes are suppose to be asynch send.  It will
    *                    not wait for the send to finish.  It will got to the next buffer and check to
    *                    see if it's been sent and if not it will then wait for the buffer to be free.
    *                    example: --slave-buffers 10  will create 10 buffers per slave node.
    * --chain-template   Takes a file name that is a kewyord list that is used as a template for every
    *                    input file.
    *                    example:
    * 
    * @param argumentParser Argument parser object to add command line options to
    *
    */ 
   void addArguments(rspfArgumentParser& argumentParser);
   virtual void initialize(rspfArgumentParser& argumentParser);
   void addFiles(rspfArgumentParser& argumentParser,
                 bool withDecoding,
                 rspf_uint32 startIdx = 1);
   void clearFilenameList();

   //! Parses the .src file specified in the command line. These contain an alternate specification
   //! of input file and associated attributes as a KWL.
   void addSrcFile(const rspfFilename& fileName);

   bool execute();
   void setDefaultValues();

protected:
   //! Used to parse command line filename specs with piped switches (in lieu of *.src file).
   //! Returns TRUE if successful.
   bool parseFilename(const rspfString& file_spec, bool decodeEntry);

   //! Determines the UL corner tiepoint of the product projection as the overall UL corner of the
   //! mosaic.
   void establishMosaicTiePoint();

   rspfUnitType theDeltaPerPixelUnit;
   rspfDpt      theDeltaPerPixelOverride;
   rspfOrthoIgenProjectionType theProjectionType;
   rspfString   theProjectionName;
   rspfString   theCrsString;
   rspf_float64 theGeoScalingLatitude;
   rspfString   theCombinerType;
   rspfString   theResamplerType;
   rspfString   theWriterType;
   rspfFilename theTemplateView;
   rspfFilename theTilingTemplate;
   rspfFilename theTilingFilename;
   rspfFilename theChainTemplate;
   rspfFilename theCombinerTemplate;
   rspfFilename theAnnotationTemplate;
   rspfFilename theWriterTemplate;
   rspfFilename theSupplementaryDirectory;
   rspfString   theSlaveBuffers;
   OriginType    theCutOriginType;
   rspfDpt      theCutOrigin;
   rspfDpt      theCutDxDy;
   rspfUnitType theCutOriginUnit;
   rspfUnitType theCutDxDyUnit;
   rspf_float64 theLowPercentClip;
   rspf_float64 theHighPercentClip;
   rspf_int32   theStdDevClip;
   bool          theUseAutoMinMaxFlag;
   bool          theClipToValidRectFlag;
   PropertyMap   theReaderProperties;
   PropertyMap   theWriterProperties;   
   rspfFilename theTargetHistoFileName;
   std::vector<rspfSrcRecord> theSrcRecords;
   rspfFilename theProductFilename;
   rspfRefPtr<rspfProjection> theReferenceProj;
   rspfFilename theMaskShpFile;
   bool          theCacheExcludedFlag;
   rspfString   thePixelReplacementMode;
   rspf_float64 theClampPixelMin;
   rspf_float64 theClampPixelMax;
   rspf_float64 theClipPixelMin;
   rspf_float64 theClipPixelMax;
   rspfString   theOutputRadiometry;
   rspfPixelType thePixelAlignment;

   /**
   * @brief Sets up the igen keyword list for the process.
   *
   * This throws rspfException on error.
   */
   void setupIgenChain();

   bool setupTiling();
   void setupCutter();

   /** 
   * Checks for the presence of a raster mask file alongside the image, and inserts the mask 
   * filter in the chain if mask file exists. Returns with current_src pointing to masker object 
   */
   rspfImageSource* setupRasterMask(rspfImageChain* single_image_chain,
                        const rspfSrcRecord& src_record);

   /**
   * Insert a partial-pixel flipper to remap null-valued pixels to min according to info in the
   * source record and/or command line. Returns with current_src pointing to flipper object.
   */
   rspfImageSource* setupPixelFlipper(rspfImageChain* single_image_chain, 
                          const rspfSrcRecord& src_record);

   /**
    * @brief Set up the writer for the process.
    * This throws rspfException on error.
    */
   void setupWriter();

   /**
    * @brief Set up the output projection for the process.
    * This throws rspfException on error.
    */
   void setupProjection();

   /**
    * Sets up any annotation from --annotate option.
    * @note This must be called after "setupView" as it needs a projection.
    */
   void setupAnnotation();

   void addFiles(rspfString fileInfoStr, 
                 std::vector<rspfString> fileInfos,
                 bool withEncodedEntry);

   /**
   * Consolidates specification of bounding rect given various ways of specifying on the command
   * line. This avoids multiple, redundant checks scattered throughout the code.
   */
   void consolidateCutRectSpec();

   /**
   * Called when histogram operation is requested. Sets up additional filters in image chain
   * for performing matching, stretching or clipping. If chain=0,
   * this implies that the target histogram is being enabled for the output mosaic. */
   void setupHistogram(rspfImageChain* chain=0, const rspfSrcRecord& src_record=rspfSrcRecord());

   //! Utility method for creating a histogram for an input image. Returns TRUE if successful.
   bool createHistogram(rspfImageChain* chain, const rspfFilename& histo_filename);

   /**
    * @brief Adds cache to the left of resampler.
    * @param chain The chain to add to.
    */
   void addChainCache(rspfImageChain* chain) const;

   /**
   * @brief Generates a log KWL file that could be fed directly to Igen. Used for verifying chain.
   */
   void generateLog();
  
   /**
   * @brief Adds a scalar remapper to the extreme right of the chain is specified by the 
   * --output-radiometry option.
   */
   void setupOutputRadiometry();

   /**
    * @brief Checks all input image projections to see if elevation is needed.
    * @return true if affected, false if not.
    */
   bool isAffectedByElevation();

   /**
    * @brief Recomputes image gsds.
    *
    * Calls rspfImageGeometry::computeGsd() on all image handlers
    * that have projections affected by elevation.  Used to recompute after a
    * delayed load of elevation.
    */
   void reComputeChainGsds();

   /**
    * GSD Determination is nontrivial since there are various command-line options that control
    * this quantity. This method considers all information before setting the product's GSD.
    */
   void setProductGsd();
};
#endif
