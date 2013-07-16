//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfAnnotationGdBitmapFont.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationGdBitmapFont_HEADER
#define rspfAnnotationGdBitmapFont_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/imaging/rspfGdFont.h>
#include <rspf/imaging/rspfGdFontExterns.h>
#include <rspf/base/rspfString.h>

/*!
 * This is a class that will have hardcoded
 * bitmap fonts.  These fonts will be from
 * the gd library.
 */
class RSPFDLLEXPORT rspfAnnotationGdBitmapFont : public rspfAnnotationObject
{
public:
   rspfAnnotationGdBitmapFont(const rspfDpt& position=rspfDpt(0,0),
                               const rspfString &text=rspfString(""),
                               rspfGdFontPtr font = rspfGdFontLarge,
                               unsigned char r = 255,
                               unsigned char g = 255,
                               unsigned char b = 255,
                               long thickness = 1);
   rspfAnnotationGdBitmapFont(const rspfAnnotationGdBitmapFont& rhs);

   virtual rspfObject* dup()const
      {
         return new rspfAnnotationGdBitmapFont(*this);
      }
   
   virtual void applyScale(double x, double y);

   /*!
    * Will draw the bitmap to the screen.
    */
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const
      {
         return rect.intersects(theBoundingRect);
      }
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;

   virtual std::ostream& print(std::ostream& out)const;
   
   /*!
    * Will retrieve the bound rectangle for this
    * object.
    */
   virtual void getBoundingRect(rspfDrect& rect)const;
   
   /*!
    * will compute the bounding rectangle.
    */
   virtual void computeBoundingRect();

   /*!
    * Sets the current bitmap font to use.
    */
   virtual void setFont(rspfGdFontPtr font);

   /*!
    * Set's the text string to render.
    */
   virtual void setText(const rspfString& text);

   /*!
    * Set's the text string to render.  This
    * will assume that the x,y passed in is
    * suppose to be the center.  We will compute
    * the upper left corner from this
    */
   virtual void setCenterText(const rspfDpt& center,
                              const rspfString& text);

   /*!
    * Set's the text string to render.  This
    * will assume that the x,y passed in is
    * suppose to be the upper left.  
    */
   virtual void setUpperLeftText(const rspfDpt& upperLeft,
                                 const rspfString& text);

   /*!
    * Sets the center position of the string.
    * will use this value to compute the actual
    * origin of upper left corner of the first
    * character.  Make sure you have the text
    * set before you call this method
    */
   virtual void setCenterTextPosition(const rspfDpt& position);

   /*!
    * The actual upper left corner of the character.
    */
   virtual void setUpperLeftTextPosition(const rspfDpt& position);

   /*!
    * Will determine if the point is within the font
    */
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   
protected:
   virtual ~rspfAnnotationGdBitmapFont();
   rspfGdFontPtr theFontPtr;
   rspfString    theText;
   rspfDrect     theBoundingRect;

   /*!
    * This position is suppose to be the upperleft
    * corner of the first character in the string
    */
   rspfDpt       thePosition;
};

#endif
