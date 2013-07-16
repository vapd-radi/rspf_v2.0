//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAnnotationPolyObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfAnnotationPolyObject_HEADER
#define rspfAnnotationPolyObject_HEADER

#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfPolygon.h>

class RSPFDLLEXPORT rspfAnnotationPolyObject : public rspfAnnotationObject
{
public:
   rspfAnnotationPolyObject(bool enableFill = false,
                             rspf_uint8 r=255,
                             rspf_uint8 g=255,
                             rspf_uint8 b=255,
                             rspf_uint8 thickness=1);
   
   rspfAnnotationPolyObject(const vector<rspfDpt>& imagePts,
                             bool enableFill = false,
                             rspf_uint8 r=255,
                             rspf_uint8 g=255,
                             rspf_uint8 b=255,
                             rspf_uint8 thickness=1);
   
   rspfAnnotationPolyObject(const rspfAnnotationPolyObject& rhs);

   rspfObject* dup()const;


   virtual void applyScale(double x, double y);

   virtual void draw(rspfRgbImage& anImage)const;

   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual rspfAnnotationObject* getNewClippedObject(
      const rspfDrect& rect)const;

   virtual std::ostream& print(std::ostream& out)const;

   virtual void getBoundingRect(rspfDrect& rect)const;

   virtual void addPoint(const rspfDpt& pt);

   virtual void setPolygon(const vector<rspfDpt>& imagePoints);

   virtual void setPolygon(const rspfPolygon& polygon);

   virtual void setPolygon(const rspfIrect& rect);

   virtual void setPolygon(const rspfDrect& rect);

   virtual void computeBoundingRect();

   virtual bool isPointWithin(const rspfDpt& imagePoint)const;

   const rspfPolygon& getPolygon()const;

   rspfPolygon& getPolygon();

   virtual void setFillFlag(bool flag);

protected:
   virtual ~rspfAnnotationPolyObject();
  rspfPolygon thePolygon;
   rspfDrect   theBoundingRect;
   bool         theFillEnabled;

TYPE_DATA
};

#endif
