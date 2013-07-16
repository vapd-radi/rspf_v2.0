//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description:
//
//*************************************************************************
// $Id: rspfGeoAnnotationMultiPolyObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationMultiPolyObject_HEADER
#define rspfGeoAnnotationMultiPolyObject_HEADER
#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfGeoPolygon.h>
#include <rspf/imaging/rspfAnnotationMultiPolyObject.h>

class RSPF_DLL rspfGeoAnnotationMultiPolyObject : public rspfGeoAnnotationObject
{
public:
   rspfGeoAnnotationMultiPolyObject();
   rspfGeoAnnotationMultiPolyObject(const vector<rspfGeoPolygon>& multiPoly,
                                     bool enableFill=false,
                                     unsigned char r=255,
                                     unsigned char g=255,
                                     unsigned char b=255,
                                     long thickness=1);
   
   rspfGeoAnnotationMultiPolyObject(const rspfGeoAnnotationMultiPolyObject& rhs);
   virtual rspfObject* dup()const;

   virtual void transform(rspfImageGeometry* projection);

   virtual void applyScale(double x,
                                double y);
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void addPoint(rspf_uint32 polygonIndex, const rspfGpt& pt);
   
   virtual void setMultiPolygon(const vector<rspfGeoPolygon>& multiPoly);

   virtual void setColor(unsigned char r, unsigned char g, unsigned char b);
   
   virtual void setThickness(rspf_uint8 thickness);

   virtual void computeBoundingRect();
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   virtual void setFillFlag(bool flag);
  
   const std::vector<rspfGeoPolygon>& getMultiPolygon()const{return theMultiPolygon;}
   std::vector<rspfGeoPolygon>& getMultiPolygon(){return theMultiPolygon;}
   
protected:
   virtual ~rspfGeoAnnotationMultiPolyObject();

   std::vector<rspfGeoPolygon>    theMultiPolygon;
   rspfDrect                      theBoundingRect;
   bool                            theFillEnabled;
   rspfRefPtr<rspfAnnotationMultiPolyObject> theProjectedPolyObject;
   
   void allocateProjectedPolygon();
TYPE_DATA
};

#endif
