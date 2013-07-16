#include <sstream>
#include <rspfGdalOgrVectorAnnotation.h>
#include <rspfOgcWktTranslator.h>
#include <rspfGdalType.h>
#include <rspfOgcWktTranslator.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/base/rspfColorProperty.h>
#include <rspf/base/rspfBooleanProperty.h>
#include <rspf/base/rspfTextProperty.h>
#include <rspf/base/rspfNumericProperty.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfAnnotationLineObject.h>
#include <rspf/imaging/rspfAnnotationMultiLineObject.h>
#include <rspf/imaging/rspfAnnotationPolyObject.h>
#include <rspf/imaging/rspfGeoAnnotationPolyObject.h>
#include <rspf/imaging/rspfGeoAnnotationPolyLineObject.h>
#include <rspf/imaging/rspfGeoAnnotationEllipseObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiEllipseObject.h>
#include <rspf/imaging/rspfGeoAnnotationMultiPolyObject.h>
#include <rspf/base/rspfPolyLine.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/support_data/rspfFgdcXmlDoc.h>
RTTI_DEF2(rspfGdalOgrVectorAnnotation,
          "rspfGdalOgrVectorAnnotation",
          rspfAnnotationSource,
          rspfViewInterface);
static rspfOgcWktTranslator wktTranslator;
static rspfTrace traceDebug("rspfGdalOgrVectorAnnotation:debug");
static const char SHAPEFILE_COLORS_AUTO_KW[] =
   "shapefile_colors_auto";
static const char NORMALIZED_RGB_BRUSH_COLOR_KW[] =
   "shapefile_normalized_rgb_brush_color";
static const char NORMALIZED_RGB_PEN_COLOR_KW[] =
   "shapefile_normalized_rgb_pen_color";
static const char POINT_SIZE_KW[] =
   "shapefile_point_size";
bool doubleLess(double first, double second, double epsilon, bool orequal = false) 
{
   if (fabs(first - second) < epsilon) 
   {
      return (orequal);
   }
   return (first < second);
}
bool doubleGreater(double first, double second, double epsilon, bool orequal = false) 
{
   if (fabs(first - second) < epsilon) 
   {
      return (orequal);
   }
   return (first > second);
}
/** container class for rgb value. */
class rspfRgbColor
{
public:
   /** Default constructor (green) */
   rspfRgbColor() : r(1), g(255), b(1) {}
   /** Constructor that takes an rgb. */
   rspfRgbColor(rspf_uint8 r, rspf_uint8 g, rspf_uint8 b)
      : r(r), g(g), b(b) {}
   
   rspf_uint8 r;
   rspf_uint8 g;
   rspf_uint8 b;
};
static rspf_uint32 currentAutoColorArrayIndex = 0;
static const rspf_uint32 AUTO_COLOR_ARRAY_COUNT = 9;
static const rspfRgbColor autoColorArray[AUTO_COLOR_ARRAY_COUNT] =
{
   rspfRgbColor(255,   1,   1), // red
   rspfRgbColor(  1, 255,   1), // green
   rspfRgbColor(  1,   1, 255), // blue
   
   rspfRgbColor(1,   255, 255),  // cyan
   rspfRgbColor(255,   1, 255),  // magenta
   rspfRgbColor(255, 255,   1),  // yellow
   rspfRgbColor(255, 165,   1), // orange
   rspfRgbColor(160,  32, 240), // purple
   rspfRgbColor(238, 130, 238) // violet
};
class rspfOgrGdalFeatureNode
{
public:
   rspfOgrGdalFeatureNode(long id,
                           const rspfDrect& rect)
      :theId(id),
       theBoundingRect(rect)
      {
      }
                           
   bool intersects(const rspfDrect& rect)const
      {
         return theBoundingRect.intersects(rect);
      }
   bool intersects(double minX, double minY,
                   double maxX, double maxY)const
      {
         return theBoundingRect.intersects(rspfDrect(minX, minY, maxX, maxY));
      }
   long theId;
   rspfDrect theBoundingRect;
};
class rspfOgrGdalLayerNode
{
public:
   rspfOgrGdalLayerNode(const rspfDrect& bounds)
      : theBoundingRect(bounds)
      {
      }
   bool intersects(const rspfDrect& rect)const
      {
         return theBoundingRect.intersects(rect);
      }
   bool intersects(double minX, double minY,
                   double maxX, double maxY)const
      {
         return theBoundingRect.intersects(rspfDrect(minX, minY, maxX, maxY));
      }
   void getIdList(std::list<long>& idList,
                  const rspfDrect& aoi)const;
   std::vector<rspfOgrGdalFeatureNode> theFeatureList;
   rspfDrect theBoundingRect;
};
void rspfOgrGdalLayerNode::getIdList(std::list<long>& idList,
                                      const rspfDrect& aoi)const
{
   if(!intersects(aoi))
   {
      return;
   }
   if(theBoundingRect.completely_within(aoi))
   {
      for(rspf_uint32 i = 0; i < theFeatureList.size(); ++i)
      {
         idList.push_back(theFeatureList[i].theId);
      }
   }
   else
   {
      for(rspf_uint32 i = 0; i < theFeatureList.size(); ++i)
      {
         if(theFeatureList[i].intersects(aoi))
         {
            idList.push_back(theFeatureList[i].theId);
         }
      }
   }
}
rspfGdalOgrVectorAnnotation::rspfGdalOgrVectorAnnotation(rspfImageSource* inputSource)
   :rspfAnnotationSource(inputSource),
    rspfViewInterface(),
    theDataSource(0),
    theDriver(0),
    theFilename(),
    theBoundingExtent(),
    theLayersToRenderFlagList(),
    theLayerTable(),
    thePenColor(255,255,255),
    theBrushColor(255,255,255),
    theFillFlag(false),
    theThickness(1),
    thePointWidthHeight(1, 1),
    theBorderSize(0.0),
    theBorderSizeUnits(RSPF_DEGREES),
    theImageBound(),
    theIsExternalGeomFlag(false),
    m_query(""),
    m_needPenColor(false),
    m_geometryDistance(0.0),
    m_geometryDistanceType(RSPF_UNIT_UNKNOWN),
    m_layerName("")
{
   getDefaults();
   
   theObject = this;
   theImageBound.makeNan();
   rspfAnnotationSource::setNumberOfBands(3);
}
rspfGdalOgrVectorAnnotation::~rspfGdalOgrVectorAnnotation()
{
   rspfViewInterface::theObject = 0;
   close();
}
void rspfGdalOgrVectorAnnotation::close()
{
   deleteTables();
   if(theDataSource)
   {
      delete theDataSource;
      theDataSource = 0;
   }
   if (theImageGeometry.valid())
   {
      theImageGeometry = 0;
   }
}
rspfFilename rspfGdalOgrVectorAnnotation::getFilename()const
{
   return theFilename;
}
void rspfGdalOgrVectorAnnotation::setQuery(const rspfString& query)
{
  m_query = query;
  open(theFilename);
}
void rspfGdalOgrVectorAnnotation::setGeometryBuffer(rspf_float64 distance, rspfUnitType type)
{
   m_geometryDistance = distance;
   m_geometryDistanceType = type;
}
bool rspfGdalOgrVectorAnnotation::open(const rspfFilename& file)
{
   const char* MODULE = "rspfGdalOgrVectorAnnotation::open";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_NOTICE) << MODULE << " entered...\nfile: " << file << "\n";
   }
   
   if(isOpen())
   {
      close();
   }
   m_layerNames.clear();
   if(file == "") return false;
#if 0 /* Commented out but please leave until I test. drb - 15 July 2011 */
   rspfString ext = file.ext().downcase();
   if ( ext != "shp" )
   {
      return false;
   }
