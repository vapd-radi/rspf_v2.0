//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfGeoAnnotationGdBitmapFont.cpp 15766 2009-10-20 12:37:09Z gpotts $
#include <rspf/imaging/rspfGeoAnnotationGdBitmapFont.h>
#include <rspf/imaging/rspfAnnotationGdBitmapFont.h>
#include <rspf/projection/rspfProjection.h>
#include <rspf/projection/rspfImageProjectionModel.h>
#include <rspf/base/rspfException.h>

RTTI_DEF1(rspfGeoAnnotationGdBitmapFont,
          "rspfGeoAnnotationGdBitmapFont",
          rspfGeoAnnotationObject)

rspfGeoAnnotationGdBitmapFont::rspfGeoAnnotationGdBitmapFont(const rspfGpt& position,
                                                               const rspfString &text,
                                                               rspfGdFontPtr font,
                                                               unsigned char r,
                                                               unsigned char g,
                                                               unsigned char b,
                                                               long thickness)
   : rspfGeoAnnotationObject(r, g, b, thickness),
     thePosition(position)
{
   theProjectedFont = new rspfAnnotationGdBitmapFont(rspfDpt(0,0),
                                                      text,
                                                      font,
                                                      r,
                                                      g,
                                                      b,
                                                      thickness);
}

rspfGeoAnnotationGdBitmapFont::rspfGeoAnnotationGdBitmapFont(const rspfGeoAnnotationGdBitmapFont& rhs)
   :rspfGeoAnnotationObject(rhs),
    theProjectedFont(rhs.theProjectedFont.valid()?(rspfAnnotationGdBitmapFont*)theProjectedFont->dup():(rspfAnnotationGdBitmapFont*)0),
    thePosition(rhs.thePosition)
{
}

rspfGeoAnnotationGdBitmapFont::~rspfGeoAnnotationGdBitmapFont()
{
   theProjectedFont=0;
}

rspfObject* rspfGeoAnnotationGdBitmapFont::dup()const
{
   return new rspfGeoAnnotationGdBitmapFont(*this);
}

void rspfGeoAnnotationGdBitmapFont::applyScale(double x, double y)
{
   thePosition.lond(thePosition.lond()*x);
   thePosition.latd(thePosition.latd()*y);

   if(theProjectedFont.valid())
   {
      theProjectedFont->applyScale(x, y);
   }
}

std::ostream& rspfGeoAnnotationGdBitmapFont::print(std::ostream& out)const
{
   out << "position:     " << thePosition;
   theProjectedFont->print(out);
   return out;
}

void rspfGeoAnnotationGdBitmapFont::draw(rspfRgbImage& anImage)const
{
   theProjectedFont->draw(anImage);
}

void rspfGeoAnnotationGdBitmapFont::getBoundingRect(rspfDrect& rect)const
{
   theProjectedFont->getBoundingRect(rect);
}

bool rspfGeoAnnotationGdBitmapFont::intersects(const rspfDrect& rect)const
{
   if(theProjectedFont.valid())
   {
      return theProjectedFont->intersects(rect);
   }

   return false;
}

rspfAnnotationObject* rspfGeoAnnotationGdBitmapFont::getNewClippedObject(const rspfDrect& rect)const
{
   if(intersects(rect))
   {
      if(theProjectedFont.valid())
      {
         return theProjectedFont->getNewClippedObject(rect);
      }
   }
   
   return (rspfAnnotationObject*)0;
}

void rspfGeoAnnotationGdBitmapFont::computeBoundingRect()
{
   theProjectedFont->computeBoundingRect();
}

void rspfGeoAnnotationGdBitmapFont::transform(rspfImageGeometry* projection)
{
   rspfDpt position;
   projection->worldToLocal(thePosition, position);
   theProjectedFont->setUpperLeftTextPosition(position);
}

void rspfGeoAnnotationGdBitmapFont::setText(const rspfString& text)
{
   theProjectedFont->setText(text);
}

void rspfGeoAnnotationGdBitmapFont::setFont(rspfGdFontPtr font)
{
   theProjectedFont->setFont(font);
}
