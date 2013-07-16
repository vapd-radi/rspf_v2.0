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
// $Id: rspfIndexToRgbLutFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfIndexToRgbLutFilter_HEADER
#define rspfIndexToRgbLutFilter_HEADER
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfRgbLutDataObject.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <vector>

class rspfImageData;

/**
 * The output is an 8-bit pseudo color of the input
 * data.  This class implements a generic way to color code
 * any form of input data.  It will use theMinValue
 * theMaxValue to color code the input data.  Note
 * this class only operates on one input band since
 * it assumes the input is an index.  It also uses
 * an rspfRgbLutDataObject that is an array of color
 * data or rspfRgbVector data.  It will convert
 * the input into normalized form and then map
 * it uniformly to the color in the RGB Lut (Lookup Table).
 * <pre>
 *
 * Note:  You can tell this filter if it should use the
 *        nearest entry for the output color or linearly
 *        interpolate the entries for smooth coloring.
 *
 * Example keyword list:
 *
 * type:  rspfIndexToRgbLutFilter
 * // if the max_value and min_value is given it will use these for normalizing the
 * // input data to be between 0 and 1.  If not given it will query the min max
 * // from the input
 *
 * max_value:  2164.000000000000000
 * min_value:  -3.000000000000000
 *
 * // it owns an rspfRgbLutDataObject and has prefix value lut.
 *
 * // the value can be nearest or linear.  This describes how the
 * // color value is computed.  If it falls bewteen 2 color values
 * // for example, 1.4 it will go to entries 1 and 2 and interpolate
 * // if linear is set else it will return the color at entry 1 if
 * // nearest is set.
 * 
 * interpolation_type:  linear 
 *
 * // the rest of the keywords is the lookup table.
 * // NOTE: you do not have to have the same number of entries as you do
 * //       input range indices.  Just know that the lowest value is mapped
 * //       to 0 and the highest value is mapped to number of entires -1.
 *
 * If you want to load from a file then just do:
 *
 * lut.lut_file: <full path to lut file>
 *
 * If you have the lut table in line then it must look like this:
 *
 * lut.entry0.b:  204
 * lut.entry0.g:  102
 * lut.entry0.r:  1
 * lut.entry1.b:  255
 * lut.entry1.g:  204
 * lut.entry1.r:  153
 * lut.entry2.b:  204
 * lut.entry2.g:  204
 * lut.entry2.r:  51
 * lut.entry3.b:  255
 * lut.entry3.g:  1
 * lut.entry3.r:  1
 * lut.entry4.b:  204
 * lut.entry4.g:  1
 * lut.entry4.r:  204
 * lut.entry5.b:  204
 * lut.entry5.g:  51
 * lut.entry5.r:  255
 * lut.entry6.b:  51
 * lut.entry6.g:  1
 * lut.entry6.r:  102
 * lut.entry7.b:  255
 * lut.entry7.g:  255
 * lut.entry7.r:  255
 * lut.number_of_entries:  8
 * lut.type:  rspfRgbLutDataObject
 *
 *
 * 
 * </pre>
 */
class RSPF_DLL rspfIndexToRgbLutFilter : public rspfImageSourceFilter
{
public:
   enum rspfIndexToRgbLutFilterInterpolationType
   {
      rspfIndexToRgbLutFilter_NEAREST = 0,
      rspfIndexToRgbLutFilter_LINEAR  = 1
   };
   /**
    * Initializes the min value to 0 and the max value to 4000.
    */
   rspfIndexToRgbLutFilter();
   rspfIndexToRgbLutFilter(rspfImageSource* inputSource,
                           const rspfRgbLutDataObject& rgbLut,
                           double minValue,
                           double maxValue,
                           rspfIndexToRgbLutFilterInterpolationType interpolationType);
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                               rspf_uint32 resLevel=0);
   
   rspf_uint32 getNumberOfOutputBands() const;
   
   virtual rspfScalarType getOutputScalarType() const;
   
   void           setLut(rspfRgbLutDataObject& lut);
   void           setLut(const rspfFilename& file);
   
   rspfFilename  getLutFile()const;
   
   double         getMinValue()const;
   double         getMaxValue()const;
   
   void           setMinValue(double value);
   void           setMaxValue(double value);
   
   virtual double getNullPixelValue()const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   rspfIndexToRgbLutFilterInterpolationType getInterpolationType()const;
   void setInterpolationType(rspfIndexToRgbLutFilterInterpolationType type);

   virtual void initialize();

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   /**
    * Saves the state of this object.
    *
    * Keywords are:    min_value 
    *                  max_value
    *                  lut.<lut keywords> -> look at the
    *                  rspfRgbLutDataObject for the keywords that
    *                                         follow after the "." .
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;

   /**
    * Loads the state of this object.
    *
    * looks for keywords:
    *                  min_value 
    *                  max_value
    *                  lut.<lut keywords> -> look at the
    *                  rspfRgbLutDataObject for the keywords that
    *                                         follow after the "." .
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
protected:
   virtual ~rspfIndexToRgbLutFilter();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
   rspfRefPtr<rspfRgbLutDataObject> theLut;
   double theMinValue;
   double theMaxValue;
   double theMinMaxDeltaLength;
   bool theMinValueOverride;
   bool theMaxValueOverride;
      
   rspfIndexToRgbLutFilterInterpolationType theInterpolationType;
   rspfRefPtr<rspfImageData> theTile;
   rspfFilename   theLutFile;
   
   virtual void normalizeValue(double value,
                               double& normalizedValue);
   virtual void getColorNormIndex(double index, rspfRgbVector& result);
   virtual void getColor(double index, rspfRgbVector& result);
   virtual rspfRefPtr<rspfImageData> convertInputTile(const rspfRefPtr<rspfImageData>& tile);
   /**
    * Precomputed from the theMaxValue - theMinValue
    */
   
TYPE_DATA
};

#endif /* #ifndef rspfIndexToRgbLutFilter_HEADER */
