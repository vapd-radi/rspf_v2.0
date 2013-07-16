#include <rspfOgrVectorTileSource.h>
#include <rspfOgcWktTranslator.h>
#include <rspfGdalType.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
RTTI_DEF1(rspfOgrVectorTileSource,
          "rspfOgrVectorTileSource",
          rspfImageHandler);
static rspfOgcWktTranslator wktTranslator;
static rspfTrace traceDebug("rspfOgrVectorTileSource:debug");
class rspfOgrVectorLayerNode
{
public:
   rspfOgrVectorLayerNode(const rspfDrect& bounds)
      : theBoundingRect(bounds)
   {
   }
   ~rspfOgrVectorLayerNode()
   {
      theGeoImage.release();
      theGeoImage = 0;
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
 
   void setGeoImage(rspfRefPtr<rspfImageGeometry> image)
   {
      theGeoImage = image;
   }
   rspfDrect theBoundingRect;  //world
   rspfRefPtr<rspfImageGeometry> theGeoImage;
};
rspfOgrVectorTileSource::rspfOgrVectorTileSource()
   :rspfImageHandler(),
    theDataSource(0),
    theImageBound(),
    theBoundingExtent()
{
}
rspfOgrVectorTileSource::~rspfOgrVectorTileSource()
{
   close();
}
bool rspfOgrVectorTileSource::open()
{
   const char* MODULE = "rspfOgrVectorTileSource::open";
  
   if (isOgrVectorDataSource() == false)
   {
      close();
      return false;
   }
   if(isOpen())
   {
      close();
   }
   theDataSource = OGRSFDriverRegistrar::Open(theImageFile,
                                              false);
   if (theDataSource)
   {
      int layerCount = theDataSource->GetLayerCount();
      theLayerVector.resize(layerCount);
      if(layerCount)
      {
         for(int i = 0; i < layerCount; ++i)
         {
            OGRLayer* layer = theDataSource->GetLayer(i);
            if(layer)
            {
               OGRSpatialReference* spatialReference = layer->GetSpatialRef();
               if(!spatialReference)
               {
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_NOTICE)
                        << MODULE
                        << " No spatial reference given, assuming geographic"
                        << endl;
                  }
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
            }
            if (layer)
            {
               layer->GetExtent(&theBoundingExtent, true);
               rspfRefPtr<rspfProjection> proj = createProjFromReference(layer->GetSpatialRef());
               rspfRefPtr<rspfImageGeometry> imageGeometry = 0;
               bool isDefaultProjection = false;
               if(proj.valid())
               {
                  imageGeometry = new rspfImageGeometry(0, proj.get());
               }
               rspfMapProjection* mapProj = 0;
               if(imageGeometry.valid())
               {
                  mapProj = PTR_CAST(rspfMapProjection, imageGeometry->getProjection());
               }
               else
               {
                  mapProj = createDefaultProj();
                  imageGeometry = new rspfImageGeometry(0, mapProj);
                  isDefaultProjection = true;
               }
               if(mapProj)
               {
                  rspfDrect rect(theBoundingExtent.MinX,
                                  theBoundingExtent.MaxY,
                                  theBoundingExtent.MaxX,
                                  theBoundingExtent.MinY,
                                  RSPF_RIGHT_HANDED);
            
                  std::vector<rspfGpt> points;
                  if (isDefaultProjection || mapProj->isGeographic())
                  {
                     rspfGpt g1(rect.ul().y, rect.ul().x);
                     rspfGpt g2(rect.ur().y, rect.ur().x);
                     rspfGpt g3(rect.lr().y, rect.lr().x);
                     rspfGpt g4(rect.ll().y, rect.ll().x);
                     points.push_back(g1);
                     points.push_back(g2);
                     points.push_back(g3);
                     points.push_back(g4);
                  }
                  else
                  {
                     rspfGpt g1 = mapProj->inverse(rect.ul());
                     rspfGpt g2 = mapProj->inverse(rect.ur());
                     rspfGpt g3 = mapProj->inverse(rect.lr());
                     rspfGpt g4 = mapProj->inverse(rect.ll());
                     points.push_back(g1);
                     points.push_back(g2);
                     points.push_back(g3);
                     points.push_back(g4);
                  }
            
                  std::vector<rspfDpt> rectTmp;
                  rectTmp.resize(4);
                  for(std::vector<rspfGpt>::size_type index=0; index < 4; ++index)
                  {
                     imageGeometry->worldToLocal(points[(int)index], rectTmp[(int)index]);
                  }
                  rspfDrect rect2 = rspfDrect(rectTmp[0],
                                                rectTmp[1],
                                                rectTmp[2],
                                                rectTmp[3]);
                  theLayerVector[i] = new rspfOgrVectorLayerNode(rect2);
                  theLayerVector[i]->setGeoImage(imageGeometry);
                  if (i == 0)
                     theImageGeometry = imageGeometry;
               }
            }
         }
      }
   }
   else
   {
      delete theDataSource;
      theDataSource = 0;
      return false;
   }
   return (theDataSource!=0);
}
bool rspfOgrVectorTileSource::saveState(rspfKeywordlist& kwl,
                                         const char* prefix) const
{
   for (unsigned int i = 0; i < theLayerVector.size(); i++)
   {
      rspfRefPtr<rspfImageGeometry> imageGeometry = theLayerVector[i]->theGeoImage;
      if(theImageGeometry.valid())
      {
         theImageGeometry->saveState(kwl, prefix);
      }
   }
   return rspfImageHandler::saveState(kwl, prefix);
}
bool rspfOgrVectorTileSource::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool success = rspfImageHandler::loadState(kwl, prefix);
   if (success)
   {
      theImageGeometry = new rspfImageGeometry();
      success = theImageGeometry->loadState(kwl, prefix);
   }
   return success;
}
rspf_uint32 rspfOgrVectorTileSource::getImageTileWidth() const
{
   return 0;
}
rspf_uint32 rspfOgrVectorTileSource::getImageTileHeight() const
{
   return 0;
}
rspfRefPtr<rspfImageGeometry> rspfOgrVectorTileSource::getInternalImageGeometry() const
{
   return theImageGeometry;
}
void rspfOgrVectorTileSource::close()
{
   for(rspf_uint32 i = 0; i < theLayerVector.size(); ++i)
   {
      if(theLayerVector[i])
      {
         delete theLayerVector[i];
      }
   }
   
   theLayerVector.clear();
   
   if(theDataSource)
   {
      delete theDataSource;
      theDataSource = 0;
   }
}
rspfRefPtr<rspfImageData> rspfOgrVectorTileSource::getTile(
   const rspfIrect& tileRect, rspf_uint32 resLevel)
{
   return 0;
}
rspf_uint32 rspfOgrVectorTileSource::getNumberOfInputBands() const
{
   return 0;
}
rspf_uint32 rspfOgrVectorTileSource::getNumberOfOutputBands() const
{
   return 0;
}
rspf_uint32 rspfOgrVectorTileSource::getNumberOfLines(rspf_uint32 /* reduced_res_level */ ) const
{
   if(theImageBound.hasNans())
   {
      return theImageBound.ul().x;
   }
   
   return theImageBound.height();
}
rspf_uint32 rspfOgrVectorTileSource::getNumberOfSamples(rspf_uint32 /* reduced_res_level */ ) const
{
   if(theImageBound.hasNans())
   {
      return theImageBound.ul().y;
   }
   
   return theImageBound.width();
}
rspf_uint32 rspfOgrVectorTileSource::getNumberOfDecimationLevels() const
{
   return 32;
}
rspfIrect rspfOgrVectorTileSource::getImageRectangle(rspf_uint32 /* reduced_res_level */ ) const
{
   return theImageBound;
}
rspfRefPtr<rspfImageGeometry> rspfOgrVectorTileSource::getImageGeometry()
{
   return theImageGeometry;
}
rspfScalarType rspfOgrVectorTileSource::getOutputScalarType() const
{
   return RSPF_SCALAR_UNKNOWN;
}
rspf_uint32 rspfOgrVectorTileSource::getTileWidth() const
{
   return 0;         
}
rspf_uint32 rspfOgrVectorTileSource::getTileHeight() const
{
   return 0;
}
bool rspfOgrVectorTileSource::isOpen()const
{
   return (theDataSource!=0);
}
bool rspfOgrVectorTileSource::isOgrVectorDataSource()const
{
   if (!theImageFile.empty())
   {
      if (theImageFile.before(":", 3).upcase() != "SDE" && 
          theImageFile.before(":", 4).upcase() != "GLTP" && 
          theImageFile.ext().downcase() != "mdb")
      {
         return false;
      }
   }
   else
   {
      return false;
   }
   
   return true;
}
double rspfOgrVectorTileSource::getNullPixelValue(rspf_uint32 band)const
{
   return 0.0;
}
double rspfOgrVectorTileSource::getMinPixelValue(rspf_uint32 band)const
{
   return 0.0;
}
double rspfOgrVectorTileSource::getMaxPixelValue(rspf_uint32 band)const
{
   return 0.0;
}
bool rspfOgrVectorTileSource::setCurrentEntry(rspf_uint32 entryIdx)
{
   if(theLayerVector.size() > 0)
   {
      if ((rspf_int32)theLayerVector.size() > entryIdx)
      {
         theImageGeometry = 0;
         theImageBound.makeNan();
         theImageGeometry = theLayerVector[entryIdx]->theGeoImage;
         theImageBound = theLayerVector[entryIdx]->theBoundingRect;
         if (theImageGeometry.valid())
         {
            return true;
         }
      }
   }
   return false;
}
rspf_uint32 rspfOgrVectorTileSource::getNumberOfEntries() const
{
   return theLayerVector.size();
}
void rspfOgrVectorTileSource::getEntryList(std::vector<rspf_uint32>& entryList) const
{
   if (theLayerVector.size() > 0)
   {
      for (rspf_uint32 i = 0; i < getNumberOfEntries(); i++)
      {
         entryList.push_back(i);
      }
   }
   else
   {
      rspfImageHandler::getEntryList(entryList);
   }
}
rspfProjection* rspfOgrVectorTileSource::createProjFromReference(OGRSpatialReference* reference)const
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfOgrVectorTileSource::createProjFromReference:   entered........" << std::endl;
   }
   if(!reference)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfOgrVectorTileSource::createProjFromReference:   leaving........" << std::endl;
      }
      return 0;
   }
   char* wktString = 0;
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
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfOgrVectorTileSource::createProjFromReference:   returning........" << std::endl;
   }
   return rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
}
rspfMapProjection* rspfOgrVectorTileSource::createDefaultProj()
{
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
   
   return proj;
}
