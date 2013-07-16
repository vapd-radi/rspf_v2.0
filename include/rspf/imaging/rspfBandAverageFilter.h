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
// $Id: rspfBandAverageFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfBandAverageFilter_HEADER
#define rspfBandAverageFilter_HEADER
#include <vector>
#include <rspf/imaging/rspfImageSourceFilter.h>

/**
 * This filter outputs a single band that is the weighted average of all the
 * input bands retrieved from the getTile.  This filter will work with any
 * input scalar type.  Note the RSPFDLLEXPORT is used for the different
 * compilers that require importing and exporting symbols for shared library
 * support.
 */
class RSPFDLLEXPORT rspfBandAverageFilter : public rspfImageSourceFilter
{
public:
   rspfBandAverageFilter();
   rspfBandAverageFilter(rspfImageSource* input,
                          const std::vector<double>& weights);

   /**
    * The data object argument is deprecated and should not be used.
    * eventually we will remove it from the base classes.  This method will
    * return 1 if our source is enabled else it calls the base class
    * getNumberOfOutputBands.
    */
   virtual rspf_uint32 getNumberOfOutputBands()const;
   
   /**
    * Since this filter is going to operate in native pixel type we will use
    * a tmeplate method to do this.  It will check the scalar tpe of the input
    * and then cat it to set the template variabel to be used within the
    * template method averageInput.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& rect,
                                               rspf_uint32 resLevel=0);

   /**
    * Since we are merging all bands we will use the null pix of the
    * first input band as our null pix.  If we are disabled we just
    * return the input s null
    */
   virtual double getNullPixelValue(rspf_uint32 band=0)const;

   /**
    * We will use the min of all bands as our min.  If we aredisabled
    * we just return the inputs min
    */
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   
   /**
    * We will use the max of all bands as our max.  If we aredisabled
    * we just return the inputs max
    */
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;

   /**
    * simple access method to get a weight
    */
   double getWeight(rspf_uint32 band)const;

   /**
    * Sinple access method to set the weight of a band,
    * We make sure that it is a positive weight
    */
   void setWeight(rspf_uint32 band, double weight);

   /**
    * allocates its tile buffer
    */
   virtual void initialize();

   /**
    * loads its weights to a keywordlist
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   /**
    * Saves its weights to a keywordlist
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

protected:
   virtual ~rspfBandAverageFilter();
   
  rspfRefPtr<rspfImageData> theTile;
   std::vector<double>         theWeights;

   void checkWeights();
   
   template<class T> void averageInput(
      T dummyVariable, // used only for template type, value ignored
      rspfRefPtr<rspfImageData> inputDataObject);

// for Runtime Type Info (RTTI)
TYPE_DATA
};

#endif
