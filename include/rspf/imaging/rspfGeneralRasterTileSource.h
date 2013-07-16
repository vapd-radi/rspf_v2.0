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
// Contains class declaration for rspfGeneralRasterTileSource.
// rspfGeneralRasterTileSource is derived from ImageHandler which is
// derived from rspfTileSource.
//*******************************************************************
//  $Id: rspfGeneralRasterTileSource.h 21631 2012-09-06 18:10:55Z dburken $

#ifndef rspfGeneralRasterTileSource_HEADER
#define rspfGeneralRasterTileSource_HEADER 1


#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfIoStream.h>
#include <rspf/imaging/rspfGeneralRasterInfo.h>
#include <vector>
  

class  rspfImageData;

class RSPF_DLL rspfGeneralRasterTileSource : public rspfImageHandler
{
public:

   rspfGeneralRasterTileSource();

   virtual rspfString getShortName()const;
   virtual rspfString getLongName()const;
   
   /**
    *  Returns a pointer to a tile given an origin representing the upper
    *  left corner of the tile to grab from the image.
    *  Satisfies pure virtual from TileSource class.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tile_rect,
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
   virtual rspf_uint32 getNumberOfOutputBands() const;
   
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
    * Note:  This handler will always return 0 for this method.
    */
   virtual rspf_uint32 getImageTileWidth() const;

   /**
    * Returns the tile width of the image or 0 if the image is not tiled.
    * Note: this is not the same as the rspfImageSource::getTileWidth which
    * returns the output tile width which can be different than the internal
    * image tile width on disk.
    * Note:  This handler will always return 0 for this method.
    */
   virtual rspf_uint32 getImageTileHeight() const;   
   
   bool isValidRLevel(rspf_uint32 reduced_res_level) const;
   
   virtual void close();
   virtual bool isOpen() const;
   virtual bool open();
   virtual bool open(const rspfGeneralRasterInfo& info);
   
   /**
    * Override base getXXXXPixValue methods since the null/min/max can be set
    * to something different.  Currently returns the same value for all bands.
    */
   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   /**
    * @brief Indicates whether or not the image handler can control output
    * band selection via the setOutputBandList method.
    *
    * Overrides: rspfImageHandler::isBandSelector
    *
    * @return true
    */
   virtual bool isBandSelector() const;

   /**
    * @brief If the image handler "isBandSeletor()" then the band selection
    * of the output chip can be controlled.
    *
    * Overrides: rspfImageHandler::setOutputBandList
    * 
    * @return true on success, false on error.
    */
   virtual bool setOutputBandList(const std::vector<rspf_uint32>& band_list);

   /** @brief Initializes bandList to the zero based order of output bands. */
   virtual void getOutputBandList(std::vector<rspf_uint32>& bandList) const;

protected:
   virtual ~rspfGeneralRasterTileSource();
   /**
    *  Methods return true on succes false on error.
    */
   virtual bool fillBuffer(const rspfIpt& origin, const rspfIpt& size);
   virtual bool fillBIP(const rspfIpt& origin, const rspfIpt& size);
   virtual bool fillBIL(const rspfIpt& origin, const rspfIpt& size);
   virtual bool fillBSQ(const rspfIpt& origin, const rspfIpt& size);
   virtual bool fillBsqMultiFile(const rspfIpt& origin, const rspfIpt& size);

   virtual rspfKeywordlist getHdrInfo(rspfFilename hdrFile);
   virtual rspfKeywordlist getXmlInfo(rspfFilename xmlFile);

   bool initializeHandler();
   
   rspfRefPtr<rspfImageData>              m_tile;
   rspf_uint8*                             m_buffer;
   rspf_uint8*                             m_lineBuffer;
   rspfInterleaveType                      m_bufferInterleave;
   std::vector<rspfRefPtr<rspfIFStream> > m_fileStrList;
   // std::vector< std::ifstream* >            m_fileStrList;   
   rspfGeneralRasterInfo                   m_rasterInfo;
   rspfIrect                               m_bufferRect;
   bool                                     m_swapBytesFlag;
   rspf_uint32                             m_bufferSizeInPixels;
   std::vector<rspf_uint32>                m_outputBandList;

private:
   
   /** @brief Allocates m_tile. */
   void allocateTile();

   /** @brief Allocates m_buffer */
   void allocateBuffer( const rspfImageData* tile );
   

TYPE_DATA
};

#endif /* #ifndef rspfGeneralRasterTileSource_HEADER */
