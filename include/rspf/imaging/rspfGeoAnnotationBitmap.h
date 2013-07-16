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
// $Id: rspfGeoAnnotationBitmap.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationBitmap_HEADER
#define rspfGeoAnnotationBitmap_HEADER
#include <rspf/imaging/rspfGeoAnnotationObject.h>

class RSPFDLLEXPORT rspfGeoAnnotationBitmap: public rspfGeoAnnotationObject
{
public:

   rspfGeoAnnotationBitmap(const rspfGpt& center= rspfGpt(0,0,0),
                            rspfRefPtr<rspfImageData> imageData=0,
                            unsigned char r = 255,
                            unsigned char g = 255,
                            unsigned char b = 255);
   
   rspfGeoAnnotationBitmap(const rspfGeoAnnotationBitmap& rhs);
   
   virtual rspfObject* dup()const;

   virtual bool intersects(const rspfDrect& rect) const;

   virtual rspfGeoAnnotationBitmap* getNewClippedObject(const rspfDrect& rect)const;

   virtual void applyScale(double x, double y);

   virtual std::ostream& print(std::ostream& out)const;
   virtual void draw(rspfRgbImage& anImage)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void transform(rspfImageGeometry* projection);

   virtual void setImageData(rspfRefPtr<rspfImageData>& imageData);
   virtual void computeBoundingRect();

protected:
   virtual ~rspfGeoAnnotationBitmap();
   
   rspfGpt                    theCenterPoint;
   rspfDpt                    theProjectedPoint;
   rspfRefPtr<rspfImageData> theImageData;

TYPE_DATA
};

#endif
