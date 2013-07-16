//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationMultiPolyLineObject.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/imaging/rspfAnnotationMultiPolyLineObject.h>
#include <rspf/imaging/rspfAnnotationMultiLineObject.h>
#include <rspf/imaging/rspfRgbImage.h>
#include <rspf/base/rspfIrect.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfLine.h>

RTTI_DEF1(rspfAnnotationMultiPolyLineObject, "rspfAnnotationMultiPolyLineObject", rspfAnnotationObject)

rspfAnnotationMultiPolyLineObject::rspfAnnotationMultiPolyLineObject()
   :rspfAnnotationObject()
{
   computeBoundingRect();
}

rspfAnnotationMultiPolyLineObject::rspfAnnotationMultiPolyLineObject(const vector<rspfPolyLine>& multiPoly,
								       unsigned char r,
								       unsigned char g,
								       unsigned char b,
								       long thickness)
   :rspfAnnotationObject(r, g, b, thickness)
{
   theMultiPolyLine = multiPoly;
   computeBoundingRect();
}

rspfAnnotationMultiPolyLineObject::~rspfAnnotationMultiPolyLineObject()
{
}

void rspfAnnotationMultiPolyLineObject::applyScale(double x,
                                                double y)
{
   for(rspf_uint32 i =0; i<theMultiPolyLine.size(); ++i)
   {
      theMultiPolyLine[i] *= rspfDpt(x, y);
   }
   computeBoundingRect();
   // theBoundingRect *= rspfDpt(x,y);
}

bool rspfAnnotationMultiPolyLineObject::intersects(const rspfDrect& rect)const
{
   // do the quick checks first
   //
   if(rect.hasNans()) return false;
   if(!rect.intersects(theBoundingRect)) return false;
   if(theMultiPolyLine.size()<1) return false;
   
   for(rspf_uint32 i =0; i < theMultiPolyLine.size(); ++i)
   {
      vector<rspfPolyLine> result;
      
      if(theMultiPolyLine[i].clipToRect(result, rect))
      {
         return true;
      }
   }

   return false;
}

rspfAnnotationObject* rspfAnnotationMultiPolyLineObject::getNewClippedObject(
   const rspfDrect& /* rect */)const
{
   rspfAnnotationObject* result = (rspfAnnotationObject*)NULL;
   
   rspfNotify(rspfNotifyLevel_WARN)
      << "rspfAnnotationMultiPolyLineObject::getNewClippedObject\n"
      << "Not implemented for " << getClassName() << endl;
   return result;
}

void rspfAnnotationMultiPolyLineObject::draw(rspfRgbImage& anImage)const
{
   if(theMultiPolyLine.size()<1) return;
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
      for(rspf_uint32 i = 0; i < theMultiPolyLine.size(); ++i)
      {
         const rspfPolyLine& poly = theMultiPolyLine[i];
         int vertexCount            = poly.getNumberOfVertices();

	 if(vertexCount > 1)
	   {

	     rspfDpt start, end;
	     start = poly[0];
	     end   = poly[1];
	     j = 1;
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
	   }
	 else if(vertexCount == 1)
	   {
		     anImage.drawLine(rspfIpt(poly[0]),
				      rspfIpt(poly[0]));
	   }
      }
   }
}

std::ostream& rspfAnnotationMultiPolyLineObject::print(std::ostream& out)const
{
   rspfAnnotationObject::print(out);
   out << endl;
   out << setw(15)<<setiosflags(ios::left)<<"type:"<<getClassName() << endl
       << setw(15)<<setiosflags(ios::left)<<"polylines:"<<theMultiPolyLine.size()<<endl;
   
   for(rspf_uint32 i = 0; i < theMultiPolyLine.size(); ++i)
   {
      out << "____________________________________________________"<<endl
          << theMultiPolyLine[i] << endl;
   }
   out << "____________________________________________________"<<endl;
   return out;
}

void rspfAnnotationMultiPolyLineObject::getBoundingRect(rspfDrect& rect)const
{   
   rect = theBoundingRect;
}

void rspfAnnotationMultiPolyLineObject::computeBoundingRect()
{
   if(theMultiPolyLine.size())
   {
      rspfDrect rect(theMultiPolyLine[0].getVertexList());
      
      for(rspf_uint32 i = 1; i < theMultiPolyLine.size(); ++i)
      {
         rspfDrect rect2(theMultiPolyLine[i].getVertexList());

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

bool rspfAnnotationMultiPolyLineObject::isPointWithin(const rspfDpt& imagePoint)const
{
//    for(rspf_uint32 i = 0; i < theMultiPolyLine.size(); ++i)
//    {
//       if(theMultiPolyLine[i].isPointWithin(imagePoint))
//       {
//          return true;
//       }
//   }

  return theBoundingRect.pointWithin(imagePoint);
}

rspfAnnotationMultiPolyLineObject::rspfAnnotationMultiPolyLineObject(const rspfAnnotationMultiPolyLineObject& rhs)
      : rspfAnnotationObject(rhs),
        theMultiPolyLine(rhs.theMultiPolyLine),
        theBoundingRect(rhs.theBoundingRect)
{
}

rspfObject* rspfAnnotationMultiPolyLineObject::dup()const
{
   return new rspfAnnotationMultiPolyLineObject(*this);
}

void rspfAnnotationMultiPolyLineObject::addPolyLine(const rspfPolyLine& poly)
{
   theMultiPolyLine.push_back(poly);
}

void rspfAnnotationMultiPolyLineObject::addPoint(rspf_uint32 polygonIndex,
                                                  const rspfDpt& pt)
{
   if(polygonIndex < theMultiPolyLine.size())
   {
      theMultiPolyLine[polygonIndex].addPoint(pt);
   }
}

void rspfAnnotationMultiPolyLineObject::setMultiPolyLine(const vector<rspfPolyLine>& multiPoly)
{
   theMultiPolyLine = multiPoly;
   computeBoundingRect();
}

const std::vector<rspfPolyLine>& rspfAnnotationMultiPolyLineObject::getMultiPolyLine()const
{
   return theMultiPolyLine;
}

std::vector<rspfPolyLine>& rspfAnnotationMultiPolyLineObject::getMultiPolyLine()
{
   return theMultiPolyLine;
}

