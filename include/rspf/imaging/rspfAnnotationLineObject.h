//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfAnnotationLineObject.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfAnnotationLineObject_HEADER
#define rspfAnnotationLineObject_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfIpt.h>

class RSPFDLLEXPORT rspfAnnotationLineObject : public rspfAnnotationObject
{
public:
   rspfAnnotationLineObject();
   rspfAnnotationLineObject(const rspfIpt& start,
                             const rspfIpt& end,
                             unsigned char r=255,
                             unsigned char g=255,
                             unsigned char b=255,
                             long thickness=1);
   
   rspfAnnotationLineObject(long x1,
                             long y1,
                             long x2,
                             long y2,
                             unsigned char r=255,
                             unsigned char g=255,
                             unsigned char b=255,
                             long thickness=1);

   rspfAnnotationLineObject(const rspfAnnotationLineObject& rhs);
       
   virtual rspfObject* dup()const;
   
   virtual void applyScale(double x, double y);

   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;

   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual void draw(rspfRgbImage& anImage)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   virtual void computeBoundingRect();

   void setLine(const rspfDpt& start, const rspfDpt& end);
   void getLine(rspfDpt& start, rspfDpt& end);

protected:
   rspfIpt theStart;
   rspfIpt theEnd;
   rspfDrect theBoundingRect;

TYPE_DATA
};

#endif
