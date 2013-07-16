//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfImageRenderer.cpp 20352 2011-12-12 17:24:52Z dburken $

#include <rspf/imaging/rspfImageRenderer.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfProcessProgressEvent.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfConnectableContainerInterface.h>
#include <rspf/base/rspfViewController.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageDataFactory.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfDiscrete3x3HatFilter.h>
#include <rspf/imaging/rspfDiscreteNearestNeighbor.h>
#include <rspf/imaging/rspfFilterResampler.h>
#include <rspf/projection/rspfImageViewProjectionTransform.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfImageViewTransformFactory.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <iostream>
using namespace std;

#ifdef RSPF_ID_ENABLED
static const char RSPF_ID[] = "$Id: rspfImageRenderer.cpp 20352 2011-12-12 17:24:52Z dburken $";
#endif

static rspfTrace traceDebug("rspfImageRenderer:debug");

RTTI_DEF2(rspfImageRenderer, "rspfImageRenderer", rspfImageSourceFilter, rspfViewInterface);

void rspfImageRenderer::rspfRendererSubRectInfo::splitView(rspfImageViewTransform* transform,
                                                             rspfRendererSubRectInfo& ulRect,
                                                             rspfRendererSubRectInfo& urRect,
                                                             rspfRendererSubRectInfo& lrRect,
                                                             rspfRendererSubRectInfo& llRect)const
{
#if 0
   rspfNotify(rspfNotifyLevel_DEBUG)
      << "ulRect = " << ulRect
      << "\nurRect = " << urRect
      << "\nlrRect = " << lrRect
      << "\nllRect = " << llRect << endl;
#endif
   
   rspfIrect vrect(m_Vul,
                    m_Vur,
                    m_Vlr,
                    m_Vll);
   
   rspfIrect tempUlRect;
   rspfIrect tempUrRect;
   rspfIrect tempLrRect;
   rspfIrect tempLlRect;
   
   if((vrect.width() == 1)&&
      (vrect.height() == 1))
   {
      tempUlRect = vrect;
      tempUrRect = vrect;
      tempLrRect = vrect;
      tempLlRect = vrect;
   }
   else
   {
      rspf_int32 w  = vrect.width();
      rspf_int32 h  = vrect.height();
      rspf_int32 w2 = w>>1;
      rspf_int32 h2 = h>>1;
      
      if((w%2) == 0) // if even
      {
         if((h%2) == 0) // if even
         {
            tempUlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y,
                                    m_Vul.x + (w2 - 1),
                                    m_Vul.y + (h2 - 1));
            
            tempUrRect = rspfIrect(m_Vul.x + w2,
                                    m_Vul.y,
                                    m_Vul.x + w2 + (w2 - 1),
                                    m_Vul.y + (h2 - 1));
            
            tempLrRect = rspfIrect(m_Vul.x + w2,
                                    m_Vul.y + h2,
                                    m_Vul.x + w2 + (w2 - 1),
                                    m_Vul.y + h2 + (h2 - 1));
            
            tempLlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y + h2,
                                    m_Vul.x + (w2 - 1),
                                    m_Vul.y + h2 + (h2 - 1));
         }
         else // odd
         {
            tempUlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y,
                                    m_Vul.x + (w2 - 1),
                                    m_Vul.y + (h2));
            
            tempUrRect = rspfIrect(m_Vul.x + w2,
                                    m_Vul.y,
                                    m_Vul.x + w2 + (w2 - 1),
                                    m_Vul.y + (h2));
            
            tempLrRect = rspfIrect(m_Vul.x + w2,
                                    m_Vul.y + h2 + 1,
                                    m_Vul.x + w2 + (w2 - 1),
                                    m_Vul.y + h2 + (h2));
            
            tempLlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y + h2 + 1,
                                    m_Vul.x + (w2 - 1),
                                    m_Vul.y + h2 + (h2));
         }
      }
      else // odd
      {
         if((h%2) == 0) // if even
         {
            tempUlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y,
                                    m_Vul.x + (w2),
                                    m_Vul.y + (h2 - 1));
            
            tempUrRect = rspfIrect(m_Vul.x + w2 + 1,
                                    m_Vul.y,
                                    m_Vul.x + w2 + (w2),
                                    m_Vul.y + (h2 - 1));
            
            tempLrRect = rspfIrect(m_Vul.x + w2+1,
                                    m_Vul.y + h2,
                                    m_Vul.x + w2 + (w2),
                                    m_Vul.y + h2 + (h2 - 1));
            
            tempLlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y + h2,
                                    m_Vul.x + (w2),
                                    m_Vul.y + h2 + (h2 - 1));
            
         }
         else // odd
         {
            tempUlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y,
                                    m_Vul.x + (w2),
                                    m_Vul.y + (h2));
            
            tempUrRect = rspfIrect(m_Vul.x + w2+1,
                                    m_Vul.y,
                                    m_Vul.x + w2 + (w2),
                                    m_Vul.y + (h2));
            
            tempLrRect = rspfIrect(m_Vul.x + w2+1,
                                    m_Vul.y + h2+1,
                                    m_Vul.x + w2 + (w2),
                                    m_Vul.y + h2 + (h2));
            
            tempLlRect = rspfIrect(m_Vul.x,
                                    m_Vul.y + h2+1,
                                    m_Vul.x + (w2),
                                    m_Vul.y + h2 + (h2));
         }
      }
   }
#if 0
   rspfNotify(rspfNotifyLevel_DEBUG)
      << "SPLIT++++++++++++++++++++++++++++++++++++++++++++" << endl
      << "current = " << vrect << endl
      << "ul      = " << tempUlRect << endl
      << "ur      = " << tempUrRect << endl
      << "lr      = " << tempLrRect << endl
      << "ll      = " << tempLlRect << endl;
#endif

   ulRect.m_Vul = tempUlRect.ul();
   ulRect.m_Vur = tempUlRect.ur();
   ulRect.m_Vlr = tempUlRect.lr();
   ulRect.m_Vll = tempUlRect.ll();
   
   urRect.m_Vul = tempUrRect.ul();
   urRect.m_Vur = tempUrRect.ur();
   urRect.m_Vlr = tempUrRect.lr();
   urRect.m_Vll = tempUrRect.ll();
   
   lrRect.m_Vul = tempLrRect.ul();
   lrRect.m_Vur = tempLrRect.ur();
   lrRect.m_Vlr = tempLrRect.lr();
   lrRect.m_Vll = tempLrRect.ll();

   llRect.m_Vul = tempLlRect.ul();
   llRect.m_Vur = tempLlRect.ur();
   llRect.m_Vlr = tempLlRect.lr();
   llRect.m_Vll = tempLlRect.ll();
   
   ulRect.transformViewToImage(transform);
   urRect.transformViewToImage(transform);
   lrRect.transformViewToImage(transform);
   llRect.transformViewToImage(transform);
}

void rspfImageRenderer::rspfRendererSubRectInfo::transformImageToView(rspfImageViewTransform* transform)
{
   rspfDpt vul;
   rspfDpt vur;
   rspfDpt vlr;
   rspfDpt vll;
   transform->imageToView(m_Iul,
                          vul);
   transform->imageToView(m_Iur,
                          vur);
   transform->imageToView(m_Ilr,
                          vlr);
   transform->imageToView(m_Ill,
                          vll);
   
   m_Vul = vul;
   m_Vur = vur;
   m_Vlr = vlr;
   m_Vll = vll;
   
}

