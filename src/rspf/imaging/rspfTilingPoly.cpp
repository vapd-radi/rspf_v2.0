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
// $Id: rspfTilingPoly.cpp 2725 2011-06-15 18:13:07Z david.burken $

#include <sstream>
#include <iomanip>
#include <rspf/imaging/rspfTilingPoly.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfEsriShapeFileInterface.h>

static rspfTrace traceDebug("rspfTilingPoly:debug");

RTTI_DEF1(rspfTilingPoly, "rspfTilingPoly", rspfTilingRect);

rspfTilingPoly::rspfTilingPoly()
      : rspfTilingRect(),
      m_shpFilename(""),
      m_useMbr(true),
      m_bufferDistance(0.0),
      m_exteriorCut(0),
      m_interiorCut(0),
      m_featureBoundingIntersect(true)
{
}

rspfTilingPoly::~rspfTilingPoly()
{
   m_exteriorCut = 0; // Ref ptr, not a leak.
   m_interiorCut = 0; // ditto...
   m_features.clear();
}

bool rspfTilingPoly::initialize(const rspfMapProjection& proj,
                             const rspfIrect& boundingRect)
{
   bool result = false;
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfTilingPoly::initialize DEBUG: Entered..."
         << "\nBounding rect === " << boundingRect
         << std::endl;
   }

   rspfDpt convertedTilingDistance = theTilingDistance;
   result = initializeBase(proj, boundingRect, convertedTilingDistance);
   if (result)
   {
      return parseShpFile();
   }
   return result;
}

bool rspfTilingPoly::next(rspfRefPtr<rspfMapProjection>& resultProjection,
                           rspfIrect& resultingBounds,
                           rspfString& resultingName)
{
   bool result = true;
   
   if(theTileId < theTotalTiles)
   {
      ostringstream idString;
      rspf_int64 tileId = theTileId + 1;
      if (tileId < static_cast<rspf_int64>(m_features.size()))
      {
         idString  << m_features[tileId].m_fid;
         resultingName = theTileNameMask;
         resultingName = resultingName.substitute("%f%", idString.str().c_str());
      }
      result = nextFeature();
      resultingBounds = m_exteriorCut->getRectangle();

      resultProjection = theMapProjection;

      ++theTileId; 
   }
   else
   {
      result = false;
   }

   return result;
}

bool rspfTilingPoly::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   kwl.add(prefix,
           "tile_source",
           m_shpFilename.c_str(),
           true);

   kwl.add(prefix,
      "paddding_use_mbr",
      m_useMbr,
      true);

   return  rspfTilingRect::saveState(kwl, prefix);;
}

bool rspfTilingPoly::loadState(const rspfKeywordlist& kwl,
                            const char* prefix)
{
   bool result = rspfTilingRect::loadState(kwl, prefix);
   
   //Get the shp file if any
   m_shpFilename = kwl.find(prefix, "tile_source");

   // Get the mask if any.
   if (!m_shpFilename.empty())
   {
      if (theTileNameMask.contains("%r%_%c%"))
      {
         theTileNameMask = theTileNameMask.substitute("%r%_%c%", "%f%", true);
      }
      result = true;
   }
 
   const char* useMbr = kwl.find(prefix, "padding_use_mbr");
   if (useMbr)
   {
      if (!rspfString(useMbr).toBool())
      {
         m_useMbr = false;
         m_bufferDistance = thePaddingSizeInPixels.x;
      }
   }

   return result;
}

bool rspfTilingPoly::nextFeature()
{
   if (m_exteriorCut == 0)
   {
      m_exteriorCut = new rspfGeoPolyCutter;
      m_exteriorCut->setView(theMapProjection.get());
   }
   
   bool result = false;
   rspf_int64 tileId = theTileId + 1;//use (theTileId+1) because theTileId is intialized as -1
   if (tileId < static_cast<rspf_int64>(m_features.size())) 
   {
      if (m_features[tileId].m_polygon.size() > 0)
      {
         if (m_features[tileId].m_polyType == rspfGeoAnnotationPolyObject::RSPF_POLY_INTERIOR_RING)
         {
            if (m_interiorCut == 0)
            {
               m_interiorCut = new rspfGeoPolyCutter;
               m_interiorCut->setView(theMapProjection.get());
               m_interiorCut->setCutType(rspfPolyCutter::RSPF_POLY_NULL_INSIDE);
            }
            m_interiorCut->setPolygon(m_features[tileId].m_polygon);
         }
         else
         {
            // std::vector<rspfGpt> points = m_features[tileId].m_polygon;
            // for (rspf_uint32 i = 0; i < points.size(); i++)
            // {
            //    rspfGpt point = points[i];
            // }
            m_exteriorCut->setPolygon(m_features[tileId].m_polygon);
            setRect();
         }
         result = true;
      }
      else if (m_features[tileId].m_multiPolys.size() > 0)
      {
         for (rspf_uint32 i = 0; i < m_features[tileId].m_multiPolys.size(); i++)
         {
            rspfGeoPolygon geoPoly = m_features[tileId].m_multiPolys[i];
            std::vector<rspfGeoPolygon> holePolys = geoPoly.getHoleList();
            if (holePolys.size() > 0)
            {
               if (m_interiorCut == 0)
               {
                  m_interiorCut = new rspfGeoPolyCutter;
                  m_interiorCut->setView(theMapProjection.get());
                  m_interiorCut->setCutType(rspfPolyCutter::RSPF_POLY_NULL_INSIDE);
               }
               m_interiorCut->clear();
               for (rspf_uint32 j = 0; j < holePolys.size(); j++)
               {
                  m_interiorCut->addPolygon(holePolys[j]);
               }
            }
            m_exteriorCut->setPolygon(m_features[tileId].m_multiPolys[i]);
            setRect();
         }
         result = true;
      }
   }
   else
   {
      result = false;
   }
   return result;
}

 void rspfTilingPoly::setRect()
 {
    rspfIrect rect = m_exteriorCut->getRectangle();
    if (rect.intersects(theImageRect))
    {
       m_featureBoundingIntersect = true;
       if (!rect.completely_within(theImageRect))
       {
          rspfIrect clipRect = rect.clipToRect(theImageRect);
          m_exteriorCut->setRectangle(clipRect);
          rect = clipRect;
       }
    }
    else
    {
       m_featureBoundingIntersect = false;
    }
    if (m_useMbr)
    {
       if (thePaddingSizeInPixels.x > 0 && thePaddingSizeInPixels.y > 0)
       {
          rspfIrect newRect = rspfIrect(rect.ul().x-(rspf_int32)thePaddingSizeInPixels.x,
             rect.ul().y-(rspf_int32)thePaddingSizeInPixels.y,
             rect.lr().x + (rspf_int32)thePaddingSizeInPixels.x,
             rect.lr().y + (rspf_int32)thePaddingSizeInPixels.y);
          rspfIrect clipRect = newRect.clipToRect(theImageRect);//in case the bounding is larger than input image after adding buffer
          m_exteriorCut->setRectangle(clipRect);
       }
    }
 }

