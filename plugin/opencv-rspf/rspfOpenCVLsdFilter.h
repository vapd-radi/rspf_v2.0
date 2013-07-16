#ifndef rspfOpenCVLsdFilter_HEADER
#define rspfOpenCVLsdFilter_HEADER

#include "rspf/plugin/rspfSharedObjectBridge.h"
#include "rspf/base/rspfString.h"
#include "rspf/imaging/rspfImageSourceFilter.h"
#include <rspf/imaging/rspfImageDataFactory.h>

#include <cv.h>
#include <highgui.h>

/** @brief OpenCV Lsd Filter 
 **
 ** Implements Canny algorithm for line segment detection.
 ** @param threshold The scale threshold
 **
 ** The function cvCanny finds the edges on the input image image and marks them in the output image edges using the Canny algorithm.
 ** The smallest of threshold1 and threshold2 is used for edge linking, the largest is used to find initial segments of strong edges. 
 **
 **/
class rspfOpenCVLsdFilter : public rspfImageSourceFilter
{
public:
   rspfOpenCVLsdFilter(rspfObject* owner=NULL);
   rspfOpenCVLsdFilter(rspfImageSource* inputSource, double scale, rspfFilename outASCII, int saveImage);
   rspfOpenCVLsdFilter(rspfObject* owner,rspfImageSource* inputSource, double scale, rspfFilename outASCII, int saveImage);
   virtual ~rspfOpenCVLsdFilter();
   rspfString getShortName()const
      {
         return rspfString("OpenCVLsd");
      }
   
   rspfString getLongName()const
      {
         return rspfString("OpenCV Lsd Filter");
      }
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect, rspf_uint32 resLevel=0);
   virtual void initialize();
   virtual rspfScalarType getOutputScalarType() const;
   rspf_uint32 getNumberOfOutputBands() const;
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   /*
   * Methods to expose thresholds for adjustment through the GUI
   */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   bool cvLsd(IplImage* input, IplImage* output, rspfIpt offset, double scale = 0.8);

protected:
   rspfRefPtr<rspfImageData> theTile;
   void runUcharTransformation(rspfImageData* tile, rspfIpt offset);
   double theScale;
   rspfFilename theOutASCII;
   int theSaveImage;
   int m_nCount;
   //std::vector< double[7] > theLinesegments;
   rspfString theLineSegments;

TYPE_DATA
};

#endif
