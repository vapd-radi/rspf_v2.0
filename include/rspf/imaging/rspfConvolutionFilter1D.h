//*******************************************************************
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// class rspfImageGaussianFilter : tile source
// class rspfConvolutionFilter1D
// one dimensional convolution
//
// efficient for gradients & separable convolution kernels (like gaussian)
//*******************************************************************
// $Id: rspfConvolutionFilter1D.h 15766 2009-10-20 12:37:09Z gpotts $

#ifndef rspfConvolutionFilter1D_HEADER
#define rspfConvolutionFilter1D_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>

/**
 * class for vertical or horizontal convolution
 *
 * PROPERTIES:
 *  -KernelSize   : length of linear kernel (>=1)
             NOTE - you need to exit the property editor for the kernel to be resized
 *  -Kernel       : Kernel vector (line matrix)
 *  -Horizontal   : boolean, otherwise vertical kernel
 *  -CenterOffset : center pixel position in the kernel, starting at 0
 *  -StrictNoData : controls NODATA use
 *      true  : any NODATA pixel in the convolution will make the center pixel NODATA
 *      false : if center is NODATA, then output center is NODATA,
 *              other NODATA pixels are processed as if they were zero in the convolution
 */
class RSPF_DLL rspfConvolutionFilter1D : public rspfImageSourceFilter
{
public:
  /**
   * own class methods
   */
   rspfConvolutionFilter1D(rspfObject* owner=NULL);

   virtual inline const std::vector<rspf_float64>& getKernel()const { return theKernel; }
   virtual void setKernel(const std::vector<rspf_float64>& aKernel);

   virtual inline bool isHorizontal()const         { return theIsHz; }
   virtual inline void setIsHorizontal(bool aIsHz) { theIsHz = aIsHz; }

   virtual inline rspf_int32 getCenterOffset()const              { return theCenterOffset; }
   virtual inline void setCenterOffset(rspf_int32 aCenterOffset) { theCenterOffset = aCenterOffset; }
   
   virtual inline bool isStrictNoData()const         { return theStrictNoData; }
   virtual inline void setStrictNoData(bool aStrict) { theStrictNoData = aStrict; }

  /** 
   * inherited methods
   */
   virtual rspfString getShortName()const{return rspfString("1D Convolution");}
   virtual rspfString getLongName()const{return rspfString("Convolves the input image with a one dimensional convolution kernel");}

   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,rspf_uint32 resLevel=0);

   virtual void initialize();

   virtual double getNullPixelValue(rspf_uint32 band=0) const;
   virtual double getMinPixelValue(rspf_uint32 band=0)  const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)  const;

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool loadState(const rspfKeywordlist& kwl,const char* prefix = 0);
   virtual bool saveState(rspfKeywordlist& kwl,const char* prefix = 0)const;
   
protected:
   virtual ~rspfConvolutionFilter1D();

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

   //! offset of center point in the Kernel
   rspf_int32                theCenterOffset;
   rspfRefPtr<rspfImageData> theTile;
   std::vector<rspf_float64>  theKernel;
   bool                        theIsHz; //! isHorizontal convolution?
   bool                        theStrictNoData; //! strictly no NODATA values used

   std::vector<double> theNullPixValue;
   std::vector<double> theMinPixValue;
   std::vector<double> theMaxPixValue;

TYPE_DATA
};

#endif /* #ifndef rspfConvolutionFilter1D_HEADER */
