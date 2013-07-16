#include <math.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/projection/rspfImageViewTransform.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfIrect.h>
RTTI_DEF2(rspfImageViewTransform, "rspfImageViewTransform", rspf2dTo2dTransform, rspfViewInterface)
rspfImageViewTransform::rspfImageViewTransform()
    :
       rspf2dTo2dTransform(),
       rspfViewInterface(NULL)
{
   rspfViewInterface::theObject = this;
}
void rspfImageViewTransform::forward(const rspfDpt& input,
                                      rspfDpt& output) const
{
   imageToView(input, output);
}
void rspfImageViewTransform::inverse(const rspfDpt& input,
                                      rspfDpt& output) const
{
   viewToImage(input, output);
}
void rspfImageViewTransform::imageToView(const rspfDpt& imagePoint,
                                          rspfDpt&       viewPoint)const
{
   viewPoint = imagePoint;
}
void rspfImageViewTransform::viewToImage(const rspfDpt& viewPoint,
                                          rspfDpt&       imagePoint)const
{
   rspf2dTo2dTransform::inverse(viewPoint, imagePoint);
}
rspfDpt rspfImageViewTransform::imageToView(const rspfDpt& imagePoint)const
{
   rspfDpt tempPt;
   
   imageToView(imagePoint, tempPt);
   
   return tempPt;
}
rspfDpt rspfImageViewTransform::viewToImage(const rspfDpt& viewPoint)const
{
   rspfDpt tempPt;
   
   viewToImage(viewPoint, tempPt);
   
   return tempPt;
}
void rspfImageViewTransform::getRoundTripErrorView(rspfDpt& result,
                                                    const rspfDpt& /* viewPt */)const
{
   result = rspfDpt(0,0);
}
rspfDpt rspfImageViewTransform::getRoundTripErrorView(const rspfDpt& viewPt)const
{
   rspfDpt result;
   getRoundTripErrorView(result, viewPt);
   return result;
}
void rspfImageViewTransform::getRoundTripErrorImage(rspfDpt& result,
                                                     const rspfDpt& /* imagePt */)const
{
   result = rspfDpt(0,0);
}
rspfDpt rspfImageViewTransform::getRoundTripErrorImage(const rspfDpt& imagePt)const
{
   rspfDpt result;
   getRoundTripErrorView(result, imagePt);
   return result;
}
  
