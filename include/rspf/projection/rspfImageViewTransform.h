#ifndef rspfImageViewTransform_HEADER
#define rspfImageViewTransform_HEADER
#include <iostream>
using namespace std;
#include <rspf/base/rspfViewInterface.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspf2dTo2dTransform.h>
class RSPFDLLEXPORT rspfImageViewTransform : public rspf2dTo2dTransform,
					       public rspfViewInterface
{
public:
   friend RSPFDLLEXPORT ostream& operator<<(ostream& out,
					     const rspfImageViewTransform& data);
   
   rspfImageViewTransform();
   virtual void forward(const rspfDpt& input, rspfDpt& output) const;
   virtual void inverse(const rspfDpt& input, rspfDpt& output) const;
  //  virtual rspfGpt inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const;
  
  /*!
   * derived classes will specify if the imageViewTransform is an
   * identity.  
   */
   virtual bool isIdentity()const=0;
   virtual bool isValid()const=0;
   virtual rspfDpt getInputMetersPerPixel()const=0;
   virtual rspfDpt getOutputMetersPerPixel()const=0;
   /*!
   * If it needs to it will use the information passed in to get the
   * scae factor by transforming the line point.x to point.x+deltaXY and 
   * point.y to point.y + deltaXY.y.  It will comput new delta's and 
   * set the ratio of deltaXY/newDeltaXY to the result scale.  
   *
   * Derived classes can overide this default implementation.  Note:
   * if the scale changes are only subpixel then it will return a scale
   * of one.  We have to see how this works before we actually keep this
   * implementation.
   */
  virtual void getImageToViewScale(rspfDpt& resultScale,
				   const rspfDpt& imagePoint,
				   const rspfDpt& deltaImagePointXY)const;
  /*!
   * If it needs to it will use the information passed in to get the
   * scae factor by transforming the line point.x to point.x+deltaXY and 
   * point.y to point.y + deltaXY.y.  It will comput new delta's and 
   * set the ratio of deltaXY/newDeltaXY to the result scale.  
   *
   * Derived classes can overide this default implementation.  Note:
   * if the scale changes are only subpixel then it will return a scale
   * of one.  We have to see how this works before we actually keep this
   * implementation.
   */
  virtual void getViewToImageScale(rspfDpt& resultScale,
				   const rspfDpt& imagePoint,
				   const rspfDpt& deltaImagePointXY)const;
  
  virtual void imageToView(const rspfDpt& imagePoint,
                           rspfDpt&       viewPoint)const;
  
  virtual void viewToImage(const rspfDpt& viewPoint,
                           rspfDpt&       imagePoint)const;
  
  virtual std::ostream& print(std::ostream& out) const;
  
  rspfDpt imageToView(const rspfDpt& imagePoint)const;
  
  rspfDpt viewToImage(const rspfDpt& viewPoint)const;
  
  /*!
   * This should compute output/input.
   */
  virtual void getScaleChangeImageToView(rspfDpt& result,
                                         const rspfDrect& imageRect);
  /*!
   * This should compute output/input.
   */
  virtual void getScaleChangeViewToImage(rspfDpt& result,
                                         const rspfDrect& viewRect);
  
  virtual void getRoundTripErrorView(rspfDpt& result,
				     const rspfDpt& viewPt)const;
  
  virtual rspfDpt getRoundTripErrorView(const rspfDpt& viewPt)const;
  
  virtual void getRoundTripErrorImage(rspfDpt& result,
                                      const rspfDpt& imagePt)const;
  
  virtual rspfDpt getRoundTripErrorImage(const rspfDpt& imagePt)const;
  virtual rspfDrect getImageToViewBounds(const rspfDrect& imageRect)const;
  
  virtual bool loadState(const rspfKeywordlist& kwl,
                         const char* prefix =0);
  
  virtual bool saveState(rspfKeywordlist& kwl,
                         const char* prefix = 0)const;
TYPE_DATA
};
#endif
