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
// $Id: rspfTilingPoly.h 2725 2011-06-15 18:13:07Z david.burken $
#ifndef rspfTilingPoly_HEADER
#define rspfTilingPoly_HEADER 1

#include <rspf/imaging/rspfTilingRect.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/imaging/rspfGeoPolyCutter.h>

class RSPF_DLL rspfTilingPoly : public rspfTilingRect
{
public:

   rspfTilingPoly();

   virtual ~rspfTilingPoly();

   virtual bool initialize(const rspfMapProjection& proj,
                           const rspfIrect& boundingRect);

   virtual bool next(rspfRefPtr<rspfMapProjection>& resultProjection,
                     rspfIrect& resultingBounds,
                     rspfString& resultingName);

   virtual bool saveState(rspfKeywordlist& kwl,
      const char* prefix=0)const;

   virtual bool loadState(const rspfKeywordlist& kwl,
      const char* prefix=0);

   bool useMbr() const;

   /** return true if exterior cut is initialized. */
   bool hasExteriorCut() const;

   /** return true if interior cut is initialized. */
   bool hasInteriorCut() const;
   
   rspfRefPtr<rspfGeoPolyCutter>& getExteriorCut();
   
   rspfRefPtr<rspfGeoPolyCutter>& getInteriorCut();

   bool isFeatureBoundingIntersect() const;

protected:

   struct rspfShpFeature
   {
      rspfShpFeature(rspf_int32 fid,
         rspf_int32 polyType,
         std::vector<rspfGpt> polygon,
         std::vector<rspfGeoPolygon> multiPolys)

         :m_fid(fid),
         m_polyType(polyType),
         m_polygon(polygon),
         m_multiPolys(multiPolys)
      {}

      rspf_int32 m_fid;
      rspf_int32 m_polyType;
      std::vector<rspfGpt> m_polygon;
      std::vector<rspfGeoPolygon> m_multiPolys;
   };

   bool parseShpFile();

   bool nextFeature();

   void setRect();

   rspfFilename          m_shpFilename;
   bool                   m_useMbr;
   rspf_float64          m_bufferDistance;
   
   rspfRefPtr<rspfGeoPolyCutter> m_exteriorCut;
   rspfRefPtr<rspfGeoPolyCutter> m_interiorCut;
   
   std::vector<rspfShpFeature> m_features;
   bool m_featureBoundingIntersect;
                    
TYPE_DATA
};

#endif

