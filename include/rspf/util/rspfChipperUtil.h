//----------------------------------------------------------------------------
//
// File: rspfChipperUtil.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
// 
// Utility class to for chipping out images. , orthorectifying imagery with an added slant
// toward doing digital elevation model(DEM) operations.
// 
//----------------------------------------------------------------------------
// $Id: rspfChipperUtil.h 22201 2013-03-20 00:53:33Z dburken $

#ifndef rspfChipperUtil_HEADER
#define rspfChipperUtil_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageSource.h>
#include <rspf/imaging/rspfSingleImageChain.h>
#include <rspf/projection/rspfMapProjection.h>

#include <map>
#include <vector>

// Forward class declarations:
class rspfArgumentParser;
class rspfDpt;
class rspfFilename;
class rspfGpt;
class rspfImageFileWriter;
class rspfImageGeometry;
class rspfImageViewAffineTransform;
class rspfIrect;
class rspfKeywordlist;

/**
 * @brief rspfChipperUtil class.
 *
 * This is a utility class to orthorectify imagery with an added slant toward
 * doing digital elevation model(DEM) operations.
 *
 * See the rspf-dem application for a code usage example.
 *
 * @note Almost all methods use throw for stack unwinding.  This is not in
 * method declarations to alleviate build errors on windows.  Sorry...
 *
 * @note "bumpshade" and "hillshade" intermixed throughout.  The class to do
 * a hillshade is the rspfBumpShadeTileSource.
 */
class RSPF_DLL rspfChipperUtil : public rspfReferenced
{
public:

   /** emumerated operations */
   enum rspfChipperOperation
   {
      RSPF_CHIPPER_OP_UNKNOWN      = 0,
      RSPF_CHIPPER_OP_HILL_SHADE   = 1,
      RSPF_CHIPPER_OP_COLOR_RELIEF = 2,
      RSPF_CHIPPER_OP_ORTHO        = 3,
      RSPF_CHIPPER_OP_2CMV         = 4, // two color multiview
      RSPF_CHIPPER_OP_CHIP         = 5  // image space
   };

   /** emumerated output projections */
   enum rspfChipperOutputProjection
   {
      RSPF_CHIPPER_PROJ_UNKNOWN    = 0,
      RSPF_CHIPPER_PROJ_GEO        = 1,
      RSPF_CHIPPER_PROJ_GEO_SCALED = 2,
      RSPF_CHIPPER_PROJ_INPUT      = 3,
      RSPF_CHIPPER_PROJ_UTM        = 4
   };

   /** default constructor */
   rspfChipperUtil();

   /** virtual destructor */
   virtual ~rspfChipperUtil();

   /**
    * @brief Initial method to be ran prior to execute.
    * @param ap Arg parser to initialize from.
    * @note Throws rspfException on error.
    * @note A throw with an error message of "usage" is used to get out when
    * a usage is printed.
    */
   bool initialize(rspfArgumentParser& ap);

   /**
    * @brief Initial method to be ran prior to execute.
    *
    * This was separated out from initialize(rspfArgumentParser& ap) because
    * you could make an initialize(const rspfKeywordlist& kwl) and then call
    * initialize().
    * 
    * @note Throws rspfException on error.
    */
   void initialize();

   /**
    * @brief execute method.  Performs the actual product write.
    * @note Throws rspfException on error.
    */
   void execute();

   /**
    * @brief Gets the output file name.
    * @param f Initialized by this with the filename.
    */
   void getOutputFilename(rspfFilename& f) const;

private:

   /**
    * @brief Initializes the output projection and propagates to image chains.
    * @note Throws rspfException on error.
    */
   void initializeOutputProjection();
   
   /** @brief Create chains for all dems. */
   void addDemSources();
   
   /** @brief Method to create a chain and add to dem layers from file. */
   void addDemSource(const rspfFilename& file);

   /**
    * @brief  Method to create a chain and add to dem layers from a
    * rspfSrcRecord.
    */
   void addDemSource(const rspfSrcRecord& rec);

   /** @brief Creates chains for all images. */
   void addImgSources();
   
   /** @brief Method to create a chain and add to img layers from file. */
   void addImgSource(const rspfFilename& file);

   /**
    * @brief  Method to create a chain and add to img layers from a
    * rspfSrcRecord.
    */
   void addImgSource(const rspfSrcRecord& rec);

