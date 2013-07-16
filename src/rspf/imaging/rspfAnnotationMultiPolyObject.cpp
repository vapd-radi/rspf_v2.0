//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfAnnotationMultiPolyObject.cpp 10867 2007-05-09 19:58:25Z gpotts $
#include <rspf/imaging/rspfAnnotationMultiPolyObject.h>
#include <rspf/imaging/rspfAnnotationMultiLineObject.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfLine.h>

RTTI_DEF1(rspfAnnotationMultiPolyObject, "rspfAnnotationMultiPolyObject", rspfAnnotationObject)

rspfAnnotationMultiPolyObject::rspfAnnotationMultiPolyObject()
   :rspfAnnotationObject(),
    theFillEnabled(false)
{
   computeBoundingRect();
}

rspfAnnotationMultiPolyObject::rspfAnnotationMultiPolyObject(const vector<rspfPolygon>& multiPoly,
                                                               bool enableFill,
                                                               unsigned char r,
                                                               unsigned char g,
                                                               unsigned char b,
                                                               long thickness)
   :rspfAnnotationObject(r, g, b, thickness),
    theFillEnabled(enableFill)
{
   theMultiPolygon = multiPoly;
   computeBoundingRect();
}

rspfAnnotationMultiPolyObject::~rspfAnnotationMultiPolyObject()
{
}

void rspfAnnotationMultiPolyObject::applyScale(double x,
                                                double y)
{
   for(rspf_uint32 i =0; i<theMultiPolygon.size(); ++i)
   {
      theMultiPolygon[i] *= rspfDpt(x, y);
   }
   
   theBoundingRect *= rspfDpt(x,y);
}

bool rspfAnnotationMultiPolyObject::intersects(const rspfDrect& rect)const
{
   // do the quick checks first
   //
   if(rect.hasNans()) return false;
   if(!rect.intersects(theBoundingRect)) return false;
   if(theMultiPolygon.size()<1) return false;
   
   for(rspf_uint32 i =0; i < theMultiPolygon.size(); ++i)
   {
      vector<rspfPolygon> result;
      
      if(theMultiPolygon[i].clipToRect(result, rect))
      {
         return true;
      }
   }

   return false;
}

