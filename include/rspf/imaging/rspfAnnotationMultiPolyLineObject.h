//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfAnnotationMultiPolyLineObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationMultiPolyLineObject_HEADER
#define rspfAnnotationMultiPolyLineObject_HEADER

#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfPolyLine.h>

class RSPFDLLEXPORT rspfAnnotationMultiPolyLineObject : public rspfAnnotationObject
{
public:
   rspfAnnotationMultiPolyLineObject();
   rspfAnnotationMultiPolyLineObject(const vector<rspfPolyLine>& multiPoly,
                                  unsigned char r,
                                  unsigned char g,
                                  unsigned char b,
                                  long thickness);
   
   rspfAnnotationMultiPolyLineObject(const rspfAnnotationMultiPolyLineObject& rhs);

   virtual rspfObject* dup()const;

   virtual void applyScale(double x,
                           double y);
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void addPolyLine(const rspfPolyLine& poly);
   virtual void addPoint(rspf_uint32 polygonIndex, const rspfDpt& pt);
   
   virtual void setMultiPolyLine(const vector<rspfPolyLine>& multiPoly);
   virtual void computeBoundingRect();
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   
   const std::vector<rspfPolyLine>& getMultiPolyLine()const;
   std::vector<rspfPolyLine>& getMultiPolyLine();
protected:
   virtual ~rspfAnnotationMultiPolyLineObject();
   vector<rspfPolyLine> theMultiPolyLine;
   rspfDrect           theBoundingRect;

TYPE_DATA
};

#endif
