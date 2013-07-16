//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
// 
//*************************************************************************
// $Id: rspfBrightnessContrastSource.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfBrightnessContrastSource_HEADER
#define rspfBrightnessContrastSource_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPF_DLL rspfBrightnessContrastSource : public rspfImageSourceFilter
{
public:

   /**
    * Currently works in hsi space so must be one or three bands.
    *
    * Todo: Make work with nband.
    */
   rspfBrightnessContrastSource();


   /**
    * Will apply the algorithm:
    *       i*c + b
    *           where b is brightnes
    *                 c is contrast
    *                 i is the input value.
    *
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);

   /**
    * @param brightness Value between -1.0 and +1.0 with zero being no
    * brightness offset.
    *
    * @param contrast Value between 0.0 and 2.0 with 1.0 being no contrast
    * adjustment.
    */
   virtual void setBrightnessContrast(rspf_float64 brightness,
                                      rspf_float64 contrast);
   
   /**
    * @param brightness Value between -1.0 and +1.0 with zero being no
    * brightness offset.
    */
   virtual void setBrightness(rspf_float64 brightness);

   /**
    * @param contrast Value between 0.0 and 2.0 with 1.0 being no contrast
    * adjustment.
    */
   virtual void setContrast(rspf_float64 contrast);

   /** @return Brightness value between -1.0 and +1.0. */
   virtual rspf_float64 getBrightness()const;

   /** @return Contrast value between 0.0 and +2.0. */
   virtual rspf_float64 getContrast()const;

   /**
    * Calls base rspfImageSourceFilter::initialize() to reset connection if
    * needed then nulls out tiles.
    *
    * @note This does not initialize tiles intentionally to avoid repeated
    * deletes and news.  Tiles will be initialized by allocate() method on
    * the first getTile call after an initialize.
    */
   virtual void initialize();
   
   /*---------------------- PROPERTY INTERFACE ---------------------------*/
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   /*--------------------- PROPERTY INTERFACE END ------------------------*/

   /**
    * Keywords searched for by this method.
    *
    * brightness: 0.0
    *
    * contrast:  1.0
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   /**
    * Keywords saved to list by this method.
    *
    * brightness: 0.0
    *
    * contrast:  1.0
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
protected:
   virtual ~rspfBrightnessContrastSource();

   /**
    * Called to initialize tiles by first get tile when theTile or theNormTile
    * are not initialized.
    */
   void allocate();

   /**
    * Applies the brightness contrast to tile.  This method works in hsi
    * space on the intensity channel; thus, preserving hue and saturation
    * of an rgb pixel.  Called if input has three bands.
    */
   void processRgbTile();

   /**
    * Applies the brightness contrast to tile.  This method simply applies
    * brightness contrast to all bands.  Called if input does not have
    * three bands.
    */
   void processNBandTile();
   
   
   rspf_float64               theBrightness;  
   rspf_float64               theContrast;  
   rspfRefPtr<rspfImageData> theTile;
   rspfRefPtr<rspfImageData> theNormTile;
   
TYPE_DATA
};

#endif /* End of "#ifndef rspfBrightnessContrastSource_HEADER" */