void rspfImageRenderer::rspfRendererSubRectInfo::transformViewToImage(rspfImageViewTransform* transform)
{
   transform->viewToImage(m_Vul, m_Iul);
   transform->viewToImage(m_Vur, m_Iur);
   transform->viewToImage(m_Vlr, m_Ilr);
   transform->viewToImage(m_Vll, m_Ill);

   if(imageHasNans())
   {
      m_ViewToImageScale.makeNan();
   }
   else
   {
      m_ViewToImageScale = rspfDpt(1.0, 1.0);
      
      rspfDpt topDelta    = m_Iur - m_Iul;
      rspfDpt rightDelta  = m_Ilr - m_Iur;
      rspfDpt bottomDelta = m_Ill - m_Ilr;
      rspfDpt leftDelta   = m_Iul - m_Ill;
      
      double topLen    = topDelta.length();
      double bottomLen = bottomDelta.length();
      double rightLen  = rightDelta.length();
      double leftLen   = leftDelta.length();
      
      double averageHoriz = ((topLen)  + (bottomLen))*.5;
      double averageVert  = ((leftLen) + (rightLen))*.5;
      
      rspfDpt deltaViewP1P2 = m_Vul - m_Vur;
      rspfDpt deltaViewP1P3 = m_Vul - m_Vll;
      
      double lengthViewP1P2 = deltaViewP1P2.length();//+1;
      double lengthViewP1P3 = deltaViewP1P3.length();//+1;

      if(lengthViewP1P2 > FLT_EPSILON)
      {
         m_ViewToImageScale.x = averageHoriz/lengthViewP1P2;
      }
      else
      {
         m_ViewToImageScale.makeNan();
      }
      if(lengthViewP1P3 > FLT_EPSILON)
      {
         m_ViewToImageScale.y = averageVert/lengthViewP1P3;
      }
      else
      {
         m_ViewToImageScale.makeNan();
      }

      if(!m_ViewToImageScale.hasNans())
      {
         m_ImageToViewScale.x = 1.0/m_ViewToImageScale.x;
         m_ImageToViewScale.y = 1.0/m_ViewToImageScale.y;
      }
      else
      {
         m_ImageToViewScale.makeNan();
      }
  }
}

void rspfImageRenderer::rspfRendererSubRectInfo::stretchImageOut(bool enableRound)
{
   rspfDpt topDelta    = m_Iur - m_Iul;
   rspfDpt rightDelta  = m_Ilr - m_Iur;
   rspfDpt bottomDelta = m_Ill - m_Ilr;
   rspfDpt leftDelta   = m_Iul - m_Ill;

   topDelta = topDelta*(1.0/topDelta.length());
   rightDelta = rightDelta*(1.0/rightDelta.length());
   bottomDelta = bottomDelta*(1.0/bottomDelta.length());
   leftDelta = leftDelta*(1.0/leftDelta.length());

   m_Iul = m_Iul + ((leftDelta - topDelta)*.5);
   m_Iur = m_Iur + ((topDelta - rightDelta)*.5);
   m_Ilr = m_Ilr + ((rightDelta - bottomDelta)*.5);
   m_Ill = m_Ill + ((bottomDelta - leftDelta)*.5);

   if(enableRound)
   {
      m_Iul = rspfIpt(rspf::round<int>(m_Iul.x),
                        rspf::round<int>(m_Iul.y));
      m_Iur = rspfIpt(rspf::round<int>(m_Iur.x),
                        rspf::round<int>(m_Iur.y));
      m_Ilr = rspfIpt(rspf::round<int>(m_Ilr.x),
                        rspf::round<int>(m_Ilr.y));
      m_Ill = rspfIpt(rspf::round<int>(m_Ill.x),
                        rspf::round<int>(m_Ill.y));
   }
}

bool rspfImageRenderer::rspfRendererSubRectInfo::isIdentity()const
{
//    rspfDpt deltaP1P2 = m_Iul - m_Iur;
//    rspfDpt deltaP1P3 = m_Iul - m_Ill;
   
//    rspfDpt deltaViewP1P2 = m_Vul - m_Vur;
//    rspfDpt deltaViewP1P3 = m_Vul - m_Vll;
   
//    bool horizontalSigns = rspfGetSign(deltaP1P2.x)==rspfGetSign(deltaViewP1P2.x);
//    bool verticalSigns   = rspfGetSign(deltaP1P3.y)==rspfGetSign(deltaViewP1P3.y);

//    // check first to see if any horizontal or vertical flipping
//    //
//    if(horizontalSigns && verticalSigns)
//    {
//       // check scales to see if they are 1
//       if(fabs(1-m_ViewToImageScale.x) <= FLT_EPSILON &&
//          fabs(1-m_ViewToImageScale.y) <= FLT_EPSILON)
//       {
//          return true;
//       }
//    }

    double iulDelta = (m_Iul-m_Vul).length();
    double iurDelta = (m_Iur-m_Vur).length();
    double ilrDelta = (m_Ilr-m_Vlr).length();
    double illDelta = (m_Ill-m_Vll).length();

    return ((iulDelta <= FLT_EPSILON)&&
            (iurDelta <= FLT_EPSILON)&&
            (ilrDelta <= FLT_EPSILON)&&
            (illDelta <= FLT_EPSILON));
}

bool rspfImageRenderer::rspfRendererSubRectInfo::canBilinearInterpolate(rspfImageViewTransform* transform,
									  double error)const
{
   if(imageHasNans())
   {
      return false;
   }
  rspfDpt imageToViewScale = getAbsValueImageToViewScales();

  double testScale = imageToViewScale.length();

  // if there is a large shrink or expansion then just return true.
  // You are probably not worried about error in bilinear interpolation
  //
  if((testScale > 500)||
     (testScale < 1.0/500.0))
  {
     return true;
  }
  if(imageToViewScale.hasNans()) return false;
  rspfDpt vUpper, vRight, vBottom, vLeft, vCenter;
  rspfDpt iUpper, iRight, iBottom, iLeft, iCenter;

  getViewMids(vUpper, vRight, vBottom, vLeft, vCenter);
  getImageMids(iUpper, iRight, iBottom, iLeft, iCenter);

  rspfDpt testCenter;

  rspfDpt iFullRes(iCenter.x*imageToViewScale.x,
		    iCenter.y*imageToViewScale.y);

  transform->viewToImage(vCenter, testCenter);

  if(testCenter.hasNans())
  {
     return false;
  }
  rspfDpt testFullRes(testCenter.x*imageToViewScale.x,
		       testCenter.y*imageToViewScale.y);

  double errorCheck1 = (testFullRes - iFullRes).length();

  iFullRes = rspfDpt(iUpper.x*imageToViewScale.x,
		      iUpper.y*imageToViewScale.y);

  transform->viewToImage(vUpper, testCenter);
  if(testCenter.hasNans())
  {
     return false;
  }
  testFullRes = rspfDpt(testCenter.x*imageToViewScale.x,
			 testCenter.y*imageToViewScale.y);
  double errorCheck2 = (testFullRes - iFullRes).length();

  iFullRes = rspfDpt(iRight.x*imageToViewScale.x,
		      iRight.y*imageToViewScale.y);

  transform->viewToImage(vRight, testCenter);
  if(testCenter.hasNans())
  {
     return false;
  }
  testFullRes = rspfDpt(testCenter.x*imageToViewScale.x,
			 testCenter.y*imageToViewScale.y);
  double errorCheck3 = (testFullRes - iFullRes).length();

  iFullRes = rspfDpt(iBottom.x*imageToViewScale.x,
		      iBottom.y*imageToViewScale.y);

  transform->viewToImage(vBottom, testCenter);
  if(testCenter.hasNans())
  {
     return false;
  }
  testFullRes = rspfDpt(testCenter.x*imageToViewScale.x,
			 testCenter.y*imageToViewScale.y);
  double errorCheck4 = (testFullRes - iFullRes).length();

  iFullRes = rspfDpt(iLeft.x*imageToViewScale.x,
		      iLeft.y*imageToViewScale.y);

  transform->viewToImage(vLeft, testCenter);
  testFullRes = rspfDpt(testCenter.x*imageToViewScale.x,
			 testCenter.y*imageToViewScale.y);
  double errorCheck5 = (testFullRes - iFullRes).length();


  return ((errorCheck1 < error)&&
	  (errorCheck2 < error)&&
	  (errorCheck3 < error)&&
	  (errorCheck4 < error)&&
	  (errorCheck5 < error));
	  
}