bool rspfTilingPoly::parseShpFile()
{
   m_features.clear();

   rspfString query = "";
   bool result = false;
   if (m_shpFilename.contains("|"))
   {
      rspfString fileName = m_shpFilename;
      std::vector<rspfString> fileList = fileName.split("|");
      if (fileList.size() > 1)
      {
         m_shpFilename = fileList[0];
         query = fileList[1];
      }
   }

   rspfRefPtr<rspfImageHandler> shpHandler = rspfImageHandlerRegistry::instance()->open(m_shpFilename);
   rspfEsriShapeFileInterface* shpInterface = PTR_CAST(rspfEsriShapeFileInterface, shpHandler.get());
   if (shpInterface != 0)
   {
      if (!query.empty())
      {
         shpInterface->setQuery(query);
      }
      if (m_useMbr == false)
      {
         rspfProjection* proj = shpHandler->getImageGeometry()->getProjection();
         rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, proj);
         if (mapProj)
         {
            if (mapProj->isGeographic())
            {
               m_bufferDistance = thePaddingSizeInPixels.x * theMapProjection->getDecimalDegreesPerPixel().x;
               shpInterface->setGeometryBuffer(m_bufferDistance, RSPF_DEGREES);
            }
            else
            {
               m_bufferDistance = thePaddingSizeInPixels.x * theMapProjection->getMetersPerPixel().x;
               shpInterface->setGeometryBuffer(m_bufferDistance, RSPF_METERS);
            }
         }
      }
      std::multimap<long, rspfAnnotationObject*> features = shpInterface->getFeatureTable();
      if (features.size() > 0)
      {
         std::multimap<long, rspfAnnotationObject*>::iterator it = features.begin();
         while (it != features.end())
         {
            rspfAnnotationObject* anno = it->second;
            if (anno != 0)
            {
               rspfGeoAnnotationPolyObject* annoPoly = PTR_CAST(rspfGeoAnnotationPolyObject, anno);
               rspfGeoAnnotationMultiPolyObject* annoMultiPoly = 0;
               if (annoPoly == 0)
               {
                  annoMultiPoly = PTR_CAST(rspfGeoAnnotationMultiPolyObject, anno);
               }
               if (annoPoly != 0)
               {
                  result = true;
                  std::vector<rspfGpt> polygon;

                  //get the points of a polygon
                  std::vector<rspfGpt> points = annoPoly->getPoints();
                  for (rspf_uint32 i = 0; i < points.size(); i++)
                  {
                     polygon.push_back(points[i]);
                  }

                  //get polygon type, if it is an internal polygon, initialize the internal cutter
                  rspfGeoAnnotationPolyObject::rspfPolyType polyType = annoPoly->getPolyType();
                  rspfShpFeature feature(it->first, polyType, polygon, std::vector<rspfGeoPolygon>()); 
                  m_features.push_back(feature);
               }
               else if (annoMultiPoly != 0)
               {
                  std::vector<rspfGeoPolygon> multiPolys = annoMultiPoly->getMultiPolygon();
                  rspfShpFeature feature(it->first, 0, std::vector<rspfGpt>(), multiPolys); 
               }
            }
            it++;
         }
      }
   }
   if (m_features.size() > 0)
   {
      theTotalTiles = m_features.size();
      result = true;
   }
   return result;
}

bool rspfTilingPoly::useMbr() const
{
   //if the use_mbr is set to true, no matter if the padding is set to 0 or greater than 0,
   //always clip the MBR of each feature
   return m_useMbr;
}

bool rspfTilingPoly::hasExteriorCut() const
{
   return m_exteriorCut.valid();
}

bool rspfTilingPoly::hasInteriorCut() const
{
   return m_interiorCut.valid();
}

rspfRefPtr<rspfGeoPolyCutter>& rspfTilingPoly::getExteriorCut()
{
   return m_exteriorCut;
}

rspfRefPtr<rspfGeoPolyCutter>& rspfTilingPoly::getInteriorCut()
{
   return m_interiorCut;
}

bool rspfTilingPoly::isFeatureBoundingIntersect() const
{
   return m_featureBoundingIntersect;
}
