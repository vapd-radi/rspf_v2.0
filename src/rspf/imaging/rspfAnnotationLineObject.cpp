//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationLineObject.cpp 13964 2009-01-14 16:30:07Z gpotts $

#include <rspf/imaging/rspfAnnotationLineObject.h>
#include <rspf/imaging/rspfRgbImage.h>

RTTI_DEF1(rspfAnnotationLineObject,
          "rspfAnnotationLineObject",
          rspfAnnotationObject)

rspfAnnotationLineObject::rspfAnnotationLineObject()
   :rspfAnnotationObject(),
    theStart(0,0),
    theEnd(0,0),
    theBoundingRect(0,0,0,0)
{
}

rspfAnnotationLineObject::rspfAnnotationLineObject(const rspfIpt& start,
                                                     const rspfIpt& end,
                                                     unsigned char r,
                                                     unsigned char g,
                                                     unsigned char b,
                                                     long thickness)
   :rspfAnnotationObject(r, g, b, thickness),
    theStart(start),
    theEnd(end)
{
   computeBoundingRect();
}

rspfAnnotationLineObject::rspfAnnotationLineObject(long x1,
                                                     long y1,
                                                     long x2,
                                                     long y2,
                                                     unsigned char r,
                                                     unsigned char g,
                                                     unsigned char b,
                                                     long thickness)
   :rspfAnnotationObject(r, g, b, thickness),
    theStart(x1, y1),
    theEnd(x2, y2)
{
   computeBoundingRect();
}

rspfAnnotationLineObject::rspfAnnotationLineObject(const rspfAnnotationLineObject& rhs)
      :rspfAnnotationObject(rhs),
       theStart(rhs.theStart),
       theEnd(rhs.theEnd),
       theBoundingRect(rhs.theBoundingRect)
{
}

rspfObject* rspfAnnotationLineObject::dup()const
{
   return new rspfAnnotationLineObject(*this);
}

void rspfAnnotationLineObject::applyScale(double x, double y)
{
   theStart.x = rspf::round<int>(theStart.x*x);
   theStart.y = rspf::round<int>(theStart.y*y);
   theEnd.x   = rspf::round<int>(theEnd.x*x);
   theEnd.y   = rspf::round<int>(theEnd.y*y);
   computeBoundingRect();
}

rspfAnnotationObject* rspfAnnotationLineObject::getNewClippedObject(const rspfDrect& rect)const
{
   rspfAnnotationLineObject* result = (rspfAnnotationLineObject*)NULL;
   rspfDpt start = theStart;
   rspfDpt end   = theEnd;
   
   if(rect.clip(start, end))
   {
      result = (rspfAnnotationLineObject*)dup();
      result->setLine(start, end);
   }
   
   return result;
}

bool rspfAnnotationLineObject::intersects(const rspfDrect& rect)const
{
   rspfDpt start = theStart;
   rspfDpt end   = theEnd;
   
   return rect.clip(start, end);
}

void rspfAnnotationLineObject::draw(rspfRgbImage& anImage)const
{
   if(anImage.getImageData().valid())
   {
      anImage.setDrawColor(theRed, theGreen, theBlue);
      anImage.setThickness(theThickness);
      rspfDrect imageRect = anImage.getImageData()->getImageRectangle();
      rspfDpt start = theStart;
      rspfDpt end   = theEnd;
      
      if(anImage.getImageData().valid())
      {
         // we need to extend it by a couple of pixels since
         // if a pixel lies on the edge and then another pixel is just off
         // the edge we will get a stair step and so for several pixels
         // the line might be inside the image rectangle but the clip
         // algorithm will only draw 1 pixel since it thinks the first
         // point is inside and the second point is outside and will
         // execute the clip algorithm keeping only the first
         // point.
         rspfDrect clipRect(imageRect.ul().x - 10,
                             imageRect.ul().y - 10,
                             imageRect.lr().x + 10,
                             imageRect.lr().y + 10);
         // now we can draw.
         if(clipRect.clip(start, end))
         {
            anImage.drawLine(rspfIpt((int)start.x,
                                      (int)start.y),
                             rspfIpt((int)end.x,
                                      (int)end.y));
         }
      }
   }
}

std::ostream& rspfAnnotationLineObject::print(std::ostream& out)const
{
   out << "line_start:    " << theStart << endl
       << "line_end:      " << theEnd   << endl
       << "bounding_rect: " << theBoundingRect << endl;
   return out;
}

void rspfAnnotationLineObject::getBoundingRect(rspfDrect& rect)const
{
   rect = theBoundingRect;
}

void rspfAnnotationLineObject::computeBoundingRect()
{
   vector<rspfDpt> pts(2);
      
   pts[0] = theStart;
   pts[1] = theEnd;

   theBoundingRect = rspfDrect(pts);
}

void rspfAnnotationLineObject::setLine(const rspfDpt& start,
                                        const rspfDpt& end)
{
   theStart = start;
   theEnd   = end;
   computeBoundingRect();
}
void rspfAnnotationLineObject::getLine(rspfDpt& start, rspfDpt& end)
{
   start = theStart;
   end   = theEnd;
}

bool rspfAnnotationLineObject::isPointWithin(const rspfDpt& point)const
{
  if(theBoundingRect.pointWithin(point))
  {
    // we will use the implicit form of a line to determine if the point is
    // on the line.  For now we will just return true if it is within the 
    // bounding rect.
    return true;
  }

  return false;
}