void rspfImageRenderer::rspfRendererSubRectInfo::getViewMids(rspfDpt& upperMid,
				     rspfDpt& rightMid,
				     rspfDpt& bottomMid,
				     rspfDpt& leftMid,
				     rspfDpt& center)const
{
  
  upperMid  = (m_Vul + m_Vur)*.5;
  rightMid  = (m_Vur + m_Vlr)*.5;
  bottomMid = (m_Vlr + m_Vll)*.5;
  leftMid   = (m_Vul + m_Vll)*.5;
  center    = (m_Vul + m_Vur + m_Vlr + m_Vll)*.25;
}

void rspfImageRenderer::rspfRendererSubRectInfo::getImageMids(rspfDpt& upperMid,
				      rspfDpt& rightMid,
				      rspfDpt& bottomMid,
				      rspfDpt& leftMid,
				      rspfDpt& center)const
{
  if(imageHasNans())
    {
      upperMid.makeNan();
      rightMid.makeNan();
      bottomMid.makeNan();
      leftMid.makeNan();
      center.makeNan();
    }
  else
    {
      upperMid  = (m_Iul + m_Iur)*.5;
      rightMid  = (m_Iur + m_Ilr)*.5;
      bottomMid = (m_Ilr + m_Ill)*.5;
      leftMid   = (m_Iul + m_Ill)*.5;
      center    = (m_Iul + m_Iur + m_Ilr + m_Ill)*.25;
    }
}

rspfDpt rspfImageRenderer::rspfRendererSubRectInfo::getParametricCenter(const rspfDpt& ul, const rspfDpt& ur, 
									   const rspfDpt& lr, const rspfDpt& ll)const
{
  rspfDpt top    = ur - ul;
  rspfDpt bottom = lr - ll;
  
  rspfDpt centerTop = ul + top * .5;
  rspfDpt centerBottom = ll + bottom * .5;

  return centerBottom + (centerBottom - centerTop)*.5;
}

rspfImageRenderer::rspfImageRenderer()
:
rspfImageSourceFilter(),
rspfViewInterface(0),
m_Resampler(0),
m_BlankTile(0),
m_Tile(0),
m_TemporaryBuffer(0),
m_StartingResLevel(0),
m_ImageViewTransform(0),
m_inputR0Rect(),
m_viewRect(),
m_rectsDirty(true),
m_MaxRecursionLevel(5),
m_AutoUpdateInputTransform(true),
m_MaxLevelsToCompute(999999) // something large so it will always compute
{
    rspfViewInterface::theObject = this;
    m_Resampler = new rspfFilterResampler();
    m_ImageViewTransform = new rspfImageViewProjectionTransform;
}

rspfImageRenderer::rspfImageRenderer(rspfImageSource* inputSource,
                                       rspfImageViewTransform* imageViewTrans)
   : rspfImageSourceFilter(inputSource),
     rspfViewInterface(0),
     m_Resampler(0),
     m_BlankTile(0),
     m_Tile(0),
     m_TemporaryBuffer(0),
     m_StartingResLevel(0),
     m_ImageViewTransform(imageViewTrans),
     m_inputR0Rect(),
     m_viewRect(),
     m_rectsDirty(true),
     m_MaxRecursionLevel(5),
     m_AutoUpdateInputTransform(true),
     m_MaxLevelsToCompute(999999) // something large so it will always compute
{
   rspfViewInterface::theObject = this;
   m_Resampler = new rspfFilterResampler();
   if(!m_ImageViewTransform.valid())
   {
      m_ImageViewTransform = new rspfImageViewProjectionTransform;
   }
}

rspfImageRenderer::~rspfImageRenderer()
{
  m_ImageViewTransform = 0;

   if(m_Resampler)
   {
      delete m_Resampler;
      m_Resampler = 0;
   }
}

rspfRefPtr<rspfImageData> rspfImageRenderer::getTile(
   const  rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   static const char MODULE[] = "rspfImageRenderer::getTile";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " Requesting view rect = "
         << tileRect << endl;
   }

   // long w = tileRect.width();
   // long h = tileRect.height();
   // rspfIpt origin = tileRect.ul();
   
   if( !m_BlankTile.valid() || !m_Tile.valid() )
   {
      allocate();
      if ( !m_BlankTile.valid() || !m_Tile.valid() )
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)
               << "rspfImageRenderer::getTile tile allocation failure!\n"
               << endl;
         }
         return rspfImageSourceFilter::getTile(tileRect, resLevel);
      }
   }

   m_BlankTile->setImageRectangle(tileRect);
   
   if(!theInputConnection)
   {
      return m_BlankTile;
   }
   
   if ( !isSourceEnabled()||(!m_ImageViewTransform.valid())||
        (!m_ImageViewTransform->isValid()) )
   {
      // This tile source bypassed, return the input tile source.
      return theInputConnection->getTile(tileRect, resLevel);  
   }

   if( m_rectsDirty )
   {
      initializeBoundingRects();

      // We can't go on without these...
      if ( m_rectsDirty )
      {
         return m_BlankTile;
      }
   }
   
   if(m_viewRect.width() < 4 && m_viewRect.height() < 4)
   {
      return m_BlankTile;
   }

   if( !theInputConnection || !m_viewRect.intersects(tileRect) )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << MODULE << "No intersection, Returning...." << endl;
      }
      return m_BlankTile;
   }
   
   if(!m_Tile)
   {
      return theInputConnection->getTile(tileRect, resLevel);
   }

   // long tw = m_Tile->getWidth();
   // long th = m_Tile->getHeight();
   
   m_Tile->setImageRectangle(tileRect);
   m_Tile->makeBlank();
 
#if 1
   // expand a small patch just to alleviate errors in the size of the rect when resampling
      rspfIrect viewRectClip = tileRect.clipToRect(rspfIrect(m_viewRect.ul() + rspfIpt(-8,-8),
                                                               m_viewRect.lr() + rspfIpt(8,8)));
  // rspfIrect viewRectClip = tileRect.clipToRect(m_viewRect);
//   std::cout << "_____________________" << std::endl;
//   std::cout << "viewRectClip = " <<  viewRectClip << std::endl;
//   std::cout << "tileRect = " <<  tileRect << std::endl;
//   std::cout << "m_viewRect = " <<  m_viewRect << std::endl;
   rspfRendererSubRectInfo subRectInfo(viewRectClip.ul(),
                                        viewRectClip.ur(),
                                        viewRectClip.lr(),
                                        viewRectClip.ll());
#else
   rspfRendererSubRectInfo subRectInfo(tileRect.ul(),
                                        tileRect.ur(),
                                        tileRect.lr(),
                                        tileRect.ll());
#endif

   subRectInfo.transformViewToImage(m_ImageViewTransform.get());
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " image rect = " << subRectInfo.getImageRect() << std::endl;
   }

   // If the image rect is completely outside of the valid image, there is no need to resample:
   // (OLK 11/18)
   if (!m_inputR0Rect.intersects(subRectInfo.getImageRect()))
   {
      return m_Tile;
   }
   
   recursiveResample(m_Tile, subRectInfo, 1);
   
   if(m_Tile.valid())
   {
      m_Tile->validate();
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << "Returning...." << endl;
   }
   return m_Tile;
}