   /**
    * @brief Creates a rspfSingleImageChain from file.
    * @param file File to open.
    * @return Ref pointer to rspfSingleImageChain.
    * @note Throws rspfException on error.
    */
   rspfRefPtr<rspfSingleImageChain> createChain(const rspfFilename& file) const;

   /**
    * @brief Creates a rspfSingleImageChain from rspfSrcRecord.
    * @param src Record.
    * @return Ref pointer to rspfSingleImageChain.
    * @note Throws rspfException on error.
    */
   rspfRefPtr<rspfSingleImageChain> createChain(const rspfSrcRecord& rec) const;

   /**
    * @brief Creates the output or view projection.
    * @note All chains should be constructed prior to calling this.
    */
   void createOutputProjection();
   
   /**
    * @brief Sets the single image chain for identity operations view to
    * a rspfImageViewAffineTransform.  This will have a rotation if
    * up is up is selected.  Also set m_outputProjection to the input's
    * for area of interest.
    */
   void createIdentityProjection();

   /**
    * @brief Gets the first input projection.
    *
    * This gets the output projection of the first dem layer if present;
    * if not, the first image layer.
    * 
    * @return ref ptr to projection, could be null.
    */
   rspfRefPtr<rspfMapProjection> getFirstInputProjection();

   /**
    * @brief Convenience method to get geographic projection.
    * @return new rspfEquDistCylProjection.
    */
   rspfRefPtr<rspfMapProjection> getNewGeoProjection();

   /**
    * @brief Convenience method to get geographic projection.
    *
    * This method sets the origin to the center of the scene bounding rect
    * of all layers.
    * @return new rspfEquDistCylProjection.
    */
   rspfRefPtr<rspfMapProjection> getNewGeoScaledProjection();

    /**
    * @brief Convenience method to get a projection from an srs code.
    * @return new rspfMapProjection.
    */  
   rspfRefPtr<rspfMapProjection> getNewProjectionFromSrsCode(const rspfString& code);

   /**
    * @brief Convenience method to get a utm projection.
    * @return new rspfUtmProjection.
    */     
   rspfRefPtr<rspfMapProjection> getNewUtmProjection();

   /**
    * @brief Convenience method to get a pointer to the  output map
    * projection.
    *
    * Callers should check for valid() as the pointer could be
    * 0 if not initialized.
    * 
    * @returns The rspfMapProjection* from the m_outputGeometry as a ref
    * pointer.
    */
   rspfRefPtr<rspfMapProjection> getMapProjection();

   /**
    * @brief Sets the projection tie point to the scene bounding rect corner.
    * @note Throws rspfException on error.
    */
   void intiailizeProjectionTiePoint();

   /**
    * @brief Initializes the projection gsd.
    *
    * This loops through all chains to find the best resolution gsd.
    *
    * @note Throws rspfException on error.
    */
   void initializeProjectionGsd();   

   /**
    * @brief Initializes the image view transform(IVT) scale.
    *
    * Chip mode only. Sets IVT scale to output / input.
    * 
    * @note Throws rspfException on error.
    */
   void initializeIvtScale();   

   /**
    * @brief Loops through all layers to get the upper left tie point.
    * @param tie Point to initialize.
    */
   void getTiePoint(rspfGpt& tie);

   /**
    * @brief Gets the upper left tie point from a chain.
    * @param chain The chain to get tie point from.
    * @param tie Point to initialize.
    * @note Throws rspfException on error.
    */
   void getTiePoint(rspfSingleImageChain* chain, rspfGpt& tie);

   /**
    * @brief Loops through all layers to get the upper left tie point.
    * @param tie Point to initialize.
    */
   void getTiePoint(rspfDpt& tie);

   /**
    * @brief Gets the upper left tie point from a chain.
    * @param chain The chain to get tie point from.
    * @param tie Point to initialize.
    * @note Throws rspfException on error.
    */
   void getTiePoint(rspfSingleImageChain* chain, rspfDpt& tie);

   /**
    * @brief Loops through all layers to get the best gsd.
    * @param gsd Point to initialize.
    */
   void getMetersPerPixel(rspfDpt& gsd);

   /**
    * @brief Gets the gsd from a chain.
    * @param chain The chain to get gsd from.
    * @param gsd Point to initialize.
    * @note Throws rspfException on error.
    */   
   void getMetersPerPixel(rspfSingleImageChain* chain, rspfDpt& gsd);

