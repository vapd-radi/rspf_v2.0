//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationMultiPolyLineObject.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfGeoAnnotationMultiPolyLineObject_HEADER
#define rspfGeoAnnotationMultiPolyLineObject_HEADER

#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/imaging/rspfAnnotationMultiPolyObject.h>
#include <rspf/imaging/rspfAnnotationMultiPolyLineObject.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfPolyLine.h>

class rspfDatum;

class RSPF_DLL rspfGeoAnnotationMultiPolyLineObject : public rspfGeoAnnotationObject
{
public:
   rspfGeoAnnotationMultiPolyLineObject();
   rspfGeoAnnotationMultiPolyLineObject(const vector<rspfPolyLine>& multiPolyLine,
					 rspf_uint8 r=255,
					 rspf_uint8 g=255,
					 rspf_uint8 b=255,
					 rspf_uint8 thickness=1);
  
   rspfGeoAnnotationMultiPolyLineObject(const rspfGeoAnnotationMultiPolyLineObject& rhs);
   virtual rspfObject* dup()const;

   virtual void transform(rspfImageGeometry* projection);

   virtual void applyScale(double x, double y);
   virtual void draw(rspfRgbImage& anImage)const;
   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual rspfAnnotationObject* getNewClippedObject(const rspfDrect& rect)const;
   virtual std::ostream& print(std::ostream& out)const;
   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void addPoint(rspf_uint32 polygonIndex,
                         const rspfGpt& pt);
   
   virtual void setMultiPolyLine(const vector<rspfPolyLine>& multiPoly);

   virtual void computeBoundingRect();
   
   virtual bool isPointWithin(const rspfDpt& imagePoint)const;
   
   virtual void setColor(rspf_uint8 r,
                         rspf_uint8 g,
                         rspf_uint8 b);
   
   virtual void setThickness(rspf_uint8 thickness);
   
   void setDatum(const rspfDatum* datum);

   const rspfDatum* getDatum()const;

   const std::vector<rspfPolyLine>& getMultiPolyLine()const;
   
   std::vector<rspfPolyLine>& getMultiPolyLine();

   /**
    * Saves the current state of this object.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /**
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
protected:
   virtual ~rspfGeoAnnotationMultiPolyLineObject();
   
   std::vector<rspfPolyLine>          theMultiPolyLine;
   rspfDrect                          theBoundingRect;
   const rspfDatum*                   theDatum;
   rspfRefPtr<rspfAnnotationMultiPolyLineObject> theProjectedPolyLineObject;
   
   void allocateProjectedPolyLine();

TYPE_DATA
};

#endif