void rspfImageRenderer::recursiveResample(rspfRefPtr<rspfImageData> outputData,
                                           const rspfRendererSubRectInfo& rectInfo,
                                           rspf_uint32 level)
{
   rspfIrect tempViewRect = rectInfo.getViewRect();
   if(rectInfo.imageIsNan())
   {
      return;
   } 
   
   if(tempViewRect.width() <2 &&
      tempViewRect.height() <2)
   {
      if(!rectInfo.imageHasNans())
      {
         fillTile(outputData,
                  rectInfo);
      }
      return;
   }
   const double error = 1;
   if(rectInfo.canBilinearInterpolate(m_ImageViewTransform.get(), error))
   {                                // then draw the tile
      fillTile(outputData,
	       rectInfo);
      return;
   }
   else
   {
      // split into four subtiles
      rspfRendererSubRectInfo ulRectInfo;
      rspfRendererSubRectInfo urRectInfo;
      rspfRendererSubRectInfo lrRectInfo;
      rspfRendererSubRectInfo llRectInfo;
      
      rectInfo.splitView(m_ImageViewTransform.get(),
			 ulRectInfo,
			 urRectInfo,
			 lrRectInfo,
			 llRectInfo);   
      
      rspfDrect vrect = rectInfo.getViewRect();
      
#if 0
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "vrect  = " << vrect
         << "\nwidth  = " << vrect.width()
         << "\nheight = " << vrect.height()
         << "\nlevel  = " << level << endl;
#endif
      bool scaleUlNeedsSplit = ((!ulRectInfo.canBilinearInterpolate(m_ImageViewTransform.get(), error))||
				ulRectInfo.imageHasNans());
      bool scaleUrNeedsSplit = ((!urRectInfo.canBilinearInterpolate(m_ImageViewTransform.get(), error))||
				urRectInfo.imageHasNans());
      bool scaleLrNeedsSplit = ((!lrRectInfo.canBilinearInterpolate(m_ImageViewTransform.get(), error))||
				lrRectInfo.imageHasNans());
      bool scaleLlNeedsSplit = ((!llRectInfo.canBilinearInterpolate(m_ImageViewTransform.get(), error))||
				llRectInfo.imageHasNans());
      
      bool tooSmall = (vrect.width() < 4) && (vrect.height()<4);
      //
      if(!tooSmall)
      {
         if(scaleUlNeedsSplit||
            scaleUrNeedsSplit||
            scaleLrNeedsSplit||
            scaleLlNeedsSplit)
         {
            if(scaleUlNeedsSplit)
            {
               recursiveResample(outputData,
                                 ulRectInfo,
                                 level + 1);
            }
            else
            {
               fillTile(outputData, ulRectInfo);
            }
            if(scaleUrNeedsSplit)
            {
               recursiveResample(outputData,
                                 urRectInfo,
                                 level + 1);
            }
            else
            {
               fillTile(outputData, urRectInfo);
            }
            
            if(scaleLrNeedsSplit)
            {
               recursiveResample(outputData,
                                 lrRectInfo,
                                 level + 1);
            }
            else
            {
               fillTile(outputData, lrRectInfo);
            }
            
            if(scaleLlNeedsSplit)
            {
               recursiveResample(outputData,
                                 llRectInfo,
                                 level + 1);
            }
            else
            {
               fillTile(outputData, llRectInfo);
            }
         }
         else
         {
            fillTile(outputData,
                     rectInfo);
         }
      }
      else if(!rectInfo.imageHasNans())
      {
         fillTile(outputData,
                  rectInfo);
      }
   }
}

#define RSET_SEARCH_THRESHHOLD 0.1

void rspfImageRenderer::fillTile(rspfRefPtr<rspfImageData> outputData,
                                  const rspfRendererSubRectInfo& rectInfo)
{
   if(!outputData.valid() || !outputData->getBuf() || rectInfo.imageHasNans())
   {
      return;
   }
   rspfDrect vrect = rectInfo.getViewRect();
   
   rspfDpt imageToViewScale = rectInfo.getAbsValueImageToViewScales();
   
   if(imageToViewScale.hasNans()) return;
   
   rspfDpt tile_size = rspfDpt(vrect.width(), vrect.height());
   double kernelSupportX, kernelSupportY;
   
   double resLevelX = log( 1.0 / imageToViewScale.x )/ log( 2.0 );
   double resLevelY = log( 1.0 / imageToViewScale.y )/ log( 2.0 );
   double resLevel0 = resLevelX < resLevelY ? resLevelX : resLevelY;
   long closestFitResLevel = (long)floor( resLevel0 );
   
   //double averageScale = (imageToViewScale.x + imageToViewScale.y) / 2.0;
   //long closestFitResLevel = (long)floor( log( 1.0 / averageScale )/ log( 2.0 ) );
   
   rspf_uint32 resLevel = closestFitResLevel<0 ? 0:closestFitResLevel;
   resLevel += m_StartingResLevel;

   //---
   // ESH 02/2009: If requested resLevel is too high, let's lower it to one
   // that is ok.
   //---
#if 0
   const rspf_uint32 NUM_LEVELS = theInputConnection->getNumberOfDecimationLevels();
   if ( (NUM_LEVELS > 0) && (resLevel >=  NUM_LEVELS) )
   {
      resLevel = NUM_LEVELS - 1;
   }
#endif
   //---
   // ESH 11/2008: Check the rset at the calculated resLevel to see
   // if it has the expected decimation factor. It it does, we can 
   // use this rset and assume it is at resLevel.
   //--- 
   rspfDpt decimation;
   decimation.makeNan(); // initialize to nan.
   theInputConnection->getDecimationFactor(resLevel, decimation);
   double requestScale = 1.0 / (1<<resLevel);
   double closestScale = decimation.hasNans() ? requestScale : decimation.x;

#if 0
   double differenceTest = 0.0;
   if (closestScale != 0.0)
   {
      differenceTest = (1.0/closestScale) - (1.0/requestScale);
   }

   //---
   // ESH 11/2008: Add in threshold test so search only happens when 
   //              necessary.
   // We do an rset search if 1 of 2 conditions is met: either
   //   1) the rset is really different in size from the requested size, or
   //   2) they're similar in size, and the actual rset is smaller than 
   //      the requested size.
   //---
   if ( (fabs(differenceTest) > RSET_SEARCH_THRESHHOLD) || 
        ((fabs(differenceTest) < RSET_SEARCH_THRESHHOLD) &&
         (differenceTest < 0.0) ) )
   {
      //---
      // ESH 11/2008: We test for the best rset. We assume 
      // that decimation level always decreases as resLevel increases, so 
      // the search can end before testing all rsets.
      //---
      rspf_uint32 savedResLevel = resLevel;
      closestScale = 1.0; // resLevel 0
      resLevel = 0;
      rspf_uint32 i;
      for( i=1; i<NUM_LEVELS; ++i )
      {
         theInputConnection->getDecimationFactor(i, decimation);
         if(decimation.hasNans() == false )
         {
            double testDiscrepancy = decimation.x - requestScale;
            if ( testDiscrepancy < 0.0 ) // we're done
            {
               break;
            }
            else
            {
               closestScale = decimation.x;
               resLevel = i;
            }
         }
         else // use the default value
         {
            closestScale = requestScale;
            resLevel = savedResLevel;
            break;
         }
      }
   }
#endif
   rspfDpt nul(rectInfo.m_Iul.x*closestScale,
                rectInfo.m_Iul.y*closestScale);
   rspfDpt nll(rectInfo.m_Ill.x*closestScale,
                rectInfo.m_Ill.y*closestScale);
   rspfDpt nlr(rectInfo.m_Ilr.x*closestScale,
                rectInfo.m_Ilr.y*closestScale);
   rspfDpt nur(rectInfo.m_Iur.x*closestScale,
                rectInfo.m_Iur.y*closestScale);
   
   m_Resampler->getKernelSupport( kernelSupportX, kernelSupportY );
   
   rspfDrect boundingRect = rspfDrect( nul, nll, nlr, nur );
   
   boundingRect = rspfIrect((rspf_int32)floor(boundingRect.ul().x - (kernelSupportX)-.5),
                             (rspf_int32)floor(boundingRect.ul().y - (kernelSupportY)-.5),
                             (rspf_int32)ceil (boundingRect.lr().x + (kernelSupportX)+.5),
                             (rspf_int32)ceil (boundingRect.lr().y + (kernelSupportY)+.5));
   
   rspfDrect requestRect = boundingRect;
   
   rspfRefPtr<rspfImageData> data = getTileAtResLevel(requestRect, resLevel);
   
   rspfDataObjectStatus status = RSPF_NULL;
   if( data.valid() )
   {
      status = data->getDataObjectStatus();
   }
   if( (status == RSPF_NULL) || (status == RSPF_EMPTY) )
   {
      return;
   }
   
   rspfDrect inputRect = m_inputR0Rect;
   inputRect = inputRect*rspfDpt(closestScale, closestScale);
   m_Resampler->setBoundingInputRect(inputRect);
   
   double denominatorY = 1.0;
   if(tile_size.y > 2)
   {
      denominatorY = tile_size.y-1.0;
   }
   
   rspfDpt newScale( imageToViewScale.x / closestScale,
                     imageToViewScale.y / closestScale );
   m_Resampler->setScaleFactor(newScale);
   
   m_Resampler->resample(data,
                         outputData,
                         vrect,
                         nul,
                         nur,
                         rspfDpt( ( (nll.x - nul.x)/denominatorY ),
                                   ( (nll.y - nul.y)/denominatorY ) ),
                         rspfDpt( ( (nlr.x - nur.x)/denominatorY ),
                                   ( (nlr.y - nur.y)/denominatorY ) ),
                         tile_size);
   
}

