//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*******************************************************************
// $Id: rspfImageRenderer.h 20236 2011-11-09 15:36:38Z dburken $

#ifndef rspfImageRenderer_HEADER
#define rspfImageRenderer_HEADER
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/projection/rspfImageViewTransform.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfViewInterface.h>
#include <rspf/base/rspfRationalNumber.h>

class rspfImageData;
class rspfDiscreteConvolutionKernel;
class rspfFilterResampler;

class RSPFDLLEXPORT rspfImageRenderer : public rspfImageSourceFilter,
                                          public rspfViewInterface
{
public:
   rspfImageRenderer();
   rspfImageRenderer(rspfImageSource* inputSource,
                      rspfImageViewTransform* imageViewTrans = NULL);

   virtual rspfString getLongName()  const;
   virtual rspfString getShortName() const;

   /**
    * the resampler will need the tile request to come from the view.
    * It will use the view to transoform this to world and then use the
    * image's projection to get it into final line sample.  This way
    * the resampler can fill a tile width by height on the screen
    * correctly.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& origin,
                                               rspf_uint32 resLevel=0);

   virtual void initialize();

   /**
    * m_Resampler will adjust the rect to whatever the view is.  So it
    * will project the full image rect onto the view and return the upright
    * bounding rect.
    */
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;

   /**
    * @brief Gets the bounding rectangle of the source.
    *
    * This is the output view bounds.
    * 
    * @param rect Initialized with bounding rectangle by this.
    * @param resLevel Reduced resolution level if applicable.
    */
   virtual void getBoundingRect(rspfIrect& rect,
                                rspf_uint32 resLevel=0) const;

   /**
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   void setImageViewTransform(rspfImageViewTransform* transform);
   rspfImageViewTransform* getImageViewTransform() { return m_ImageViewTransform.get(); }

   //! Returns instance to the input image geometry. This may be a NULL pointer.
   //! This is only valid if the IVT is a projection type IVT (IVPT) 
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   virtual bool setView(rspfObject* baseObject);
   rspfFilterResampler* getResampler() { return m_Resampler; }
   virtual rspfObject* getView();
   virtual const rspfObject* getView()const;

   virtual void getDecimationFactor(rspf_uint32 resLevel, rspfDpt& result)const;
  virtual void getDecimationFactors(vector<rspfDpt>& decimations)const;
  virtual rspf_uint32 getNumberOfDecimationLevels()const;
  
   virtual void setAutoUpdateInputTransformFlag(bool flag){ m_AutoUpdateInputTransform = flag; }
   
   /**
    * ordering specifies how the vertices should be arranged.
    * valid image vertices is basically the tightly fit convex hull
    * of the image.  Usually an image has NULL values and are
    * internally not upright rectangular.  This can cause
    * problems some spatial filters.
    *
    * We need to make sure that the resampler overrides this method.
    * it needs to transform the vertices to the view's side.
    */
   virtual void getValidImageVertices(vector<rspfIpt>& validVertices,
                                      rspfVertexOrdering ordering=RSPF_CLOCKWISE_ORDER,
                                      rspf_uint32 resLevel=0)const;

   void setMaxLevelsToCompute(rspf_uint32 maxLevels);
   rspf_uint32 getMaxLevelsToCompute()const;
   
   void connectInputEvent(rspfConnectionEvent& event);
   void disconnectInputEvent(rspfConnectionEvent& event);
   void propertyEvent(rspfPropertyEvent& event);
   virtual void refreshEvent(rspfRefreshEvent& event);
   
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;

   /**
    * @brief Enables source.
    *
    * Overrides rspfSource::enableSource so bounding rects are recomputed on
    * state change.
    */
   virtual void enableSource();

   /**
    * @brief Disables source.
    *
    * Overrides rspfSource::disableSource so bounding rects are recomputed on
    * state change.
    */
   virtual void disableSource();

   /**
    * @brief Sets the enable flag.
    *
    * @param flag True to enable, false to disable.
    * 
    * Overrides rspfSource::setEnableFlag so bounding rects are recomputed on
    * state change.
    */
   virtual void setEnableFlag(bool flag);

   
protected:
   virtual ~rspfImageRenderer();

private:
   
   class rspfRendererSubRectInfo
   {
   public:
      friend std::ostream& operator <<(std::ostream& out, const rspfRendererSubRectInfo& rhs)
      {
         return out << "vul:   " << rhs.m_Vul << endl
            << "vur:   " << rhs.m_Vur << endl
            << "vlr:   " << rhs.m_Vlr << endl
            << "vll:   " << rhs.m_Vll << endl
            << "iul:   " << rhs.m_Iul << endl
            << "iur:   " << rhs.m_Iur << endl
            << "ilr:   " << rhs.m_Ilr << endl
            << "ill:   " << rhs.m_Ill << endl
            << "scale: " << rhs.m_ViewToImageScale << endl;

      }

      rspfRendererSubRectInfo();
      rspfRendererSubRectInfo(const rspfDpt& vul,
                               const rspfDpt& vur,
                               const rspfDpt& vlr,
                               const rspfDpt& vll);
         
      bool imageHasNans()const;
      bool imageIsNan()const;
      bool viewHasNans()const;
      bool viewIsNan()const;
      void splitView(rspfImageViewTransform* transform,
                     rspfRendererSubRectInfo& ulRect,
                     rspfRendererSubRectInfo& urRect,
                     rspfRendererSubRectInfo& lrRect,
                     rspfRendererSubRectInfo& llRect)const;
      
      void transformViewToImage(rspfImageViewTransform* transform);
      void transformImageToView(rspfImageViewTransform* transform);
      
      void roundToInteger();
      void stretchImageOut(bool enableRound=false);
      rspfDrect getViewRect()const;
      rspfDrect getImageRect()const;
      void roundImageToInteger();
      void roundViewToInteger();
      bool isViewEqual(const rspfRendererSubRectInfo& infoRect)const;
      bool isViewEqual(const rspfDrect& viewRect)const;
      rspfDpt getAbsValueViewToImageScales()const;
      rspfDpt getAbsValueImageToViewScales()const;
      bool isViewAPoint()const;
      bool isIdentity()const;
      bool canBilinearInterpolate(rspfImageViewTransform* transform, double error)const;

      rspfDpt getParametricCenter(const rspfDpt& ul, const rspfDpt& ur, 
				    const rspfDpt& lr, const rspfDpt& ll)const;

     void getViewMids(rspfDpt& upperMid,
		      rspfDpt& rightMid,
		      rspfDpt& bottomMid,
		      rspfDpt& leftMid,
		      rspfDpt& center)const;

     void getImageMids(rspfDpt& upperMid,
		       rspfDpt& rightMid,
		       rspfDpt& bottomMid,
		       rspfDpt& leftMid,
		       rspfDpt& center)const;

      rspfDpt m_Iul;
      rspfDpt m_Iur;
      rspfDpt m_Ilr;
      rspfDpt m_Ill;

      rspfIpt m_Vul;
      rspfIpt m_Vur;
      rspfIpt m_Vlr;
      rspfIpt m_Vll;

      rspfDpt m_ViewToImageScale;
      rspfDpt m_ImageToViewScale;
      
   };

   void recursiveResample(rspfRefPtr<rspfImageData> outputData,
                          const rspfRendererSubRectInfo& rectInfo,
			  rspf_uint32 level);
   

   void fillTile(rspfRefPtr<rspfImageData> outputData,
                 const rspfRendererSubRectInfo& rectInfo);
                 
   rspfIrect getBoundingImageRect()const;

   
   //! this is called on a property event and on input connection changes.
   void checkIVT();

   /**
    * @brief Initializes m_inputR0Rect and m_viewBoundingRect and sets
    * m_rectsDirty appropriately.
    */
   void initializeBoundingRects();

   rspfRefPtr<rspfImageData> getTileAtResLevel(const rspfIrect& boundingRect,
                                     rspf_uint32 resLevel);
  template <class T>
  void resampleTileToDecimation(T dummyVariable,
				rspfRefPtr<rspfImageData> result,
				rspfRefPtr<rspfImageData> tile,
				rspf_uint32 multiplier);

   long computeClosestResLevel(const std::vector<rspfDpt>& decimationFactors,
                               double scale)const;
   void stretchQuadOut(const rspfDpt& amount,
                       rspfDpt& ul,
                       rspfDpt& ur,
                       rspfDpt& lr,
                       rspfDpt& ll);

   /**
    * Called on first getTile, will initialize all data needed.
    */
   void allocate();

   /**
    *  Deletes all allocated tiles.
    */
   void deallocate();

   rspfFilterResampler*       m_Resampler;
   rspfRefPtr<rspfImageData> m_BlankTile;
   rspfRefPtr<rspfImageData> m_Tile;
   rspfRefPtr<rspfImageData> m_TemporaryBuffer;

   /**
    * This is going to allow us to chain multiple
    * renderers together.  So if we have one
    * renderer doing a scale and they pass an r-level
    * down and we have another renderer within the
    * chain he will be starting at a different r-level.
    * The default will be r-level 0 request coming
    * from the right.
    */
   rspf_uint32             m_StartingResLevel;
   rspfRefPtr<rspfImageViewTransform> m_ImageViewTransform;

   rspfIrect               m_inputR0Rect;
   rspfIrect               m_viewRect;
   bool                     m_rectsDirty;

   rspf_uint32             m_MaxRecursionLevel;
   bool                     m_AutoUpdateInputTransform;
   rspf_uint32             m_MaxLevelsToCompute;
   
TYPE_DATA
};

