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
// $Id: rspfHistogramEqualization.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfHistogramEqualization_HEADER
#define rspfHistogramEqualization_HEADER
#include <rspf/imaging/rspfImageSourceHistogramFilter.h>

class RSPFDLLEXPORT rspfHistogramEqualization : public rspfImageSourceHistogramFilter
{
public:
   rspfHistogramEqualization();
   rspfHistogramEqualization(rspfImageSource* inputSource,
                              rspfRefPtr<rspfMultiResLevelHistogram> histogram);
   rspfHistogramEqualization(rspfImageSource* inputSource,
                              rspfRefPtr<rspfMultiResLevelHistogram> histogram,
                              bool inverseFlag);
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);

   virtual void setHistogram(rspfRefPtr<rspfMultiResLevelHistogram> histogram);
   virtual bool setHistogram(const rspfFilename& filename);

   virtual bool getInverseFlag()const;
   virtual void initialize();
   virtual void setInverseFlag(bool inverseFlag);
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;

   virtual std::ostream& print(std::ostream& out) const;

protected:
   virtual ~rspfHistogramEqualization();

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();
   
  rspfRefPtr<rspfImageData> theTile;
   /*!
    * This will be used in some of the histogram
    * based operations.
    */
   rspfRefPtr<rspfMultiResLevelHistogram> theAccumulationHistogram;

   /*!
    * Indicates if you should equalize or unequalize an input stream.
    * If the inverse flag is true then it will indicate that we should
    * assume that the input stream is already equalized and we should
    * invert based on the current accumulation histogram.
    *
    *
    * This flag is defaulted to false.
    */
   bool                         theInverseFlag;
   /*!
    * We will create a LUT for each band that directly maps
    * an input value to an equalized output.
    */
   vector<double*> theForwardLut;
   
   /*!
    * We will create a LUT for each band that directly maps
    * an input value to an un-equalized output.
    */
   vector<double*> theInverseLut;

   
  std::vector<rspf_uint32> theBandList;
   virtual void computeAccumulationHistogram();

  template<class T>
  rspfRefPtr<rspfImageData> runEqualizationAlgorithm(T dummyVariable,
                                                       rspfRefPtr<rspfImageData> tile);

   /*!
    * We will pre-compute the luts for the forward and inverse
    * equalization.  Note: the inverse is a little more tricky
    * and since we must fill the empty spots missed by the
    * inverse.  We use a linear interpolation between valid
    * values for this.
    */
   virtual void initializeLuts();
   virtual void deleteLuts();

TYPE_DATA
};
#endif /* #ifndef rspfHistogramEqualization_HEADER */