long rspfImageRenderer::computeClosestResLevel(const std::vector<rspfDpt>& decimationFactors,
                                                double scale)const
{
   long result = 0;
   long upper  = (long)decimationFactors.size();
   bool done   = false;
   
   if(upper > 1)
   {
      while((result < upper)&&!done)
      {
         if(scale < decimationFactors[result].x)
         {
            ++result;
         }
         else
         {
            done = true;
         }
      }
      
      // now use the higher res level and resample down
      //
      if(result)
      {
         --result;
      }
   }
   return result;
}

rspfIrect rspfImageRenderer::getBoundingRect(rspf_uint32 resLevel)const
{
   //---
   // 01 November 2011:
   // Backed out expand code as the rspfImageViewProjectionTransform::getImageToViewBounds
   // does not handle image on the edge of international date line. D. Burken
   //---
   
   // The input bounding rect as returned here corresponds to "pixel-is-point", i.e., the center
   // of the pixel area for the corners and not the edges which extend 1/2 pixel in all 
   // directions. Because the view is probably a different GSD, first expand the input
   // bounding rect to the edge before transforming to a view rect. Then shrink the view rect by
   // 1/2 pixel to get it to the pixel center (RSPF convention for bounding rect -- this may 
   // need to be revisited in light of the need to match edges, not pixel centers, OLK 09/11).

   // Code functionality moved to below method. (drb - 08 Nov. 2011)
   
   rspfIrect result;
   getBoundingRect( result, resLevel );
   return result;
}

void rspfImageRenderer::getBoundingRect(rspfIrect& rect, rspf_uint32 resLevel) const
{
   if ( isSourceEnabled() )
   {
      rect = m_viewRect;
   }
   else if ( theInputConnection )
   {
      rect = theInputConnection->getBoundingRect(resLevel);
   }
   else
   {
      rect.makeNan();
   }

#if 0 /* Please leave for debug. */
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageRenderer::getBoundingRect(rect, resLevel) debug:\nbounds  = "
         << rect << "\n";
   }
#endif  
}

void rspfImageRenderer::initializeBoundingRects()
{
   m_rectsDirty = true;

   // Get the input bounding rect:
   if ( theInputConnection )
   {
      m_inputR0Rect = theInputConnection->getBoundingRect(0);
   
      if ( m_ImageViewTransform.valid() && !m_inputR0Rect.hasNans() )
      {
         // This will call rspf::round<int> on the dpt's.
         m_viewRect = m_ImageViewTransform->getImageToViewBounds(m_inputR0Rect);
         if ( m_viewRect.hasNans() == false )
         {
            // Clear the dirty flag:
            m_rectsDirty = false;
         }
      }
   }

   if ( m_rectsDirty )
   {
      m_viewRect.makeNan();
   }

#if 0 /* Please leave for debug. */
   rspfNotify(rspfNotifyLevel_DEBUG)
      << "rspfImageRenderer::getBoundingRect(rect, resLevel) debug:\n"
      << "\ninput rect: " << m_inputR0Rect
      << "\nview rect:  " << m_viewRect << endl;
#endif
}

void rspfImageRenderer::initialize()
{
   // Call the base class initialize.
   // Note:  This will reset "theInputConnection" if it changed...
   rspfImageSourceFilter::initialize();

   deallocate();

   m_rectsDirty = true;

   // we will only do this if we are enabled for this could be expensive
   if (m_ImageViewTransform.valid() && !m_ImageViewTransform->isValid() && isSourceEnabled())
   {
      checkIVT(); // This can initialize bounding rects and clear dirty flag.
   }

   if ( m_rectsDirty )
   {
      initializeBoundingRects();
   }
}

void rspfImageRenderer::deallocate()
{
   m_Tile            = 0;
   m_BlankTile       = 0;
   m_TemporaryBuffer = 0;
}

void rspfImageRenderer::allocate()
{
   deallocate();

   if(theInputConnection)
   {
      m_Tile = rspfImageDataFactory::instance()->create(this, this);
      m_BlankTile  = rspfImageDataFactory::instance()->create(this, this);
      
      m_Tile->initialize();
   }
}

bool rspfImageRenderer::saveState(rspfKeywordlist& kwl,
                                   const char* prefix)const
{
   if(m_ImageViewTransform.valid())
   {
      rspfString newPrefix = rspfString(prefix) + rspfString("image_view_trans.");
      
      m_ImageViewTransform->saveState(kwl, newPrefix.c_str());
   }
   if(m_Resampler)
   {
      m_Resampler->saveState(kwl,
                              (rspfString(prefix)+"resampler.").c_str());
   }
   kwl.add(prefix,
           "max_levels_to_compute",
           m_MaxLevelsToCompute);
   
   return rspfImageSource::saveState(kwl, prefix);
}

bool rspfImageRenderer::loadState(const rspfKeywordlist& kwl,
                                   const char* prefix)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfImageRenderer::loadState entered..." << endl;
      
#ifdef RSPF_ID_ENABLED
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "RSPF_ID:  " << RSPF_ID << endl;
#endif      
   }
   
   rspfString newPrefix = rspfString(prefix) + rspfString("image_view_trans.");
   bool result = rspfImageSourceFilter::loadState(kwl, prefix);
   
   if(m_Resampler)
   {
      m_Resampler->loadState(kwl,
                              (rspfString(prefix)+"resampler.").c_str());
   }
   m_ImageViewTransform = 0;
   m_ImageViewTransform = rspfImageViewTransformFactory::instance()->createTransform(kwl, newPrefix.c_str());
   if(!m_ImageViewTransform)
   {
      m_ImageViewTransform = new rspfImageViewProjectionTransform;
   }
   const char* maxLevelsToCompute = kwl.find(prefix,
                                             "max_levels_to_compute");
   if(maxLevelsToCompute)
   {
      m_MaxLevelsToCompute = rspfString(maxLevelsToCompute).toUInt32();
   }
   
   return result;
}

