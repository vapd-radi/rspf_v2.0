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
// $Id: rspf3x3ConvolutionFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspf3x3ConvolutionFilter_HEADER
#define rspf3x3ConvolutionFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>


class rspf3x3ConvolutionFilter : public rspfImageSourceFilter
{
public:
   rspf3x3ConvolutionFilter(rspfObject* owner=NULL);

   virtual rspfString getShortName()const{return rspfString("3x3 Convolution");}
   virtual rspfString getLongName()const{return rspfString("Convolves the input image with a 3x3 kernel");}

   virtual void getKernel(double kernel[3][3]);
   
   virtual void setKernel(double kernel[3][3]);

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);

   virtual void initialize();

   virtual double getNullPixelValue(rspf_uint32 band=0) const;
   virtual double getMinPixelValue(rspf_uint32 band=0)  const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)  const;

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;


   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0)const;
   
protected:
   virtual ~rspf3x3ConvolutionFilter();

   /**
    * Allocates theTile.
    */
   void allocate();

   /**
    * Clears data members theNullPixValue, theMinPixValue, and
    * theMaxPixValue.
    */
   void clearNullMinMax();

   /**
    * Computes null, min, and max considering input connection and theKernel.
    */
   void computeNullMinMax();
   
   rspfRefPtr<rspfImageData> theTile;
   double theKernel[3][3];

   vector<double> theNullPixValue;
   vector<double> theMinPixValue;
   vector<double> theMaxPixValue;   
   
   /*!
    * Convolve full means that the input data is full and has
    * no null data.  We don't have to compare for nulls here
    */
   template<class T>
   void convolveFull(T,
                     rspfRefPtr<rspfImageData> inputData,
                     rspfRefPtr<rspfImageData> outputData);
   
   /*!
    * Convolve partial means that the input data is has some
    * null data.  We will have to compare nulls
    */
   template<class T>
   void convolvePartial(T,
                        rspfRefPtr<rspfImageData> inputData,
                        rspfRefPtr<rspfImageData> outputData);
                       

TYPE_DATA
};

#endif /* #ifndef rspf3x3ConvolutionFilter_HEADER */
