#ifndef rspfGdalOgrVectorAnnotation_HEADER
#define rspfGdalOgrVectorAnnotation_HEADER
#include <map>
#include <vector>
#include <list>
#include <gdal.h>
#include <ogrsf_frmts.h>
#include "../rspfPluginConstants.h"
#include <rspf/base/rspfViewInterface.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/imaging/rspfAnnotationSource.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <rspf/projection/rspfProjection.h>
class rspfProjection;
class rspfMapProjection;
class rspfOgrGdalLayerNode;
class rspfOgrGdalFeatureNode;
class rspfAnnotationObject;
class RSPF_PLUGINS_DLL rspfGdalOgrVectorAnnotation :
   public rspfAnnotationSource,
   public rspfViewInterface
{
public:
   rspfGdalOgrVectorAnnotation(rspfImageSource* inputSource=0);
   virtual ~rspfGdalOgrVectorAnnotation();
   virtual bool open();
   virtual bool open(const rspfFilename& file);
   virtual bool isOpen()const;
   virtual void close();
   virtual rspfFilename getFilename()const;
   
   virtual bool setView(rspfObject* baseObject);
   
   virtual rspfObject*       getView();
   virtual const rspfObject* getView()const;
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry() const;
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;
   virtual void computeBoundingRect();
   virtual void drawAnnotations(rspfRefPtr<rspfImageData> tile);
   virtual void setBrushColor(const rspfRgbVector& brushColor);
   virtual void setPenColor(const rspfRgbVector& penColor);
   virtual rspfRgbVector getPenColor()const;
   virtual rspfRgbVector getBrushColor()const;
   virtual double getPointRadius()const;
   virtual void setPointRadius(double r);
   virtual bool getFillFlag()const;
   virtual void setFillFlag(bool flag);
   virtual void setThickness(rspf_int32 thickness);
   virtual rspf_int32 getThickness()const;
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   virtual std::ostream& print(std::ostream& out) const;
   std::multimap<long, rspfAnnotationObject*> getFeatureTable();
   void setQuery(const rspfString& query);
   void setGeometryBuffer(rspf_float64 distance, rspfUnitType type);
   void initializeBoundingRec(vector<rspfGpt> points);
   bool setCurrentEntry(rspf_uint32 entryIdx);
protected:
   OGRDataSource                      *theDataSource;
   OGRSFDriver                        *theDriver;
   rspfFilename                       theFilename;
   OGREnvelope                         theBoundingExtent;
   rspfRefPtr<rspfImageGeometry>     theImageGeometry;
   vector<bool>                        theLayersToRenderFlagList;
   std::vector<rspfOgrGdalLayerNode*> theLayerTable;
   rspfRgbVector                      thePenColor;
   rspfRgbVector                      theBrushColor;
   bool                                theFillFlag;
   rspf_uint8                         theThickness;
   rspfDpt                            thePointWidthHeight;
   double                              theBorderSize;
   rspfUnitType                       theBorderSizeUnits;
   rspfDrect                          theImageBound;
   bool                                theIsExternalGeomFlag;
   
   std::multimap<long, rspfAnnotationObject*> theFeatureCacheTable;
   rspfString                                 m_query;
   bool                                        m_needPenColor;
   rspf_float64                               m_geometryDistance;
   rspfUnitType                               m_geometryDistanceType;
   rspfString                                 m_layerName;
   std::vector<rspfString>                    m_layerNames;
   
   void computeDefaultView();
   /** Uses theViewProjection */
   void transformObjectsFromView();
   
   void loadPoint(long id, OGRPoint* point, rspfMapProjection* mapProj);
   void loadMultiPoint(long id, OGRMultiPoint* multiPoint, rspfMapProjection* mapProj);
   void loadMultiPolygon(long id, OGRMultiPolygon* multiPolygon, rspfMapProjection* mapProj);
   void loadPolygon(long id, OGRPolygon* polygon, rspfMapProjection* mapProj);
   void loadLineString(long id, OGRLineString* lineString, rspfMapProjection* mapProj);
   void loadMultiLineString(long id, OGRMultiLineString* multiLineString, rspfMapProjection* mapProj);
   
   void getFeatures(std::list<long>& result,
                    const rspfIrect& rect);
   void getFeature(vector<rspfAnnotationObject*>& featureList,
                   long id);
   rspfProjection* createProjFromReference(OGRSpatialReference* reference)const;
   void initializeTables();
   void deleteTables();
   void updateAnnotationSettings();
   /**
    * Will set theViewProjection if geometry file is present with projection.
    * Also sets theIsExternalGeomFlag.
    */
   void loadExternalGeometryFile();
   /**
    * Will set theViewProjection if xml file is present with projection.
    * Also sets theIsExternalGeomFlag.
    */
   void loadExternalImageGeometryFromXml();
   /**
    * Looks for "file.omd" and loads pen, brush and point settings if present.
    */
   void loadOmdFile();
   /**
    * Will set thePenColor and theBrushColor if keyword found in preferences.
    *
    * Keyword example:
    * shapefile_normalized_rgb_pen_color:   0.004 1.0 0.004
    * shapefile_normalized_rgb_brush_color: 0.004 1.0 0.004
    */
   void getDefaults();
   /**
    * @brief Checks for nan scale and tie point.  Sets to some default if
    * nan.
    */
   void verifyViewParams();
TYPE_DATA
   
}; // End of: class rspfGdalOgrVectorAnnotation
#endif
