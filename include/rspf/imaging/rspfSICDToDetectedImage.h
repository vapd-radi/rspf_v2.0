//*******************************************************************
// Copyright (C) 2010 Radiant Blue Tecnologies Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: William Watkins
// 
//*************************************************************************
#ifndef rspfSICDToDetectedImage_HEADER
#define rspfSICDToDetectedImage_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>

class RSPF_DLL rspfSICDToDetectedImage : public rspfImageSourceFilter
{
public:

   /**
    * Currently works with SICD data but will generally work for conforming Complex formats    */

	rspfSICDToDetectedImage();


   /**
    * Applies the sqrt(I^2 + Q^2) to generate the Detected SAR image.
	  For 8 bit data the input is assumed to be Magnitude and Phase (Not complex format) and 
	  so it simply returns the magnitude which is the Detected SAR image.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   /**
    * Calls base rspfImageSourceFilter::initialize() to reset connection if
    * needed then nulls out tiles.
    *
    * @note This does not initialize tiles intentionally to avoid repeated
    * deletes and news.  Tiles will be initialized by allocate() method on
    * the first getTile call after an initialize.
    */
   virtual void initialize();
//	virtual rspf_uint32 getNumberOfOutputBands() const
//	{
//		return 1;
//	}
   
   /*---------------------- PROPERTY INTERFACE ---------------------------*/
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   /*--------------------- PROPERTY INTERFACE END ------------------------*/

protected:
   virtual ~rspfSICDToDetectedImage();

   /**
    * Called to initialize tiles by first get tile when theTile or theNormTile
    * are not initialized.
    */
   void allocate();

   /**
    * Applies the brightness contrast to tile.  This method simply applies
    * brightness contrast to all bands.  Called if input does not have
    * three bands.
    */
   template <class T> void processComplexTile(T dummy, rspfImageData* tile);
   template<class T> void processAmplitudeAngleTile(T dummy, rspfImageData* tile);
	
   rspfRefPtr<rspfImageData> theTile;
   
TYPE_DATA
};

#endif /* End of "#ifndef rspfSICDToDetectedImage_HEADER" */
