//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
// 
// Description:  Class provides an elevation image source from the elevation
// manager.
//
//********************************************************************
// $Id: rspfElevImageSource.h 17108 2010-04-15 21:08:06Z dburken $

#ifndef rspfElevImageSource_HEADER
#define rspfElevImageSource_HEADER

#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfGrect.h>

class rspfElevSource;

class rspfElevImageSource : public rspfImageSource
{
public:

   rspfElevImageSource();
   rspfElevImageSource(rspfObject* owner);
   rspfElevImageSource(rspfObject* owner,
                        const rspfGpt& tie,
                        double latSpacing,  // decimal degrees
                        double lonSpacing ,
                        rspf_uint32 numberLines,
                        rspf_uint32 numberSamples);
   rspfElevImageSource(rspfObject* owner,
                        const rspfKeywordlist& kwl,
                        const char* prefix=0);
   

   /*!
    *  Returns a pointer to a tile given an origin representing the upper left
    *  corner of the tile to grab from the image.
    *  Satisfies pure virtual from TileSource class.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const  rspfIrect& rect,
                                   rspf_uint32 resLevel=0);

   /*!
    *  Populates the keyword list with image geometry information.  This
    *  method is used to relay projection/model information to users.
    *  Returns true if geometry info is present, false if not.
    */
   virtual bool getImageGeometry(rspfKeywordlist& kwl,
                                 const char* prefix=0);
   
   /*!
    *  Returns the number of bands in the image.  Alway one for this source.
    */
   virtual rspf_uint32 getNumberOfOutputBands()const { return 1; }


   /*!
    * Returns the zero-based (relative) image rectangle for the reduced
    * resolution data set (rrds) passed in.  Note that rrds 0 is the highest
    * resolution rrds.
    */
   virtual rspfIrect getImageRectangle(rspf_uint32 reduced_res_level = 0) const; 

   /*!
    * Returns the absolute bounding rectangle of the image. The upper left
    * corner may be non-zero if this is a subimage.
    */
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel = 0) const
      {
         return getImageRectangle(resLevel);
      }
   
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

   bool canConnectMyInputTo(rspf_int32 /* inputIndex */,
                            const rspfConnectableObject* /* object */ )const
   {      
      return false;
   }
      
   virtual void initialize();

   /*!
    *  Satisfy pure virtuals from base classes...
    */
   virtual void getDecimationFactor(rspf_uint32 resLevel, rspfDpt& result) const;
   virtual void getDecimationFactors(vector<rspfDpt>& decimations) const;
   virtual rspf_uint32 getNumberOfDecimationLevels()const;
   virtual rspf_uint32 getNumberOfInputBands() const;
   virtual rspfScalarType getOutputScalarType() const;
   virtual rspf_uint32 getTileWidth() const;
   virtual rspf_uint32 getTileHeight() const;
   virtual void changeTileSize(const rspfIpt& size);

   // access methods
   virtual rspfGpt getTiePoint()         const { return theTiePoint; }
   virtual rspf_float64 getLatitudeSpacing()  const { return theLatSpacing; }
   virtual rspf_float64 getLongitudeSpacing() const { return theLonSpacing; }
   virtual rspf_uint32  getNumberOfLines(rspf_uint32 reduced_res_level = 0) const;
   virtual rspf_uint32 getNumberOfSamples(rspf_uint32 reduced_res_level = 0) const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   
   // set methods
   virtual void setTiePoint(const rspfGpt& gpt) { theTiePoint = gpt; }
   virtual void setLatitudeSpacing(rspf_float64 sp) { theLatSpacing = sp; }
   virtual void setLongitudeSpacing(rspf_float64 sp) { theLonSpacing = sp; }
   virtual void setNumberOfLines(rspf_uint32 lines) { theNumberOfLines = lines; }
   virtual void setNumberOfSamples(rspf_uint32 samps) { theNumberOfSamps = samps;}
   virtual void setMinPixelValue(rspf_float64 min_pix);
   virtual void setMaxPixelValue(rspf_float64 max_pix);
   
protected:
   virtual ~rspfElevImageSource();

   rspfElevSource*            theElevManager;
   rspfRefPtr<rspfImageData> theTile;
   rspfGpt                    theTiePoint;      // upper left tie point
   rspf_float64               theLatSpacing;    // in decimal degrees
   rspf_float64               theLonSpacing;    // in decimal degrees
   rspf_uint32                theNumberOfLines;
   rspf_uint32                theNumberOfSamps;
   
TYPE_DATA
};

#endif
