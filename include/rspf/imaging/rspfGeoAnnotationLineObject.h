//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
// Description:
//
//*************************************************************************
// $Id: rspfGeoAnnotationLineObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationLineObject_HEADER
#define rspfGeoAnnotationLineObject_HEADER
#include <rspf/base/rspfGpt.h>
#include <rspf/imaging/rspfGeoAnnotationObject.h>

class rspfAnnotationLineObject;

class rspfGeoAnnotationLineObject : public rspfGeoAnnotationObject
{
public:
   rspfGeoAnnotationLineObject(const rspfGpt& start,
                                const rspfGpt& end,
                                unsigned char r=255,
                                unsigned char g=255,
                                unsigned char b=255,
                                long thickness=1);
   rspfGeoAnnotationLineObject(const rspfGeoAnnotationLineObject& rhs);

   virtual rspfObject* dup()const;

   virtual void applyScale(double x, double y);
   virtual void transform(rspfImageGeometry* projection);

   virtual std::ostream& print(std::ostream& out)const;
   virtual bool intersects(const rspfDrect& rect)const;
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual void draw(rspfRgbImage& anImage)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();

protected:
   virtual ~rspfGeoAnnotationLineObject();
   
   rspfAnnotationLineObject* theProjectedLineObject;
   rspfGpt                   theStart;
   rspfGpt                   theEnd;

TYPE_DATA
};

#endif