   /**
    * @brief Gets value of key "central_meridan" if set, nan if not.
    *
    * @return Value as a double or nan if keyord is not set.
    * 
    * @note Throws rspfException on range error.
    */
   rspf_float64 getCentralMeridian() const;

   /**
    * @brief Gets value of key "origin_latitude" if set, nan if not.
    *
    * @return Value as a double or nan if keyord is not set.
    * 
    * @note Throws rspfException on range error.
    */
   rspf_float64 getOriginLatitude() const;

   /**
    * @brief Loops through all layers to get the scene center ground point.
    * @param gpt Point to initialize.
    * @note Throws rspfException on error.
    */
   void getSceneCenter(rspfGpt& gpt);

   /**
    * @brief Gets the scene center from a chain.
    * @param chain The chain to get scene center from.
    * @param gpt Point to initialize.
    * @note Throws rspfException on error.
    */   
   void getSceneCenter(rspfSingleImageChain* chain, rspfGpt& gpt);

   /**
    * @brief Creates a new writer.
    *
    * This will use the writer option (-w or --writer), if present; else,
    * it will be derived from the output file extention.
    *
    * This will also set any writer properties passed in.
    *
    * @return new rspfImageFileWriter.
    * @note Throws rspfException on error.
    */
   rspfRefPtr<rspfImageFileWriter> createNewWriter() const;

   /**
    * @brief loops through all chains and sets the output projection.
    * @note Throws rspfException on error.
    */
   void propagateOutputProjectionToChains();

   /**
    * @brief Combines all layers into an rspfImageMosaic.
    * @return rspfRefPtr with pointer to rspfImageSource.  Can be null.
    */
   rspfRefPtr<rspfImageSource> combineLayers(
      std::vector< rspfRefPtr<rspfSingleImageChain> >& layers) const;

   /** @brief Combines dems(m_demLayer) and images(m_imgLayer). */
   rspfRefPtr<rspfImageSource> combineLayers();

   /** @brief Combines two images into a two color multi view chain.
    * @return rspfRefPtr with pointer to rspfImageSource.  Can be null.
    */
   rspfRefPtr<rspfImageSource> combine2CmvLayers();

   /**
    * @brief Creates rspfIndexToRgbLutFilter and connects to source.
    * @param Source to connect to.
    * @return End of chain with lut filter on it.
    * @note Throws rspfException on error.
    */
   rspfRefPtr<rspfImageSource> addIndexToRgbLutFilter(
      rspfRefPtr<rspfImageSource> &source) const;

   /**
    * @brief Creates rspfScalarRemapper and connects to source.
    * @param Source to connect to.
    * @param scalar Scalar type.
    * @return End of chain with remapper on it.
    * @note Throws rspfException on error.
    */
   rspfRefPtr<rspfImageSource> addScalarRemapper(
      rspfRefPtr<rspfImageSource> &source,
      rspfScalarType scalar) const;

   /**
    * @brief Set up rspfHistogramRemapper for a chain.
    * @param chain Chain to set up.
    * @return true on success, false on error.
    */
   bool setupChainHistogram( rspfRefPtr<rspfSingleImageChain>& chain) const;

   /**
    * @brief Sets entry for a chain.
    * @param chain Chain to set up.
    * @param entryIndex Zero based index.
    * @return true on success, false on error.
    */
   bool setChainEntry( rspfRefPtr<rspfSingleImageChain>& chain,
                       rspf_uint32 entryIndex ) const;

   /**
    * @brief Initializes "rect" with the output area of interest.
    *
    * Initialization will either come from user defined cut options or the
    * source bounding rect with user options taking precidence.
    *
    * @param source Should be the end of the processing chain.
    * @param rect Rectangle to initialize.  This is in output (view) space.
    *
    * @note Throws rspfException on error.
    */
   void getAreaOfInterest( rspfImageSource* source, rspfIrect& rect ) const;

   /**
    * @brief Method to calculate and initialize scale and area of interest
    * for making a thumbnail.
    *
    * Sets the scale of the output projection so that the adjusted rectangle
    * meets the cut rect and demension requirements set in options.
    *
    * @param originalRect Original scene area of interest.
    * @param adjustedRect New rect for thumbnail.
    *
    * @note Throws rspfException on error.
    */
   void initializeThumbnailProjection(const rspfIrect& originalRect,
                                      rspfIrect& adjustedRect);

