#ifndef rspfOgrGdalTileSource_HEADER
#define rspfOgrGdalTileSource_HEADER 1
#include <vector>
#include <list>
#include <map>
#include <rspfGdalOgrVectorAnnotation.h>
#include "../rspfPluginConstants.h"
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfEsriShapeFileInterface.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfViewInterface.h>
class rspfGdalOgrVectorAnnotation;
class rspfProjection;
class rspfMapProjection;
class rspfImageProjectionModel;
class rspfOgrGdalLayerNode;
class rspfOgrGdalFeatureNode;
class rspfAnnotationObject;
class RSPF_PLUGINS_DLL rspfOgrGdalTileSource :
   public rspfImageHandler,
   public rspfViewInterface,
   public rspfEsriShapeFileInterface
{
public:
   rspfOgrGdalTileSource();
   virtual ~rspfOgrGdalTileSource();
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
   virtual rspfObject* getView();
   virtual const rspfObject* getView()const;
   /**
    * @brief Sets the output view.
    *
    * This will transform the shape file points to the view passed in.
    *
    * @param baseObject This can be a rspfProjection or an
    * rspfImageProjectionModel.  If rspfImageProjectionModel this will
    * transform world points to line sample; then, convert any
    * line sample to the correct target rrds point.  The target rrds is
    * set via rspfImageModel::setTargetRrds.
    *
    * @param ownsItFlag If true the memory will be handled for baseObject by
    * the underlying code.
    *
    * @return True on success, false on error.
    *
    * @see rspfImageModel::setTargetRrds
    */
   virtual bool setView(rspfObject*  baseObject);
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   virtual std::multimap<long, rspfAnnotationObject*> getFeatureTable(); 
   virtual void setQuery(const rspfString& query);
   virtual void setGeometryBuffer(rspf_float64 distance, rspfUnitType type);
   virtual bool setCurrentEntry(rspf_uint32 entryIdx);
private:
   rspfRefPtr<rspfGdalOgrVectorAnnotation> theAnnotationSource;
TYPE_DATA
};
#endif