rspfAnnotationObject* rspfAnnotationMultiPolyObject::getNewClippedObject(const rspfDrect& rect)const
{
   rspfAnnotationObject* result = (rspfAnnotationObject*)NULL;

   if(theBoundingRect.intersects(rect))
   {
      rspfDrect clipRect(rect.ul().x - 10,
                          rect.ul().y - 10,
                          rect.lr().x + 10,
                          rect.lr().y + 10);
      
      if(theFillEnabled)
      {
         vector<rspfPolygon> polyListResult;
         
         for(rspf_uint32 i = 0; i < theMultiPolygon.size(); ++i)
         {
            vector<rspfPolygon> tempList;
            
            if(theMultiPolygon[i].clipToRect(tempList, rect))
            {
               polyListResult.insert(polyListResult.end(),
                                     tempList.begin(),
                                     tempList.end());
            }
         }
         
         if(polyListResult.size())
         {
            result = new rspfAnnotationMultiPolyObject(polyListResult,
                                                        theFillEnabled,
                                                        theRed,
                                                        theGreen,
                                                        theBlue,
                                                        theThickness);
         }
      }
      else
      {
         vector<rspfPolyLine> lineListResult;
         vector<rspfPolyLine> tempResult;

         for(rspf_uint32 i = 0; i< theMultiPolygon.size();++i)
         {
            rspfPolyLine polyLine = theMultiPolygon[i];

            if(polyLine.clipToRect(tempResult,
                                   clipRect))
            {
               lineListResult.insert(lineListResult.end(),
                                     tempResult.begin(),
                                     tempResult.end());
            }
         }
         
         if(lineListResult.size())
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

void rspfAnnotationMultiPolyObject::draw(rspfRgbImage& anImage)const
{
   if(theMultiPolygon.size()<1) return;
   if(theBoundingRect.hasNans()) return;
   
   
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

      int j = 0;
      for(rspf_uint32 i = 0; i < theMultiPolygon.size(); ++i)
      {
         const rspfPolygon& poly = theMultiPolygon[i];
         int vertexCount          = poly.getVertexCount();
         if(!theFillEnabled)
         {
            if(poly.getNumberOfVertices() == 1)
            {
               rspfDpt start, end;
               start = poly[0];
               end   = poly[0];
               if(clipRect.clip(start, end))
               {
                  anImage.drawLine(rspfIpt(start),
                                   rspfIpt(end));
               }
            }
            else if(poly.getNumberOfVertices() == 2)
            {
               rspfDpt start, end;
               start = poly[0];
               end   = poly[1];
               if(clipRect.clip(start, end))
               {
                  anImage.drawLine(rspfIpt(start),
                                   rspfIpt(end));
               }
            }
            else if(vertexCount > 2)
            {
               rspfDpt start, end;
               j = 0;
               while(j<vertexCount)
               {
                  start = poly[j];
                  end   = poly[(j+1)%vertexCount];
                  if(clipRect.clip(start, end))
                  {
                     anImage.drawLine(rspfIpt(start),
                                      rspfIpt(end));
                  }
                  ++j;
               }
#if 0
               rspfDpt start, end;
               start = poly[vertexCount-1];
               end   = poly[0];
               j = 0;
               do
               {
                  if(clipRect.clip(start, end))
                  {
                     anImage.drawLine(rspfIpt(start),
                                      rspfIpt(end));
                  }
                  ++j;
                  start = poly[j-1];
                  end   = poly[j];
               }while(j < vertexCount);
#endif
            }
         }
         else
         {
            vector<rspfPolygon> result;
            if(poly.clipToRect(result, imageRect))
            {
               for(j = 0; j < (int)result.size();++j)
               {
                  anImage.drawFilledPolygon(result[j].getVertexList());
               }
            }
         }
      }
   }
}

std::ostream& rspfAnnotationMultiPolyObject::print(std::ostream& out)const
{
   rspfAnnotationObject::print(out);
   out << endl;
   out << setw(15)<<setiosflags(ios::left)<<"type:"<<getClassName() << endl
       << setw(15)<<setiosflags(ios::left)<<"polygons:"<<theMultiPolygon.size()<<endl
       << setw(15)<<setiosflags(ios::left)<<"fill enable: " << theFillEnabled << endl;
   
   for(rspf_uint32 i = 0; i < theMultiPolygon.size(); ++i)
   {
      out << "____________________________________________________"<<endl
          << theMultiPolygon[i] << endl;
   }
   out << "____________________________________________________"<<endl;
   return out;
}

void rspfAnnotationMultiPolyObject::getBoundingRect(rspfDrect& rect)const
{   
   rect = theBoundingRect;
}

void rspfAnnotationMultiPolyObject::clear()
{
   theMultiPolygon.clear();
}

void rspfAnnotationMultiPolyObject::computeBoundingRect()
{
   if(theMultiPolygon.size())
   {
      rspfDrect rect(theMultiPolygon[0]);
      
      for(rspf_uint32 i = 1; i < theMultiPolygon.size(); ++i)
      {
         rspfDrect rect2(theMultiPolygon[i]);

         if(rect.hasNans())
         {
            rect = rect2;
         }
         else
         {
            if(!rect2.hasNans())
            {
               rect = rect.combine(rect2);
            }
         }
      }
      
      theBoundingRect = rect;
   }
   else
   {
      theBoundingRect.makeNan();
   }
   if(!theBoundingRect.hasNans())
   {
      rspfIpt origin = theBoundingRect.ul();
      theBoundingRect = rspfDrect(origin.x - theThickness/2,
                                   origin.y - theThickness/2,
                                   origin.x + (theBoundingRect.width()-1) + theThickness/2,
                                   origin.y + (theBoundingRect.height()-1) + theThickness/2);
   }
}

bool rspfAnnotationMultiPolyObject::isPointWithin(const rspfDpt& imagePoint)const
{
   for(rspf_uint32 i = 0; i < theMultiPolygon.size(); ++i)
   {
      if(theMultiPolygon[i].isPointWithin(imagePoint))
      {
         return true;
      }
   }

   return false;
}
