//----------------------------------------------------------------------------
// File: rspfInfo.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: rspfInfo class declaration
//
// See class doxygen descriptions below for more.
// 
//----------------------------------------------------------------------------
// $Id: rspfInfo.h 22187 2013-03-07 20:29:00Z dburken $

#ifndef rspfInfo_HEADER
#define rspfInfo_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageHandler.h>

#include <ostream>

class rspfArgumentParser;
class rspfGpt;

/**
 * @brief rspfInfo class.
 *
 * This is a utility class for getting information from the rspf library.
 * This includes information from an image, general library queries, like
 * loaded plugins, and just general stuff like height for point, conversions
 * and so on that are easily obtained through the library.
 */
class RSPF_DLL rspfInfo : public rspfReferenced
{
public:

   /** default constructor */
   rspfInfo();

   /** virtual destructor */
   virtual ~rspfInfo();

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
    * @brief execute method.
    *
    * Performs the actual dump of information.  This executes any options
    * set including image operations, i.e. -i -p --dno and so on.
    * 
    * @note Throws rspfException on error.
    */
   void execute();

   /**
    * @brief handles image options.
    *
    * Handles image type info opions,  i.e. -i -p --dno and so on.
    *
    * @return Number of consumed options.
    */
   rspf_uint32 executeImageOptions(const rspfFilename& file);

   /**
    * @brief getImageInfo Method to open image "file" and get image info
    * in the form of a rspfKeywordlist.
    *
    * Flags turn on various pieces of info.  These equate to options in
    * rspf-info for image information.
    *
    * This method opens and closes "file" under the hood so if an image is
    * open and it's not the same as "file" it will be closed by this call.
    * On exite the image will be closed.
    *
    * @param file Image file to get information for.
    * @param dumpFlag      rspf-info -d
    * @param dnoFlag       rspf-info --dno
    * @param imageGeomFlag rspf-info -p
    * @param imageInfoFlag rspf-info -i 
    * @param metaDataFlag  rspf-info -m 
    * @param paletteFlag   rspf-info --palette
    * @param kwl Initialized by this method.
    */
   void getImageInfo(const rspfFilename& file,
                     bool dumpFlag,
                     bool dnoFlag,
                     bool imageGeomFlag,
                     bool imageInfoFlag,
                     bool metaDataFlag,
                     bool paletteFlag,
                     rspfKeywordlist& kwl);
   
   /**
    * @brief Opens image handler and stores in m_img data member.
    * @param Image to open.
    * @note Throws rspfException if image cannot be opened.
    */
   void openImage(const rspfFilename& file);

   /** @brief Closes image if open. */
   void closeImage();

   /**
    * @return Ref pointer to the image handler.  Can be null if an image is
    * not open.
    */
   rspfRefPtr<rspfImageHandler> getImageHandler();

   /** @brief Dumps the image information from rspfInfoFactoryRegistry */
   void dumpImage(const rspfFilename& file, bool dnoFlag) const;

   /**
    * @brief Dumps the image information from rspfInfoFactoryRegistry to
    * keyword list.
    * @param file Image to dump.
    * @param dnoFlag Dump no overviews. If true do not dump overviews.
    * @param kwl Initialized by this method.
    */
   void dumpImage(const rspfFilename& file,
                  bool dnoFlag,
                  rspfKeywordlist& kwl) const;

   /**
    * @brief Prints factories.
    * @param keywordListFlag If true the result of a saveState will be output
    * for each factory.
    * */
   void printFactories(bool keywordListFlag) const;

   /**
    * @brief Populates keyword list with metadata.
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    */
   void getImageMetadata(rspfKeywordlist& kwl) const;

   /**
    * @brief Populates keyword list with palette data.
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    */
   void getImagePalette(rspfKeywordlist& kwl);

   /**
    * @brief Populates keyword list with general image information.
    *
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageInfo(rspfKeywordlist& kwl, bool dnoFlag);

   /**
    * @brief Populates keyword list with general image information.
    *
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * @param dnoFlag If true no entries flaged as overviews will be output.
    *
    * This requires open image.
    *
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageInfo(rspf_uint32 entry, rspfKeywordlist& kwl, bool dnoFlag);

   /**
    * @brief Populates keyword list with image geometry/projection information.
    *
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageGeometryInfo(rspfKeywordlist& kwl, bool dnoFlag);

   /**
    * @brief Populates keyword list with image geometry/projection information.
    *
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * @param dnoFlag If true no entries flaged as overviews will be output.
    *
    * This requires open image.
    *
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageGeometryInfo( rspf_uint32 entry, 
                              rspfKeywordlist& kwl, 
                              bool dnoFlag );

   void getCenterImage(rspfKeywordlist& kwl);
   void getCenterImage(rspf_uint32 entry, rspfKeywordlist& kwl);
   void getCenterGround(rspfKeywordlist& kwl);
   void getCenterGround(rspf_uint32 entry, rspfKeywordlist& kwl);

   /**
    * @brief Populates keyword list with up_is_up_rotation.
    *
    * @param kwl Keyword list to populate.
    *
    * This requires open image.
    */
   void getUpIsUpAngle(rspfKeywordlist& kwl);