#endif
   
   theDataSource = OGRSFDriverRegistrar::Open( file.c_str(), false, &theDriver );
   if ( !theDataSource || !theDriver )
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_NOTICE) << "OGRSFDriverRegistrar::Open failed...\n";
      }
      return false;   
   }
   
   theFilename = file;
   loadExternalGeometryFile();
   loadOmdFile();
   
   theLayersToRenderFlagList.clear();
   vector<rspfGpt> points;
   if(isOpen())
   {
      int i = 0;
      int layerCount = 0;
      if (!m_query.empty())
      {
        layerCount = 1;
      }
      else if (!m_layerName.empty())
      {
         layerCount = 1;
      }
      else
      {
        layerCount = theDataSource->GetLayerCount();
      }
      
      bool successfulTest = true;
      if(layerCount)
      {
         theLayersToRenderFlagList.resize(layerCount);
         for(i = 0; (i < layerCount)&&successfulTest; ++i)
         {
            OGRLayer* layer = NULL;
            if (!m_query.empty())
            {
              layer = theDataSource->ExecuteSQL(m_query, NULL, NULL);
            }
            else if (!m_layerName.empty())
            {
               layer = theDataSource->GetLayerByName(m_layerName.c_str());
            }
            else
            {
              layer = theDataSource->GetLayer(i);
            }
            
            if(layer)
            {
               OGRSpatialReference* spatialReference = layer->GetSpatialRef();
               theLayersToRenderFlagList[i] = true;
               m_layerNames.push_back(rspfString(layer->GetLayerDefn()->GetName()));
               
               if(!spatialReference)
               {
                  if (!theImageGeometry.valid())
                  {
                     loadExternalImageGeometryFromXml();
                  }
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_NOTICE)
                     << MODULE
                     << " No spatial reference given, assuming geographic"
                     << endl;
                  }
               }
               else if(spatialReference->IsLocal())
               {
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_NOTICE)
                     << MODULE
                     << " Only geographic vectors and  projected vectors "
                     << "are supported, layer " << i << " is local" << endl;
                  }
                  successfulTest = false;
               }
               
            }
            else
            {
               if(traceDebug())
               {
                  rspfNotify(rspfNotifyLevel_NOTICE)
                  
                     << MODULE
                     << " layer " << i << " is null." << endl;
               }
               successfulTest = false;
            }
            
            if(successfulTest&&layer)
            {
               if (!theImageGeometry.valid())
               {
                  rspfRefPtr<rspfProjection> proj = createProjFromReference(layer->GetSpatialRef());
                  if(proj.valid())
                  {
                     theImageGeometry = new rspfImageGeometry(0, proj.get());
                  }
               }
               rspfMapProjection* mapProj = 0;
               if(theImageGeometry.valid())
               {
                 mapProj = PTR_CAST(rspfMapProjection, theImageGeometry->getProjection());
               }
               if (!mapProj)
               {
                  theImageGeometry = 0; 
               }
               
               if(i == 0)
               {
                  layer->GetExtent(&theBoundingExtent, true);
                  if(mapProj)
                  {
                     if (layer->GetSpatialRef())
                     {
                        double unitValue = layer->GetSpatialRef()->GetLinearUnits();
                        theBoundingExtent.MinX = theBoundingExtent.MinX * unitValue;
                        theBoundingExtent.MaxY = theBoundingExtent.MaxY * unitValue;
                        theBoundingExtent.MaxX = theBoundingExtent.MaxX * unitValue;
                        theBoundingExtent.MinY = theBoundingExtent.MinY * unitValue;
                     }
                     rspfDrect rect(theBoundingExtent.MinX,
                                     theBoundingExtent.MaxY,
                                     theBoundingExtent.MaxX,
                                     theBoundingExtent.MinY,
                                     RSPF_RIGHT_HANDED);
                     
                     rspfGpt g1 = mapProj->inverse(rect.ul());
                     rspfGpt g2 = mapProj->inverse(rect.ur());
                     rspfGpt g3 = mapProj->inverse(rect.lr());
                     rspfGpt g4 = mapProj->inverse(rect.ll());
                     rspfDrect rect2 = rspfDrect(rspfDpt(g1),
                                                   rspfDpt(g2),
                                                   rspfDpt(g3),
                                                   rspfDpt(g4));
                     
                     theBoundingExtent.MinX = rect2.ul().x;
                     theBoundingExtent.MinY = rect2.ul().y;
                     theBoundingExtent.MaxX = rect2.lr().x;
                     theBoundingExtent.MaxY = rect2.lr().y;
                     points.push_back(g1);
                     points.push_back(g2);
                     points.push_back(g3);
                     points.push_back(g4);
                  }
               }
               else
               {
                  OGREnvelope extent;
                  layer->GetExtent(&extent, true);
                  if(mapProj)
                  {
                     rspfDrect rect(extent.MinX,
                                     extent.MaxY,
                                     extent.MaxX,
                                     extent.MinY,
                                     RSPF_RIGHT_HANDED);
                  
                     rspfGpt g1 = mapProj->inverse(rect.ul());
                     rspfGpt g2 = mapProj->inverse(rect.ur());
                     rspfGpt g3 = mapProj->inverse(rect.lr());
                     rspfGpt g4 = mapProj->inverse(rect.ll());
                     rspfDrect rect2 = rspfDrect(rspfDpt(g1),
                                                   rspfDpt(g2),
                                                   rspfDpt(g3),
                                                   rspfDpt(g4));
                     extent.MinX = rect2.ul().x;
                     extent.MinY = rect2.ul().y;
                     extent.MaxX = rect2.lr().x;
                     extent.MaxY = rect2.lr().y;
                     if (points.size() == 4)
                     {
                        if (doubleLess(g1.lon, points[0].lon, 0.0001))
                        {
                           points[0].lon = g1.lon;
                        }
                        if (doubleGreater(g1.lat, points[0].lat, 0.0001))
                        {
                           points[0].lat = g1.lat;
                        }
                        if (doubleGreater(g2.lon, points[1].lon, 0.0001))
                        {
                           points[1].lon = g2.lon;
                        }
                        if (doubleGreater(g2.lat, points[1].lat, 0.0001))
                        {
                           points[1].lat = g2.lat;
                        }
                        if (doubleGreater(g3.lon, points[2].lon, 0.0001))
                        {
                           points[2].lon = g3.lon;
                        }
                        if (doubleLess(g3.lat, points[2].lat, 0.0001))
                        {
                           points[2].lat = g3.lat;
                        }
                        if (doubleLess(g4.lon, points[3].lon, 0.0001))
                        {
                           points[3].lon = g4.lon;
                        }
                        if (doubleLess(g4.lat, points[3].lat, 0.0001))
                        {
                           points[3].lat = g4.lat;
                        }
                     }
                  }
                  theBoundingExtent.MinX = std::min(extent.MinX,
                                                    theBoundingExtent.MinX);
                  theBoundingExtent.MinY = std::min(extent.MinY,
                                                    theBoundingExtent.MinY);
                  theBoundingExtent.MaxX = std::max(extent.MaxX,
                                                    theBoundingExtent.MaxX);
                  theBoundingExtent.MaxY = std::max(extent.MaxY,
                                                    theBoundingExtent.MaxY);
               }
            }
            if (!m_query.empty() && layer != NULL)
            {
              theDataSource->ReleaseResultSet(layer);
            }  
         }
      }
      if(!successfulTest)
      {
         delete theDataSource;
         theDataSource = NULL;
         theLayersToRenderFlagList.clear();
         
         return false;
      }
   }
   if(traceDebug())
   {
      CLOG << "Extents = "
           << theBoundingExtent.MinX << ", "
           << theBoundingExtent.MinY << ", "
           << theBoundingExtent.MaxX << ", "
           << theBoundingExtent.MaxY << endl;
   }
   if(!theImageGeometry.valid())
   {
      computeDefaultView();
   }
   else
   {
      verifyViewParams();
   }
   initializeBoundingRec(points);
  
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " DEBUG:"
         << "\ntheViewProjection:"
         << endl;
      if(theImageGeometry.valid())
      {
         theImageGeometry->print(rspfNotify(rspfNotifyLevel_DEBUG));
      }
      print(rspfNotify(rspfNotifyLevel_DEBUG));
   }
   return (theDataSource!=NULL);
}
void rspfGdalOgrVectorAnnotation::initializeBoundingRec(vector<rspfGpt> points)
{
   theImageBound.makeNan();
   if (points.size() == 0)
   {
      points.push_back(rspfGpt(theBoundingExtent.MaxY, theBoundingExtent.MinX));
      points.push_back(rspfGpt(theBoundingExtent.MaxY, theBoundingExtent.MaxX));
      points.push_back(rspfGpt(theBoundingExtent.MinY, theBoundingExtent.MaxX));
      points.push_back(rspfGpt(theBoundingExtent.MinY, theBoundingExtent.MinX));
   }
   if(theImageGeometry.valid())
   {
      std::vector<rspfDpt> rectTmp;
      rectTmp.resize(points.size());
      for(std::vector<rspfGpt>::size_type index=0; index < points.size(); ++index)
      {
         theImageGeometry->worldToLocal(points[(int)index], rectTmp[(int)index]);
      }
      if (rectTmp.size() > 3)
      {
         rspfDrect rect2 = rspfDrect(rectTmp[0],
            rectTmp[1],
            rectTmp[2],
            rectTmp[3]);
         theImageBound = rect2;
      }
   }
}
bool rspfGdalOgrVectorAnnotation::setView(rspfObject* baseObject)
{
   bool result = false;
   if(baseObject)
   {
      rspfProjection* p = PTR_CAST(rspfProjection, baseObject);
      if (p)
      {
         if(!theImageGeometry)
         {
            theImageGeometry = new rspfImageGeometry(0, p);
         }
         else
         {
            theImageGeometry->setProjection(p);
         }
         
         transformObjectsFromView();
         result = true;
         
      }
      else
      {
         rspfImageGeometry* geom = dynamic_cast<rspfImageGeometry*> (baseObject);
         if(geom)
         {
            theImageGeometry = geom;
            transformObjectsFromView();
            result = true;
         }
      }
   } // if (baseObject)
   return result;
}
rspfObject* rspfGdalOgrVectorAnnotation::getView()
{
   return theImageGeometry.get();
}
rspfRefPtr<rspfImageGeometry> rspfGdalOgrVectorAnnotation::getImageGeometry() const
{
   return theImageGeometry;
}
void rspfGdalOgrVectorAnnotation::computeDefaultView()
{
   if (theImageGeometry.valid())
      return;
   if(!isOpen())
      return;
   rspfEquDistCylProjection* proj = new rspfEquDistCylProjection;
   rspf_float64 centerLat = (theBoundingExtent.MaxY + theBoundingExtent.MinY ) / 2.0;
   rspf_float64 centerLon = (theBoundingExtent.MaxX + theBoundingExtent.MinX ) / 2.0;
   rspf_float64 deltaLat  = theBoundingExtent.MaxY - theBoundingExtent.MinY;
   rspf_float64 scaleLat = deltaLat / 1024.0;
   rspf_float64 scaleLon = scaleLat*rspf::cosd(std::fabs(centerLat)); 
   rspfGpt origin(centerLat, centerLon, 0.0);
   rspfDpt scale(scaleLon, scaleLat);
   proj->setOrigin(origin);
   proj->setUlGpt( rspfGpt(theBoundingExtent.MaxY,
      theBoundingExtent.MinX) );
   proj->setDecimalDegreesPerPixel(scale);
   theImageGeometry = new rspfImageGeometry(0, proj);
}
rspfIrect rspfGdalOgrVectorAnnotation::getBoundingRect(rspf_uint32 /* resLevel */ )const
{
   return theImageBound;
}
void rspfGdalOgrVectorAnnotation::computeBoundingRect()
{
   std::multimap<long, rspfAnnotationObject*>::iterator iter = theFeatureCacheTable.begin();
   
   theImageBound.makeNan();
   while(iter != theFeatureCacheTable.end())
   {
      rspfGeoAnnotationObject* obj = PTR_CAST(rspfGeoAnnotationObject,
                                               (*iter).second);
      
      if(obj)
      {
         rspfDrect rect;
         obj->getBoundingRect(rect);
         
         if(theImageBound.hasNans())
         {
            theImageBound = rect;
         }
         else
         {
            if(!rect.hasNans())
            {
               theImageBound = theImageBound.combine(rect);
            }
         }
      }
      ++iter;
   }
   theImageBound.stretchOut();
}
void rspfGdalOgrVectorAnnotation::drawAnnotations(
   rspfRefPtr<rspfImageData> tile)
{
   if (theFeatureCacheTable.size() == 0)
   {
      initializeTables();
   }
   if( theImageGeometry.valid())
   {
      list<long> featuresToRender;
      rspfIrect tileRect = tile->getImageRectangle();
      
      getFeatures(featuresToRender, tileRect);
      
      list<long>::iterator current = featuresToRender.begin();
      
      rspfRefPtr<rspfRgbImage> image = new rspfRgbImage;
      
      image->setCurrentImageData(tile);
      vector<rspfAnnotationObject*> objectList;
      
      while(current!=featuresToRender.end())
      {
         getFeature(objectList, *current);
         ++current;
      }
      
      for(rspf_uint32 i = 0; i < objectList.size();++i)
      {
         objectList[i]->draw(*image.get());
        if (theFillFlag && m_needPenColor) //need to draw both the brush and line (pen) for a polygon
        {
          rspfObject* objectDup = objectList[i]->dup();
          rspfGeoAnnotationPolyObject* polyObject = PTR_CAST(rspfGeoAnnotationPolyObject, objectDup);
          if (polyObject)//check if it is the polygon object
          {
            polyObject->setColor(thePenColor.getR(), thePenColor.getG(), thePenColor.getB());
            polyObject->setThickness(theThickness);
            polyObject->setFillFlag(false);
            polyObject->draw(*image.get());
          }
          delete objectDup;
        }
      }
      
      tile->validate();
   }
}
void rspfGdalOgrVectorAnnotation::updateAnnotationSettings()
{
   std::multimap<long, rspfAnnotationObject*>::iterator iter = theFeatureCacheTable.begin();
   while(iter != theFeatureCacheTable.end())
   {
      iter->second->setThickness(theThickness);
      iter->second->setColor(thePenColor.getR(),
                             thePenColor.getG(),
                             thePenColor.getB());
      
      if(PTR_CAST(rspfGeoAnnotationPolyObject, iter->second))
      {
         rspfGeoAnnotationPolyObject* poly =
            (rspfGeoAnnotationPolyObject*)(iter->second);
         poly->setFillFlag(theFillFlag);
      }
      else if(PTR_CAST(rspfGeoAnnotationMultiPolyObject, iter->second))
      {
         rspfGeoAnnotationMultiPolyObject* poly =
            (rspfGeoAnnotationMultiPolyObject*)(iter->second);
         poly->setFillFlag(theFillFlag);
      }
      else if(PTR_CAST(rspfGeoAnnotationEllipseObject, iter->second))
      {
         rspfGeoAnnotationEllipseObject* ell = (rspfGeoAnnotationEllipseObject*)(iter->second);
         ell->setWidthHeight(thePointWidthHeight);
         ell->setFillFlag(theFillFlag);
         ell->transform(theImageGeometry.get());
      }
      if(theFillFlag)
      {
         iter->second->setColor(theBrushColor.getR(),
                                theBrushColor.getG(),
                                theBrushColor.getB());
      }
      ++iter;
   }
}
void rspfGdalOgrVectorAnnotation::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
   rspfString name  = property->getName();
   rspfString value = property->valueToString();
   if(name == rspfKeywordNames::PEN_COLOR_KW)
   {
      int r;  
      int g;  
      int b;
      std::istringstream in(value);
      in >> r >> g >> b;
      thePenColor.setR((unsigned char)r);
      thePenColor.setG((unsigned char)g);
      thePenColor.setB((unsigned char)b);
      updateAnnotationSettings();
   }
   else if(name == rspfKeywordNames::BRUSH_COLOR_KW)
   {
      int r;  
      int g;  
      int b;
      std::istringstream in(value);
      in >> r >> g >> b;
      theBrushColor.setR((unsigned char)r);
      theBrushColor.setG((unsigned char)g);
      theBrushColor.setB((unsigned char)b);
      updateAnnotationSettings();
   }
   else if(name == rspfKeywordNames::FILL_FLAG_KW)
   {
      theFillFlag = value.toBool();
      updateAnnotationSettings();
   }
   else if(name == rspfKeywordNames::THICKNESS_KW)
   {
      setThickness(value.toInt32());
      updateAnnotationSettings();
   }
   else if(name == rspfKeywordNames::BORDER_SIZE_KW)
   {
   }
   else if(name == rspfKeywordNames::POINT_WIDTH_HEIGHT_KW)
   {
      std::istringstream in(value);
      in >> thePointWidthHeight.x;
      in >> thePointWidthHeight.y;
      updateAnnotationSettings();
   }
   else
   {
      rspfAnnotationSource::setProperty(property);
   }
}
rspfRefPtr<rspfProperty> rspfGdalOgrVectorAnnotation::getProperty(const rspfString& name)const
{
   rspfRefPtr<rspfProperty> result;
   if(name == rspfKeywordNames::PEN_COLOR_KW)
   {
      result = new rspfColorProperty(name,
                                      thePenColor);
      result->setCacheRefreshBit();
   }
   else if(name == rspfKeywordNames::BRUSH_COLOR_KW)
   {
      result = new rspfColorProperty(name,
                                      theBrushColor);
      result->setCacheRefreshBit();
   }
   else if(name == rspfKeywordNames::FILL_FLAG_KW)
   {
      result = new rspfBooleanProperty(name,
                                        theFillFlag);
      result->setCacheRefreshBit();
      
   }
   else if(name == rspfKeywordNames::THICKNESS_KW)
   {
      rspfNumericProperty* prop =
         new rspfNumericProperty(name,
                                  rspfString::toString(getThickness()),
                                  1.0,
                                  255.0);
      prop->setNumericType(rspfNumericProperty::rspfNumericPropertyType_INT);
      result = prop;
      result->setFullRefreshBit();
   }
   else if(name == rspfKeywordNames::BORDER_SIZE_KW)
   {
   }
   else if(name == rspfKeywordNames::POINT_WIDTH_HEIGHT_KW)
   {
      result = new rspfTextProperty(name,
                                     rspfString::toString(thePointWidthHeight.x) +
                                     " " +
                                     rspfString::toString(thePointWidthHeight.y));
      result->setFullRefreshBit();
   }
   else
   {
      result = rspfAnnotationSource::getProperty(name);
   }
   
   return result;
}
void rspfGdalOgrVectorAnnotation::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   
   propertyNames.push_back(rspfKeywordNames::PEN_COLOR_KW);
   propertyNames.push_back(rspfKeywordNames::BRUSH_COLOR_KW);
   propertyNames.push_back(rspfKeywordNames::FILL_FLAG_KW);
   propertyNames.push_back(rspfKeywordNames::THICKNESS_KW);
   propertyNames.push_back(rspfKeywordNames::BORDER_SIZE_KW);
   propertyNames.push_back(rspfKeywordNames::POINT_WIDTH_HEIGHT_KW);
}
bool rspfGdalOgrVectorAnnotation::saveState(rspfKeywordlist& kwl,
                                             const char* prefix)const
{
   rspfString s;
   
   kwl.add(prefix,
           rspfKeywordNames::FILENAME_KW,
           theFilename.c_str(),
           true);
   s = rspfString::toString((int)thePenColor.getR()) + " " +
       rspfString::toString((int)thePenColor.getG()) + " " +
       rspfString::toString((int)thePenColor.getB());
   
   kwl.add(prefix,
           rspfKeywordNames::PEN_COLOR_KW,
           s.c_str(),
           true);
   s = rspfString::toString((int)theBrushColor.getR()) + " " +
       rspfString::toString((int)theBrushColor.getG()) + " " +
       rspfString::toString((int)theBrushColor.getB());
   
   kwl.add(prefix,
           rspfKeywordNames::BRUSH_COLOR_KW,
           s.c_str(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::FILL_FLAG_KW,
           (int)theFillFlag,
           true);
   kwl.add(prefix,
           rspfKeywordNames::THICKNESS_KW,
           getThickness(),
           true);
   rspfString border;
   border = rspfString::toString(theBorderSize);
   border += " degrees";
   kwl.add(prefix,
           rspfKeywordNames::BORDER_SIZE_KW,
           border,
           true);
   kwl.add(prefix,
           rspfString(rspfString(rspfKeywordNames::BORDER_SIZE_KW)+
                       "."+
                       rspfKeywordNames::UNITS_KW).c_str(),
           rspfUnitTypeLut::instance()->getEntryString(theBorderSizeUnits),
           true);
   
   s = rspfString::toString((int)thePointWidthHeight.x) + " " +
       rspfString::toString((int)thePointWidthHeight.y);
   
   kwl.add(prefix,
           rspfKeywordNames::POINT_WIDTH_HEIGHT_KW,
           s.c_str(),
           true);
   if (!m_query.empty())
   {
     kwl.add(prefix,
       rspfKeywordNames::QUERY_KW,
       m_query.c_str(),
       true);
   }
   
   if(theImageGeometry.valid())
   {
      rspfString newPrefix = prefix;
      newPrefix += "view_proj.";
      theImageGeometry->saveState(kwl,
                                   newPrefix.c_str());
   }
   
   return rspfAnnotationSource::saveState(kwl, prefix);
}
bool rspfGdalOgrVectorAnnotation::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   const char* filename    = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   const char* penColor    = kwl.find(prefix, rspfKeywordNames::PEN_COLOR_KW);
   const char* brushColor  = kwl.find(prefix, rspfKeywordNames::BRUSH_COLOR_KW);
   const char* fillFlag    = kwl.find(prefix, rspfKeywordNames::FILL_FLAG_KW);
   const char* thickness   = kwl.find(prefix, rspfKeywordNames::THICKNESS_KW);
   const char* pointWh     = kwl.find(prefix, rspfKeywordNames::POINT_WIDTH_HEIGHT_KW);
   const char* border_size = kwl.find(prefix, rspfKeywordNames::BORDER_SIZE_KW);
   const char* query       = kwl.find(prefix, rspfKeywordNames::QUERY_KW);
   
   deleteTables();
   if(thickness)
   {
      setThickness(rspfString(thickness).toInt32());
   }
   
   if(penColor)
   {
      int r = 0;
      int g = 0;
      int b = 0;
      rspfString penColorStr = rspfString(penColor);
      if (penColorStr.split(",").size() == 3)
      {
         r = penColorStr.split(",")[0].toInt();
         g = penColorStr.split(",")[1].toInt();
         b = penColorStr.split(",")[2].toInt();
         if (r == 0 && g == 0 && b == 0)
         {
            r = 1;
            g = 1;
            b = 1;
         }
      }
      thePenColor = rspfRgbVector((rspf_uint8)r, (rspf_uint8)g, (rspf_uint8)b);
      m_needPenColor = true;
   }
   if(brushColor)
   {
      int r = 0;
      int g = 0;
      int b = 0;
      rspfString brushColorStr = rspfString(brushColor);
      if (brushColorStr.split(",").size() == 3)
      {
         r = brushColorStr.split(",")[0].toInt();
         g = brushColorStr.split(",")[1].toInt();
         b = brushColorStr.split(",")[2].toInt();
         if (r == 0 && g == 0 && b == 0)
         {
            r = 1;
            g = 1;
            b = 1;
         }
      }
      theBrushColor = rspfRgbVector((rspf_uint8)r, (rspf_uint8)g, (rspf_uint8)b);
   }
   if(pointWh)
   {
      double w, h;
      std::istringstream s(pointWh);
      s>>w>>h;
      thePointWidthHeight = rspfDpt(w, h);
   }
   
   if(fillFlag)
   {
      theFillFlag = rspfString(fillFlag).toBool();
   }
   theBorderSize = 0.0;
   if(border_size)
   {
      theBorderSize = rspfString(border_size).toDouble();
      rspfString unitPrefix = rspfString(prefix) +
                               rspfKeywordNames::BORDER_SIZE_KW +
                               rspfString(".");
      
      theBorderSizeUnits = (rspfUnitType)rspfUnitTypeLut::instance()->
         getEntryNumber(kwl,
                                                                                unitPrefix.c_str());
      if(theBorderSizeUnits != RSPF_UNIT_UNKNOWN)
      {
         rspfUnitConversionTool unitConvert(theBorderSize,
                                             theBorderSizeUnits);
         
         theBorderSize      = unitConvert.getValue(RSPF_DEGREES);
         theBorderSizeUnits = RSPF_DEGREES;
      }
      else // assume degrees
      {
         theBorderSizeUnits = RSPF_DEGREES;
      }
   }
   else
   {
      theBorderSize      = 0.0;
      theBorderSizeUnits = RSPF_DEGREES;
   }
   
   if(filename)
   {
      if(!open(rspfFilename(filename)))
      {
         return false;
      }
   }
   if (query)
   {
     setQuery(query);
   }
   bool status = rspfAnnotationSource::loadState(kwl, prefix);
   
   initializeTables();
   return status;
}
std::ostream& rspfGdalOgrVectorAnnotation::print(std::ostream& out) const
{
   out << "rspfGdalOgrVectorAnnotation::print"
       << "\ntheLayersToRenderFlagList.size(): "
       << theLayersToRenderFlagList.size()
       << "\ntheLayerTable.size(): " << theLayerTable.size();
   rspf_uint32 i;
   for(i=0; i<theLayersToRenderFlagList.size(); ++i)
   {
      out << "layer[" << i << "]:"
          << (theLayersToRenderFlagList[i]?"enabled":"disabled")
          << std::endl;
   }
   return rspfAnnotationSource::print(out);
}
void rspfGdalOgrVectorAnnotation::transformObjectsFromView()
{
   if (theImageGeometry.valid())
   {
      if (theFeatureCacheTable.size() == 0)
      {
         initializeTables();
      }
      std::multimap<long, rspfAnnotationObject*>::iterator iter =
         theFeatureCacheTable.begin();
      
      while(iter != theFeatureCacheTable.end())
      {
         rspfGeoAnnotationObject* obj = PTR_CAST(rspfGeoAnnotationObject,
                                                  (*iter).second);
         
         if(obj&&theImageGeometry.valid())
         {
            obj->transform(theImageGeometry.get());
         }
         ++iter;
      }
      computeBoundingRect();
   }
}
void rspfGdalOgrVectorAnnotation::getFeatures(std::list<long>& result,
                                               const rspfIrect& rect)
{
   if (isOpen())
   {
      rspfGpt gp1;
      rspfGpt gp2;
      rspfGpt gp3;
      rspfGpt gp4;
      rspfDpt dp1 = rect.ul();
      rspfDpt dp2 = rect.ur();
      rspfDpt dp3 = rect.lr();
      rspfDpt dp4 = rect.ll();
      
      if (theImageGeometry.valid())
      {
         theImageGeometry->localToWorld(dp1, gp1);
         theImageGeometry->localToWorld(dp2, gp2);
         theImageGeometry->localToWorld(dp3, gp3);
         theImageGeometry->localToWorld(dp4, gp4);
         double maxX = std::max( gp1.lond(), std::max( gp2.lond(), std::max(gp3.lond(), gp4.lond())));
         double minX = std::min( gp1.lond(), std::min( gp2.lond(), std::min(gp3.lond(), gp4.lond())));
         double maxY = std::max( gp1.latd(), std::max( gp2.latd(), std::max(gp3.latd(), gp4.latd())));
         double minY = std::min( gp1.latd(), std::min( gp2.latd(), std::min(gp3.latd(), gp4.latd())));
         
         rspfDrect bounds(minX, minY, maxX, maxY);
         
         for(rspf_uint32 layerI = 0;
             layerI < theLayersToRenderFlagList.size();
             ++layerI)
         {
            if(theLayersToRenderFlagList[layerI])
            {
               if(theLayerTable[layerI])
               {
                  theLayerTable[layerI]->getIdList(result, bounds);
               }
            }
         }
      }
   }
}
void rspfGdalOgrVectorAnnotation::initializeTables()
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalOgrVectorAnnotation::initializeTables(): entered.........." << std::endl;
   }
   if(theLayerTable.size())
   {
      deleteTables();
   }
   if(isOpen())
   {
      int upper = theLayersToRenderFlagList.size();
      theLayerTable.resize(upper);
      for(int i = 0; i < upper; ++i)
      {  
         if(theLayersToRenderFlagList[i])
         {
            OGREnvelope extent;
            OGRLayer* layer = NULL;
            if (!m_query.empty())
            {
              layer = theDataSource->ExecuteSQL(m_query, NULL, NULL);
            }
            else if (!m_layerName.empty())
            {
               layer = theDataSource->GetLayerByName(m_layerName.c_str());
            }
            else
            {
              layer = theDataSource->GetLayer(i);
            }
            rspfRefPtr<rspfProjection> proj;
            if (theIsExternalGeomFlag&&theImageGeometry.valid())
            {
               proj = theImageGeometry->getProjection();
            }
            else
            {
               proj = createProjFromReference(layer->GetSpatialRef());
            }
            rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, proj.get());
            
            layer->ResetReading();
            layer->GetExtent(&extent, true);
            layer->ResetReading();
            OGRFeature* feature = NULL;
            if(mapProj)
            {
               rspfDrect rect(extent.MinX,
                               extent.MaxY,
                               extent.MaxX,
                               extent.MinY,
                               RSPF_RIGHT_HANDED);
               rspfGpt g1 = mapProj->inverse(rect.ul());
               rspfGpt g2 = mapProj->inverse(rect.ur());
               rspfGpt g3 = mapProj->inverse(rect.lr());
               rspfGpt g4 = mapProj->inverse(rect.ll());
               rspfDrect rect2 = rspfDrect(rspfDpt(g1),
                                            rspfDpt(g2),
                                            rspfDpt(g3),
                                            rspfDpt(g4));
               theLayerTable[i] = new rspfOgrGdalLayerNode(rect2);
            }
            else
            {
               theLayerTable[i] = new rspfOgrGdalLayerNode(rspfDrect(extent.MinX,
                                                                       extent.MinY,
                                                                       extent.MaxX,
                                                                       extent.MaxY));
            }
           
            while( (feature = layer->GetNextFeature()) != NULL)
            {
                  if(feature)
                  {
                     OGRGeometry* geom = feature->GetGeometryRef();
                     
                     if(geom)
                     {
                        switch(geom->getGeometryType())
                        {
                           case wkbMultiPoint:
                           case wkbMultiPoint25D:
                           {
                              if(traceDebug())
                              {
                                 rspfNotify(rspfNotifyLevel_DEBUG) << "Loading multi point" << std::endl;
                              }
                              loadMultiPoint(feature->GetFID(),
                                             (OGRMultiPoint*)geom,
                                             mapProj);
                              break;
                           }
                           case wkbPolygon25D:
                           case wkbPolygon:
                           {
                              if(traceDebug())
                              {
                                 rspfNotify(rspfNotifyLevel_DEBUG) << "Loading polygon" << std::endl;
                              }
                              if (m_geometryDistance > 0.0)
                              {
                                 OGRPolygon* poly = (OGRPolygon*)geom;
                                 OGRLinearRing* ring = poly->getExteriorRing();
                                 int numPoints = ring->getNumPoints();
                                 OGRGeometry* bufferGeom = geom->Buffer(m_geometryDistance, numPoints);
                                 loadPolygon(feature->GetFID(),
                                    (OGRPolygon*)bufferGeom,
                                    mapProj);
                              }
                              else
                              {
                                 loadPolygon(feature->GetFID(),
                                    (OGRPolygon*)geom,
                                    mapProj);
                              }
                              break;
                           }
                           case wkbLineString25D:
                           case wkbLineString:
                           {
                              if(traceDebug())
                              {
                                 rspfNotify(rspfNotifyLevel_DEBUG) << "Loading line string" << std::endl;
                              }
                              loadLineString(feature->GetFID(),
                                             (OGRLineString*)geom,
                                             mapProj);
                              break;
                           }
                           case wkbPoint:
                           case wkbPoint25D:
                           {
                              if(traceDebug())
                              {
                                 rspfNotify(rspfNotifyLevel_DEBUG) << "Loading point" << std::endl;
                              }
                              loadPoint(feature->GetFID(),
                                        (OGRPoint*)geom,
                                        mapProj);
                              break;
                           }
                           case wkbMultiPolygon25D:
                           case wkbMultiPolygon:
                           {
                              if(traceDebug())
                              {
                                 rspfNotify(rspfNotifyLevel_DEBUG) << "Loading multi polygon" << std::endl;
                              }
                              if (m_geometryDistance > 0.0)
                              {
                                 OGRGeometry* bufferGeom = geom->Buffer(m_geometryDistance);
                                 loadMultiPolygon(feature->GetFID(),
                                    (OGRMultiPolygon*)bufferGeom,
                                    mapProj);
                              }
                              else
                              {
                                 loadMultiPolygon(feature->GetFID(),
                                    (OGRMultiPolygon*)geom,
                                    mapProj);
                              }
                              break;
                                               
                           }
                           case wkbMultiLineString:
                           {
                              if(traceDebug())
                              {
                                 rspfNotify(rspfNotifyLevel_DEBUG) << "Loading line string" << std::endl;
                              }
                              loadMultiLineString(feature->GetFID(),
                                    (OGRMultiLineString*)geom,
                                    mapProj);
                                 break;
                              }
                           default:
                           {
                              if(traceDebug())
                              {
                                 rspfNotify(rspfNotifyLevel_WARN)
                                    << "rspfGdalOgrVectorAnnotation::initializeTables WARNING\n"
                                    
                                    << OGRGeometryTypeToName(geom->getGeometryType())
                                    <<" NOT SUPPORTED!"
                                    << endl;
                              }
                              break;
                           }
                        }
                        geom->getEnvelope(&extent);
                        if(mapProj)
                        {
                           rspfDrect rect(extent.MinX,
                                           extent.MaxY,
                                           extent.MaxX,
                                           extent.MinY,
                                           RSPF_RIGHT_HANDED);
                           rspfGpt g1 = mapProj->inverse(rect.ul());
                           rspfGpt g2 = mapProj->inverse(rect.ur());
                           rspfGpt g3 = mapProj->inverse(rect.lr());
                           rspfGpt g4 = mapProj->inverse(rect.ll());
                           
                           theLayerTable[i]->theFeatureList.push_back(rspfOgrGdalFeatureNode(feature->GetFID(),
                                                                                              rspfDrect(rspfDpt(g1),
                                                                                                         rspfDpt(g2),
                                                                                                         rspfDpt(g3),
                                                                                                         rspfDpt(g4))));
                           
                        }
                        else
                        {
                           theLayerTable[i]->theFeatureList.push_back(rspfOgrGdalFeatureNode(feature->GetFID(),
                                                                                              rspfDrect(extent.MinX,
                                                                                                         extent.MinY,
                                                                                                         extent.MaxX,
                                                                                                         extent.MaxY)));
                        }
                     }
               }
               delete feature;
            }
            if (!m_query.empty() && layer != NULL)
            {
              theDataSource->ReleaseResultSet(layer);
            }  
         }
         else
         {
            theLayerTable[i] = NULL;
         }
      }
   }
   computeBoundingRect();
   updateAnnotationSettings();
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfGdalOgrVectorAnnotation::initializeTables(): leaving..."
         << std::endl;
   }
}
void rspfGdalOgrVectorAnnotation::deleteTables()
{
   for(rspf_uint32 i = 0; i < theLayerTable.size(); ++i)
   {
      if(theLayerTable[i])
      {
         delete theLayerTable[i];
      }
   }
   theLayerTable.clear();
   std::multimap<long, rspfAnnotationObject*>::iterator current = theFeatureCacheTable.begin();
   while(current != theFeatureCacheTable.end())
   {
      ((*current).second)->unref();
      ++current;
   }
   
   theFeatureCacheTable.clear();
}
void rspfGdalOgrVectorAnnotation::getFeature(vector<rspfAnnotationObject*>& featureList,
                                              long id)
{
   std::multimap<long, rspfAnnotationObject*>::iterator iter = theFeatureCacheTable.find(id);
   
   while( (iter != theFeatureCacheTable.end()) && ((*iter).first == id)  )
   {
      featureList.push_back((*iter).second);
      ++iter;
   }
}
rspfProjection* rspfGdalOgrVectorAnnotation::createProjFromReference(OGRSpatialReference* reference)const
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalOgrVectorAnnotation::createProjFromReference:   entered........" << std::endl;
   }
   if(!reference)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalOgrVectorAnnotation::createProjFromReference:   leaving 1........" << std::endl;
      }
      return NULL;
   }
   if(reference->IsGeographic()||reference->IsLocal())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalOgrVectorAnnotation::createProjFromReference:   leaving 2........" << std::endl;
      }
      return NULL;
   }
   char* wktString = NULL;
   rspfKeywordlist kwl;
   reference->exportToWkt(&wktString);
   wktTranslator.toOssimKwl(wktString,
                            kwl);
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "wktString === " << wktString << std::endl;
      rspfNotify(rspfNotifyLevel_DEBUG) << "KWL === " << kwl << std::endl;
   }
   OGRFree(wktString);
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalOgrVectorAnnotation::createProjFromReference:   returning........" << std::endl;
   }
   return rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
}
void rspfGdalOgrVectorAnnotation::loadPolygon(long id, OGRPolygon* poly, rspfMapProjection* mapProj)
{
   OGRLinearRing* ring = poly->getExteriorRing();
   
   rspfGpt origin;
   
   if(theImageGeometry.valid()&&theImageGeometry->getProjection())
   {
      origin = theImageGeometry->getProjection()->origin();
   }
   
   rspfRgbVector color;
   
   if(theFillFlag)
   {
      color = theBrushColor;
   }
   else
   {
      color = thePenColor;
   }
   
   if(ring)
   {
      int upper = ring->getNumPoints();
      vector<rspfGpt> points(upper);
      for(int i = 0; i < upper; ++i)
      {
         OGRPoint ogrPt;
         ring->getPoint(i, &ogrPt);
         if(mapProj)
         {
            points[i] = mapProj->inverse(rspfDpt(ogrPt.getX(),
                                                  ogrPt.getY()));
         }
         else
         {
            points[i] = rspfGpt(ogrPt.getY(),
                                 ogrPt.getX(),
                                 ogrPt.getZ(),
                                 origin.datum());
         }
      }
      rspfGeoAnnotationObject* annotation =
         new rspfGeoAnnotationPolyObject(points,
                                          theFillFlag,
                                          color.getR(),
                                          color.getG(),
                                          color.getB(),
                                          theThickness);
      if(theImageGeometry.valid())
      {
         annotation->transform(theImageGeometry.get());
      }
      
      theFeatureCacheTable.insert(make_pair(id,
                                            annotation));
   }
   int bound = poly->getNumInteriorRings();
   if(bound)
   {
      for(int i = 0; i < bound; ++i)
       {
          ring = poly->getInteriorRing(i);
          if(ring)
          {
             int j = 0;
             int upper = ring->getNumPoints();
             vector<rspfGpt> points(upper);
             for(j = 0; j < upper; ++j)
             {
                OGRPoint ogrPt;
                ring->getPoint(j, &ogrPt);
                if(mapProj)
                {
                   rspfDpt eastingNorthing(ogrPt.getX(),
                                            ogrPt.getY());
                   
                   points[j] = mapProj->inverse(eastingNorthing);
                   
                }
                else
                {
                   points[j] = rspfGpt(ogrPt.getY(),
                                        ogrPt.getX(),
                                        ogrPt.getZ(),
                                        origin.datum());
                }
             }
             rspfGeoAnnotationPolyObject* annotation =
                new rspfGeoAnnotationPolyObject(points,
                                                 theFillFlag,
                                                 color.getR(),
                                                 color.getG(),
                                                 color.getB(),
                                                 theThickness);
             annotation->setPolyType(rspfGeoAnnotationPolyObject::RSPF_POLY_INTERIOR_RING);
       
             if(theImageGeometry.valid())
             {
                annotation->transform(theImageGeometry.get());
             }
             theFeatureCacheTable.insert(make_pair(id,
                                              annotation));
          }
       }
    }
}
void rspfGdalOgrVectorAnnotation::loadLineString(long id, OGRLineString* lineString,
                                            rspfMapProjection* mapProj)
{
    int upper = lineString->getNumPoints();
    rspfGpt origin;
    if(theImageGeometry.valid()&&theImageGeometry->getProjection())
    {
       origin = theImageGeometry->getProjection()->origin();
    }
    
   rspfRgbVector color;
   
   if(theFillFlag)
   {
      color = theBrushColor;
   }
   else
   {
      color = thePenColor;
   }
   vector<rspfGpt> polyLine(upper);
   for(int i = 0; i < upper; ++i)
   {
      OGRPoint ogrPt;
      
      lineString->getPoint(i, &ogrPt);
      
      if(mapProj)
      {
         rspfDpt eastingNorthing(ogrPt.getX(),
                                  ogrPt.getY());
         
         polyLine[i] = mapProj->inverse(eastingNorthing);
      }
      else
      {
         polyLine[i] = rspfGpt(ogrPt.getY(),
                                ogrPt.getX(),
                                ogrPt.getZ(),
                                origin.datum());
      }
   }
   
   rspfGeoAnnotationPolyLineObject* annotation =
      new rspfGeoAnnotationPolyLineObject(polyLine,
                                           color.getR(),
                                           color.getG(),
                                           color.getB(),
                                           theThickness);
   if(theImageGeometry.valid())
   {
      annotation->transform(theImageGeometry.get());
   }
   theFeatureCacheTable.insert(make_pair(id,
                                         annotation));
}
void rspfGdalOgrVectorAnnotation::loadMultiLineString(long id, OGRMultiLineString* multiLineString,
   rspfMapProjection* mapProj)
{
   rspfRgbVector color;
   if(theFillFlag)
   {
      color = theBrushColor;
   }
   else
   {
      color = thePenColor;
   }
   rspf_uint32 numGeometries = multiLineString->getNumGeometries();
   rspfGpt origin;
   if(theImageGeometry.valid()&&theImageGeometry->getProjection())
   {
      origin = theImageGeometry->getProjection()->origin();
   }
   vector<rspfGeoPolygon> geoPoly;
   for(rspf_uint32 geomIdx = 0; geomIdx < numGeometries; ++geomIdx)
   {
      OGRGeometry* geomRef = multiLineString->getGeometryRef(geomIdx);
      OGRLineString* lineString = (OGRLineString*)geomRef;
      if (lineString)
      {
         int upper = lineString->getNumPoints();
         vector<rspfGpt> polyLine(upper);
         for(int i = 0; i < upper; ++i)
         {
            OGRPoint ogrPt;
            lineString->getPoint(i, &ogrPt);
            if(mapProj)
            {
               rspfDpt eastingNorthing(ogrPt.getX(),
                  ogrPt.getY());
               polyLine[i] = mapProj->inverse(eastingNorthing);
            }
            else
            {
               polyLine[i] = rspfGpt(ogrPt.getY(),
                  ogrPt.getX(),
                  ogrPt.getZ(),
                  origin.datum());
            }
         }
         rspfGeoAnnotationPolyLineObject* annotation =
            new rspfGeoAnnotationPolyLineObject(polyLine,
            color.getR(),
            color.getG(),
            color.getB(),
            theThickness);
         if(theImageGeometry.valid())
         {
            annotation->transform(theImageGeometry.get());
         }
         theFeatureCacheTable.insert(make_pair(id,
            annotation));
      }
   }
}
void rspfGdalOgrVectorAnnotation::loadPoint(long id, OGRPoint* point, rspfMapProjection* mapProj)
{
   rspfGpt origin;
   rspfRgbVector color;
   
   if(theFillFlag)
   {
      color = theBrushColor;
   }
   else
   {
      color = thePenColor;
   }
   if(theImageGeometry.valid()&&theImageGeometry->getProjection())
   {
      origin = theImageGeometry->getProjection()->origin();
   }
   
   rspfGpt gpt;
   if(mapProj)
   {
      rspfDpt eastingNorthing(point->getX(),
                               point->getY());
      
      gpt = mapProj->inverse(eastingNorthing);
   }
   else
   {
      gpt = rspfGpt(point->getY(),
                     point->getX(),
                     point->getZ(),
                     origin.datum());
   }
   
   
   rspfGeoAnnotationEllipseObject* annotation =
      new rspfGeoAnnotationEllipseObject(gpt,
                                          thePointWidthHeight,
                                          theFillFlag,
                                          color.getR(),
                                          color.getG(),
                                          color.getB(),
                                          theThickness);
   if(theImageGeometry.valid())
   {
      annotation->transform(theImageGeometry.get());
   }
   theFeatureCacheTable.insert(make_pair(id, annotation));
}
void rspfGdalOgrVectorAnnotation::loadMultiPoint(long id,
                                                  OGRMultiPoint* multiPoint,
                                                  rspfMapProjection* mapProj)
{
   rspf_uint32 numGeometries = multiPoint->getNumGeometries();
   rspfRgbVector color;
   
   if(theFillFlag)
   {
      color = theBrushColor;
   }
   else
   {
      color = thePenColor;
   }
   
   rspfGeoAnnotationMultiEllipseObject* annotation =
      new rspfGeoAnnotationMultiEllipseObject(thePointWidthHeight,
                                               theFillFlag,
                                               color.getR(),
                                               color.getG(),
                                               color.getB(),
                                               theThickness);
   rspfGpt origin;
   if(theImageGeometry.valid()&&theImageGeometry->getProjection())
   {
      origin = theImageGeometry->getProjection()->origin();
   }
   for(rspf_uint32 i = 0; i < numGeometries; ++i)
   {
      OGRGeometry* geomRef = multiPoint->getGeometryRef(i);
      if( geomRef &&
          ( (geomRef->getGeometryType()==wkbPoint) ||
            (geomRef->getGeometryType()==wkbPoint25D) ) )
      {
         OGRPoint* point = (OGRPoint*)geomRef;
         
         if(mapProj)
         {
            rspfDpt eastingNorthing(point->getX(),
                                     point->getY());
            
            annotation->addPoint(mapProj->inverse(eastingNorthing));
         }
         annotation->addPoint(rspfGpt(point->getY(),
                                       point->getX(),
                                       point->getZ(),
                                       origin.datum()));
      }
   }
   if(theImageGeometry.valid())
   {
      annotation->transform(theImageGeometry.get());
   }
   theFeatureCacheTable.insert(make_pair(id, annotation));
}
void rspfGdalOgrVectorAnnotation::loadMultiPolygon(
   long id,
   OGRMultiPolygon* multiPolygon,
   rspfMapProjection* mapProj)
{
   rspfRgbVector color;
   
   if(theFillFlag)
   {
      color = theBrushColor;
   }
   else
   {
      color = thePenColor;
   }
   rspfGpt origin;
   rspf_uint32 numGeometries = multiPolygon->getNumGeometries();
   if(theImageGeometry.valid()&&theImageGeometry->getProjection())
   {
      origin = theImageGeometry->getProjection()->origin();
   }
   vector<rspfGeoPolygon> geoPoly;
   for(rspf_uint32 geomIdx = 0; geomIdx < numGeometries; ++geomIdx)
   {
      OGRGeometry* geomRef = multiPolygon->getGeometryRef(geomIdx);
      if( geomRef &&
          ( (geomRef->getGeometryType()==wkbPolygon) ||
            (geomRef->getGeometryType()==wkbPolygon25D) ) )
      {
         geoPoly.push_back(rspfGeoPolygon());
         OGRPolygon* poly = (OGRPolygon*)geomRef;
         OGRLinearRing* ring = poly->getExteriorRing();
         rspf_uint32 currentPoly = geoPoly.size()-1;
         if(ring)
         {
            rspf_uint32 upper = ring->getNumPoints();
            for(rspf_uint32 ringPointIdx = 0;
                ringPointIdx < upper;
                ++ringPointIdx)
            {
               OGRPoint ogrPt;
               ring->getPoint(ringPointIdx, &ogrPt);
               if(mapProj)
               {
                  geoPoly[currentPoly].addPoint(
                     mapProj->inverse(rspfDpt(ogrPt.getX(), ogrPt.getY())));
               }
               else
               {
                  geoPoly[currentPoly].addPoint(ogrPt.getY(),
                                                ogrPt.getX(),
                                                ogrPt.getZ(),
                                                origin.datum());
               }
            }
         }
         
         rspf_uint32 bound = poly->getNumInteriorRings();
         if(bound)
         {
            for(rspf_uint32 interiorRingIdx = 0;
                interiorRingIdx < bound;
                ++interiorRingIdx)
            {
               ring = poly->getInteriorRing(interiorRingIdx);
               if(ring)
               {
                  geoPoly.push_back(rspfGeoPolygon());
                  currentPoly = geoPoly.size()-1;
                  rspf_uint32 upper = ring->getNumPoints();
                  for(rspf_uint32 interiorRingPointIdx = 0;
                      interiorRingPointIdx < upper;
                      ++interiorRingPointIdx)
                  {
                     OGRPoint ogrPt;
                     ring->getPoint(interiorRingPointIdx, &ogrPt);
                     if(mapProj)
                     {
                        geoPoly[currentPoly].addPoint(
                           mapProj->inverse(rspfDpt(ogrPt.getX(),
                                                     ogrPt.getY())));
                     }
                     else
                     {
                        geoPoly[currentPoly].addPoint(ogrPt.getY(),
                                                      ogrPt.getX(),
                                                      ogrPt.getZ(),
                                                      origin.datum());
                     }
                  }
               }
            }
         }
      }
   }
   
   if(geoPoly.size())
   {
      rspfGeoAnnotationMultiPolyObject* annotation =
         new rspfGeoAnnotationMultiPolyObject(geoPoly,
                                               theFillFlag,
                                               color.getR(),
                                               color.getG(),
                                               color.getB(),
                                               theThickness);
      if(theImageGeometry.valid())
      {
         annotation->transform(theImageGeometry.get());
      }
      theFeatureCacheTable.insert(make_pair(id, annotation));
   }
}
bool rspfGdalOgrVectorAnnotation::open()
{
   return open(theFilename);
}
bool rspfGdalOgrVectorAnnotation::isOpen()const
{
   return (theDataSource!=NULL);
}
const rspfObject* rspfGdalOgrVectorAnnotation::getView()const
{
   return theImageGeometry.get();
}
void rspfGdalOgrVectorAnnotation::setBrushColor(const rspfRgbVector& brushColor)
{
   theBrushColor = brushColor;
}
   
