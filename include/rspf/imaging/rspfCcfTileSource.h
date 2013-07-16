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
// Contains class declaration for CcfTileSource.
//
// A ccf (Chip Chunk Format) is a double tiled format.  Image data is
// arranged in chips and chunks.  Each chip is 32 x 32 pixels.  Each chunk
// is 8 x 8 chips for a single band image.  Bands are interleaved by chip
// so a rgb image has a red chip a green chip, then a blue chip which
// would make the chunk 24 x 8 chips.
//
//*******************************************************************
//  $Id: rspfCcfTileSource.h 16075 2009-12-10 15:46:43Z gpotts $

#ifndef rspfCcfTileSource_HEADER
#define rspfCcfTileSource_HEADER

#include <fstream>

#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfCcfHead.h>

class  rspfImageData;

class RSPF_DLL rspfCcfTileSource : public rspfImageHandler
{
public:

   rspfCcfTileSource();
   
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
   
   virtual void close();
   
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
   virtual rspf_uint32 getNumberOfOutputBands()const;

   /**
     *  Returns the number of lines in the image.
     *  Satisfies pure virtual from ImageHandler class.
     */
   virtual rspf_uint32 getNumberOfLines(
      rspf_uint32 reduced_res_level = 0) const;
   
   /**
    *  Returns the number of samples in the image.  
    *  Satisfies pure virtual from ImageHandler class.
    */
   virtual rspf_uint32 getNumberOfSamples(
      rspf_uint32 reduced_res_level = 0) const;

   /**
    *  Returns the number of reduced resolution data sets (rrds).
    *  Note:  The full res image is counted as a data set so an image with no
    *         reduced resolution data set will have a count of one.
    */
   virtual rspf_uint32 getNumberOfDecimationLevels() const;
   
   /**
    *  Returns the zero based image rectangle for the reduced resolution data
    *  set (rrds) passed in.  Note that rrds 0 is the highest resolution rrds.
    */
   virtual rspfIrect getImageRectangle(
      rspf_uint32 reduced_res_level = 0) const;
   
   /**
    *  Set the output band list.  Use to set the number and order of output
    *  bands.  Will set an error if out of range.
    */
   bool setOutputBandList(const vector<rspf_uint32>& outputBandList);

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
    * @brief Gets a property for matching name.
    * @param name The name of the property to get.
    * @return Returns property matching "name".
    */
   virtual rspfRefPtr<rspfProperty> getProperty(
      const rspfString& name)const;
   
   /**
    * @brief Gets a list of property names available.
    * @param propertyNames The list to push back names to.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
	
   
protected:
   virtual ~rspfCcfTileSource();

   void initVerticesFromHeader();
   
   // Disallow these...
   rspfCcfTileSource(const rspfCcfTileSource& source);
   rspfCcfTileSource& operator=(const rspfCcfTileSource& source); 
   
   /**
    *  Returns true on success, false on error.
    */
   bool fillBuffer(const  rspfIrect& tile_rect,
                   const  rspfIrect& clip_rect,
                   const  rspfIrect& image_rect,
                   rspf_uint32 reduced_res_level,
                   rspfImageData* tile);

   /**
    *  Returns true on success, false on error.
    */
   bool fillUshortBuffer(const  rspfIrect& tile_rect,
                         const  rspfIrect& clip_rect,
                         const  rspfIrect& image_rect,
                         rspf_uint32 reduced_res_level,
                         rspfImageData* tile);
   
   /**
    *  Returns true on success, false on error.
    */
   bool fillUcharBuffer(const  rspfIrect& tile_rect,
                        const  rspfIrect& clip_rect,
                        const  rspfIrect& image_rect,
                        rspf_uint32 reduced_res_level,
                        rspfImageData* tile);

   /**
    *  Adjust point to even 256 boundary.  Assumes 0,0 origin.
    */
   void adjustToStartOfChunk(rspfIpt& pt) const;

   /**
    *  Adjust point to even 32 boundary.  Assumes 0,0 origin.
    */
   void adjustToStartOfChip(rspfIpt& pt) const;

   bool isEcgGeom(const rspfFilename& filename)const;

   rspfCcfHead                theCcfHead;
   rspfRefPtr<rspfImageData> theTile;
   rspf_uint8*                theChipBuffer;
   ifstream*                   theFileStr;
   vector<rspf_uint32>        theOutputBandList;
   rspf_int32                 theByteOrder;


TYPE_DATA
};
   
#endif