   /**
    * @brief Populates keyword list with up_is_up_rotation.
    *
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * @param dnoFlag If true no entries flaged as overviews will be output.
    *
    * This requires open image.
    */
   void getUpIsUpAngle(rspf_uint32 entry, rspfKeywordlist& kwl);
   
   /**
    * @brief Populates keyword list with image rectangle.
    *
    * This requires open image.
    *
    * @param kwl Keyword list to populate.
    */
   void getImageRect(rspfKeywordlist& kwl);

   /**
    * @brief Populates keyword list with image rectangle.
    *
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * 
    * @param kwl Keyword list to populate.
    *
    * This requires open image.
    */
   void getImageRect(rspf_uint32 entry, rspfKeywordlist& kwl);

   /**
    * @return true if current open image entry is an overview.
    */
   bool isImageEntryOverview() const;

   /** @brief Dumps rspf preferences/configuration data. */
   void printConfiguration() const;

   /**
    * @brief Dumps rspf preferences/configuration data.
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& printConfiguration(std::ostream& out) const;

   /** @brief Dumps datum list to stdout. */
   void printDatums() const;

   /** @brief Dumps datum list to stream. */
   std::ostream& printDatums(std::ostream& out) const;

   /** @brief Converts degrees to radians and outputs to stdout. */
   void deg2rad(const rspf_float64& degrees) const;

   /**
    * @brief Converts degrees to radians and outputs to stream.
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& deg2rad(const rspf_float64& degrees, std::ostream& out) const;

   /** @brief Converts radians to degrees and outputs to stdout. */
   void rad2deg(const rspf_float64& radians) const;

   /**
    * @brief Converts radians to degrees and outputs to stream. 
    * @param out Output to write to.
    * @return stream
    */
   std::ostream& rad2deg(const rspf_float64& radians, std::ostream& out) const;

   /** @brief Converts feet to meters and outputs to stdout. */
   void ft2mtrs(const rspf_float64& feet, bool us_survey) const;

   /**
    * @brief Converts feet to meters and outputs to stream.
    * @param out Output to write to out.
    * @return stream
    */
   std::ostream& ft2mtrs(const rspf_float64& feet, bool us_survey, std::ostream& out) const;

   /** @brief Converts meters to feet and outputs to stdout. */
   void mtrs2ft(const rspf_float64& meters, bool us_survey) const;

   /**
    * @brief Converts meters to feet and outputs to stream.
    * @param out Output to write to out.
    * @return stream
    */   
   std::ostream& mtrs2ft(const rspf_float64& meters, bool us_survey, std::ostream& out) const;

   /**
    * @brief Get meters per degree for a given latitude and outputs to stdout.
    * @param latitude
    */
   void mtrsPerDeg(const rspf_float64& latitude) const;

   /**
    * @brief Get meters per degree for a given latitude and outputs to stream.
    * @param latitude
    * @param out Output to write to out.
    * @return stream
    */   
   std::ostream& mtrsPerDeg(const rspf_float64& latitude, std::ostream& out) const;

   /**
    * @brief Gets the height for ground point (latitude, longitude). Outputs
    * to stdout.
    * @param gpt Ground point.
    */
   void outputHeight(const rspfGpt& gpt) const;

   /**
    * @brief Gets the height for ground point (latitude, longitude). Outputs
    * to out.
    * @param out Output to write to out.
    * @return stream
    */
   std::ostream& outputHeight(const rspfGpt& gpt, std::ostream& out) const;

   /** @brief Prints loaded plugins to stdout. */
   void printPlugins() const;

   /**
    * @brief Prints loaded plugins to stream.
    * @param out Output to write to out.
    * @return stream
    */
   std::ostream& printPlugins(std::ostream& out) const;

   /**
    * @brief Test a plugin load  and outputs to stdout.
    * 
    * @param plugin Plugin to test.
    */
   void testPlugin(const rspfFilename& plugin) const;

   /**
    * @brief Test a plugin load outputs to stream.
    * 
    * @param plugin Plugin to test.
    * @param out Stream to write to.
    * @param stream
    */
   std::ostream& testPlugin(const rspfFilename& plugin, std::ostream& out) const;

   /** @brief Prints overview types to stdout. */
   void printOverviewTypes() const;

   /** @brief Prints overview types to stream. */
   std::ostream& printOverviewTypes(std::ostream& out) const;

   /** @breif Prints projections to stdout. */
   void printProjections() const;

   /** @breif Prints projections to stream. */
   std::ostream& printProjections(std::ostream& out) const;

   /** @brief Prints reader properties to stdout. */
   void printReaderProps() const;

