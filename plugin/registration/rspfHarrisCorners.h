// class rspfHarrisCorner : tile source
// computes harris corners using an image chain
// calcultes x and y derivatives, then forms three 2nd order products, smoothg them and combines to form a cornerness map
// then selects local maxima above a certain threshold
//
// INPUT:  one band image
// OUTPUT: one band, rspf_float64
//         corners are the only non NULL pixels (non RSPF_DBL_NAN). value=cornerness
// PROPERTIES
//      K  = harris cornerness parameter
//         min_cornerness = minimum cornerness
//         smoooth_radius = gaussian smoothing standard deviation
//

#ifndef rspfHarrisCorners_HEADER
#define rspfHarrisCorners_HEADER

#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/imaging/rspfCastTileSourceFilter.h>
#include <rspf/imaging/rspfConvolutionFilter1D.h>
#include <rspf/imaging/rspfImageGaussianFilter.h>
#include <rspf/imaging/rspfEquationCombiner.h>
#include "rspfExtremaFilter.h"
#include "rspfDensityReducer.h"
#include "rspfMultiplier.h"
#include "rspfSquareFunction.h"
#include "rspfRegistrationExports.h"


class RSPF_REGISTRATION_DLL rspfHarrisCorners : public rspfImageSourceFilter
{
public:
  /**
   *own methods
   */
   rspfHarrisCorners();
   virtual ~rspfHarrisCorners();

   inline rspf_float64 getK()const { return theK; }
   void setK(const rspf_float64& v);

   inline rspf_float64 getGaussStd()const { return theGaussStd; }
   void setGaussStd(const rspf_float64& v);

   inline rspf_float64 getMinCornerness()const { return theMinCornerness; }
   void setMinCornerness(const rspf_float64& v);

   inline rspf_float64 getDensity()const { return theDensity; }
   void setDensity(const rspf_float64& d);

  /**
   * inherited methods re-implemented due to type change in output
   */
   virtual void getOutputBandList(std::vector<rspf_uint32>& bandList) const;
   virtual rspfScalarType getOutputScalarType() const;
   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   
  /**
   * inherited methods
   */
   virtual void initialize();
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect &tileRect,rspf_uint32 resLevel=0);
   
   //! implemented to pass events to inner processes
   virtual void connectInputEvent(rspfConnectionEvent &event);
   virtual void disconnectInputEvent(rspfConnectionEvent &event);

   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   virtual bool loadState(const rspfKeywordlist& kwl,const char* prefix = 0);   
   virtual bool saveState(rspfKeywordlist& kwl,const char* prefix = 0)const;
   
protected:
   /**
    * protected methods
    */
   void initializeProcesses();
   rspfString getCornernessEquation()const;

  /**
   *parameters
   */
   rspf_float64 theK;
   rspf_float64 theGaussStd;
   rspf_float64 theMinCornerness; //not used now
   rspf_float64 theDensity; //corner density in pixel^-2

  /** 
   * subprocesses
   */
   rspfRefPtr<rspfCastTileSourceFilter> theCastP;
   rspfRefPtr<rspfConvolutionFilter1D>  theDxP;
   rspfRefPtr<rspfConvolutionFilter1D>  theDyP;
   rspfRefPtr<rspfMultiplier>           theProductP;
   std::vector<rspfRefPtr<rspfSquareFunction> >      theSquaresP;
   std::vector<rspfRefPtr<rspfImageGaussianFilter> >  theSmoothP;
   rspfRefPtr<rspfEquationCombiner>     theCornernessP;
   rspfRefPtr<rspfExtremaFilter>        theLocalMaxP;
   rspfRefPtr<rspfDensityReducer>       theFilterP;

TYPE_DATA
};

#endif /* #ifndef rspfHarrisCorners_HEADER */
