//----------------------------------------------------------------------------
// Copyright (C) 2004 Garrett Potts, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// $Id: rspfVpfTileSource.h 17932 2010-08-19 20:34:35Z dburken $
//----------------------------------------------------------------------------
#ifndef rspfVpfTileSource_HEADER
#define rspfVpfTileSource_HEADER

#include <rspf/imaging/rspfVpfAnnotationSource.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfViewInterface.h>


class RSPFDLLEXPORT rspfVpfTileSource : public rspfImageHandler,
                                          public rspfViewInterface
{
public:
   
   rspfVpfTileSource();

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
    * Note:  The full res image is counted as a data set so an image with no
    *        reduced resolution data set will have a count of one.
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
                          const char* prefix);
   
   //! Returns the image geometry object associated with this tile source or NULL if non defined.
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

   virtual rspfObject* getView();

   virtual const rspfObject* getView()const;
   
   virtual bool setView(rspfObject*  baseObject);

   void getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& featureList);
  
   void setAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& featureList);

   void transformObjects();

   void computeBoundingRect();

protected:
   virtual ~rspfVpfTileSource();
   rspfRefPtr<rspfVpfAnnotationSource> m_AnnotationSource;
   
   
TYPE_DATA
};

#endif
