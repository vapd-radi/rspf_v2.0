//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
//*******************************************************************
//  $Id: rspfDiscrete3x3HatFilter.h 11418 2007-07-27 16:24:15Z dburken $
#ifndef rspfDiscrete3x3HatFilter_HEADER
#define rspfDiscrete3x3HatFilter_HEADER
#include <rspf/imaging/rspfDiscreteConvolutionKernel.h>
class rspfDiscrete3x3HatFilter : public rspfDiscreteConvolutionKernel
{
public:
   rspfDiscrete3x3HatFilter();

   /*!
    * Will expect a data buffer of size width*height
    * and is row ordered.
    */
   virtual void convolve(const float* data,
                         double& result,
                         float nullPixel=RSPF_DEFAULT_NULL_PIX_FLOAT)const;
   
   virtual void convolveSubImage(const float* data,
                                 long dataWidth,
                                 double& result,
                                 float nullPixel=RSPF_DEFAULT_NULL_PIX_FLOAT)const;
   
   virtual void convolve(const unsigned char* data,
                         double& result,
                         rspf_uint8 nullPixel=RSPF_DEFAULT_NULL_PIX_UINT8)const;
   /*!
    * this allows you to pass a subImage to
    * the convolution engine.  It needs to know
    * the width of the buffer so it can increment
    * to the next element.
    */
   virtual void convolveSubImage(const unsigned char* data,
                                 long dataWidth,
                                 double& result,
                                 rspf_uint8 nullPixel=RSPF_DEFAULT_NULL_PIX_UINT8)const;
   
   virtual void buildConvolution(double widthPercent,
                                 double heightPercent);   
};

#endif
