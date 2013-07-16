//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationMultiLineObject.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/imaging/rspfAnnotationMultiLineObject.h>

RTTI_DEF1(rspfAnnotationMultiLineObject,
          "rspfAnnotationMultiLineObject",
          rspfAnnotationObject);

void rspfAnnotationMultiLineObject::draw(rspfRgbImage& anImage)const
{
   if(anImage.getImageData().valid())
   {
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
         
         for(rspf_uint32 i = 0; i < thePolyLineList.size(); ++i)
         {
            const vector<rspfDpt>& vList = thePolyLineList[i].getVertexList();
            
            if(vList.size() == 1)
            {
               anImage.drawLine(rspfIpt(vList[0]),
                                rspfIpt(vList[0]));
            }
            else
            {
               for(rspf_uint32 i2 = 0; i2 < (vList.size()-1); ++i2)
               {
                  rspfDpt start = vList[i2];
                  rspfDpt end   = vList[i2+1];
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
      }
   }
}

bool rspfAnnotationMultiLineObject::intersects(const rspfDrect& rect)const
{

   for(rspf_uint32 i = 0; i < thePolyLineList.size(); ++i)
   {
      if(thePolyLineList[i].isWithin(rect))
      {
         return true;
      }
   }
   return false;
}


void rspfAnnotationMultiLineObject::applyScale(double x,
                                                double y)
{
   for(rspf_uint32 i=0; i < thePolyLineList.size(); ++i)
   {
      vector<rspfDpt>& vList = thePolyLineList[i].getVertexList();
      
      for(rspf_uint32 i2 = 0; i2 < vList.size(); ++i2)
      {
         vList[i].x *= x;
         vList[i].y *= y;
         vList[i].x *= x;
         vList[i].y *= y;
      }
   }
   theBoundingRect *= rspfDpt(x, y);
}

rspfAnnotationObject* rspfAnnotationMultiLineObject::getNewClippedObject(const rspfDrect& rect)const
{
   rspfAnnotationMultiLineObject* result = (rspfAnnotationMultiLineObject*)NULL;

   if(intersects(rect))
   {
      vector<rspfPolyLine> lineList;
      vector<rspfPolyLine> tempResult;
      
      rspfDrect clipRect(rect.ul().x - 10,
                          rect.ul().y - 10,
                          rect.lr().x + 10,
                          rect.lr().y + 10);

      for(rspf_uint32 i =0; i< thePolyLineList.size();++i)
      {
         if(thePolyLineList[i].clipToRect(tempResult, clipRect))
         {
            lineList.insert(lineList.end(),
                            tempResult.begin(),
                            tempResult.end());
         }
      }
      
      if(lineList.size() > 0)
      {
         result = new rspfAnnotationMultiLineObject(lineList,
                                                     theRed,
                                                     theGreen,
                                                     theBlue,
                                                     theThickness);
      }
   }
   
   return result;
}

bool rspfAnnotationMultiLineObject::isPointWithin(const rspfDpt& imagePoint)const
{
   for(rspf_uint32 i=0; i < thePolyLineList.size(); ++i)
   {
      if(thePolyLineList[i].isPointWithin(imagePoint))
      {
         return true;
      }
      
   }

   return false;
}

void rspfAnnotationMultiLineObject::computeBoundingRect()
{
   theBoundingRect.makeNan();
   
   if(thePolyLineList.size() == 1)
   {
      theBoundingRect = thePolyLineList[0].getBoundingRect();
   }
   else if(thePolyLineList.size() > 1)
   {
      for(rspf_uint32 i = 0; i < thePolyLineList.size(); ++i)
      {
         rspfDrect rect = thePolyLineList[i].getBoundingRect();
         if(theBoundingRect.hasNans())
         {
            theBoundingRect = rect;
         }
         else if(!rect.hasNans())
         {
            theBoundingRect = theBoundingRect.combine(rect);
         }
      }
   }
}

std::ostream& rspfAnnotationMultiLineObject::print(std::ostream& out)const
{
   rspfAnnotationObject::print(out);

   for(rspf_uint32 i = 0; i < thePolyLineList.size();++i)
   {
      out << thePolyLineList[i] << endl
          <<"______________________________________"<<endl;
   }
   return out;
}