void rspfGdalOgrVectorAnnotation::setPenColor(const rspfRgbVector& penColor)
{
   thePenColor = penColor;
}
rspfRgbVector rspfGdalOgrVectorAnnotation::getPenColor()const
{
   return thePenColor;
}
rspfRgbVector rspfGdalOgrVectorAnnotation::getBrushColor()const
{
   return theBrushColor;
}
double rspfGdalOgrVectorAnnotation::getPointRadius()const
{
   return thePointWidthHeight.x/2.0;
}
void rspfGdalOgrVectorAnnotation::setPointRadius(double r)
{
   thePointWidthHeight = rspfDpt(fabs(r)*2, fabs(r)*2);
}
bool rspfGdalOgrVectorAnnotation::getFillFlag()const
{
   return theFillFlag;
}
void rspfGdalOgrVectorAnnotation::setFillFlag(bool flag)
{
   theFillFlag=flag;
}
void rspfGdalOgrVectorAnnotation::setThickness(rspf_int32 thickness)
{
   if ( (thickness > 0) && (thickness < 256) )
   {
      theThickness = static_cast<rspf_uint8>(thickness);
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "rspfGdalOgrVectorAnnotation::setThickness range error: "
         << thickness
         << "\nValid range: 1 to 255"
         << std::endl;
   }
}
rspf_int32 rspfGdalOgrVectorAnnotation::getThickness()const
{
   return static_cast<rspf_int32>(theThickness);
}
void rspfGdalOgrVectorAnnotation::loadExternalGeometryFile()
{
   rspfFilename filename = theFilename;
   filename.setExtension(".geom");
   if(!filename.exists())
   {
      filename.setExtension(".GEOM");
      if(!filename.exists())
      {
         return;
      }
   }
   rspfKeywordlist kwl;
   if(kwl.addFile(filename))
   {
      rspfRefPtr<rspfImageGeometry> geom = new rspfImageGeometry;
      geom->loadState(kwl);
      if(geom->getProjection())
      {
         theImageGeometry = geom;
         rspfMapProjection* mapProj = PTR_CAST(rspfMapProjection, theImageGeometry->getProjection());
         if (mapProj)
         {
            theIsExternalGeomFlag = true;
            if (traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)
                  << "rspfGdalOgrVectorAnnotation::loadExternalGeometryFile"
                  << " DEBUG:"
                  << "\nExternal projection loaded from geometry file."
                  << "\nProjection dump:" << std::endl;
               mapProj->print(rspfNotify(rspfNotifyLevel_DEBUG));
            }
         }
      }
   }
}
void rspfGdalOgrVectorAnnotation::loadExternalImageGeometryFromXml()
{
   rspfFilename filename = theFilename;
   rspfString fileBase = filename.noExtension();
   rspfFilename xmlFile = rspfString(fileBase + ".xml");
   if (!xmlFile.exists())//try the xml file which includes the entire source file name
   {
      xmlFile = theFilename + ".xml";
   }
   if (!xmlFile.exists())
   {
      return;
   }
   rspfFgdcXmlDoc* fgdcXmlDoc = new rspfFgdcXmlDoc;
   if ( fgdcXmlDoc->open(xmlFile) )
   {
      rspfRefPtr<rspfProjection> proj = fgdcXmlDoc->getProjection();
      if ( proj.valid() )
      {
         theImageGeometry = new rspfImageGeometry;
         theImageGeometry->setProjection( proj.get() );
         theIsExternalGeomFlag = true;
      }
   }
   delete fgdcXmlDoc;
   fgdcXmlDoc = 0;
}
void rspfGdalOgrVectorAnnotation::loadOmdFile()
{
   rspfFilename filename = theFilename;
   filename.setExtension(".omd");
   if(!filename.exists())
   {
      filename.setExtension(".OMD");
   }
   if( filename.exists() )
   {
      rspfKeywordlist kwl;
      if( kwl.addFile(filename) )
      {
         const char* lookup = 0;
         lookup = kwl.find(rspfKeywordNames::BORDER_SIZE_KW);
         if (lookup)
         {
            theBorderSize = rspfString(lookup).toDouble();
            rspfString unitPrefix = rspfKeywordNames::BORDER_SIZE_KW +
               rspfString(".");
      
            theBorderSizeUnits = (rspfUnitType)rspfUnitTypeLut::instance()->
               getEntryNumber(kwl, unitPrefix.c_str());
            if(theBorderSizeUnits != RSPF_UNIT_UNKNOWN)
            {
               rspfUnitConversionTool unitConvert(theBorderSize,
                                                   theBorderSizeUnits);
         
               theBorderSize      = unitConvert.getValue(RSPF_DEGREES);
               theBorderSizeUnits = RSPF_DEGREES;
            }
            else // assume degrees
            {
               theBorderSizeUnits = RSPF_DEGREES;
            }
         }
         lookup = kwl.find(rspfKeywordNames::BRUSH_COLOR_KW);
         if (lookup)
         {
            int r, g, b;
            std::istringstream s(lookup);
            s>>r>>g>>b;
            theBrushColor = rspfRgbVector((rspf_uint8)r,
                                           (rspf_uint8)g,
                                           (rspf_uint8)b);
         }
         else
         {
            lookup = kwl.find(NORMALIZED_RGB_BRUSH_COLOR_KW);
            if (lookup)
            {
               rspf_float64 r;
               rspf_float64 g;
               rspf_float64 b;
               
               std::istringstream in(lookup);
               in >> r >> g >> b;
               
               if ( (r >= 0.0) && (r <=1.0) )
               {
                  theBrushColor.setR(static_cast<rspf_uint8>(r*255.0+0.5));
               }
               if ( (g >= 0.0) && (g <=1.0) )
               {
                  theBrushColor.setG(static_cast<rspf_uint8>(g*255.0+0.5));
               }
               if ( (b >= 0.0) && (b <=1.0) )
               {
                  theBrushColor.setB(static_cast<rspf_uint8>(b*255.0+0.5));
               }
            }
         }
         lookup = kwl.find(rspfKeywordNames::FILL_FLAG_KW);
         if (lookup)
         {
            theFillFlag = rspfString(lookup).toBool();
         }
         
         lookup = kwl.find(rspfKeywordNames::PEN_COLOR_KW);
         if (lookup)
         {
            int r, g, b;
            std::istringstream s(lookup);
            s>>r>>g>>b;
            thePenColor = rspfRgbVector((rspf_uint8)r,
                                         (rspf_uint8)g,
                                         (rspf_uint8)b);
         }
         else
         {
            lookup = kwl.find(NORMALIZED_RGB_PEN_COLOR_KW);
            if (lookup)
            {
               rspf_float64 r;
               rspf_float64 g;
               rspf_float64 b;
               
               std::istringstream in(lookup);
               in >> r >> g >> b;
               
               if ( (r >= 0.0) && (r <=1.0) )
               {
                  thePenColor.setR(static_cast<rspf_uint8>(r * 255.0 + 0.5));
               }
               if ( (g >= 0.0) && (g <=1.0) )
               {
                  thePenColor.setG(static_cast<rspf_uint8>(g * 255.0 + 0.5));
               }
               if ( (b >= 0.0) && (b <=1.0) )
               {
                  thePenColor.setB(static_cast<rspf_uint8>(b * 255.0 + 0.5));
               }
            }
         }
         lookup = kwl.find(rspfKeywordNames::POINT_WIDTH_HEIGHT_KW);
         if (!lookup)
         {
            lookup = kwl.find(POINT_SIZE_KW);
         }
         if (lookup)
         {
            rspf_float64 x;
            rspf_float64 y;
            
            std::istringstream in(lookup);
            in >> x >> y;
            
            if ( (x > 0.0) && (y > 0.0) )
            {
               thePointWidthHeight.x = x;
               thePointWidthHeight.y = y;
            }
         }
         lookup = kwl.find(rspfKeywordNames::THICKNESS_KW);
         if (lookup)
         {
            setThickness(rspfString(lookup).toInt32());
         }
         updateAnnotationSettings();
         
      } // matches: if( kwl.addFile(filename) )
      
   } // matches: if(filename.exists())
}
void rspfGdalOgrVectorAnnotation::getDefaults()
{
   const char* lookup;
   bool autocolors = false;
   lookup = rspfPreferences::instance()->
      findPreference(SHAPEFILE_COLORS_AUTO_KW);
   if (lookup)
   {
      autocolors = rspfString::toBool(rspfString(lookup));
   }
   
   if (autocolors)
   {
      if (currentAutoColorArrayIndex >= AUTO_COLOR_ARRAY_COUNT)
      {
         currentAutoColorArrayIndex = 0;
      }
      thePenColor.setR(autoColorArray[currentAutoColorArrayIndex].r);
      thePenColor.setG(autoColorArray[currentAutoColorArrayIndex].g);
      thePenColor.setB(autoColorArray[currentAutoColorArrayIndex].b);
      theBrushColor.setR(autoColorArray[currentAutoColorArrayIndex].r);
      theBrushColor.setG(autoColorArray[currentAutoColorArrayIndex].g);
      theBrushColor.setB(autoColorArray[currentAutoColorArrayIndex].b);
      
      ++currentAutoColorArrayIndex;
      if (currentAutoColorArrayIndex >= AUTO_COLOR_ARRAY_COUNT)
      {
         currentAutoColorArrayIndex = 0;
      }
      
   } // End of: if (autocolors)
   else
   {
      lookup = rspfPreferences::instance()->
         findPreference(NORMALIZED_RGB_PEN_COLOR_KW);
      if (lookup)
      {
         rspf_float64 r;
         rspf_float64 g;
         rspf_float64 b;
         
         std::istringstream in(lookup);
         in >> r >> g >> b;
         
         if ( (r >= 0.0) && (r <=1.0) )
         {
            thePenColor.setR(static_cast<rspf_uint8>(r * 255.0 + 0.5));
         }
         if ( (g >= 0.0) && (g <=1.0) )
         {
            thePenColor.setG(static_cast<rspf_uint8>(g * 255.0 + 0.5));
         }
         if ( (b >= 0.0) && (b <=1.0) )
         {
            thePenColor.setB(static_cast<rspf_uint8>(b * 255.0 + 0.5));
         }
      }
      
      lookup = rspfPreferences::instance()->
         findPreference(NORMALIZED_RGB_BRUSH_COLOR_KW);
      if (lookup)
      {
         rspf_float64 r;
         rspf_float64 g;
         rspf_float64 b;
         
         std::istringstream in(lookup);
         in >> r >> g >> b;
         
         if ( (r >= 0.0) && (r <=1.0) )
         {
            theBrushColor.setR(static_cast<rspf_uint8>(r * 255.0 + 0.5));
         }
         if ( (g >= 0.0) && (g <=1.0) )
         {
            theBrushColor.setG(static_cast<rspf_uint8>(g * 255.0 + 0.5));
         }
         if ( (b >= 0.0) && (b <=1.0) )
         {
            theBrushColor.setB(static_cast<rspf_uint8>(b * 255.0 + 0.5));
         }
      }
      
   } // End of: if (autocolors){...}else{
   lookup = rspfPreferences::instance()->
      findPreference(POINT_SIZE_KW);
   if (lookup)
   {
      rspf_float64 x;
      rspf_float64 y;
      
      std::istringstream in(lookup);
      in >> x >> y;
      if ( (x > 0.0) && (y > 0.0) )
      {
         thePointWidthHeight.x = x;
         thePointWidthHeight.y = y;
      }
   }   
}
void rspfGdalOgrVectorAnnotation::verifyViewParams()
{
   if (!theImageGeometry.valid())
   {
      return;
   }
   
   rspfMapProjection* proj = PTR_CAST(rspfMapProjection,
                                       theImageGeometry->getProjection());
   if (!proj)
   {
      return;
   }
   rspfGpt ulGpt = proj->getUlGpt();
   if ( ulGpt.isLatNan() || ulGpt.isLonNan() )
   {
      proj->setUlGpt( rspfGpt(theBoundingExtent.MaxY,
                               theBoundingExtent.MinX) );
   }
   
   if (proj->isGeographic())
   {
      rspfDpt pt = proj->getDecimalDegreesPerPixel();
      if( pt.hasNans() )
      {
         rspf_float64 d = 1.0 / 11131.49079;
         proj->setDecimalDegreesPerPixel(rspfDpt(d, d));
      }
   }
   else
   {
      rspfDpt pt = proj->getMetersPerPixel();
      if (pt.hasNans())
      {
         proj->setMetersPerPixel(rspfDpt(10.0, 10.0));
      }
   }
}
std::multimap<long, rspfAnnotationObject*> rspfGdalOgrVectorAnnotation::getFeatureTable()
{
   if (theFeatureCacheTable.size() == 0)
   {
      initializeTables();
   }
   return theFeatureCacheTable;
}
bool rspfGdalOgrVectorAnnotation::setCurrentEntry(rspf_uint32 entryIdx)
{
   if (entryIdx < m_layerNames.size())
   {
      m_layerName = m_layerNames[entryIdx];
      return open(theFilename);
   }
   return false;
}