   /** @brief Prints reader properties to stream. */
   std::ostream& printReaderProps(std::ostream& out) const;

   /** @brief Prints resampler filters to stdout. */
   void printResamplerFilters() const;

   /** @brief Prints resampler filters to stream. */
   std::ostream& printResamplerFilters(std::ostream& out) const;

   /** @brief Prints list of available writers to stdout. */
   void printWriters() const;

   /** @brief Prints list of available writers to stream. */
   std::ostream& printWriters(std::ostream& out) const;

   /** @brief Prints writer properties to stdout. */
   void printWriterProps() const;

   /** @brief Prints writer properties to stream. */
   std::ostream& printWriterProps(std::ostream& out) const;

   /**
    * @brief Gets the radiometry string, i.e. "8-bit" and so on, from scalar.
    * @param scalar Scalar type.
    * @param s String to initialize.
    */
   void getRadiometry(rspfScalarType scalar, std::string& s) const;
   
private:
 
   /**
    * @brief Populates keyword list with metadata.
    * @param ih Pointer to an image handler.
    * @param kwl Keyword list to populate.
    */
   void getImageMetadata( const rspfImageHandler* ih, 
                          rspfKeywordlist& kwl ) const;
 
  /**
   * @brief Populates keyword list with palette data.
   * @param ih Pointer to an image handler.
   * @param kwl Keyword list to populate.
   */
   void getImagePalette( rspfImageHandler* ih, 
                         rspfKeywordlist& kwl ) const;
  
   /**
    * @brief Populates keyword list with general image information.
    * @param ih Pointer to an image handler.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageInfo( rspfImageHandler* ih, 
                      rspfKeywordlist& kwl, 
                      bool dnoFlag ) const;

   /**
    * @brief Populates keyword list with general image information.
    * @param ih Pointer to an image handler.
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageInfo( rspfImageHandler* ih, 
                      rspf_uint32 entry, 
                      rspfKeywordlist& kwl, 
                      bool dnoFlag ) const;
   
   /**
    * @brief Populates keyword list with image geometry/projection information.
    * @param ih Pointer to an image handler.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    */
   void getImageGeometryInfo( rspfImageHandler* ih,
                              rspfKeywordlist& kwl, 
                              bool dnoFlag ) const;

   /**
    * @brief Populates keyword list with image geometry/projection information.
    * @param ih Pointer to an image handler.
    * @param entry Entry number to select.  Note this is the entry number
    * from the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    * @param dnoFlag If true no entries flaged as overviews will be output.
    * @return true if entry info was saved to keyword list false if not.
    */
   bool getImageGeometryInfo( rspfImageHandler* ih,
                              rspf_uint32 entry, 
                              rspfKeywordlist& kwl, 
                              bool dnoFlag ) const;

   void getCenterImage( rspfImageHandler* ih,
                        rspfKeywordlist& kwl ) const;
   void getCenterImage( rspfImageHandler* ih,
                        rspf_uint32 entry, 
                        rspfKeywordlist& kwl ) const;
   void getCenterGround( rspfImageHandler* ih,
                         rspfKeywordlist& kwl ) const;
   void getCenterGround( rspfImageHandler* ih,
                         rspf_uint32 entry, 
                         rspfKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with up_is_up_rotation.
    * @param kwl Keyword list to populate.
    */
   void getUpIsUpAngle( rspfImageHandler* ih,
                        rspfKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with up_is_up_rotation.
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    */  
   void getUpIsUpAngle( rspfImageHandler* ih,
                        rspf_uint32 entry, 
                        rspfKeywordlist& kwl ) const;
   
   /**
    * @brief Populates keyword list with image rectangle.
    *
    * @param kwl Keyword list to populate.
    */
   void getImageRect( rspfImageHandler* ih,
                      rspfKeywordlist& kwl ) const;

   /**
    * @brief Populates keyword list with image rectangle.
    * @param entry Entry number to select.  Note this is the entry number from
    * the getEntryList call not a simple zero based entry index.
    * @param kwl Keyword list to populate.
    */
   void getImageRect( rspfImageHandler* ih,
                      rspf_uint32 entry, 
                      rspfKeywordlist& kwl ) const;

   /** @return true if current open image entry is an overview. */
   bool isImageEntryOverview( const rspfImageHandler* ih ) const;

  /**
   * @brief Opens image.
   * @param Image to open.
   * @return rspfRefPtr with image handler.
   * @note Throws rspfException if image cannot be opened.
   */
   rspfRefPtr<rspfImageHandler> openImageHandler(const rspfFilename& file) const;
   
   /** @brief Initializes arg parser and outputs usage. */
   void usage(rspfArgumentParser& ap);

   /** Hold all options passed into intialize except writer props. */
   rspfRefPtr<rspfKeywordlist> m_kwl;

   /** Holds the open image. */
   rspfRefPtr<rspfImageHandler> m_img;
};

#endif /* #ifndef rspfInfo_HEADER */
