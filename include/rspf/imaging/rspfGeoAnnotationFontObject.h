//*************************************************************************
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationFontObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationFontObject_HEADER
#define rspfGeoAnnotationFontObject_HEADER
#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/imaging/rspfAnnotationFontObject.h>

class rspfFont;

class rspfGeoAnnotationFontObject : public rspfGeoAnnotationObject
{
public:
   rspfGeoAnnotationFontObject();
   
   
   rspfGeoAnnotationFontObject(const rspfGpt& location,
                                const rspfString& s,
                                const rspfIpt& pixelSize = rspfIpt(12, 12),
                                double rotation = 0.0,
                                const rspfDpt& scale = rspfDpt(1.0, 1.0),
                                const rspfDpt& shear = rspfDpt(0.0,0.0),
                                rspf_uint8 r=255,
                                rspf_uint8 g=255,
                                rspf_uint8 b=255);
   rspfGeoAnnotationFontObject(const rspfGeoAnnotationFontObject& rhs);
   
   virtual rspfObject* dup()const;

   rspfAnnotationFontObject* getFontObject();

   void setFont(rspfFont* font);

   rspfFont* getFont();

   virtual void setPointSize(const rspfIpt& size);

   virtual void setRotation(double rotation);

   virtual void setScale(const rspfDpt& scale);

   virtual void setShear(const rspfDpt& shear);
   
   virtual void setColor(rspf_uint8 r,
                         rspf_uint8 g,
                         rspf_uint8 b);

   virtual void transform(rspfImageGeometry* projection);

   virtual void setCenterGround(const rspfGpt& gpt);
   
   virtual void draw(rspfRgbImage& anImage)const;
   
   virtual void computeBoundingRect();
   
   virtual void getBoundingRect(rspfDrect& rect)const;
   
   virtual rspfAnnotationObject* getNewClippedObject(
      const rspfDrect& rect)const;
   
   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual void applyScale(double x, double y);
   
   /**
    * Saves the current state of this object.
    *
    * @see loadState for keywords.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;
   
   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    *
    * Keywords:
    *
    * For center format is: ( latitude, longitude, height, datum_code )
    * center: ( 42.01, -90.71, 0.0, WGE )
    *
    * For point_size format is: "( x, y )"
    * point_size:  ( 18, 18 )
    *
    * For scale format is: "( x, y )"
    * scale: ( 1.0, 1.0 )
    *
    * For shear format is: "( x, y )"
    * shear: ( 0.0, 0.0 )
    *
    * text: Airport
    *
    * USE ONE OF family_name:
    * family_name: URW Palladio L
    * family_name: gd sans
    * family_name: URW Bookman L
    * family_name: Nimbus Roman No9 L
    * family_name: Nimbus Sans L
    * family_name: URW Palladio L
    * family_name: Century Schoolbook L
    * family_name: Nimbus Mono L
    * family_name: Nimbus Sans L Condensed
    *
    * USE ONE OF style_name:
    * style_name:  normal
    * style_name:  bold
    * style_name:  italic
    * style_name:  bold italic
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
protected:
   virtual ~rspfGeoAnnotationFontObject();
   
   rspfGpt                   theCenterGround;
   rspfRefPtr<rspfFont>     theFont;
   rspfFontInformation       theFontInfo;
   rspfRefPtr<rspfAnnotationFontObject> theAnnotationFontObject;

TYPE_DATA
};

#endif
