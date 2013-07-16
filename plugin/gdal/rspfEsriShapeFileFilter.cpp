#include <cstdio>
#include <cstdlib>
#include <sstream>
using namespace std;
#include <rspfEsriShapeFileFilter.h>
#include <rspfShapeFile.h>
#include <rspf/imaging/rspfAnnotationPolyObject.h>
#include <rspf/imaging/rspfGeoAnnotationPolyLineObject.h>
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/imaging/rspfGeoAnnotationPolyObject.h>
#include <rspf/imaging/rspfGeoAnnotationEllipseObject.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfMapProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
RTTI_DEF2(rspfEsriShapeFileFilter,
          "rspfEsriShapeFileFilter",
          rspfAnnotationSource,
          rspfViewInterface);
static const rspfTrace traceDebug("rspfEsriShapeFileFilter:debug");
rspfEsriShapeFileFilter::rspfEsriShapeFileFilter(rspfImageSource* inputSource)
   :rspfAnnotationSource(inputSource),
    rspfViewInterface(),
    theCoordinateSystem(RSPF_GEOGRAPHIC_SPACE),
    theUnitType(RSPF_METERS),
    theTree((SHPTree*)0),
    theMaxQuadTreeLevels(10),
    thePenColor(255,255,255),
    theBrushColor(255,255,255),
    theFillFlag(false),
    theThickness(1),
    thePointWidthHeight(1, 1),
    theBorderSize(0.0),
    theBorderSizeUnits(RSPF_DEGREES)
{
   rspfViewInterface::theObject = this;
   rspfAnnotationSource::setNumberOfBands(3);
   theBoundingRect.makeNan();
   theMinArray[0] = theMinArray[1] = theMinArray[2] = theMinArray[3] = rspf::nan();
   theMaxArray[0] = theMaxArray[1] = theMaxArray[2] = theMaxArray[3] = rspf::nan();
}
rspfEsriShapeFileFilter::~rspfEsriShapeFileFilter()
{
   removeViewProjection();
   
   if(theTree)
   {
      SHPDestroyTree(theTree);
   }
   deleteCache();
}
bool rspfEsriShapeFileFilter::setView(rspfObject* baseObject)
{
   rspfProjection* proj = PTR_CAST(rspfProjection, baseObject);
   if(proj)
   {
      if(theImageGeometry.valid())
      {
         theImageGeometry->setProjection(proj);
      }
      else
      {
         theImageGeometry = new rspfImageGeometry(0, proj);
      }
      return true;
   }
   else
   {
      rspfImageGeometry* geom = dynamic_cast<rspfImageGeometry*> (baseObject);
      if(geom)
      {
         theImageGeometry = geom;
         return true;
      }
   }
   return false;
}
rspfObject* rspfEsriShapeFileFilter::getView()
{
   return theImageGeometry.get();
}
const rspfObject* rspfEsriShapeFileFilter::getView()const
{
   return theImageGeometry.get();
}
bool rspfEsriShapeFileFilter::addObject(rspfAnnotationObject* /*anObject*/)
{
   rspfNotify(rspfNotifyLevel_WARN)
      << "rspfEsriShapeFileFilter::addObject\n"
      << "Can't add objects to layer, must go through Esri loadShapeFile"
      <<endl;
   
   return false;
}
void rspfEsriShapeFileFilter::computeBoundingRect()
{
   
   std::multimap<int, rspfAnnotationObject*>::iterator iter = theShapeCache.begin();
   
   theBoundingRect.makeNan();
   while(iter != theShapeCache.end())
   {
       rspfDrect rect = (*iter).second->getBoundingRect();
      if(theBoundingRect.hasNans())
      {
         theBoundingRect = rect;
      }
      else
      {
         if(!rect.hasNans())
         {
            theBoundingRect = theBoundingRect.combine(rect);
         }
      }
      
      ++iter;
   }
}
rspfIrect rspfEsriShapeFileFilter::getBoundingRect(rspf_uint32 resLevel)const
{
   if(!isSourceEnabled()||
      getInput())
   {
      if(getInput())
      {
         rspfImageSource* input = PTR_CAST(rspfImageSource, getInput());
         if(input)
         {
            return input->getBoundingRect(resLevel);
         }
      }
   }
   return theBoundingRect;
}
void rspfEsriShapeFileFilter::drawAnnotations(rspfRefPtr<rspfImageData> tile)
{
   rspfAnnotationSource::drawAnnotations(tile);
   
   if (!theTree||!theShapeFile.isOpen()) return;
   if(theImageGeometry.valid())
   {
      rspfIrect rect = tile->getImageRectangle();
      rect = rspfIrect(rect.ul().x,
                        rect.ul().y,
                        rect.lr().x,
                        rect.lr().y);
      double boundsMin[2];
      double boundsMax[2];
      
      rspfGpt gp1;
      rspfGpt gp2;
      rspfGpt gp3;
      rspfGpt gp4;
      theImageGeometry->localToWorld(rect.ul(),
                                           gp1);
      theImageGeometry->localToWorld(rect.ur(),
                                           gp2);
      theImageGeometry->localToWorld(rect.lr(),
                                           gp3);
      theImageGeometry->localToWorld(rect.ll(),
                                           gp4);
      rspfDrect boundsRect( rspfDpt(gp1.lond(),
                                      gp1.latd()),
                             rspfDpt(gp2.lond(),
                                      gp2.latd()),
                             rspfDpt(gp3.lond(),
                                      gp3.latd()),
                             rspfDpt(gp4.lond(),
                                      gp4.latd()),
                             RSPF_RIGHT_HANDED);
      boundsMin[0] = boundsRect.ul().x;
      boundsMin[1] = boundsRect.lr().y;
      boundsMax[0] = boundsRect.lr().x;
      boundsMax[1] = boundsRect.ul().y;
      int n;
      int *array=(int*)0;
      
      array = SHPTreeFindLikelyShapes(theTree,
                                      boundsMin,
                                      boundsMax,
                                      &n);
      
      theImage->setCurrentImageData(tile);
      if(n&&array)
      {
         for(int i = 0; i < n; ++i)
         {
            std::multimap<int, rspfAnnotationObject*>::iterator iter = theShapeCache.find(array[i]);
            while( ((*iter).first == array[i]) && (iter != theShapeCache.end()) )
            {
               (*iter).second->draw(*theImage);
               ++iter;
            }
         }
         
         free(array);
      }
   }
}
void rspfEsriShapeFileFilter::transformObjects(rspfImageGeometry* geom)
{
   std::multimap<int, rspfAnnotationObject*>::iterator iter = theShapeCache.begin();
   rspfImageGeometry* tempGeom = theImageGeometry.get();
   if(geom)
   {
      tempGeom = geom;
   }
   if(!tempGeom) return;
   
   while(iter != theShapeCache.end())
   {
      rspfGeoAnnotationObject* obj = PTR_CAST(rspfGeoAnnotationObject,
                                               (*iter).second);
      if(obj)
      {
         obj->transform(tempGeom);
      }
      ++iter;
   }
   computeBoundingRect();
}
void rspfEsriShapeFileFilter::setImageGeometry(rspfImageGeometry* geom)
{
   theImageGeometry = geom;
   transformObjects();
}
rspfRefPtr<rspfImageGeometry> rspfEsriShapeFileFilter::getImageGeometry()
{
   if( !theImageGeometry )
   {
      theImageGeometry = new rspfImageGeometry(0, new rspfEquDistCylProjection());
   }
   return theImageGeometry;
}
void rspfEsriShapeFileFilter::removeViewProjection()
{
   theImageGeometry = 0;
}
void rspfEsriShapeFileFilter::deleteCache()
{
   std::multimap<int, rspfAnnotationObject*>::iterator iter = theShapeCache.begin();
   while(iter != theShapeCache.end())
   {
      if ((*iter).second)
      {
        (*iter).second->unref();
      }
      ++iter;
   }
   theShapeCache.clear();
}
void rspfEsriShapeFileFilter::checkAndSetDefaultView()
{
   if(!theImageGeometry.valid())
   {
      getImageGeometry();
      if(theImageGeometry.valid())
      {
         transformObjects();
      }
   }
}
bool rspfEsriShapeFileFilter::loadShapeFile(const rspfFilename& shapeFile)
{
   if(theTree)
   {
      SHPDestroyTree(theTree);
      theTree = (SHPTree*)0;
   }
   theShapeFile.open(shapeFile);
   deleteCache();
   deleteAll();
   
   if(theShapeFile.isOpen())
   {
      theShapeFile.getBounds(theMinArray[0],theMinArray[1],theMinArray[2],theMinArray[3],
                             theMaxArray[0],theMaxArray[1],theMaxArray[2],theMaxArray[3]);
      theTree = SHPCreateTree(theShapeFile.getHandle(),
                              2,
                              theMaxQuadTreeLevels,
                              theMinArray,
                              theMaxArray);  
      
      rspfShapeObject obj;
      for(int index = 0 ; index < theShapeFile.getNumberOfShapes(); ++index)
      {
         obj.loadShape(theShapeFile,
                       index);
         
         if(obj.isLoaded())
         {
            switch(obj.getType())
            {
               case SHPT_POLYGON:
               case SHPT_POLYGONZ:
               {
                  loadPolygon(obj);
                  break;
               }
               case SHPT_POINT:
               case SHPT_POINTZ:
               {
                  loadPoint(obj);
                  break;
               }
               case SHPT_ARC:
               case SHPT_ARCZ:
               {
                  loadArc(obj);
                  break;
               }
               case SHPT_NULL:
               {
                  break;
               }
               default:
               {
                  rspfNotify(rspfNotifyLevel_WARN)
                     << "rspfEsriShapeFileFilter::loadShapeFile\n"
                     << "SHAPE " << obj.getTypeByName()
                     << " Not supported" <<  endl;
                  break;
               }
            }
         }
      }
      
      theCurrentObject = theShapeCache .begin();
      if(theImageGeometry.valid())
      {
         transformObjects();
      }
      else
      {
         checkAndSetDefaultView();
      }
   }
   
   return true;
}
void rspfEsriShapeFileFilter::loadPolygon(rspfShapeObject& obj)
{
   int starti = 0;
   int endi   = 0;
   if(obj.getNumberOfParts() > 1)
   {
      starti = obj.getShapeObject()->panPartStart[0];
      endi   = obj.getShapeObject()->panPartStart[1];
   }
   else
   {
      starti = 0;
      endi   = obj.getShapeObject()->nVertices;
   }
   
   vector<rspfGpt> groundPolygon;
   for(rspf_uint32 part = 0; part < obj.getNumberOfParts(); ++part)
   {
      if(obj.getPartType(part) != SHPP_RING)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfEsriShapeFileFilter::loadPolygon\n"
            << "Part = " << obj.getPartByName(part)
            << " not supported for shape = "
            << obj.getTypeByName() << endl;
         break;
      }
      groundPolygon.clear();
      for(rspf_int32 vertexNumber = starti; vertexNumber < endi; ++vertexNumber)
      {
         groundPolygon.push_back(rspfGpt(obj.getShapeObject()->padfY[vertexNumber],
                                          obj.getShapeObject()->padfX[vertexNumber]));
         
      }
      starti = endi;   
      if((part + 2) < obj.getNumberOfParts())
      {  
         endi = obj.getShapeObject()->panPartStart[part+2];
      }
      else
      {
         endi = obj.getShapeObject()->nVertices;
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
      if(theBorderSize != 0.0)
      {
         rspfGeoPolygon tempPoly(groundPolygon);
         rspfGeoPolygon tempPoly2;
         
         tempPoly.stretchOut(tempPoly2,
                             theBorderSize);
         groundPolygon = tempPoly2.getVertexList();
      }
      
      rspfGeoAnnotationObject *newGeoObj = new rspfGeoAnnotationPolyObject(groundPolygon,
                                                                             theFillFlag,
                                                                             color.getR(),
                                                                             color.getG(),
                                                                             color.getB(),
                                                                             theThickness);
      newGeoObj->setName(theFeatureName);
      theShapeCache.insert(make_pair(obj.getId(),
                                     newGeoObj));
   }
}
void rspfEsriShapeFileFilter::loadPoint(rspfShapeObject& obj)
{
   int n   = obj.getNumberOfVertices();
   if(n)
   {
      rspfGpt gpt(obj.getShapeObject()->padfY[0],
                   obj.getShapeObject()->padfX[0]);
      
      rspfRgbVector color;
      
      if(theFillFlag)
      {
         color = theBrushColor;
      }
      else
      {
         color = thePenColor;
      }
      rspfGeoAnnotationEllipseObject *newGeoObj =
         new rspfGeoAnnotationEllipseObject(gpt,
                                             thePointWidthHeight,
                                             theFillFlag,
                                             color.getR(),
                                             color.getG(),
                                             color.getB(),
                                             theThickness);
      newGeoObj->setEllipseWidthHeightUnitType(RSPF_PIXEL);
      newGeoObj->setName(theFeatureName);
      theShapeCache.insert(make_pair(obj.getId(),
                                     newGeoObj));
   }
}
void rspfEsriShapeFileFilter::loadArc(rspfShapeObject& obj)
{
   int starti = 0;
   int endi   = 0;
   if(obj.getNumberOfParts() > 1)
   {
      starti = obj.getShapeObject()->panPartStart[0];
      endi   = obj.getShapeObject()->panPartStart[1];
   }
   else
   {
      starti = 0;
      endi   = obj.getShapeObject()->nVertices;
   }
   
   vector<rspfGpt> groundPolygon;
   for(rspf_uint32 part = 0; part < obj.getNumberOfParts(); ++part)
   {
      if(obj.getPartType(part) != SHPP_RING)
      {
         rspfNotify(rspfNotifyLevel_WARN)
            << "rspfEsriShapeFileFilter::loadArc\n"
            << "Part = " << obj.getPartByName(part)
            << " not supported for shape = "
            << obj.getTypeByName() << endl;
         break;
      }
      groundPolygon.clear();
      for(rspf_int32 vertexNumber = starti; vertexNumber < endi; ++vertexNumber)
      {
         groundPolygon.push_back(rspfGpt(obj.getShapeObject()->padfY[vertexNumber],
                                          obj.getShapeObject()->padfX[vertexNumber]));
         
      }
      starti = endi;   
      if((part + 2) < obj.getNumberOfParts())
      {  
         endi = obj.getShapeObject()->panPartStart[part+2];
      }
      else
      {
         endi = obj.getShapeObject()->nVertices;
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
      
      rspfGeoAnnotationObject *newGeoObj = new rspfGeoAnnotationPolyLineObject(groundPolygon,
                                                                                 color.getR(),
                                                                                 color.getG(),
                                                                                 color.getB(),
                                                                                 theThickness);
      newGeoObj->setName(theFeatureName);
      theShapeCache.insert(make_pair(obj.getId(),
                                     newGeoObj));
   }
}
bool rspfEsriShapeFileFilter::saveState(rspfKeywordlist& kwl,
                                         const char* prefix)const
{
   rspfString s;
   
   kwl.add(prefix,
           rspfKeywordNames::FILENAME_KW,
           theShapeFile.getFilename(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::MAX_QUADTREE_LEVELS_KW,
           theMaxQuadTreeLevels,
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
           rspfKeywordNames::FEATURE_NAME_KW,
           theFeatureName.c_str(),
           true);
   kwl.add(prefix,
           rspfKeywordNames::THICKNESS_KW,
           theThickness,
           true);
   rspfString border;
   border = rspfString::toString(theBorderSize);
   border += " degrees";
   kwl.add(prefix,
           rspfKeywordNames::BORDER_SIZE_KW,
           border,
           true);
   
   s = rspfString::toString((int)thePointWidthHeight.x) + " " +
       rspfString::toString((int)thePointWidthHeight.y) + " ";
   
   kwl.add(prefix,
           rspfKeywordNames::POINT_WIDTH_HEIGHT_KW,
           s.c_str(),
           true);
   
   if(theImageGeometry.valid())
   {
      rspfString newPrefix = prefix;
      newPrefix += "view_proj.";
      theImageGeometry->saveState(kwl, newPrefix.c_str());
   }
   
   return rspfAnnotationSource::saveState(kwl, prefix);
}
bool rspfEsriShapeFileFilter::loadState(const rspfKeywordlist& kwl,
                                         const char* prefix)
{
   
   const char* quadLevels  = kwl.find(prefix, rspfKeywordNames::MAX_QUADTREE_LEVELS_KW);
   const char* filename    = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   const char* penColor    = kwl.find(prefix, rspfKeywordNames::PEN_COLOR_KW);
   const char* brushColor  = kwl.find(prefix, rspfKeywordNames::BRUSH_COLOR_KW);
   const char* featureName = kwl.find(prefix, rspfKeywordNames::FEATURE_NAME_KW);
   const char* fillFlag    = kwl.find(prefix, rspfKeywordNames::FILL_FLAG_KW);
   const char* thickness   = kwl.find(prefix, rspfKeywordNames::THICKNESS_KW);
   const char* pointWh     = kwl.find(prefix, rspfKeywordNames::POINT_WIDTH_HEIGHT_KW);
   const char* border_size = kwl.find(prefix, rspfKeywordNames::BORDER_SIZE_KW);
   
   deleteCache();
   if(thickness)
   {
      theThickness = rspfString(thickness).toLong();
   }
   if(quadLevels)
   {
      theMaxQuadTreeLevels = rspfString(quadLevels).toLong();
   }
   
   if(penColor)
   {
      int r, g, b;
      istringstream s(penColor);
      s>>r>>g>>b;
      thePenColor = rspfRgbVector((rspf_uint8)r, (rspf_uint8)g, (rspf_uint8)b);
   }
   if(brushColor)
   {
      int r, g, b;
      istringstream s(brushColor);
      s>>r>>g>>b;
      theBrushColor = rspfRgbVector((rspf_uint8)r, (rspf_uint8)g, (rspf_uint8)b);
   }
   if(pointWh)
   {
      double w, h;
      istringstream s(pointWh);
      s>>w>>h;
      thePointWidthHeight = rspfDpt(w, h);
   }
   
   if(fillFlag)
   {
      theFillFlag = rspfString(fillFlag).toBool();
   }
   if(border_size)
   {
      istringstream input(border_size);
      rspfString s;
      input >> s;
      theBorderSize = s.toDouble();
      
      rspfString s2;
      
      input >> s2;
      s2 = s2.upcase();
      
      if(s2 == "US")
      {
         theBorderSizeUnits = RSPF_US_SURVEY_FEET;
      }
      else if(s2 == "METERS")
      {
         theBorderSizeUnits = RSPF_METERS;
      }
      else if(s2 == "FEET")
      {
         theBorderSizeUnits = RSPF_FEET;
      }
      else
      {
         theBorderSizeUnits = RSPF_DEGREES;
      }
      rspfUnitConversionTool unitConvert(theBorderSize,
                                          theBorderSizeUnits);
      
      theBorderSize      = unitConvert.getValue(RSPF_DEGREES);
      theBorderSizeUnits = RSPF_DEGREES;
   }
   else
   {
      theBorderSize      = 0.0;
      theBorderSizeUnits = RSPF_DEGREES;
   }
   
   theFeatureName = featureName;
   
   rspfString newPrefix = prefix;
   newPrefix += "view_proj.";
   
   theImageGeometry = new rspfImageGeometry;
   if(!theImageGeometry->loadState(kwl, newPrefix.c_str()))
   {
      theImageGeometry = 0;
   }
   
   if(filename)
   {
      loadShapeFile(rspfFilename(filename));
   }
   
   checkAndSetDefaultView();
   
   return rspfAnnotationSource::loadState(kwl, prefix);
}
