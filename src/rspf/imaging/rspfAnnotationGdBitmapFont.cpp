//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfAnnotationGdBitmapFont.cpp 12276 2008-01-07 19:58:43Z dburken $
#include <rspf/imaging/rspfAnnotationGdBitmapFont.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfCommon.h>

#include <rspf/imaging/gdfonts.inc>
#include <rspf/imaging/gdfontl.inc>
#include <rspf/imaging/gdfontg.inc>
#include <rspf/imaging/gdfontmb.inc>
#include <rspf/imaging/gdfontt.inc>

rspfAnnotationGdBitmapFont::rspfAnnotationGdBitmapFont(const rspfDpt& position,
                                                         const rspfString &text,
                                                         rspfGdFontPtr font,
                                                         unsigned char r,
                                                         unsigned char g,
                                                         unsigned char b,
                                                         long thickness)
   :rspfAnnotationObject(r, g, b, thickness),
    theFontPtr(font),
    theText(text),
    thePosition(position)
{
   computeBoundingRect();
}

rspfAnnotationGdBitmapFont::rspfAnnotationGdBitmapFont(const rspfAnnotationGdBitmapFont& rhs)
   : rspfAnnotationObject(rhs),
     theFontPtr(rhs.theFontPtr),
     theText(rhs.theText),
     theBoundingRect(rhs.theBoundingRect),
     thePosition(rhs.thePosition)
{
}

rspfAnnotationGdBitmapFont::~rspfAnnotationGdBitmapFont()
{
}

void rspfAnnotationGdBitmapFont::getBoundingRect(rspfDrect& rect)const
{
   rect = theBoundingRect;
}

void rspfAnnotationGdBitmapFont::applyScale(double x, double y)
{
   thePosition.x *= x;
   thePosition.y *= y;

   theBoundingRect *= rspfDpt(x, y);
}

void rspfAnnotationGdBitmapFont::draw(rspfRgbImage& anImage)const
{
   if(!theFontPtr||
   !anImage.getImageData())
   {
      return;
   }
   rspfDpt origin = anImage.getImageData()->getOrigin();

   rspfDrect rect = anImage.getImageData()->getImageRectangle();

   if(theBoundingRect.intersects(rect))
   {
      anImage.setDrawColor(theRed, theGreen, theBlue);
      rspfDpt position(thePosition.x - origin.x,
                        thePosition.y - origin.y);
      for(std::string::size_type character = 0;
          character < theText.size();
          ++character)
      {
         long charOffset = theText[character];

         // we need to shift the offset to a positive value.
         // if it goes negative we just add 256 to the value.
         if(charOffset < 0)
         {
            charOffset += 256;
         }
         // get the starting offset to the bitmap
         long offset = charOffset*theFontPtr->w*theFontPtr->h;
         for(long row = 0; row < theFontPtr->h; ++row)
         {
            for(long col = 0; col < theFontPtr->w; ++col)
            {
               if(theFontPtr->data[(int)offset])
               {
                  anImage.slowPlotPixel(rspf::round<int>(position.x + col),
                                        rspf::round<int>(position.y + row),
                                        theRed,
                                        theGreen,
                                        theBlue);
               }
               ++offset;
            }
         }
         position.x += theFontPtr->w;
      }
   }
}

rspfAnnotationObject* rspfAnnotationGdBitmapFont::getNewClippedObject(const rspfDrect& rect)const
{
   rspfAnnotationGdBitmapFont* result = (rspfAnnotationGdBitmapFont*)NULL;

   if(intersects(rect))
   {
      result = (rspfAnnotationGdBitmapFont*)dup();
   }

   return result;
}

std::ostream& rspfAnnotationGdBitmapFont::print(std::ostream& out)const
{
   out << "text_value:        " << theText << endl
       << "text_bouding_rect: " << theBoundingRect << endl
       << "text_position:     " << thePosition << endl;
   return out;
}

void rspfAnnotationGdBitmapFont::computeBoundingRect()
{
   if(theFontPtr)
   {
      theBoundingRect = rspfDrect(thePosition.x,
                                   thePosition.y,
                                   thePosition.x + theText.size()*theFontPtr->w,
                                   thePosition.y + theFontPtr->h);
   }
}

void rspfAnnotationGdBitmapFont::setFont(rspfGdFontPtr font)
{
   theFontPtr = font;
}

void rspfAnnotationGdBitmapFont::setText(const rspfString& text)
{
   theText = text;
   computeBoundingRect();
}

void rspfAnnotationGdBitmapFont::setUpperLeftText(const rspfDpt& upperLeft,
                                                   const rspfString& text)
{
   setText(text);
   setUpperLeftTextPosition(upperLeft);
}

void rspfAnnotationGdBitmapFont::setUpperLeftTextPosition(const rspfDpt& position)
{
   thePosition = position;
   computeBoundingRect();
}

void rspfAnnotationGdBitmapFont::setCenterText(const rspfDpt& upperLeft,
                                                   const rspfString& text)
{
   setText(text);
   setCenterTextPosition(upperLeft);
}

void rspfAnnotationGdBitmapFont::setCenterTextPosition(const rspfDpt& position)
{
   thePosition.x = position.x - theBoundingRect.width()/2.0;
   thePosition.y = position.y - theBoundingRect.height()/2.0;
   computeBoundingRect();
}

bool rspfAnnotationGdBitmapFont::isPointWithin(const rspfDpt& imagePoint)const
{
   return theBoundingRect.pointWithin(imagePoint);
}
