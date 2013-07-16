//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfAnnotationMultiPolyObject.h 17815 2010-08-03 13:23:14Z dburken $
#ifndef rspfAnnotationMultiPolyObject_HEADER
#define rspfAnnotationMultiPolyObject_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfPolygon.h>

class RSPFDLLEXPORT rspfAnnotationMultiPolyObject : public rspfAnnotationObject
{
public:
   rspfAnnotationMultiPolyObject();
   rspfAnnotationMultiPolyObject(const vector<rspfPolygon>& multiPoly,
                                  bool enableFill,
                                  unsigned char r,
                                  unsigned char g,
                                  unsigned char b,
                                  long thickness);
   
   rspfAnnotationMultiPolyObject(const rspfAnnotationMultiPolyObject& rhs)
      : rspfAnnotationObject(rhs),
        theMultiPolygon(rhs.theMultiPolygon),
        theBoundingRect(rhs.theBoundingRect),
        theFillEnabled(rhs.theFillEnabled)
      {
      }
   virtual rspfObject* dup()const
      {
         return new rspfAnnotationMultiPolyObject(*this);
      }
   virtual void applyScale(double x,
                           double y);
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void clear();
   virtual void addPolygon(rspf_uint32 polygonIndex,
                           const rspfPolygon& poly)
      {
         if(polygonIndex < theMultiPolygon.size())
         {
             theMultiPolygon[polygonIndex] = poly;
         }
      }
   virtual void addPoint(rspf_uint32 polygonIndex,
                         const rspfDpt& pt)
      {
         if(polygonIndex < theMultiPolygon.size())
         {
            theMultiPolygon[polygonIndex].addPoint(pt);
         }
      }
   
   virtual void setMultiPolygon(const vector<rspfPolygon>& multiPoly)
      {
         theMultiPolygon = multiPoly;
         computeBoundingRect();
      }
   virtual void computeBoundingRect();
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   
   const std::vector<rspfPolygon>& getMultiPolygon()const{return theMultiPolygon;}
   std::vector<rspfPolygon>& getMultiPolygon(){return theMultiPolygon;}
   void setFillFlag(bool flag)
      {
         theFillEnabled = flag;
      }
protected:
   virtual ~rspfAnnotationMultiPolyObject();
   vector<rspfPolygon> theMultiPolygon;
   rspfDrect           theBoundingRect;
   bool                 theFillEnabled;

TYPE_DATA
};

#endif
