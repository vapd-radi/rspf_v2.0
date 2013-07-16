//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationFontObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationFontObject_HEADER
#define rspfAnnotationFontObject_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfFontInformation.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/font/rspfFont.h>

/**
 * Class to draw fonts onto an image.
 * @note  This class requires a font path to be set for the factory:
 * "rspfFontFactoryRegistry".  You should set at a minimum the keyword
 * "font.dir1" in your rspf_prefernces file.
 * Example from a RedHat 9.0 system:
 * font.dir1: /usr/share/fonts/default/Type1
 *
 * @see file rspf/etc/templates/rspf_prefernces_template
 */
class RSPFDLLEXPORT rspfAnnotationFontObject : public rspfAnnotationObject
{

public:
  friend class rspfGeoAnnotationFontObject;
   rspfAnnotationFontObject();
   rspfAnnotationFontObject(const rspfIpt& upperLeft,
                             const rspfString& s,
                             const rspfIpt& pixelSize = rspfIpt(12, 12),
                             double rotation = 0.0,
                             const rspfDpt& scale = rspfDpt(1.0, 1.0),
                             const rspfDpt& shear = rspfDpt(0.0,0.0),
                             unsigned char r=255,
                             unsigned char g=255,
                             unsigned char b=255);
   rspfAnnotationFontObject(const rspfAnnotationFontObject& rhs);
                             
   

   virtual rspfObject* dup()const;

   virtual void draw(rspfRgbImage& anImage)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void setString(const rspfString& s);
   virtual rspfString getString()const;
   virtual void computeBoundingRect();
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual bool intersects(const rspfDrect& rect)const;
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   virtual void setFont(rspfFont* font);
   virtual void setCenterPosition(const rspfIpt& position);
   virtual void setUpperLeftPosition(const rspfIpt& position);
   virtual void setPointSize(const rspfIpt& size);
   virtual void setRotation(double rotation);
   virtual void setScale(const rspfDpt& scale);
   virtual void setShear(const rspfDpt& shear);
   virtual void setGeometryInformation(const rspfFontInformation& info);
   virtual void applyScale(double x, double y);
                           
protected:
   virtual ~rspfAnnotationFontObject();
   mutable rspfRefPtr<rspfFont>   theFont;
   rspfIpt             thePosition;
   rspfString          theString;
   rspfIpt             thePixelSize;
   double               theRotation;
   double               theHorizontalScale;
   double               theVerticalScale;
   double               theHorizontalShear;
   double               theVerticalShear;
   rspfIrect           theBoundingRect;

   void setFontInfo()const;
TYPE_DATA   
};

#endif
