//*******************************************************************
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
// class rspfImageGaussianFilter : tile source
//*******************************************************************
// $Id: rspfImageGaussianFilter.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfImageGaussianFilter_HEADER
#define rspfImageGaussianFilter_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfConvolutionFilter1D.h>

/**
 * class for symmetric Gaussian filtering
 * implemented as two separable horizontal/vertical gaussian filters
 *
 * PROPERTIES:
 * -GaussStd is the standard deviation of the gaussian
 *  (filter widths are equal to 2*floor(2.5*GaussStd+0.5)+1)
 * -strictNoData selects the behaviour for NODATA pixels
 *   true  : any NODATA pixels in the convolution will Nullify the center pixel
 *   false : center pixel will be NODATA only if it was NODATA before 
 *     other NODATA pixels are processed as zero in the convolution calculation
 */
class RSPF_DLL rspfImageGaussianFilter : public rspfImageSourceFilter
{
public:
  /** 
   * own methods
   */
   rspfImageGaussianFilter();

   inline rspf_float64 getGaussStd()const { return theGaussStd; }
   void setGaussStd(const rspf_float64& v);

   inline bool isStrictNoData()const { return theStrictNoData; }
   void setStrictNoData(bool aStrict);

  /** 
   * inherited methods
   */
   virtual void initialize();
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect &tileRect,rspf_uint32 resLevel=0);

   virtual void connectInputEvent(rspfConnectionEvent &event);
   virtual void disconnectInputEvent(rspfConnectionEvent &event);

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool loadState(const rspfKeywordlist& kwl,const char* prefix = 0);   
   virtual bool saveState(rspfKeywordlist& kwl,const char* prefix = 0)const;
   
protected:
   virtual ~rspfImageGaussianFilter();
  /**
   * protected methods
   */
   void initializeProcesses();
   void updateKernels();

  /**
   * parameters
   */
   rspf_float64 theGaussStd;
   bool          theStrictNoData;

  /**
   * subprocesses
   */
   rspfRefPtr<rspfConvolutionFilter1D> theHF; //horizontal filter
   rspfRefPtr<rspfConvolutionFilter1D> theVF; //vertical filter

TYPE_DATA
};

#endif /* #ifndef rspfImageGaussianFilter_HEADER */
