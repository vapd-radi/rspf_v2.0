//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfNBandToIndexFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfNBandToIndexFilter_HEADER
#define rspfNBandToIndexFilter_HEADER
#include <rspf/base/rspfNBandLutDataObject.h>
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
 * type:  rspfNBandToIndexFilter
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
 * lut.type:  rspfNBandLutDataObject
 *
 * </pre>
 */
class RSPF_DLL rspfNBandToIndexFilter : public rspfImageSourceFilter
{
public:
   /*!
    * Initializes the min value to 0 and the max value to 4000.
    */
   rspfNBandToIndexFilter();
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                               rspf_uint32 resLevel=0);
   
   virtual rspf_uint32 getNumberOfOutputBands() const;
   
   virtual rspfScalarType getOutputScalarType() const;
   
   void setLut(rspfNBandLutDataObject& lut);

   void setLut(const rspfFilename& file);

   virtual bool isSourceEnabled()const;
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

   /**
    * Will set the property whose name matches the argument
    * "property->getName()".
    *
    * @param property Object containing property to set.
    */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);

   /**
    * @param name Name of property to return.
    * 
    * @returns A pointer to a property object which matches "name".  Returns
    * NULL if no match is found.
    */
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;

   /**
    * Pushes this's names onto the list of property names.
    *
    * @param propertyNames array to add this's property names to.
    */
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
protected:
   virtual ~rspfNBandToIndexFilter();
   void allocate();

   
   virtual rspfRefPtr<rspfImageData> convertInputTile(rspfImageData* inputTile);

   template <class T> rspfRefPtr<rspfImageData> convertOutputTileTemplate(T inputDummy,
                                                                            rspfImageData* inputTile);
   template <class T1, class T2> rspfRefPtr<rspfImageData> convertInputTileToOutputTemplate(T1 inputDummy,
                                                                                              T2 outputDummy,
                                                                                              rspfImageData* inputTile);
   
   rspfRefPtr<rspfNBandLutDataObject> theLut;
   rspfRefPtr<rspfImageData>          theTile;
   rspfFilename                        theLutFilename;
   bool                                 theKeepQuantizedValueFlag;
   std::vector<rspfNBandLutDataObject::LUT_ENTRY_TYPE> theMinValues;
   std::vector<rspfNBandLutDataObject::LUT_ENTRY_TYPE> theMaxValues;
   std::vector<rspfNBandLutDataObject::LUT_ENTRY_TYPE> theNullValues;
TYPE_DATA
};

#endif
