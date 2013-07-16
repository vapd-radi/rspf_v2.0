//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationPolyLineObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationPolyLineObject_HEADER
#define rspfGeoAnnotationPolyLineObject_HEADER
#include <rspf/base/rspfGpt.h>
#include <rspf/imaging/rspfGeoAnnotationObject.h>

class rspfAnnotationMultiLineObject;

class RSPFDLLEXPORT rspfGeoAnnotationPolyLineObject : public rspfGeoAnnotationObject
{
public:
   rspfGeoAnnotationPolyLineObject(const vector<rspfGpt>& groundPts,
                                     rspf_uint8 r=255,
                                     rspf_uint8 g=255,
                                     rspf_uint8 b=255,
                                     rspf_uint8 thickness=1);
   rspfGeoAnnotationPolyLineObject(const rspfGeoAnnotationPolyLineObject& rhs);
   virtual rspfObject* dup()const;

   virtual void applyScale(double x, double y);

   virtual void transform(rspfImageGeometry* projection);
   
   virtual std::ostream& print(std::ostream& out)const;
   virtual void draw(rspfRgbImage& anImage)const;
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();

   /**
    * Sets the color of thePolygon. RGB range 0 to 255
    *
    * @param r DN value for red.
    * @param g DN value for green.
    * @param b DN value for blue.
    */
   virtual void setColor(rspf_uint8 r,
                         rspf_uint8 g,
                         rspf_uint8 b);
   /**
    * Sets the pen thickness of thePolygon. range 0 to 255
    *
    * @param thickness Pen thickness.
    */
   virtual void setThickness(rspf_uint8 thickness);

protected:
   virtual ~rspfGeoAnnotationPolyLineObject();
   
   std::vector<rspfGpt>           thePolygon;
   rspfAnnotationMultiLineObject* theProjectedMultiLineObject;

TYPE_DATA
};

#endif