void rspfImageRenderer::setImageViewTransform(rspfImageViewTransform* ivt)
{
   m_ImageViewTransform = ivt;
   
   m_rectsDirty = true; // Want to recompute bounding rects.
   
   if ( m_ImageViewTransform.valid() && !m_ImageViewTransform->isValid() && isSourceEnabled() )
   {
      checkIVT(); // This can initialize bounding rects and clear dirty flag.
   }
   
   if ( m_rectsDirty ) 
   {
      initializeBoundingRects();
   }
}

bool rspfImageRenderer::setView(rspfObject* baseObject)
{
   bool new_view_set = false;
   if(m_ImageViewTransform.valid())
   {
      new_view_set = m_ImageViewTransform->setView(baseObject);

      // If view changes reset the bounding rects.
      initializeBoundingRects();
   }
   return new_view_set;
}

rspfObject* rspfImageRenderer::getView()
{
   if(m_ImageViewTransform.valid())
   {
      return m_ImageViewTransform->getView();
   }
   return 0;
}

const rspfObject* rspfImageRenderer::getView()const
{
   if(m_ImageViewTransform.valid())
   {
      return m_ImageViewTransform->getView();
   }
   return 0;
}

void rspfImageRenderer::getValidImageVertices(vector<rspfIpt>& validVertices,
                                               rspfVertexOrdering ordering,
                                               rspf_uint32 resLevel)const
{
   if(theInputConnection&&m_ImageViewTransform.valid()&&m_ImageViewTransform->isValid())
   {
      theInputConnection->getValidImageVertices(validVertices, ordering, resLevel);
      if(isSourceEnabled())
      {
         rspf_uint32 inputSize = (rspf_uint32)validVertices.size();
         if(inputSize)
         {
            rspfDpt viewPt;
            rspf_uint32 idx = 0;
            // transform each point to the view
            for(idx = 0; idx < inputSize; ++idx)
            {
               m_ImageViewTransform->imageToView(validVertices[idx], viewPt);
               
               // round it to the nearest pixel
               //
               validVertices[idx] = rspfIpt(viewPt);
            }
         }
      }
   }
}

//*************************************************************************************************
// Returns the geometry associated with the image being served out of the renderer. This
// corresponds to the view geometry defined in theIVT.
//*************************************************************************************************
rspfRefPtr<rspfImageGeometry> rspfImageRenderer::getImageGeometry()
{
   // Make sure the IVT was properly initialized
   if (m_ImageViewTransform.valid() && !m_ImageViewTransform->isValid())
      checkIVT();

   rspfImageViewProjectionTransform* ivpt = PTR_CAST(rspfImageViewProjectionTransform, 
                                                      m_ImageViewTransform.get());
   if (ivpt)
   {
      // we need to return the right side since the geometry changed to a view geometry
      return ivpt->getViewGeometry();
   }

   return rspfRefPtr<rspfImageGeometry>();
}

void rspfImageRenderer::connectInputEvent(rspfConnectionEvent& /* event */)
{
   theInputConnection = PTR_CAST(rspfImageSource, getInput(0));
   if(!m_ImageViewTransform.valid())
      m_ImageViewTransform  = new rspfImageViewProjectionTransform;
   
   checkIVT();
   initialize();
}

void rspfImageRenderer::disconnectInputEvent(rspfConnectionEvent& /* event */)
{
   rspfImageViewProjectionTransform* ivpt = PTR_CAST(rspfImageViewProjectionTransform,
                                                      m_ImageViewTransform.get());
   if(ivpt)
      ivpt->setImageGeometry(0);
   
   theInputConnection = 0;
}

void rspfImageRenderer::propertyEvent(rspfPropertyEvent& /* event */)
{
   checkIVT();
   initialize();
}

void rspfImageRenderer::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfString tempName = property->getName();
   
   if((tempName == "Filter type")||
      (tempName == "filter_type"))
   {
      if(m_Resampler)
      {
         m_Resampler->setFilterType(property->valueToString());
      }
   }
   //   else if(tempName == "Blur factor")
   //     {
   //       if(m_Resampler)
   // 	{
   // 	  m_Resampler->setBlurFactor(property->valueToString().toDouble());
   // 	}
   //     }
   else
   {
      rspfImageSourceFilter::setProperty(property);
   }
}
      
rspfRefPtr<rspfProperty> rspfImageRenderer::getProperty(const rspfString& name)const
{
   rspfString tempName = name;
   
   if((tempName == "Filter type")||
      (tempName == "filter_type"))
   {
      std::vector<rspfString> filterNames;
      m_Resampler->getFilterTypes(filterNames);
      
      rspfStringProperty* stringProp = new rspfStringProperty("filter_type",
								m_Resampler->getMinifyFilterTypeAsString(),
								false,
								filterNames);
      stringProp->clearChangeType();
      stringProp->setReadOnlyFlag(false);
      stringProp->setCacheRefreshBit();
      
      return stringProp;
   }
//   else if(tempName == "Blur factor")
//   {
//      rspfNumericProperty* numericProperty = new rspfNumericProperty("Blur factor",
//                                                                       rspfString::toString((double)m_Resampler->getBlurFactor()));
   
//      numericProperty->setConstraints(0.0, 50.0);
//      numericProperty->setNumericType(rspfNumericProperty::rspfNumericPropertyType_FLOAT64);
//      numericProperty->setCacheRefreshBit();
   
//      return numericProperty;
//   }
   
   return rspfImageSourceFilter::getProperty(name);
}

void rspfImageRenderer::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
  rspfImageSourceFilter::getPropertyNames(propertyNames);

  propertyNames.push_back("Filter type");
//  propertyNames.push_back("Blur factor");
}

