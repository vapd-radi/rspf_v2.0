//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:
//
// Contains class declaration for TiffOverviewBuilder.
//
//*******************************************************************
//  $Id: rspfTiffOverviewBuilder.h 22232 2013-04-13 20:06:19Z dburken $

#ifndef rspfTiffOverviewBuilder_HEADER
#define rspfTiffOverviewBuilder_HEADER

#include <vector>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>

#include <rspf/imaging/rspfOverviewBuilderBase.h>
#include <rspf/imaging/rspfFilterResampler.h>

#include <tiffio.h>

class rspfConnectableObject;
class rspfFilename;
class rspfImageGeometry;

class RSPF_DLL rspfTiffOverviewBuilder
   :
      public rspfOverviewBuilderBase
{
public:

   enum
   {
      DEFAULT_COMPRESS_QUALITY=75
   };

   /** default constructor */
   rspfTiffOverviewBuilder();
   
   /** virtual destructor */
   virtual ~rspfTiffOverviewBuilder();

   /**
    * Supports BOX or NEAREST NEIGHBOR.  When indexed you should probably use nearest neighbor
    */ 
   void setResampleType(rspfFilterResampler::rspfFilterResamplerType resampleType);
   
   /**
    *  Builds overview file and sets "theOutputFile" to that of
    *  the overview_file.
    *
    *  @param overview_file The overview file name to output.
    *
    *  @param copy_all  If set to true the entire image will be
    *  copied.  This can be used to convert an image to a tiled tif.
    *  
    *  @return trueon success, false on error.
    */
   bool buildOverview(const rspfFilename& overview_file,
                      bool copy_all=false);

   /**
    * Calls buildOverview.  This method uses "theOutputFile" for the file
    * name.
    *
    * If the copy_all flag is set the entire image will be copied.  This can
    * be used to convert an image to a tiled tif.
    *
    * @return true on success, false on error.
    *
    * @note If setOutputFile was not called the output name will be derived
    * from the image name.  If image was "foo.tif" the overview file will
    * be "foo.ovr".
    */
   virtual bool execute();

   /**
    *  Sets the compression type to use when building overviews.
    *  
    *  @param compression_type Current supported types:
    * - COMPRESSION_JPEG
    * - COMPRESSION_LZW
    * - COMPRESSION_DEFLATE
    * - COMPRESSION_PACKBITS
    * - COMPRESSION_NONE (default)
    */
   void setCompressionType(rspf_uint16 compression_type);

   /**
    *  Sets the compression quality for use when using a compression type
    *  of COMPRESSION_JPEG.
    *
    *  @param quality Range 1 to 100 with 100 being best quality.
    */
   void setJpegCompressionQuality(rspf_int32 quality);

   /**
    * @brief Method to return copy all flag.
    * @return The copy all flag.  If true all data will be written to the
    * overview including R0.
    */
   bool getCopyAllFlag() const;

   /**
    * @brief Sets internal overviews flag.
    * @param flag If true, and input source is tiff format, overviews will be
    * built internally.
    */
   void setInternalOverviewsFlag(bool flag);

   /** @return The intenal overview flag. */
   bool getInternalOverviewsFlag() const;

   /**
    * @brief Sets theCopyAllFlag.
    * @param flag The flag. If true all data will be written to the
    * overview including R0.
    */
   void setCopyAllFlag(bool flag);

   /** @return rspfObject* to this object. */
   virtual rspfObject* getObject();

   /**
    * @return const rspfObject* to this object.
    */
   virtual const rspfObject* getObject() const;

   /**
    * @return true if input is an image handler.
    */
   virtual bool canConnectMyInputTo(rspf_int32 index,
                                    const rspfConnectableObject* obj) const;

   /**
    * @brief Sets the input to the builder. Satisfies pure virtual from
    * rspfOverviewBuilderBase.
    *
    * @param imageSource The input to the builder.
    *
    * @return True on successful initializion, false on error.
    */
   virtual bool setInputSource(rspfImageHandler* imageSource);
   
   /**
    * @brief Sets the output filename.
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * @param file The output file name.
    */
   virtual void  setOutputFile(const rspfFilename& file);

   /**
    * @brief Gets the output file name. (pure virtual)
    *
    * @return The output file name or rspfFilename::NIL if it was not set
    * yet and the image handle has not been initialized.
    *
    * @note This is non-const as a call to this may force initialization of
    * overview builder output file name if the it was not set already.
    *
    * @note This will return rspfFilename::NIL unless one of was called,
    * setInputSource or setOutputFile.
    */
   virtual rspfFilename getOutputFile() const;

   void setOutputTileSize(const rspfIpt& tileSize);

   /**
    * @brief Sets the overview output type.
    *
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * 
    * Currently handled types are:
    * "rspf_tiff_nearest" and "rspf_tiff_box"
    *
    * @param type This should be the string representing the type.  This method
    * will do nothing if type is not handled and return false.
    *
    * @return true if type is handled, false if not.
    */
   virtual bool setOverviewType(const rspfString& type);

   /**
    * @brief Gets the overview type.
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * @return The overview output type as a string.
    */
   virtual rspfString getOverviewType() const;

   /**
    * @brief Method to populate class supported types.
    * Satisfies pure virtual from rspfOverviewBuilderBase.
    * @param typeList List of rspfStrings to add to.
    */
   virtual void getTypeNameList(std::vector<rspfString>& typeList)const;

   /**
    * @brief Method to set properties.
    * @param property Property to set.
    *
    * @note Currently supported property:
    * name=levels, value should be list of levels separated by a comma with
    * no spaces. Example: "2,4,8,16,32,64"
    */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);

   /**
    * @brief Method to populate the list of property names.
    * @param propertyNames List to populate.  This does not clear the list
    * just adds to it.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
  
private:

   /**
    *  Copy the full resolution image data to the output tif image.
    */
  bool writeR0(TIFF* tif);

   /**
    *  Write reduced resolution data set to the tif file.
    *
    *  @param firstRestLevel used to tell method that if a histogram is needed, do it on
    *  that res level.
    */
   bool writeRn(rspfImageHandler* imageHandler,
                TIFF* tif,
                rspf_uint32 resLevel,
                bool firstResLevel);
   
   /**
    *  Set the tiff tags for the appropriate resLevel.  Level zero is the
    *  full resolution image.
    *
    *  @param tif Pointer to the tif file.
    *  @param outputRect The dimensions (zero based) of res set.
    *  @param rrds_level The current reduced res level.
    */
   bool setTags(TIFF* tif,
                const rspfIrect& outputRect,
                rspf_int32 resLevel) const;

   /**
    *  Writes geotiff tags.
    *  @param geom
    *  @param boundingRect
    *  @param resLevel Zero base, 0 being full res.
    *  @param tif
    *  @return true on success, false on error.
    */
   bool setGeotiffTags(const rspfImageGeometry* geom,
                       const rspfDrect& boundingRect,
                       rspf_uint32 resLevel,
                       TIFF* tif);

   TIFF* openTiff( const rspfString& filename ) const;

   void closeTiff(TIFF* tif);

   /**
    * @returns true if m_internalOverviewsFlag and input image supports
    * building internal overviews.  Currently only tiff reader.
    */
   bool buildInternalOverviews() const;

   /**
    * @return true if m_copyAllFlag is set and build internal overviews is not
    * set.
    */
   bool copyR0() const;

   // Disallow these...
   rspfTiffOverviewBuilder(const rspfTiffOverviewBuilder& source);
   rspfTiffOverviewBuilder& operator=(const rspfTiffOverviewBuilder& rhs); 

   std::vector<rspf_uint8>                           m_nullDataBuffer;
   rspf_int32                                        m_bytesPerPixel;
   rspf_int32                                        m_bitsPerSample;
   rspf_int32                                        m_tileWidth;
   rspf_int32                                        m_tileHeight;
   rspf_int32                                        m_tileSizeInBytes;
   rspf_int32                                        m_sampleFormat;
   rspf_int32                                        m_currentTiffDir;
   rspf_uint16                                       m_tiffCompressType;
   rspf_int32                                        m_jpegCompressQuality;
   rspfFilterResampler::rspfFilterResamplerType     m_resampleType;
   std::vector<double>                                m_nullPixelValues;
   bool                                               m_copyAllFlag;
   bool                                               m_outputTileSizeSetFlag;
   bool                                               m_internalOverviewsFlag;

TYPE_DATA   
};
   
#endif
