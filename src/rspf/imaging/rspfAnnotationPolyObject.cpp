//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationPolyObject.cpp 13964 2009-01-14 16:30:07Z gpotts $
#include <rspf/imaging/rspfAnnotationPolyObject.h>
#include <rspf/imaging/rspfAnnotationMultiPolyObject.h>
#include <rspf/imaging/rspfAnnotationMultiLineObject.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfIrect.h>

RTTI_DEF1(rspfAnnotationPolyObject,
          "rspfAnnotationPolyObject",
          rspfAnnotationObject)

   rspfAnnotationPolyObject::rspfAnnotationPolyObject(bool enableFill,
                                                        rspf_uint8 r,
                                                        rspf_uint8 g,
                                                        rspf_uint8 b,
                                                        rspf_uint8 thickness)
      :rspfAnnotationObject(r, g, b, thickness),
       thePolygon(),
       theBoundingRect(),
       theFillEnabled(enableFill)
{
}

rspfAnnotationPolyObject::rspfAnnotationPolyObject(
   const vector<rspfDpt>& imagePts,
   bool enableFill,
   rspf_uint8 r,
   rspf_uint8 g,
   rspf_uint8 b,
   rspf_uint8 thickness)
   :rspfAnnotationObject(r, g, b, thickness),
    theFillEnabled(enableFill)
{
   thePolygon = imagePts;
   computeBoundingRect();
}

rspfAnnotationPolyObject::rspfAnnotationPolyObject(
   const rspfAnnotationPolyObject& rhs)
   : rspfAnnotationObject(rhs),
     thePolygon(rhs.thePolygon),
     theBoundingRect(rhs.theBoundingRect),
     theFillEnabled(rhs.theFillEnabled)
{
}

rspfObject* rspfAnnotationPolyObject::dup()const
{
   return new rspfAnnotationPolyObject(*this);
}

rspfAnnotationPolyObject::~rspfAnnotationPolyObject()
{
}

void rspfAnnotationPolyObject::applyScale(double x, double y)
{
   thePolygon      *= rspfDpt(x, y);
   computeBoundingRect();
}

bool rspfAnnotationPolyObject::intersects(const rspfDrect& rect)const
{
   // do the quick checks first
   //
   if(rect.hasNans()) return false;
   if(!rect.intersects(theBoundingRect)) return false;
   
   if(!theFillEnabled)
   {
      int vertexCount = thePolygon.getVertexCount();
      rspfDpt start, end;
      int j = 0;
      while(j<vertexCount)
      {
         start = thePolygon[j];
         end   = thePolygon[(j+1)%vertexCount];
         if(rect.clip(start, end))
         {
            return true;
         }
         ++j;
      }
//       start = thePolygon[vertexCount-1];
//       end   = thePolygon[0];
//       int i = 0;
//       do
//       {
//          if(rect.clip(start, end))
//          {
//             return true;
//          }
//          ++i;
//          start = thePolygon[i];
//          end   = thePolygon[i-1];
//       }while(i < vertexCount);
   }
   else
   {
      vector<rspfPolygon> result;
      return thePolygon.clipToRect(result, rect);
   }

   return false;
}

rspfAnnotationObject* rspfAnnotationPolyObject::getNewClippedObject(const rspfDrect& rect)const
{
   rspfAnnotationObject* result = (rspfAnnotationObject*)NULL;

   if(theBoundingRect.intersects(rect))
   {
      if(theFillEnabled)
      {
         vector<rspfPolygon> resultPoly;
         
         if(thePolygon.clipToRect(resultPoly, rect))
         {
            if(resultPoly.size() == 1)
            {
               result = new rspfAnnotationPolyObject(resultPoly[0].getVertexList(),
                                                      theFillEnabled,
                                                      theRed,
                                                      theGreen,
                                                      theBlue,
                                                      theThickness);
            }
            else
            {
               result = new rspfAnnotationMultiPolyObject(resultPoly,
                                                           theFillEnabled,
                                                           theRed,
                                                           theGreen,
                                                           theBlue,
                                                           theThickness);
            }
         }
      }
      else
      {
         vector<rspfPolyLine> lineListResult;
         rspfPolyLine polyLine = thePolygon;
         
         if(polyLine.clipToRect(lineListResult,
                                rect))
         {
            result = new rspfAnnotationMultiLineObject(lineListResult,
                                                        theRed,
                                                        theGreen,
                                                        theBlue,
                                                        theThickness);
         }
      }
   }

   return result;
}

