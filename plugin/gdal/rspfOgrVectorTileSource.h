#ifndef rspfOgrVectorTileSource_HEADER
#define rspfOgrVectorTileSource_HEADER
#include <vector>
#include <list>
#include <map>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDrect.h>
#include <ogrsf_frmts.h>
#include <gdal.h>
class rspfProjection;
class rspfMapProjection;
class rspfImageProjectionModel;
class rspfOgrVectorLayerNode;	
class rspfImageGeometry;
class rspfOgrVectorTileSource :

   public rspfImageHandler
{
public:
   rspfOgrVectorTileSource();
   virtual ~rspfOgrVectorTileSource();
   virtual void close();
   /**
    *  @return Returns true on success, false on error.
    *
    *  @note This method relies on the data member rspfImageData::theImageFile
    *  being set.  Callers should do a "setFilename" prior to calling this
    *  method or use the rspfImageHandler::open that takes a file name and an
    *  entry index.
    */      
   virtual bool open();
   /*!
    *  Returns a pointer to a tile given an origin representing the upper
    *  left corner of the tile to grab from the image.
    *  Satisfies pure virtual from TileSource class.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   /*!
    *  Returns the number of bands in the image.
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfInputBands() const;
    /*!
    * Returns the number of bands in a tile returned from this TileSource.
    * Note: we are supporting sources that can have multiple data objects.
    * If you want to know the scalar type of an object you can pass in the 
    */
   virtual rspf_uint32 getNumberOfOutputBands() const;
  
   /*!
    *  Returns the number of bands in the image.
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfLines(rspf_uint32 reduced_res_level = 0) const;
   /*!
    *  Returns the number of bands available from an image.
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfSamples(rspf_uint32 reduced_res_level = 0) const;
   /*!
    * Returns the number of reduced resolution data sets (rrds).
    * 
    * Note:  Shape files should never have reduced res sets so this method is
    * returns "8" to avoid the question of "Do you want to build res sets".
    */
   virtual rspf_uint32 getNumberOfDecimationLevels() const;
   /*!
    * Returns the zero based image rectangle for the reduced resolution data
    * set (rrds) passed in.  Note that rrds 0 is the highest resolution rrds.
    */
   virtual rspfIrect getImageRectangle(rspf_uint32 reduced_res_level = 0) const;
   /*!
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual rspfRefPtr<rspfImageGeometry> getInternalImageGeometry() const;
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   /*!
    * Returns the output pixel type of the tile source.
    */
   virtual rspfScalarType getOutputScalarType() const;
   /*!
    * Returns the width of the output tile.
    */
   virtual rspf_uint32 getTileWidth() const;
   
   /*!
    * Returns the height of the output tile.
    */
   virtual rspf_uint32 getTileHeight() const;
   /*!
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual rspf_uint32 getImageTileWidth() const;
   /*!
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual rspf_uint32 getImageTileHeight() const;   
   virtual bool isOpen()const;
   
   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
      
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
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
   rspfProjection* createProjFromReference(OGRSpatialReference* reference) const;
   rspfMapProjection* createDefaultProj();
   bool isOgrVectorDataSource()const;
private:
   std::vector<rspfOgrVectorLayerNode*> theLayerVector;
   OGRDataSource*                      theDataSource;
   rspfRefPtr<rspfImageGeometry>     theImageGeometry;
   rspfDrect                          theImageBound;
   OGREnvelope                         theBoundingExtent;
TYPE_DATA
};
#endif
