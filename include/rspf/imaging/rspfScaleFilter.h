//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfScaleFilter.h 17932 2010-08-19 20:34:35Z dburken $
#ifndef rspfScaleFilter_HEADER
#define rspfScaleFilter_HEADER
#include <vector>
#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/matrix/newmat.h>
#include <rspf/matrix/newmatio.h>
#include <rspf/imaging/rspfFilter.h>

/*!
 */
class rspfScaleFilter : public rspfImageSourceFilter
{
public:
   enum rspfScaleFilterType
   {
      rspfScaleFilter_NEAREST_NEIGHBOR = 0,
      rspfScaleFilter_BOX              = 1,
      rspfScaleFilter_GAUSSIAN         = 2,
      rspfScaleFilter_CUBIC            = 3,
      rspfScaleFilter_HANNING          = 4,
      rspfScaleFilter_HAMMING          = 5,
      rspfScaleFilter_LANCZOS          = 6,
      rspfScaleFilter_MITCHELL         = 7,
      rspfScaleFilter_CATROM           = 8,
      rspfScaleFilter_BLACKMAN         = 9,
      rspfScaleFilter_BLACKMAN_SINC    = 10,
      rspfScaleFilter_BLACKMAN_BESSEL  = 11,
      rspfScaleFilter_QUADRATIC        = 12,
      rspfScaleFilter_TRIANGLE         = 13,
      rspfScaleFilter_HERMITE          = 14
   };
   
   rspfScaleFilter();
   rspfScaleFilter(rspfImageSource* inputSource,
                    const rspfDpt& scaleFactor);
   
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                   rspf_uint32 resLevel=0);
   
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;

   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   virtual void initialize();
   virtual void getDecimationFactor(rspf_uint32 resLevel,
                                    rspfDpt& result)const
      {
         result.makeNan();

         if(resLevel == 0)
         {
            result.x = 1.0;
            result.y = 1.0;
         }
      }
   virtual void getDecimationFactors(vector<rspfDpt>& decimations)const
      {
         decimations.push_back(rspfDpt(1.0, 1.0));
      }
   virtual rspf_uint32 getNumberOfDecimationLevels()const
      {
         // only full res output since we are scaling
         //
         return 1;
      }
   void setFilterType(rspfScaleFilterType filterType);
   void setFilterType(rspfScaleFilterType minifyFilterType,
                      rspfScaleFilterType magnifyFilterType);
   
   void setFilterType(const rspfString& minifyType,
                      const rspfString& magnifyType)
      {
         setFilterType(getFilterType(minifyType),
                       getFilterType(magnifyType));
      }
   void setMinifyFilterType(const rspfString& minifyType)
      {
         setMinifyFilterType(getFilterType(minifyType));
      }
   void setMagnifyFilterType(const rspfString& magnifyType)
      {
         setMagnifyFilterType(getFilterType(magnifyType));
      }
   void setMinifyFilterType(rspfScaleFilterType filterType)
      {
         setFilterType(filterType,
                       m_MagnifyFilterType);
      }
   void setMagnifyFilterType(rspfScaleFilterType filterType)
      {
         setFilterType(m_MinifyFilterType,filterType);
      }
   rspfString getMinifyFilterTypeAsString()const
      {
         return getFilterTypeAsString(m_MinifyFilterType);
      }
   rspfString getMagnifyFilterTypeAsString()const
      {
         return getFilterTypeAsString(m_MagnifyFilterType);
      }
   void setScaleFactor(const rspfDpt& scale);
   void setBlurFactor(rspf_float64 blur)
      {
         m_BlurFactor = blur;
      }
   rspf_float64 getBlurFactor()const
      {
         return m_BlurFactor;
      }
   const rspfDpt& getScaleFactor()const
      {
         return m_ScaleFactor;
      }
   /*!
    * Saves the state of this object.
    */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;

   /*!
    * Loads the state of this object.
    */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   
protected:
   virtual ~rspfScaleFilter();
  void allocate();

   //! If this object is maintaining an rspfImageGeometry, this method needs to be called after 
   //! a scale change so that the geometry's projection is modified accordingly.
   void updateGeometry();
   
   rspfRefPtr<rspfImageData> m_BlankTile;
   rspfRefPtr<rspfImageData> m_Tile;
   rspfFilter*                m_MinifyFilter;
   rspfFilter*                m_MagnifyFilter;
   rspfScaleFilterType        m_MinifyFilterType;
   rspfScaleFilterType        m_MagnifyFilterType;
   rspfDpt                    m_ScaleFactor;
   rspfDpt                    m_InverseScaleFactor;
   rspfIpt                    m_TileSize;
   rspfIrect                  m_InputRect;
   rspf_float64               m_BlurFactor;
   rspfRefPtr<rspfImageGeometry> m_ScaledGeometry; //!< The input image geometry, altered by the scale

   template <class T>
   void runFilterTemplate(T dummy,
                          const rspfIrect& imageRect,
                          const rspfIrect& viewRect);
   template <class T>
   void runHorizontalFilterTemplate(T dummy,
                                    const rspfRefPtr<rspfImageData>& input,
                                    rspfRefPtr<rspfImageData>& output);
   template <class T>
   void runVerticalFilterTemplate(T dummy,
                                  const rspfRefPtr<rspfImageData>& input,
                                  rspfRefPtr<rspfImageData>& output);
    void runFilter(const rspfIrect& imageRect,
                   const rspfIrect& viewRect);

   void getSupport(double& x, double& y);
   const rspfFilter* getHorizontalFilter()const;
   const rspfFilter* getVerticalFilter()const;
   
   rspfString getFilterTypeAsString(rspfScaleFilterType type)const;
   rspfScaleFilterType getFilterType(const rspfString& type)const;
   
   rspfIrect scaleRect(const rspfIrect input,
                        const rspfDpt& scaleFactor)const;
   rspfFilter* createNewFilter(rspfScaleFilterType filterType,
                                rspfScaleFilterType& result);
TYPE_DATA
};

#endif