void rspfAnnotationPolyObject::draw(rspfRgbImage& anImage)const
{
   if(thePolygon.getVertexCount() < 2) return;
   if(theBoundingRect.hasNans()) return;
   int vertexCount = thePolygon.getVertexCount();
   
   anImage.setDrawColor(theRed, theGreen, theBlue);
   anImage.setThickness(theThickness);
   rspfDrect imageRect = anImage.getImageData()->getImageRectangle();
   if(theBoundingRect.intersects(imageRect))
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
      
      if(!theFillEnabled)
      {
         rspfDpt start, end;
         if(thePolygon.getNumberOfVertices() == 1)
         {
            start = thePolygon[0];
            end   = thePolygon[0];
            if(clipRect.clip(start, end))
            {
               anImage.drawLine(rspfIpt(start),
                                rspfIpt(end));
            }
         }
         else if(thePolygon.getNumberOfVertices() == 2)
         {
            start = thePolygon[0];
            end   = thePolygon[1];
            if(clipRect.clip(start, end))
            {
               anImage.drawLine(rspfIpt(start),
                                rspfIpt(end));
            }
         }
         else
         {
            int j = 0;
            while(j<vertexCount)
            {
               start = thePolygon[j];
               end   = thePolygon[(j+1)%vertexCount];
               if(clipRect.clip(start, end))
               {
                  anImage.drawLine(rspfIpt(start),
                                   rspfIpt(end));
               }
               ++j;
            }
         }
#if 0
               rspfDpt start, end;
               start = thePolygon[vertexCount-1];
               end   = thePolygon[0];
               int i = 0;
               do
               {
                  if(clipRect.clip(start, end))
                  {
                     anImage.drawLine(rspfIpt((int)start.x, (int)start.y),
                                      rspfIpt((int)end.x, (int)end.y));
                  }
                  ++i;
                  start = thePolygon[i];
                  end   = thePolygon[i-1];
               }while(i < vertexCount);
#endif    
      }
      else
      {
         vector<rspfPolygon> result;
         if(thePolygon.clipToRect(result, imageRect))
         {
            for(int i = 0; i < (int)result.size();++i)
            {
               anImage.drawFilledPolygon(result[i].getVertexList());
            }
         }
      }
   }
}

std::ostream& rspfAnnotationPolyObject::print(std::ostream& out)const
{
   out << "number_of_points:  " << thePolygon.getVertexCount();
   if(thePolygon.getVertexCount() > 0)
   {
      for(long index =0; index < (long)(thePolygon.getVertexCount()-1); ++index)
      {
         out << thePolygon[index] << endl;
            
      }
      out << thePolygon[thePolygon.getVertexCount()-1] << endl;
   }
   out << "fill_enabled:      " << theFillEnabled << endl
       << "bounding_rect:     " << theBoundingRect << endl;
   return out;
}

void rspfAnnotationPolyObject::getBoundingRect(rspfDrect& rect)const
{
   rect = theBoundingRect;
}

void rspfAnnotationPolyObject::addPoint(const rspfDpt& pt)
{
   thePolygon.addPoint(pt);
}

void rspfAnnotationPolyObject::setPolygon(const vector<rspfDpt>& imagePoints)
{
   thePolygon = imagePoints;
}

void rspfAnnotationPolyObject::setPolygon(const rspfPolygon& polygon)
{
   thePolygon = polygon;
}

void rspfAnnotationPolyObject::setPolygon(const rspfIrect& rect)
{
   thePolygon = rect;
}

void rspfAnnotationPolyObject::setPolygon(const rspfDrect& rect)
{
   thePolygon = rect;
}

void rspfAnnotationPolyObject::computeBoundingRect()
{
   theBoundingRect = rspfDrect(thePolygon);
}

bool rspfAnnotationPolyObject::isPointWithin(const rspfDpt& imagePoint)const
{
   return theBoundingRect.pointWithin(imagePoint);
}

const rspfPolygon& rspfAnnotationPolyObject::getPolygon() const
{
   return thePolygon;
}

rspfPolygon& rspfAnnotationPolyObject::getPolygon()
{
   return thePolygon;
}

void rspfAnnotationPolyObject::setFillFlag(bool flag)
{
   theFillEnabled = flag;
}
