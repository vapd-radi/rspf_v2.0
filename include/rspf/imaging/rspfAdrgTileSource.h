//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
// 
// Author: Ken Melero
// 
// Description: This class give the capability to access tiles from an
//              ADRG file.
//
//********************************************************************
// $Id: rspfAdrgTileSource.h 17932 2010-08-19 20:34:35Z dburken $

#ifndef rspfAdrgTileSource_HEADER
#define rspfAdrgTileSource_HEADER

#include <fstream>

#include <rspf/imaging/rspfImageHandler.h>

class rspfImageData;
class rspfAdrgHeader;

class RSPF_DLL rspfAdrgTileSource : public rspfImageHandler
{
public:
   
   enum
   {
      ADRG_TILE_WIDTH  = 128,
      ADRG_TILE_HEIGHT = 128,
      ADRG_TILE_SIZE   = 16384, // 128 * 128,
      BAND_OFFSET      = 16384, // 8 * 2048,
      NUMBER_OF_BANDS  = 3
   };
   
   enum ADRG_COLOR
   {
      RED_BAND    = 0,  // the ADRG rgb value
      GREEN_BAND  = 1,
      BLUE_BAND   = 2
   };
      
   rspfAdrgTileSource();

   virtual rspfString getShortName()const;
   virtual rspfString getLongName()const;

   /**
    *  @return Returns true on success, false on error.
    *
    *  @note This method relies on the data member rspfImageData::theImageFile
    *  being set.  Callers should do a "setFilename" prior to calling this
    *  method or use the rspfImageHandler::open that takes a file name and an
    *  entry index.
    */
   virtual bool open();

   /**
    *  Returns a pointer to a tile given an origin representing the upper
    *  left corner of the tile to grab from the image.
    *  Satisfies pure virtual requirement from TileSource class.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
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
   virtual rspf_uint32 getNumberOfInputBands() const;
   
   virtual rspf_uint32 getNumberOfOutputBands()const;

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
    *  Returns the image geometry object associated with this tile source or NULL if non defined.
    */
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   
   /**
    * Returns the output pixel type of the tile source.
    */
   virtual rspfScalarType getOutputScalarType() const;

   /**
    * Returns the width of the output tile.
    */
   virtual rspf_uint32 getTileWidth() const;

   virtual void close();
   /**
    * Returns the height of the output tile.
    */
   virtual rspf_uint32 getTileHeight() const;

   virtual bool isOpen()const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileWidth which
    * returns the output tile width, which can be different than the
    * internal image tile width on disk.
    */
   virtual rspf_uint32 getImageTileWidth() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileHeight which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    */
   virtual rspf_uint32 getImageTileHeight() const;

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
   
protected:
   virtual ~rspfAdrgTileSource();
   
   /**
    *  Adjust point to even 128 boundary.  Assumes 0,0 origin.
    */
   void adjustToStartOfTile(rspfIpt& pt) const;
   
   /**
    *  Returns true on success, false on error.
    */
   bool fillBuffer(const rspfIrect& tile_rect,
                   const rspfIrect& clip_rect,
                   rspfImageData* tile);
   
   rspfRefPtr<rspfImageData>  m_Tile;
   rspf_uint8*                 m_TileBuffer;
   std::ifstream                m_FileStr;
   rspfAdrgHeader*             m_AdrgHeader;

TYPE_DATA
};

#endif
