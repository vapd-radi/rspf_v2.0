#ifndef rspfHdfReader_HEADER
#define rspfHdfReader_HEADER 1
#include "../rspfPluginConstants.h"
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/support_data/rspfJ2kSizRecord.h>
#include <rspf/imaging/rspfAppFixedTileCache.h>
#include <rspfGdalTileSource.h>
class rspfImageData;
class RSPF_PLUGINS_DLL rspfHdfReader : public rspfImageHandler
{
public:
   /** default construtor */
   rspfHdfReader();
   
   /** virtural destructor */
   virtual ~rspfHdfReader();
   /**
    * @brief Returns short name.
    * @return "rspf_hdf_reader"
    */
   virtual rspfString getShortName() const;
   
   /**
    * @brief Returns long  name.
    * @return "rspf hdf reader"
    */
   virtual rspfString getLongName()  const;
   /**
    * @brief Returns class name.
    * @return "rspfHdfReader"
    */
   virtual rspfString getClassName() const;
   /**
    *  @brief Method to grab a tile(rectangle) from image.
    *
    *  @param rect The zero based rectangle to grab.
    *
    *  @param resLevel The reduced resolution level to grab from.
    *
    *  @return The ref pointer with the image data pointer.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const  rspfIrect& rect,
                                               rspf_uint32 resLevel=0);
   virtual bool getTile(rspfImageData* result, rspf_uint32 resLevel=0);   
   /**
    *  Returns the number of bands in the image.
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfInputBands() const;
   /**
    * Returns the number of bands in a tile returned from this TileSource.
    * Note: we are supporting sources that can have multiple data objects.
    * If you want to know the scalar type of an object you can pass in the
    */
   virtual rspf_uint32 getNumberOfOutputBands()const; 
   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual rspf_uint32 getImageTileWidth() const;
   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileHeight which
    * returns the output tile height which can be different than the internal
    * image tile height on disk.
    */
   virtual rspf_uint32 getImageTileHeight() const;
   /**
    * Returns the output pixel type of the tile source.
    */
   virtual rspfScalarType getOutputScalarType() const;
 
   /**
    * @brief Gets number of lines for res level.
    *
    *  @param resLevel Reduced resolution level to return lines of.
    *  Default = 0
    *
    *  @return The number of lines for specified reduced resolution level.
    */
   virtual rspf_uint32 getNumberOfLines(rspf_uint32 resLevel = 0) const;
   /**
    *  @brief Gets the number of samples for res level.
    *
    *  @param resLevel Reduced resolution level to return samples of.
    *  Default = 0
    *
    *  @return The number of samples for specified reduced resolution level.
    */
   virtual rspf_uint32 getNumberOfSamples(rspf_uint32 resLevel = 0) const;
   /**
    * @brief Open method.
    * @return true on success, false on error.
    */
   virtual bool open();
   /**
    *  @brief Method to test for open file stream.
    *
    *  @return true if open, false if not.
    */
   virtual bool isOpen()const;
   /**
    * @brief Method to close current entry.
    *
    */
   virtual void close();
   /**
    * Returns the image geometry object associated with this tile source or
    * NULL if non defined.  The geometry contains full-to-local image
    * transform as well as projection (image-to-world).
    */
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);   
   virtual bool setCurrentEntry(rspf_uint32 entryIdx);
    /**
    * @return The number of entries (images) in the image file.
    */
   virtual rspf_uint32 getNumberOfEntries()const;
   
   /**
    * @param entryList This is the list to initialize with entry indexes.
    *
    * @note This implementation returns puts one entry "0" in the list.
    */
   virtual void getEntryList(std::vector<rspf_uint32>& entryList) const;
   /**
    * @return The current entry number.
    *
    */
   virtual rspf_uint32 getCurrentEntry() const { return m_currentEntryRender; }
   bool isSupportedExtension();
   std::vector<rspf_uint32> getCurrentEntryList(rspf_uint32 entryId) const;
   rspfString getEntryString(rspf_uint32 entryId) const;
   rspfString getDriverName();
   virtual bool setOutputBandList(const std::vector<rspf_uint32>& band_list);
private:
  bool isSDSDataset(rspfString fileName);
  rspfRefPtr<rspfGdalTileSource>                              m_gdalTileSource;
  mutable std::vector<rspf_uint32>                             m_entryFileList;
  rspf_uint32                                                  m_numberOfBands;
  rspfScalarType                                               m_scalarType;
  rspf_uint32                                                  m_currentEntryRender;
  rspfRefPtr<rspfImageData>                                   m_tile;
TYPE_DATA
};
#endif /* #ifndef rspfHdfReader_HEADER */
