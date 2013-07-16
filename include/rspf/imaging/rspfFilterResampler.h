//*******************************************************************
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
// Contributor:
//         David A. Horner (DAH) http://dave.thehorners.com
//
//*************************************************************************
// $Id: rspfFilterResampler.h 9696 2006-10-11 17:51:26Z gpotts $
#ifndef rspfFilterResampler_HEADER
#define rspfFilterResampler_HEADER

#include <vector>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfFilter.h>
#include <rspf/imaging/rspfFilterTable.h>

class RSPFDLLEXPORT rspfFilterResampler
{
public:
   enum rspfFilterResamplerType
   {
      rspfFilterResampler_NEAREST_NEIGHBOR = 0,
      rspfFilterResampler_BOX              = 1,
      rspfFilterResampler_GAUSSIAN         = 2,
      rspfFilterResampler_CUBIC            = 3,
      rspfFilterResampler_HANNING          = 4,
      rspfFilterResampler_HAMMING          = 5,
      rspfFilterResampler_LANCZOS          = 6,
      rspfFilterResampler_MITCHELL         = 7,
      rspfFilterResampler_CATROM           = 8,
      rspfFilterResampler_BLACKMAN         = 9,
      rspfFilterResampler_BLACKMAN_SINC    = 10,
      rspfFilterResampler_BLACKMAN_BESSEL  = 11,
      rspfFilterResampler_QUADRATIC        = 12,
      rspfFilterResampler_TRIANGLE         = 13,
      rspfFilterResampler_BILINEAR         = 13,
      rspfFilterResampler_HERMITE          = 14,
      rspfFilterResampler_BELL             = 15,
      rspfFilterResampler_BSPLINE          = 16
      
   };
   rspfFilterResampler();
   virtual ~rspfFilterResampler();

  virtual void resample(const rspfRefPtr<rspfImageData>& input,
			rspfRefPtr<rspfImageData>& output,
			const rspfDpt& ul,
			const rspfDpt& ur,
			const rspfDpt& deltaUl,
			const rspfDpt& deltaUr,
			const rspfDpt& length);

  virtual void resample(const rspfRefPtr<rspfImageData>& input,
			rspfRefPtr<rspfImageData>& output,
			const rspfIrect& outputSubRect,
			const rspfDpt& ul,
			const rspfDpt& ur,
			const rspfDpt& deltaUl,
			const rspfDpt& deltaUr,
			const rspfDpt& length);

  void setFilterType(rspfFilterResamplerType filterType);
  void setFilterType(rspfFilterResamplerType minifyFilterType,
		     rspfFilterResamplerType magnifyFilterType);
  void setFilterType(const rspfString& type);
  void setFilterType(const rspfString& minifyType,
		     const rspfString& magnifyType);

  void setMinifyFilterType(const rspfString& minifyType);
  void setMagnifyFilterType(const rspfString& magnifyType);
  void setMinifyFilterType(rspfFilterResamplerType filterType);
  void setMagnifyFilterType(rspfFilterResamplerType filterType);
  rspfString getMinifyFilterTypeAsString()const;
  rspfString getMagnifyFilterTypeAsString()const;
  void setScaleFactor(const rspfDpt& scale);
  void setBlurFactor(rspf_float64 blur);

  void setBoundingInputRect(const rspfIrect& rect)
  {
    theInputRect = rect;
  }
  rspfIrect getBoundingInputRect()const
  {
    return theInputRect;
  }

  rspf_float64 getBlurFactor()const;

  const rspfDpt& getScaleFactor()const
  {
    return theScaleFactor;
  }
  /*!
   * Saves the state of this object.
   */
  virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
  
  /*!
   * Loads the state of this object.
   */
  virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
  
  virtual void getKernelSupport(double& x,
				double & y)const;

  virtual void getFilterTypes(std::vector<rspfString>& filterTypes)const;

private:
  const rspfFilter* getHorizontalFilter()const;
  const rspfFilter* getVerticalFilter()const;

  template <class T>
  void resampleBilinearTile(T dummy,
			    const rspfRefPtr<rspfImageData>& input,
			    rspfRefPtr<rspfImageData>& output,
			    const rspfIrect& outputSubRect,
			    const rspfDpt& inputUl,
			    const rspfDpt& inputUr,
			    const rspfDpt& deltaUl,
			    const rspfDpt& deltaUr,
			    const rspfDpt& outLength);
  
   void computeTable();
   rspfString getFilterTypeAsString(rspfFilterResamplerType type)const;
   rspfFilterResamplerType getFilterType(const rspfString& type)const;
   rspfFilter* createNewFilter(rspfFilterResamplerType filterType,
                                rspfFilterResamplerType& result);


   rspfFilter*             theMinifyFilter;
   rspfFilter*             theMagnifyFilter;
   rspfFilterTable         theFilterTable;
   
   rspfFilterResamplerType theMinifyFilterType;
   rspfFilterResamplerType theMagnifyFilterType;
   rspfDpt                 theScaleFactor;
   rspfDpt                 theInverseScaleFactor;
   
   rspfIrect               theInputRect;
   rspf_float64            theBlurFactor;
};

#endif
