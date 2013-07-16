//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationMultiLineObject.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAnnotationMultiLineObject_HEADER
#define rspfAnnotationMultiLineObject_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfPolyLine.h>

class RSPFDLLEXPORT rspfAnnotationMultiLineObject : public rspfAnnotationObject
{
public:
   rspfAnnotationMultiLineObject();
   rspfAnnotationMultiLineObject(const std::vector<rspfPolyLine>& lineList,
                                  unsigned char r=255,
                                  unsigned char g=255,
                                  unsigned char b=255,
                                  long thickness=1)
      :rspfAnnotationObject(r, g, b, thickness),
       thePolyLineList(lineList)
      {
         computeBoundingRect();
      }

   rspfAnnotationMultiLineObject(const rspfPolyLine& lineList,
                                  unsigned char r=255,
                                  unsigned char g=255,
                                  unsigned char b=255,
                                  long thickness=1)
      :rspfAnnotationObject(r, g, b, thickness)
      {
         thePolyLineList.push_back(lineList);
         computeBoundingRect();
      }
   

   rspfAnnotationMultiLineObject(const rspfAnnotationMultiLineObject& rhs)
      :rspfAnnotationObject(rhs),
       thePolyLineList(rhs.thePolyLineList),
       theBoundingRect(rhs.theBoundingRect)
      {
      }
       
   virtual rspfObject* dup()const
      {
         return new rspfAnnotationMultiLineObject(*this);
      }
   
   virtual void applyScale(double x,
                           double y);
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual bool intersects(const rspfDrect& rect)const;   
   virtual void draw(rspfRgbImage& anImage)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const
      {
         rect = theBoundingRect;
      }
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   virtual void computeBoundingRect();

   virtual const vector<rspfPolyLine>& getPolyLineList()const
      {
         return thePolyLineList;
      }

   virtual vector<rspfPolyLine>& getPolyLineList()
      {
         return thePolyLineList;
      }
   virtual void setPolyLine(const rspfPolyLine& line)
      {
         thePolyLineList.clear();
         thePolyLineList.push_back(line);
      }
protected:
   vector<rspfPolyLine> thePolyLineList;
   rspfDrect            theBoundingRect;

TYPE_DATA
};

#endif
