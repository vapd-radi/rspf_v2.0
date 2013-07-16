//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationMultiEllipseObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationMultiEllipseObject_HEADER
#define rspfGeoAnnotationMultiEllipseObject_HEADER
#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <vector>
class rspfAnnotationMultiEllipseObject;
class RSPF_DLL rspfGeoAnnotationMultiEllipseObject : public rspfGeoAnnotationObject
{
public:
   rspfGeoAnnotationMultiEllipseObject();
   rspfGeoAnnotationMultiEllipseObject(const std::vector<rspfGpt>& centerPoints,
                                        const rspfDpt& widthHeight,
                                        bool enableFill = false,
                                        unsigned char r = 255,
                                        unsigned char g = 255,
                                        unsigned char b = 255,
                                        long thickness = 1);
   rspfGeoAnnotationMultiEllipseObject(const rspfDpt& widthHeight,
                                        bool enableFill = false,
                                        unsigned char r = 255,
                                        unsigned char g = 255,
                                        unsigned char b = 255,
                                        long thickness = 1);
   rspfGeoAnnotationMultiEllipseObject(const rspfGeoAnnotationMultiEllipseObject& rhs);
   
   virtual rspfObject* dup()const;

   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;

   virtual void transform(rspfImageGeometry* projection);
   
   virtual void applyScale(double x, double y);

   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual void addPoint(const rspfGpt& point);

   virtual void setColor(unsigned char r,
                         unsigned char g,
                         unsigned char b);
   virtual void setThickness(rspf_uint8 thickness);
   
   virtual void setPoint(int i, const rspfGpt& point);

   virtual void setWidthHeight(const rspfDpt& widthHeight);
   virtual void resize(rspf_uint32 newSize);

   virtual void setFillFlag(bool fillFlag);
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();
   
protected:
   rspfAnnotationMultiEllipseObject* theProjectedObject;
   
   std::vector<rspfGpt> thePointList;
   rspfDpt              theWidthHeight;
   bool                  theFillFlag;
};

#endif /* #ifndef rspfGeoAnnotationMultiEllipseObject_HEADER */