bool rspfImageViewTransform::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   return rspf2dTo2dTransform::loadState(kwl, prefix);
}
bool rspfImageViewTransform::saveState(rspfKeywordlist& kwl,
                                        const char* prefix)const
{
   return rspf2dTo2dTransform::saveState(kwl, prefix);
}
void rspfImageViewTransform::getScaleChangeImageToView(rspfDpt& result,
                                                        const rspfDrect& imageRect)
{
   result.makeNan();
   if(!imageRect.hasNans())
   {
      rspfDpt vul;
      rspfDpt vur;
      rspfDpt vlr;
      rspfDpt vll;
      imageToView(imageRect.ul(),
                  vul);
      imageToView(imageRect.ur(),
                  vur);
      imageToView(imageRect.lr(),
                  vlr);
      imageToView(imageRect.ll(),
                  vll);
      if(!vul.hasNans()&&
         !vur.hasNans()&&
         !vlr.hasNans()&&
         !vll.hasNans())
      {
         
         double deltaTop    = (vul - vur).length();
         double deltaBottom = (vll - vlr).length();
         double deltaRight  = (vur - vlr).length();
         
         double w = imageRect.width();
         double h = imageRect.height();
         result.x = (deltaTop/w + deltaBottom/w)*.5;
         result.y = (deltaRight/h + deltaRight/h)*.5;
      }
   }
}
void rspfImageViewTransform::getScaleChangeViewToImage(rspfDpt& result,
                                                        const rspfDrect& viewRect)
{
   result.makeNan();
   if(!viewRect.hasNans())
   {
      rspfDpt iul;
      rspfDpt iur;
      rspfDpt ilr;
      rspfDpt ill;
      imageToView(viewRect.ul(),
                  iul);
      imageToView(viewRect.ur(),
                  iur);
      imageToView(viewRect.lr(),
                  ilr);
      imageToView(viewRect.ll(),
                  ill);
      if(!iul.hasNans()&&
         !iur.hasNans()&&
         !ilr.hasNans()&&
         !ill.hasNans())
      {
         
         double deltaTop    = (iul - iur).length();
         double deltaBottom = (ill - ilr).length();
         double deltaRight  = (iur - ilr).length();
         
         double w = viewRect.width();
         double h = viewRect.height();
         result.x = (deltaTop/w + deltaBottom/w)*.5;
         result.y = (deltaRight/h + deltaRight/h)*.5;
      }
   }
}
void rspfImageViewTransform::getImageToViewScale(rspfDpt& resultScale,
						  const rspfDpt& imagePoint,
						  const rspfDpt& deltaImagePointXY)const
{
  rspfDpt p1 = imagePoint;
  rspfDpt p2(imagePoint.x + deltaImagePointXY.x,
	      imagePoint.y);
  rspfDpt p3(imagePoint.x,
	      imagePoint.y + deltaImagePointXY.y);
  
  rspfDpt transformedP1;
  rspfDpt transformedP2;
  rspfDpt transformedP3;
  imageToView(p1, transformedP1);
  imageToView(p2, transformedP2);
  imageToView(p3, transformedP3);
  rspfDpt deltaP1P2 = transformedP1 - transformedP2;
  rspfDpt deltaP1P3 = transformedP1 - transformedP3;
 
  double sumSquaredSqrtP1P2 = sqrt((deltaP1P2.x*deltaP1P2.x) +
			       (deltaP1P2.y*deltaP1P2.y));
  double sumSquaredSqrtP1P3 = sqrt((deltaP1P3.x*deltaP1P3.x) +
			       (deltaP1P3.y*deltaP1P3.y));
  resultScale.x = 0;
  resultScale.y = 0;
  if(sumSquaredSqrtP1P2 > FLT_EPSILON)
    {
      resultScale.x = sumSquaredSqrtP1P2/deltaImagePointXY.x;
    }
  if(sumSquaredSqrtP1P3 > FLT_EPSILON)
    {
      resultScale.y = sumSquaredSqrtP1P3/deltaImagePointXY.y;
    }
}
void rspfImageViewTransform::getViewToImageScale(rspfDpt& resultScale,
						  const rspfDpt& viewPoint,
						  const rspfDpt& deltaViewPointXY)const
{
  rspfDpt p1 = viewPoint;
  rspfDpt p2(viewPoint.x + deltaViewPointXY.x,
	      viewPoint.y);
  rspfDpt p3(viewPoint.x,
	      viewPoint.y + deltaViewPointXY.y);
  rspfDpt transformedP1;
  rspfDpt transformedP2;
  rspfDpt transformedP3;
  viewToImage(p1, transformedP1);
  viewToImage(p2, transformedP2);
  viewToImage(p3, transformedP3);
  rspfDpt deltaP1P2 = transformedP1 - transformedP2;
  rspfDpt deltaP1P3 = transformedP1 - transformedP3;
  double sumSquaredSqrtP1P2 = sqrt((deltaP1P2.x*deltaP1P2.x) +
                                   (deltaP1P2.y*deltaP1P2.y));
  double sumSquaredSqrtP1P3 = sqrt((deltaP1P3.x*deltaP1P3.x) +
                                   (deltaP1P3.y*deltaP1P3.y));
  resultScale.x = 0;
  resultScale.y = 0;
  if(sumSquaredSqrtP1P2 > FLT_EPSILON)
    {
      resultScale.x = sumSquaredSqrtP1P2/deltaViewPointXY.x;
    }
  if(sumSquaredSqrtP1P3 > FLT_EPSILON)
    {
      resultScale.y = sumSquaredSqrtP1P3/deltaViewPointXY.y;
    }
}
rspfDrect rspfImageViewTransform::getImageToViewBounds(const rspfDrect& imageRect)const
{
   rspfDpt p1;
   rspfDpt p2;
   rspfDpt p3;
   rspfDpt p4;
   imageToView(imageRect.ul(), p1);
   imageToView(imageRect.ur(), p2);
   imageToView(imageRect.lr(), p3);
   imageToView(imageRect.ll(), p4);
   return rspfDrect(p1, p2, p3, p4);
}
std::ostream& rspfImageViewTransform::print(std::ostream& out) const
{
   return out;
}
std::ostream& operator<<(std::ostream& out,
                         const rspfImageViewTransform& data)
{
   return data.print(out);
}