inline rspfImageRenderer::rspfRendererSubRectInfo::rspfRendererSubRectInfo()
{
   m_Vul.makeNan();
   m_Vur.makeNan();
   m_Vlr.makeNan();
   m_Vll.makeNan();
   m_Iul.makeNan();
   m_Iur.makeNan();
   m_Ilr.makeNan();
   m_Ill.makeNan();
   m_ViewToImageScale.makeNan();
   m_ImageToViewScale.makeNan();            
}

inline rspfImageRenderer::rspfRendererSubRectInfo::rspfRendererSubRectInfo(const rspfDpt& vul,
                         const rspfDpt& vur,
                         const rspfDpt& vlr,
                         const rspfDpt& vll)
                         :m_Vul(vul),
                         m_Vur(vur),
                         m_Vlr(vlr),
                         m_Vll(vll)
{
   m_Iul.makeNan();
   m_Iur.makeNan();
   m_Ilr.makeNan();
   m_Ill.makeNan();
   m_ViewToImageScale.makeNan();
   m_ImageToViewScale.makeNan();            
}

inline bool rspfImageRenderer::rspfRendererSubRectInfo::imageHasNans()const
{
   return ( m_Iul.hasNans()||
      m_Iur.hasNans()||
      m_Ilr.hasNans()||
      m_Ill.hasNans());
}

