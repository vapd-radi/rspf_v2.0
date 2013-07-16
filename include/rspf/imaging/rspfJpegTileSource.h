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
// Contains class declaration for JpegTileSource.
// JpegTileSource is derived from ImageHandler which is derived from
// TileSource.
//*******************************************************************
//  $Id: rspfJpegTileSource.h 22117 2013-01-18 21:04:23Z dburken $

#ifndef rspfJpegTileSource_HEADER
#define rspfJpegTileSource_HEADER

#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfAppFixedTileCache.h>

class  rspfImageData;

class RSPF_DLL rspfJpegTileSource : public rspfImageHandler
{
public:

   rspfJpegTileSource();

   rspfJpegTileSource(const rspfKeywordlist& kwl,
                       const char* prefix=0);

   rspfJpegTileSource(const char* jpeg_file);


   virtual rspfString getShortName() const;
   virtual rspfString getLongName()  const;

   /**
    *  Returns a pointer to a tile given an origin representing the upper
    *  left corner of the tile to grab from the image.
    *  Satisfies pure virtual from TileSource class.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const  rspfIrect& rect,
                                               rspf_uint32 resLevel=0);

   /**
    * Method to get a tile.   
    *
    * @param result The tile to stuff.  Note The requested rectangle in full
    * image space and bands should be set in the result tile prior to
    * passing.  It will be an error if:
    * result.getNumberOfBands() != this->getNumberOfOutputBands()
    *
    * @return true on success false on error.  If return is false, result
    *  is undefined so caller should handle appropriately with makeBlank or
    * whatever.
    */
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
     *  Returns the number of lines in the image.
     *  Satisfies pure virtual from ImageHandler class.
     */
   virtual rspf_uint32 getNumberOfLines(rspf_uint32 reduced_res_level = 0) const;

   /**
    *  Returns the number of samples in the image.
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfSamples(rspf_uint32 reduced_res_level = 0) const;

   /**
    * Returns the zero based image rectangle for the reduced resolution data
    * set (rrds) passed in.  Note that rrds 0 is the highest resolution rrds.
    */
   virtual rspfIrect getImageRectangle(rspf_uint32 reduced_res_level = 0) const;

   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Returns the output pixel type of the tile source.
    */
   virtual rspfScalarType getOutputScalarType() const;

   /**
    * Returns the width of the output tile.
    */
   virtual rspf_uint32    getTileWidth() const;

   /**
    * Returns the height of the output tile.
    */
   virtual rspf_uint32    getTileHeight() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual rspf_uint32 getImageTileWidth() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual rspf_uint32 getImageTileHeight() const;

   bool isValidRLevel(rspf_uint32 reduced_res_level) const;


   bool isOpen()const;
   /**
    *  Returns true if the image_file can be opened and is a valid tiff file.
    */
   bool open(const rspfFilename& jpeg_file);
   virtual void close();

   /**
    * @brief Gets a property for matching name.
    * @param name The name of the property to get.
    * @return Returns property matching "name".
    */
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   
   /**
    * @brief Gets a list of property names available.
    * @param propertyNames The list to push back names to.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   /**
    * Returns the image geometry object associated with this tile source or
    * NULL if non defined.  The geometry contains full-to-local image
    * transform as well as projection (image-to-world).
    */
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   
protected:

   /**
    * @param Method to get geometry from the xml file or internal geotiff
    * tags.
    */
   virtual rspfRefPtr<rspfImageGeometry> getInternalImageGeometry() const; 
   
   virtual ~rspfJpegTileSource();
	class PrivateData;
   /**
    *  Returns true if no errors initializing object.
    *
    *  Notes:
    *  - Callers of this method must ensure "theTiffPtr" data member
    *    is initialized.
    *  - This method was added to consolidate object initialization code
    *    between constructor and public open method.
    */
   virtual bool open();

   void allocate();
   void destroy();
   void restart();

   /**
    * @note this method assumes that setImageRectangle has been called on
    * theTile.
    */
   void fillTile(const rspfIrect& clip_rect, rspfImageData* tile);

   rspfRefPtr<rspfImageData>  theTile;
   rspfRefPtr<rspfImageData>  theCacheTile;
   rspf_uint8*                 theLineBuffer;
   FILE*                        theFilePtr;
   rspfIrect                   theBufferRect;
   rspfIrect                   theImageRect;
   rspf_uint32                 theNumberOfBands;
   rspfIpt                     theCacheSize;

   PrivateData*                 thePrivateData;

   rspfAppFixedTileCache::rspfAppFixedCacheId theCacheId;

TYPE_DATA
};

#endif
