#ifndef rspfEsriShapeFileFilter_HEADER
#define rspfEsriShapeFileFilter_HEADER
#include <map>
#include <shapefil.h>
#include <rspfShapeFile.h>
#include <rspf/imaging/rspfAnnotationSource.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfViewInterface.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/imaging/rspfImageGeometry.h>
class rspfGeoAnnotationObject;
class rspfAnnotationObject;
class rspfProjection;
/*!
 * class rspfEsriShapeFileFilter
 *
 * This class is used to render shape files.  Iif this filter has
 * an input connection to an rspfImageSourceInterface then it will
 * draw any vectors over the input tile.  If it is not connected
 * it will just render its vector data to a tile and return it.
 * <pre>
 * supported keywords:
 *
 *   max_quadtree_levels:    // number of levels for faster hit testing
 *                           // defaults to 10
 *
 *   fill_flag:              // 1 for true 0 for false.  Default is false
 *                           // and specifies how all closed surfaces are
 *                           // to be drawn
 *
 *   feature_name:           // currently not used.
 *
 *   pen_color:              // specified in Red Green Blue and is the color used
 *                           // if the fill flag is not specified.  Future changes
 *                           // might be to include this as an outline color for
 *                           // filled objects.  Default is white. Example: 255 255 255
 *
 *   brush_color:            // Color used by objects that have the fill flag enabled
 *                           // format is Red Green Blue. Default is white.
 *
 *   line_thickness:         // line drawing thickness in pixels.
 *
 *   border_size:            // can be us feet, feet, meters, degrees,
 *                           // will automatically expand all polygons by the
 *                           // specified size.  Uses the centroid to estimate
 *                           // non degree units when converting to degrees.
 *
 *   point_width_height:     // Specifies the width and height of a point object in
 *                           // pixels.  Default is 1 and has the following format
 *                           // w h.  Example 1 1
 *
 *   filename:               // The esri shape file to be used
 *
 * example Keyword list:  See rspfAnnotationSource for any additional keywords
 *
 *
 * brush_color:  255 255 255
 * feature_name:
 * filename:
 * fill_flag:  0
 * max_quadtree_levels:  10
 * pen_color:  255 255 255
 * point_width_height:  1 1
 * thickness:  1
 * border_size: 25 meters
 * type:  rspfEsriShapeFileFilter
 *
 * </pre>
 */
class RSPF_PLUGINS_DLL rspfEsriShapeFileFilter :
   public rspfAnnotationSource,
   public rspfViewInterface
{
public:
   rspfEsriShapeFileFilter(rspfImageSource* inputSource=NULL);
   ~rspfEsriShapeFileFilter();
   virtual bool setView(rspfObject* baseObject);
   virtual rspfObject*       getView();
   virtual const rspfObject* getView()const;
   virtual bool addObject(rspfAnnotationObject* anObject);
   virtual void transformObjects(rspfImageGeometry* geom=0);
   virtual void setImageGeometry(rspfImageGeometry* projection);
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;
   virtual void computeBoundingRect();
   virtual void drawAnnotations(rspfRefPtr<rspfImageData> tile);
   /*!
    * Will delete the current objects within the layer and add all
    * objects in the passed in file.
    */
   virtual bool loadShapeFile(const rspfFilename& shapeFile);
   virtual rspfFilename getFilename()const
      {
         return theShapeFile.getFilename();
      }
   virtual rspf_int32 getMaxQuadTreeLevels()const
      {
         return theMaxQuadTreeLevels;
      }
   virtual void setMaxQuadTreeLevels(rspf_int32 levels)
      {
         theMaxQuadTreeLevels = (levels>0?levels:10);
      }
   virtual void setBrushColor(const rspfRgbVector& brushColor)
      {
         theBrushColor = brushColor;
      }
   virtual void setPenColor(const rspfRgbVector& penColor)
      {
         thePenColor = penColor;
      }
   virtual rspfRgbVector getPenColor()const
      {
         return thePenColor;
      }
   virtual rspfRgbVector getBrushColor()const
      {
         return theBrushColor;
      }
   virtual bool getFillFlag()const
      {
         return theFillFlag;
      }
   virtual void setFillFlag(bool flag)
      {
         theFillFlag=flag;
      }
   virtual rspfString getFeatureName()const
      {
         return theFeatureName;
      }
   virtual void setFeatureName(const rspfString& name)
      {
         theFeatureName = name;
      }
   virtual void setThickness(rspf_int32 thickness)
      {
         theThickness = thickness >= 0? thickness:1;
      }
   virtual rspf_int32 getThickness()const
      {
         return theThickness;
      }
   virtual void setPointRadius(double r)
      {
         thePointWidthHeight = rspfDpt(fabs(r)*2, fabs(r)*2);
      }
   virtual double getPointRadius()const
      {
         return thePointWidthHeight.x/2.0;
      }
   virtual rspfAnnotationObject* nextObject(bool restart=false)
      {
         if(restart)
         {
            theCurrentObject = theShapeCache.begin();
         }
         else
         {
            if(theCurrentObject != theShapeCache.end())
            {
               ++theCurrentObject;
            }
         }
         if(theCurrentObject == theShapeCache.end())
         {
            return (rspfAnnotationObject*)NULL;
         }
         return (*theCurrentObject).second;
      }
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
protected:
   /*!
    *  This holds the output projection.  This is used to transform
    * the objects to the projection plane.
    */
   rspfRefPtr<rspfImageGeometry> theImageGeometry;
   /*!
    * If the shape file is projected we must know the projector.
    * This will allow us to define the geographic objects correctly.
    */
   rspfRefPtr<rspfProjection> theShapeFileProjection;
   /*!
    * Specifies if its image/pixel space, Geographic, projected ...etc.
    */
   rspfCoordinateSystemType theCoordinateSystem;
   /*!
    * If its projected this spceifies the type of unit.  We
    * need to know if the projection is in meters, feet, us survey feet
    * ... etc.
    */
   rspfUnitType             theUnitType;
   SHPTree*                  theTree;
   rspfShapeFile            theShapeFile;
   double                    theMinArray[4];
   double                    theMaxArray[4];
   int                       theMaxQuadTreeLevels;
   rspfRgbVector            thePenColor;
   rspfRgbVector            theBrushColor;
   bool                      theFillFlag;
   rspf_int32               theThickness;
   rspfString               theFeatureName;
   rspfDpt                  thePointWidthHeight;
   double                    theBorderSize;
   rspfUnitType             theBorderSizeUnits;
   mutable std::multimap<int, rspfAnnotationObject*>::iterator theCurrentObject;
   std::multimap<int, rspfAnnotationObject*> theShapeCache;
   rspfDrect theBoundingRect;
   void removeViewProjection();
   void deleteCache();
   void checkAndSetDefaultView();
   virtual void loadPolygon(rspfShapeObject& obj);
   virtual void loadPoint(rspfShapeObject& obj);
   virtual void loadArc(rspfShapeObject& obj);
TYPE_DATA
};
#endif