//*************************************************************************************************
// Insures that a proper IVT is established.
//*************************************************************************************************
void rspfImageRenderer::checkIVT()
{
   if(!isSourceEnabled())
      return;

   // Check validity of the IVT:
   if (m_ImageViewTransform->isValid())
      return;

   // Detected uninitialized IVT. We are only concerned with projection IVTs (IVPTs) so 
   // make sure that's what we're working with:
   rspfImageViewProjectionTransform* ivpt = 
      PTR_CAST(rspfImageViewProjectionTransform, m_ImageViewTransform.get());
   rspfImageSource* inputSrc = PTR_CAST(rspfImageSource, getInput(0));

   if(!ivpt || !inputSrc) 
      return; // nothing to do here yet.

   // Fetch the input image geometry from the IVPT to see if one needs to be established:
   rspfRefPtr<rspfImageGeometry> inputGeom = ivpt->getImageGeometry();
   if ( !inputGeom )
   {
      // Ask the input source for a geometry:
      inputGeom = inputSrc->getImageGeometry();
      if ( !inputGeom )
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN)<<"rspfImageRenderer::checkTransform() -- "
            "No input image geometry could be established for this renderer."<<endl;
         }
         return;
      }
      ivpt->setImageGeometry( inputGeom.get() );
      m_rectsDirty = true;
   }

   // Now check the output view geometry:
   rspfRefPtr<rspfImageGeometry> outputGeom = ivpt->getViewGeometry();
   if (!outputGeom)
   {
      rspfRefPtr<rspfImageGeometry> myOutGeom = new rspfImageGeometry;

      //---
      // If the input geometry sports a map projection instead of a 3D
      // projector, use the same map projection for the view.
      // 
      // Note: Don't use map projections with model transforms as they don't
      // allow for changing resolution.
      //---
      const rspfProjection*  inputProj = inputGeom->getProjection();
      const rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, inputProj);
      if (mapProj && !mapProj->hasModelTransform() )
      {
         rspfProjection* my_proj = PTR_CAST(rspfProjection, mapProj->dup());
         myOutGeom->setProjection(my_proj);
      }
      else 
      {
         // The input geometry uses a 3D projection, so let's default here to a
         // rspfEquDistCylProjection for the view:
         rspfMapProjection* myMapProj = new rspfEquDistCylProjection;
         rspfDpt meters = inputGeom->getMetersPerPixel();
         double GSD = (meters.x + meters.y)/2.0;
         meters.x = GSD;
         meters.y = GSD;
         if(inputProj)
         {
            //---
            // Update the map projection.  Since rspfMapProjection::setOrigin calls
            // rspfMapProjection::update we do that before setUlTiePoints as it in
            // turn calls setUlEastingNorthing(forward(gpt)) which depends on the orgin.
            //---
            myMapProj->setOrigin(inputProj->origin());
            myMapProj->setUlTiePoints( inputProj->origin() );

         }
         myMapProj->setMetersPerPixel(meters);
         myOutGeom->setProjection(myMapProj);
      }
      
      // Set up our IVT with the new output geometry:
      ivpt->setViewGeometry(myOutGeom.get());

      // Must reinitialize bounding rects before calling rspfImageGeometry::setImageSize().
      initializeBoundingRects();

      // Set the size on the rspfImageGeometry.
      rspfIrect rect;
      getBoundingRect(rect, 0);
      myOutGeom->setImageSize( rect.size() );
   }

   if ( m_rectsDirty )
   {
      initializeBoundingRects();
   }
}

void rspfImageRenderer::getDecimationFactor(rspf_uint32 resLevel,
					     rspfDpt& result)const
{
   if(isSourceEnabled())
   {
      result = rspfDpt(1,1);
   }
   else
   {
      rspfImageSourceFilter::getDecimationFactor(resLevel,
						  result);
   }
}

void rspfImageRenderer::getDecimationFactors(vector<rspfDpt>& decimations)const
{
   if(isSourceEnabled())
   {
      decimations.push_back(rspfDpt(1,1));
   }
   else
   {
      rspfImageSourceFilter::getDecimationFactors(decimations);
   }
}

rspf_uint32 rspfImageRenderer::getNumberOfDecimationLevels()const
{
   if(isSourceEnabled())
   {
      return 1;
   }
   
   return rspfImageSourceFilter::getNumberOfDecimationLevels();
}

void rspfImageRenderer::stretchQuadOut(const rspfDpt& amount,
                                        rspfDpt& ul,
                                        rspfDpt& ur,
                                        rspfDpt& lr,
                                        rspfDpt& ll)
{
   rspfDpt upper  = ur - ul;
   rspfDpt right  = lr - ur;
   rspfDpt bottom = ll - lr;
   rspfDpt left   = ul - ll;
   
   upper = (upper*(1.0 / upper.length()));
   upper.x *= amount.x;
   upper.y *= amount.y;
   right = (right*(1.0 / right.length()));
   right.x *= amount.x;
   right.y *= amount.y;
   bottom = (bottom*(1.0 / bottom.length()));
   bottom.x *= amount.x;
   bottom.y *= amount.y;
   left = (left*(1.0 / left.length()));
   left.x *= amount.x;
   left.y *= amount.y;
   
   
   ul = ul - upper + left;
   ur = ur + upper - right;
   lr = lr + left - bottom;
   ll = ll - left + bottom;
}

