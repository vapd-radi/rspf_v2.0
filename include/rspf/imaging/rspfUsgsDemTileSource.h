//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Description:
//
// Contains class declaration of rspfUsgsDemTileSource.
// The rspfUsgsDemTileSource class is derived from rspfImageHandler and
// is intended to be an image handler for USGS dem files.
//
//********************************************************************
// $Id: rspfUsgsDemTileSource.h 19640 2011-05-25 15:58:00Z oscarkramer $

#ifndef rspfUsgsDemTileSource_HEADER
#define rspfUsgsDemTileSource_HEADER

#include <rspf/imaging/rspfImageHandler.h>

class rspfImageData;
class rspfDemGrid;

class RSPF_DLL rspfUsgsDemTileSource : public rspfImageHandler
{
public:
      
   rspfUsgsDemTileSource();

   virtual rspfString getShortName() const;
   virtual rspfString getLongName()  const;

   /**
    *  Returns true if the "theImageFile can be opened.
    *
    *  If the extension is not equal to ".dem" this will attempt to look for
    *  an ".omd" file with the keyword pair "dem_type:  usgs_dem" in it.
    *
    *  Optionally users can change the scalar type from signed 16 bit to
    *  float with the keyword pair "scalar_type: rspf_float32"
    */
   virtual bool open();

   virtual rspfRefPtr<rspfImageData> getTile(const  rspfIrect& tile_rect,
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
    *  Satisfies pure virtual requirement from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfInputBands()  const;
   virtual rspf_uint32 getNumberOfOutputBands() const;
   
   /**
    *  Returns the number of lines in the image.
    *  Satisfies pure virtual requirement from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfLines(rspf_uint32 reduced_res_level = 0) const;
   
   /**
    *  Returns the number of samples in the image.  
    *  Satisfies pure virtual requirement from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfSamples(rspf_uint32 reduced_res_level = 0) const;

   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

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
    *
    * Keywords picked up by this method:
    * dem_type: usgs_dem
    * scalar_type: rspf_sint16 (default)
    * or
    * scalar_type: rspf_float32
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
   virtual rspf_uint32 getTileWidth() const;

   /**
    * Returns the height of the output tile.
    */
   virtual rspf_uint32 getTileHeight() const;

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

   virtual bool isOpen()const;
   
   /**
    * Override base-class method to make sure the internal geometry is explored before extending.
    */
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

protected:

   /**
    * @brief Initializes theGeometry from USGS DEM header.
    *
    * This is called by base rspfImageHandler::getImageGeometry if
    * theGeometry is not set.  External callers should not go through this
    * method but call "getImageGeometry" instead for efficiency reasons.
    * 
    * @return Pointer to geometry or null if header not found.
    */
   virtual rspfRefPtr<rspfImageGeometry> getInternalImageGeometry() const;
   
   virtual ~rspfUsgsDemTileSource();
   // Disallow operator= and copy constrution...
   const rspfUsgsDemTileSource& operator=(const  rspfUsgsDemTileSource& rhs);
   rspfUsgsDemTileSource(const rspfUsgsDemTileSource&);
   
   /**
    *  Returns true on success, false on error.
    */
   template <class T> bool fillBuffer(T, // dummy template variable
                                      const rspfIrect& tile_rect,
                                      const rspfIrect& clip_rect,
                                      rspfImageData* tile);

   void gatherStatistics();

   rspfDemGrid*               theDem;
   rspfRefPtr<rspfImageData> theTile;
   rspf_float64               theNullValue;
   rspf_float64               theMinHeight;
   rspf_float64               theMaxHeight;
   bool                        theFeetFlag;  // elevation units = feet
   bool                        theIsDemFlag;

   /**
    * This can be either RSPF_SINT16 or RSPF_FLOAT32.  Default = RSPF_SINT16
    * Can be changed via ".omd" file keyword:
    */
   rspfScalarType             theScalarType;
   
TYPE_DATA
};

#endif  // #ifndef rspfUsgsDemTileSource_HEADER
