//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfRgbToIndexFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfRgbToIndexFilter_HEADER
#define rspfRgbToIndexFilter_HEADER
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfRgbLutDataObject.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <vector>

class rspfImageData;

/*!
 * It will map the input data to an 8-bit index table.  The table
 * <pre>
 *
 * Example keyword list:
 *
 * type:  rspfRgbToIndexFilter
 *
 * If you want to load from a file then just do:
 *
 * lut.filename: <full path to lut file>
 *
 * If you have the lut table in line then it must look like this:
 *
 * lut.entry0:  204 102 1
 * lut.entry1:  255 204 153
 * lut.entry2:  51 204 204
 * lut.number_of_entries:  3
 * lut.type:  rspfRgbLutDataObject
 *
 * </pre>
 */
class rspfRgbToIndexFilter : public rspfImageSourceFilter
{
public:
   /*!
    * Initializes the min value to 0 and the max value to 4000.
    */
   rspfRgbToIndexFilter();
   rspfRgbToIndexFilter(rspfImageSource* inputSource,
                           const rspfRgbLutDataObject& rgbLut);
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                               rspf_uint32 resLevel=0);
   
   virtual rspf_uint32 getNumberOfOutputBands() const;
   
   virtual rspfScalarType getOutputScalarType() const;
   
   void setLut(rspfRgbLutDataObject& lut);

   void setLut(const rspfFilename& file);

   virtual void disableSource();
   
   virtual double getNullPixelValue(rspf_uint32 band=0)const;

   virtual double getMinPixelValue(rspf_uint32 band=0)const;

   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   
   virtual void initialize();

   /*!
    * Saves the state of this object.
    */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;

   /*!
    * Loads the state of this object.
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
protected:
   virtual ~rspfRgbToIndexFilter();
   void allocate();
   
   virtual rspfRefPtr<rspfImageData> convertInputTile(rspfRefPtr<rspfImageData>& tile);

   rspfRefPtr<rspfRgbLutDataObject>       theLut;
   rspfRefPtr<rspfImageData> theTile;
   
TYPE_DATA
};

#endif