rspfRefPtr<rspfImageData>  rspfImageRenderer::getTileAtResLevel(const rspfIrect& boundingRect,
                                                       rspf_uint32 resLevel)
{
   if(!theInputConnection)
   {
      m_BlankTile->setImageRectangle(boundingRect);
      
      return m_BlankTile;
   }
   
   rspf_uint32 levels = theInputConnection->getNumberOfDecimationLevels();
   
   // rspf_uint32 maxValue = (rspf_uint32)rspf::max((rspf_uint32)m_BoundingRect.width(),
   //                                            (rspf_uint32)m_BoundingRect.height());
   if(resLevel == 0)
   {
      return theInputConnection->getTile(boundingRect);
   }
   if(resLevel < levels)
   {
      if(theInputConnection)
      {
         return theInputConnection->getTile(boundingRect,
                                            resLevel);
      }
      else
      {
         return m_BlankTile;
      }
   }
   else if((resLevel - levels) < m_MaxLevelsToCompute)
   {
      // check to see how many decimations we must achiev for the
      // request
      //
      int decimations = (resLevel - (levels-1));
      rspfIpt tileSize(theInputConnection->getTileWidth(),
                        theInputConnection->getTileHeight());
      
      rspf_int32 multiplier = (1 << decimations);
      
      rspf_int32 xIndex = 0;
      rspf_int32 yIndex = 0;
      
      // adjust the tilesize so it at least will cover the multiplier
      // We will probably come up with something better later but for now
      // this will do.
      if(multiplier > tileSize.x)
      {
         tileSize.x = multiplier;
      }
      if(multiplier > tileSize.y)
      {
         tileSize.y = multiplier;
      }
      
      // set the rect that covers the requested RLevel to the valid RLevel. 
      // the valid RLevel is what is available on the input side.
      //
      rspfIrect requestedRectAtValidRLevel = boundingRect;
      requestedRectAtValidRLevel.stretchToTileBoundary(tileSize);
      rspfIrect mappedRequestedRect = requestedRectAtValidRLevel;
      requestedRectAtValidRLevel  = requestedRectAtValidRLevel*((double)multiplier);
      
      if(!m_TemporaryBuffer)
      {
         m_TemporaryBuffer = (rspfImageData*)m_BlankTile->dup();
         m_TemporaryBuffer->setImageRectangle(mappedRequestedRect);
         m_TemporaryBuffer->initialize();
      }
      else
      {
         m_TemporaryBuffer->setImageRectangle(mappedRequestedRect);
         m_TemporaryBuffer->makeBlank();
      }
      
     // rspf_uint32 totalCount   = ((requestedRectAtValidRLevel.lr().y-requestedRectAtValidRLevel.ul().y)*
   //                                (requestedRectAtValidRLevel.lr().x-requestedRectAtValidRLevel.ul().x));
      rspf_uint32 currentCount = 0;
      rspfIrect boundingRect = theInputConnection->getBoundingRect(levels-1);
      for(yIndex = requestedRectAtValidRLevel.ul().y;yIndex < requestedRectAtValidRLevel.lr().y; yIndex += tileSize.y)
      {
         for(xIndex = requestedRectAtValidRLevel.ul().x; xIndex < requestedRectAtValidRLevel.lr().x; xIndex+=tileSize.x)
         {
            rspfIrect request(xIndex,
                               yIndex,
                               xIndex + (tileSize.x-1),
                               yIndex + (tileSize.y-1));
            rspfRefPtr<rspfImageData> data = theInputConnection->getTile(request, levels-1);
            
            if(data.valid() && (data->getDataObjectStatus()!=RSPF_EMPTY)&&
               data->getBuf()&&
               boundingRect.intersects(request))
            {
               switch(data->getScalarType())
               {
                  case RSPF_UINT8:
                  {
                     resampleTileToDecimation((rspf_uint8)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case RSPF_SINT16:
                  {
                     resampleTileToDecimation((rspf_sint16)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case RSPF_UINT16:
                  case RSPF_USHORT11:
                  {
                     resampleTileToDecimation((rspf_uint16)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case RSPF_FLOAT32:
                  case RSPF_NORMALIZED_FLOAT:
                  {
                     resampleTileToDecimation((rspf_float32)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case RSPF_FLOAT64:
                  case RSPF_NORMALIZED_DOUBLE:
                  {
                     resampleTileToDecimation((rspf_float64)0,
                                              m_TemporaryBuffer,
                                              data,
                                              multiplier);
                     break;
                  }
                  case RSPF_SCALAR_UNKNOWN:
                  default:
                  {
                     break;
                  }
               }
            }
            ++currentCount;
         }
      }
      m_TemporaryBuffer->validate();
      return m_TemporaryBuffer;
   }

   return 0;
}

void rspfImageRenderer::setMaxLevelsToCompute(rspf_uint32 maxLevels)
{
   m_MaxLevelsToCompute = maxLevels;
}

rspf_uint32 rspfImageRenderer::getMaxLevelsToCompute()const
{
   return m_MaxLevelsToCompute;
}

template <class T>
void rspfImageRenderer::resampleTileToDecimation(T /* dummyVariable */,
						  rspfRefPtr<rspfImageData> result,
						  rspfRefPtr<rspfImageData> tile,
						  rspf_uint32 multiplier)
{
   if(tile->getDataObjectStatus() == RSPF_EMPTY ||
      !tile->getBuf())
   {
      return;
   }
   
   rspf_int32 maxX     = (rspf_int32)tile->getWidth();
   rspf_int32 maxY     = (rspf_int32)tile->getHeight();
   rspf_int32 resultHeight = result->getHeight();
   rspf_int32* offsetX = new rspf_int32[maxX];
   rspf_int32* offsetY = new rspf_int32[maxY];
   rspf_int32 i        = 0;
   rspf_int32 resultWidth     = (rspf_int32)result->getWidth();
   rspfIpt tileOrigin   = tile->getOrigin();
   rspfIpt resultOrigin = result->getOrigin();
   
   // create a lookup table. that maps the tile to the result
   for(i = 0; i < maxX; ++i)
   {
      offsetX[i] = (i+tileOrigin.x)/(rspf_int32)multiplier - resultOrigin.x;
      if(offsetX[i] < 0 )
      {
         offsetX[i] = 0;
      }
      else if(offsetX[i] >= resultWidth)
      {
         offsetX[i] = resultWidth-1;
      }
   }
   for(i = 0; i < maxY; ++i)
   {
      offsetY[i] = ( ((i+tileOrigin.y)/(rspf_int32)multiplier) - resultOrigin.y);
      if(offsetY[i] < 0 )
      {
         offsetY[i] = 0;
      }
      else if(offsetY[i] >= resultHeight)
      {
         offsetY[i] = resultHeight-1;
      }
      offsetY[i] *= resultWidth;
   }
   
   if(tile->getDataObjectStatus() == RSPF_FULL)
   {
      rspf_int32 numberOfBands = (rspf_int32)std::min(result->getNumberOfBands(),
                                                        tile->getNumberOfBands());
      rspf_int32 band = 0;
      for(band = 0; band < numberOfBands; ++band)
      {
         T* tileBuf   = static_cast<T*>(tile->getBuf(band));
         T* resultBuf = static_cast<T*>(result->getBuf(band));
         rspf_int32 dx = 0;
         rspf_int32 dy = 0;
         rspf_int32 boxAverageX = 0;
         rspf_int32 boxAverageY = 0;
         for(dy = 0; dy < maxY; dy+=multiplier)
         {
            for(dx = 0; dx < maxX; dx+=multiplier)
            {
               double sum = 0.0;
               for(boxAverageY = 0; 
                   ((boxAverageY < (rspf_int32)multiplier)&& 
                    ((boxAverageY+dy)<maxY)); ++boxAverageY)
               {
                  for(boxAverageX = 0; 
                      ((boxAverageX < (rspf_int32)multiplier)&& 
                       ((boxAverageX+dx)<maxX)); ++boxAverageX)
                  {
                     sum += tileBuf[((boxAverageY+dy)*maxX + boxAverageX + dx)];
                  }
               }
               sum /= (double)(multiplier*multiplier);
               resultBuf[ offsetX[dx] + offsetY[dy] ] = (T)sum;
            }
         }
      }
   }
   else
   {
      rspf_int32 numberOfBands = (rspf_int32)std::min(result->getNumberOfBands(),
                                                        tile->getNumberOfBands());
      rspf_int32 band = 0;
      for(band = 0; band < numberOfBands; ++band)
      {
         T* tileBuf   = static_cast<T*>(tile->getBuf(band));
         T* resultBuf = static_cast<T*>(result->getBuf(band));
         T tileBufNp  = static_cast<T>(tile->getNullPix(band));
         rspf_int32 dx = 0;
         rspf_int32 dy = 0;
         rspf_int32 boxAverageX = 0;
         rspf_int32 boxAverageY = 0;
         for(dy = 0; dy < maxY; dy+=multiplier)
         {
            for(dx = 0; dx < maxX; dx+=multiplier)
            {
               double sum = 0.0;
               if(tileBuf[((dy+(multiplier>>1))*maxX + dx+(multiplier>>1))] != tileBufNp)
               {
                  rspf_uint32 nullCount = 0;
                  for(boxAverageY = 0; 
                      ((boxAverageY < (rspf_int32)multiplier)&& 
                       ((boxAverageY+dy)<maxY)); ++boxAverageY)
                  {
                     for(boxAverageX = 0; 
                         ((boxAverageX < (rspf_int32)multiplier)&& 
                          ((boxAverageX+dx)<maxX)); ++boxAverageX)
                     {
                        T value = tileBuf[((boxAverageY+dy)*maxX + boxAverageX + dx)];
                        if(value != tileBufNp)
                        {
                           sum += value;
                        }
                        else
                        {
                           ++nullCount;
                        }
                     }
                  }
                  rspf_uint32 area = multiplier*multiplier;
                  sum /= (double)(area);
                  if(nullCount!= area)
                  {
                     resultBuf[ offsetX[dx] + offsetY[dy] ] = (T)sum;
                  }
               }
            }
         }
      }
      
   }
   
  delete [] offsetX;
  delete [] offsetY;
}

rspfString rspfImageRenderer::getLongName() const
{
   return rspfString("Image Renderer");
}

rspfString rspfImageRenderer::getShortName() const
{
   return rspfString("Image Renderer"); 
}

void rspfImageRenderer::refreshEvent(rspfRefreshEvent& event)
{
   rspfImageSourceFilter::refreshEvent(event);
   rspfImageSourceFilter::initialize(); // init connections
   if((event.getObject()!=this)&&
      (event.getRefreshType() & rspfRefreshEvent::REFRESH_GEOMETRY))
   {
      rspfRefPtr<rspfImageGeometry> inputGeom =
         theInputConnection?theInputConnection->getImageGeometry():0;
      if(inputGeom.valid())
      {
         rspfImageViewProjectionTransform* ivpt = PTR_CAST(rspfImageViewProjectionTransform, 
                                                            m_ImageViewTransform.get());
         if(ivpt)
         {
            ivpt->setImageGeometry(inputGeom.get());
         }
      }
   }
   initialize();
}

void rspfImageRenderer::enableSource()
{
   if ( isSourceEnabled() == false )
   {
      setEnableFlag( true );
   }
}

void rspfImageRenderer::disableSource()
{
   if ( isSourceEnabled() )
   {
      setEnableFlag( false );
   }
}

void rspfImageRenderer::setEnableFlag(bool flag)
{
   if ( getEnableFlag() != flag )
   {
      rspfImageSourceFilter::setEnableFlag( flag );
      checkIVT();
      initialize();
   }
}
