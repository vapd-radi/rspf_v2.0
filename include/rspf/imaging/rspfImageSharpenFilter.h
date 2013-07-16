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
// $Id: rspfImageSharpenFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfImageSharpenFilter_HEADER
#define rspfImageSharpenFilter_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfConvolutionSource.h>
class RSPFDLLEXPORT rspfImageSharpenFilter : public rspfImageSourceFilter
{
public:
   rspfImageSharpenFilter(rspfObject* owner=NULL);

   virtual rspfString getShortName()const;
   virtual rspfString getLongName()const;
   
   rspf_uint32 getWidth()const;
   rspf_float64 getSigma()const;
   void setWidthAndSigma(rspf_uint32 w, rspf_float64 sigma);
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   
   virtual void initialize();
   
   virtual void connectInputEvent(rspfConnectionEvent &event);
   virtual void disconnectInputEvent(rspfConnectionEvent &event);
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
protected:
   virtual ~rspfImageSharpenFilter();
   inline double laplacianOfGaussian(double x, double y, double sigma)
   {
      double r2 = x*x+y*y;
      double sigma2 = sigma*sigma;
      return ((1.0/(M_PI*sigma2*sigma2))*
              (1.0-r2/(2.0*sigma2))*
              (exp(-r2/(2.0*sigma2))));
      
   }
   
   void buildConvolutionMatrix();
   

   /*!
    * Convolve full means that the input data is full and has
    * no null data.  We don't have to compare for nulls here
    */
   template<class T>
   void sharpenFull(T,
                    const rspfRefPtr<rspfImageData>& inputData,
                    rspfRefPtr<rspfImageData>& outputData);
   
   /*!
    * Convolve partial means that the input data is has some
    * null data.  We will have to compare nulls
    */
   template<class T>
   void sharpenPartial(T,
                       const rspfRefPtr<rspfImageData>& inputData,
                       rspfRefPtr<rspfImageData>& outputData);

   rspfRefPtr<rspfConvolutionSource> theConvolutionSource;
   rspf_uint32 theWidth;
   rspf_float64 theSigma;
TYPE_DATA
};

#endif /* #ifndef rspfImageSharpenFilter_HEADER */