inline bool rspfImageRenderer::rspfRendererSubRectInfo::imageIsNan()const
{
   return ( m_Iul.hasNans()&&
      m_Iur.hasNans()&&
      m_Ilr.hasNans()&&
      m_Ill.hasNans());
}

inline bool rspfImageRenderer::rspfRendererSubRectInfo::viewHasNans()const
{
   return ( m_Vul.hasNans()||
      m_Vur.hasNans()||
      m_Vlr.hasNans()||
      m_Vll.hasNans());
}

inline bool rspfImageRenderer::rspfRendererSubRectInfo::viewIsNan()const
{
   return ( m_Vul.hasNans()&&
      m_Vur.hasNans()&&
      m_Vlr.hasNans()&&
      m_Vll.hasNans());
}

inline void rspfImageRenderer::rspfRendererSubRectInfo::roundToInteger()
{
   m_Iul = rspfIpt(m_Iul);
   m_Iur = rspfIpt(m_Iur);
   m_Ilr = rspfIpt(m_Ilr);
   m_Ill = rspfIpt(m_Ill);

   m_Vul = rspfIpt(m_Vul);
   m_Vur = rspfIpt(m_Vur);
   m_Vlr = rspfIpt(m_Vlr);
   m_Vll = rspfIpt(m_Vll);
}

inline rspfDrect rspfImageRenderer::rspfRendererSubRectInfo::getViewRect()const
{
   return rspfDrect(m_Vul,
      m_Vur,
      m_Vlr,
      m_Vll);            
}

inline rspfDrect rspfImageRenderer::rspfRendererSubRectInfo::getImageRect()const
{
   return rspfDrect(m_Iul,
      m_Iur,
      m_Ilr,
      m_Ill);
}

inline void rspfImageRenderer::rspfRendererSubRectInfo::roundImageToInteger()
{
   m_Iul = rspfIpt(m_Iul);
   m_Iur = rspfIpt(m_Iur);
   m_Ilr = rspfIpt(m_Ilr);
   m_Ill = rspfIpt(m_Ill);
}

inline void rspfImageRenderer::rspfRendererSubRectInfo::roundViewToInteger()
{
   m_Vul = rspfIpt(m_Vul);
   m_Vur = rspfIpt(m_Vur);
   m_Vlr = rspfIpt(m_Vlr);
   m_Vll = rspfIpt(m_Vll);
}

inline bool rspfImageRenderer::rspfRendererSubRectInfo::isViewEqual(const rspfRendererSubRectInfo& infoRect)const
{
   return ( (m_Vul == infoRect.m_Vul)&&
      (m_Vur == infoRect.m_Vur)&&
      (m_Vlr == infoRect.m_Vlr)&&
      (m_Vll == infoRect.m_Vll));
}

inline bool rspfImageRenderer::rspfRendererSubRectInfo::isViewEqual(const rspfDrect& viewRect)const
{
   return ( (m_Vul == viewRect.ul())&&
      (m_Vur == viewRect.ur())&&
      (m_Vlr == viewRect.lr())&&
      (m_Vll == viewRect.ll()));
}

inline rspfDpt rspfImageRenderer::rspfRendererSubRectInfo::getAbsValueViewToImageScales()const
{
   if(m_ViewToImageScale.hasNans())
   {
      return m_ImageToViewScale;
   }
   return rspfDpt(fabs(m_ViewToImageScale.x), fabs(m_ViewToImageScale.y));
}

inline rspfDpt rspfImageRenderer::rspfRendererSubRectInfo::getAbsValueImageToViewScales()const
{
   if(m_ImageToViewScale.hasNans())
   {
      return m_ImageToViewScale;
   }

   return rspfDpt(fabs(m_ImageToViewScale.x), fabs(m_ImageToViewScale.y));
}

inline bool rspfImageRenderer::rspfRendererSubRectInfo::isViewAPoint()const
{
   return ((m_Vul == m_Vur)&&
      (m_Vul == m_Vlr)&&
      (m_Vul == m_Vll));
}

#endif
