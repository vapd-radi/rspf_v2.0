//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Mingjie Su
// 
// Description: implementation for image generator
//
//*************************************************************************
// $Id: rspfTilingRect.h 2725 2011-06-15 18:13:07Z david.burken $
#ifndef rspfTilingRect_HEADER
#define rspfTilingRect_HEADER 1

#include <rspf/imaging/rspfTiling.h>
#include <rspf/imaging/rspfGeoPolyCutter.h>
#include <rspf/imaging/rspfGeoAnnotationPolyObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiPolyObject.h>


class RSPF_DLL rspfTilingRect : public rspfTiling
{
public:

   rspfTilingRect();

   virtual ~rspfTilingRect();

   virtual bool initialize(const rspfMapProjection& proj,
                           const rspfIrect& boundingRect);

   virtual bool next(rspfRefPtr<rspfMapProjection>& resultProjection,
                     rspfIrect& resultingBounds,
                     rspfString& resultingName);

   virtual bool saveState(rspfKeywordlist& kwl,
      const char* prefix=0)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
      const char* prefix=0);

protected:

   bool initializeBase(const rspfMapProjection& proj, 
                       const rspfIrect& boundingRect,
                       rspfDpt& convertedTilingDistance);

   void getConvertedDistance(rspfDpt& pt, rspfDpt inputDistance) const;

   rspfDpt getPaddingSize() const;

   bool                   m_clipToAoi;
   rspf_float64          m_lastWidth;
   rspf_float64          m_lastHeight;
   bool                   m_useOffset;
   rspfDpt               m_offsetInPixels;
   rspfDpt               m_tilingDistanceInPixels;
   rspfDpt               m_tilinResolutionDistance;
   rspfUnitType          m_tilingResolutionUnitType;
   rspfDpt               m_fractionalPixelMisalignment;
                    
TYPE_DATA
};

#endif

