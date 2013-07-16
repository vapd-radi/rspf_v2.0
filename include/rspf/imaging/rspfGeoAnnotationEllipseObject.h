//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationEllipseObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationEllipseObject_HEADER
#define rspfGeoAnnotationEllipseObject_HEADER
#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/imaging/rspfAnnotationEllipseObject.h>

class RSPFDLLEXPORT rspfGeoAnnotationEllipseObject : public rspfGeoAnnotationObject
{
public:
   
   /*!
    * It is important to note that the width and height
    * should be specified in width = deltaLon and
    * height = deltaLat.  Use the rspfDpt .lat and .lon
    * attributes to set it.
    */
   rspfGeoAnnotationEllipseObject(const rspfGpt& center = rspfGpt(0,0),
                                   const rspfDpt& widthHeight = rspfDpt(0,0),
                                   bool enableFill = false,
                                   rspf_uint8 r = 255,
                                   rspf_uint8 g = 255,
                                   rspf_uint8 b = 255,
                                   rspf_uint8 thickness = 1);
   rspfGeoAnnotationEllipseObject(const rspfGeoAnnotationEllipseObject& rhs);

   virtual rspfObject* dup()const;

   virtual void applyScale(double x, double y);
   virtual std::ostream& print(std::ostream& out)const;
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();
   virtual void transform(rspfImageGeometry* projection);

   /**
    * Saves the current state of this object.
    *
    * Keywords:
    *
    * center:  ( 42.403388263727003, -90.708557648541046, 0.0, WGE )
    *
    * ellipse_width: 500
    *
    * ellipse_height: 500
    *
    * units: meters
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual void setEllipseWidthHeightUnitType(rspfUnitType type);

   /**
    * Sets the width and height of point.
    *
    * @param pt Point with pt.x being width and pt.y being height.
    */
   virtual void setWidthHeight(const rspfDpt& pt);

   /**
    * @brief Sets the azimuth.
    * @param azimuth in degrees.
    */
   virtual void setAzimuth(rspf_float64 azimuth);

   /**
    * @brief Gets the azimuth.
    * @return The azimuth in decimal degrees.
    */
   virtual rspf_float64 getAzimuth() const;

   /**
    * Sets the color of theProjectedEllipse. RGB range 0 to 255
    *
    * @param r DN value for red.
    * @param g DN value for green.
    * @param b DN value for blue.
    */
   virtual void setColor(rspf_uint8 r,
                         rspf_uint8 g,
                         rspf_uint8 b);
   /**
    * Sets the pen thickness of theProjectedEllipse. range 0 to 255
    *
    * @param thickness Pen thickness.
    */
   virtual void setThickness(rspf_uint8 thickness);

   /**
    * Sets the fill flag.
    *
    * @bool flag True to fill, false to not fill.
    */
   virtual void setFillFlag(bool flag);

   /** @return The fill flag. */
   virtual bool getFillFlag() const;

   /**
    * @brief Turns on and off drawing of axes on the ellipse.
    * This flag is only used if fill is turned off.
    *
    * @param flag true to enable, false to disable.
    */
   virtual void setDrawAxesFlag(bool flag);

   /** @return The draw axes flag. */
   virtual bool getDrawAxesFlag() const;

   /**
    * @brief Sets the center ground point.
    * @param gpt The center ground point.
    */
   virtual void setCenter(const rspfGpt& gpt);

   /** @param gpt The point to initialize with the center. */
   virtual void getCenter(rspfGpt& gpt) const;

protected:
   virtual ~rspfGeoAnnotationEllipseObject();

   void getWidthHeightInPixels(rspfDpt& widthHeight,
                               const rspfImageGeometry* projection) const;
   
   rspfRefPtr<rspfAnnotationEllipseObject> theProjectedEllipse;
   rspfGpt                      theCenter;
   rspfDpt                      theWidthHeight;

   rspfUnitType                 theEllipseWidthHeightUnitType;
   
TYPE_DATA   
};

#endif
