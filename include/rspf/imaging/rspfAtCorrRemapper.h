//*******************************************************************
// Copyright (C) 2002 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Kathy Minear
//
// Description:
// Takes in DNs for any number of bands
// Converts DNs to Radiance at the satellite values Lsat
// Converts Lsat to Surface Reflectance values
//
//*************************************************************************
// $Id: rspfAtCorrRemapper.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAtCorrRemapper_HEADER
#define rspfAtCorrRemapper_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

class rspfAtCorrRemapper : public rspfImageSourceFilter
{
public:

   rspfAtCorrRemapper(rspfObject* owner = NULL,
                       rspfImageSource* inputSource =  NULL,
                       const rspfString& sensorType = "");


   virtual rspfString getShortName() const;

   virtual void initialize();

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tile_rect,
                                               rspf_uint32 resLevel=0);
   
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual ostream& print(ostream& os) const;
   friend ostream& operator << (ostream& os,  const rspfAtCorrRemapper& hr);

   vector<double> getNormMinPixelValues() const;
   vector<double> getNormMaxPixelValues() const;

   void getNormMinPixelValues(vector<double>& v) const;
   void getNormMaxPixelValues(vector<double>& v) const;

   const rspfString& getSensorType() const;

   void setSensorType(const rspfString& sensorType);

protected:
   virtual ~rspfAtCorrRemapper();

   /*!
    * Method to set unset the enable flag.
    */
   void verifyEnabled();
   virtual void interpolate(const rspfDpt& pt,
                            int band,
                            double& a,
                            double& b,
                            double& c)const;
                            
   rspfRefPtr<rspfImageData> theTile;
   double*            theSurfaceReflectance;
   bool               theUseInterpolationFlag;
   vector<double>     theMinPixelValue;
   vector<double>     theMaxPixelValue;
   vector<double>     theXaArray;
   vector<double>     theXbArray;
   vector<double>     theXcArray;
   vector<double>     theBiasArray;
   vector<double>     theGainArray;
   vector<double>     theCalCoefArray;
   vector<double>     theBandWidthArray;
   rspfString        theSensorType;

TYPE_DATA
};

#endif /* #ifndef rspfAtCorrRemapper_HEADER */
