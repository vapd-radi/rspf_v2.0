//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfGeoAnnotationGdBitmapFont.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationGdBitmapFont_HEADER
#define rspfGeoAnnotationGdBitmapFont_HEADER

#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/imaging/rspfGdFont.h>
#include <rspf/imaging/rspfGdFontExterns.h>
class rspfAnnotationGdBitmapFont;

class RSPFDLLEXPORT rspfGeoAnnotationGdBitmapFont : public rspfGeoAnnotationObject
{
public:
   rspfGeoAnnotationGdBitmapFont(const rspfGpt& position=rspfGpt(0,0,0),
                                  const rspfString &text=rspfString(""),
                                  rspfGdFontPtr font = rspfGdFontLarge,
                                  unsigned char r = 255,
                                  unsigned char g = 255,
                                  unsigned char b = 255,
                                  long thickness = 1);
   rspfGeoAnnotationGdBitmapFont(const rspfGeoAnnotationGdBitmapFont& rhs);
   virtual rspfObject* dup()const;
   virtual void applyScale(double x, double y);
   virtual std::ostream& print(std::ostream& out)const;
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();
   /*!
    * Add the projection interface to all
    * geographically defined objects.
    */
   virtual void transform(rspfImageGeometry* projection);

   virtual void setText(const rspfString& text);
   virtual void setFont(rspfGdFontPtr font);

protected:
   virtual ~rspfGeoAnnotationGdBitmapFont();

   rspfRefPtr<rspfAnnotationGdBitmapFont> theProjectedFont;

   /*!
    * Will be the upper left corner of text;
    */
   rspfGpt                     thePosition;
TYPE_DATA
};

#endif