   /** @return true if BANDS keyword is set; false, if not. */
   bool hasBandSelection() const;

   /**
    * @brief Gets the band list if BANDS keyword is set.
    *
    * NOTE: BANDS keyword values are ONE based.  bandList values are
    * ZERO based.
    *
    * @param bandList List initialized by this.
    */
   void getBandList( std::vector<rspf_uint32>& bandList ) const;

   /** @return true if color table (lut) is set; false, if not. */
   bool hasLutFile() const;

   /**
    * @return true if any bump share options have been set by user; false,
    * if not.
    */
   bool hasBumpShadeArg() const;

   /** @return true if thumbnail option is set; false, if not. */
   bool hasThumbnailResolution() const;

   bool hasHistogramOperation() const;

   /** @return true if file extension is "hgt", "dem" or contains "dtN" (dted). */
   bool isDemFile(const rspfFilename& file) const;

   /** @return true if file extension is "src" */
   bool isSrcFile(const rspfFilename& file) const;

   /** @brief Initializes m_srcKwl if option was set. */
   void initializeSrcKwl();

   /**
    * @return The number of DEM_KW and IMG_KW found in the m_kwl and m_srcKwl
    * keyword list.
    */
   rspf_uint32 getNumberOfInputs() const;

   /**
    * @brief Gets the emumerated output projection type.
    *
    * This looks in m_kwl for rspfKeywordNames::PROJECTION_KW.
    * @return The enumerated output projection type.
    * @note This does not cover SRS keyword which could be any type of projection.
    */
   rspfChipperOutputProjection getOutputProjectionType() const;

   /**
    * @brief Returns the scalar type from OUTPUT_RADIOMETRY_KW keyword if
    * present. Deprecated SCALE_2_8_BIT_KW is also checked.
    *
    * @return rspfScalarType Note this can be RSPF_SCALAR_UNKNOWN if the
    * keywords are not present.
    */
   rspfScalarType getOutputScalarType() const;

   /** @return true if scale to eight bit option is set; false, if not. */
   bool scaleToEightBit() const;

   /** @return true if snap tie to origin option is set; false, if not. */
   bool snapTieToOrigin() const;

   /** @return true if up_is_up_kw option is set; false, if not. */
   bool upIsUp() const;

   /** @return true if north_up_kw option is set; false, if not. */
   bool northUp() const;

   /** @return true if operation is "chip" or identity; false, if not. */
   bool isIdentity() const;

   /**
    * @return The entry number if set.  Zero if rspfKeywordNames::ENTRY_KW not
    * found.
    */
   rspf_uint32 getEntryNumber() const;

   /**
    * @return True if any input has a sensor model input, false if all input
    * projections are map projections.
    */
   bool hasSensorModelInput() const;

   /**
    * @brief Adds application arguments to the argument parser.
    * @param ap Parser to add to.
    */
   void addArguments(rspfArgumentParser& ap);
   
   /** @brief Initializes arg parser and outputs usage. */
   void usage(rspfArgumentParser& ap);

   /** Enumerated operation to perform. */
   rspfChipperOperation m_operation;
   
   /** Hold all options passed into intialize except writer props. */
   rspfRefPtr<rspfKeywordlist> m_kwl;

   /** Hold contents of src file if --src is used. */
   rspfRefPtr<rspfKeywordlist> m_srcKwl;

   /**
    * The image geometry.  In chip mode this will be from the input image. So
    * this may or may not have a map projection. In any other mode it
    * will the view or output geometry which will be a map projection.
    */
   rspfRefPtr<rspfImageGeometry> m_geom;

   /**
    * Image view transform(IVT). Only set/used in "chip"(identity) operation as
    * the IVT for the resampler(rspfImageRenderer).
    */
   rspfRefPtr<rspfImageViewAffineTransform> m_ivt;

   /**  Array of dem chains. */
   std::vector< rspfRefPtr<rspfSingleImageChain> > m_demLayer;

   /**  Array of image source chains. */
   std::vector< rspfRefPtr<rspfSingleImageChain> > m_imgLayer;
};

#endif /* #ifndef rspfChipperUtil_HEADER */
