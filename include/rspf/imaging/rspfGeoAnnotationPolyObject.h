//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfGeoAnnotationPolyObject.h 19734 2011-06-06 23:45:36Z dburken $
#ifndef rspfGeoAnnotationPolyObject_HEADER
#define rspfGeoAnnotationPolyObject_HEADER

#include <vector>
#include <rspf/base/rspfGpt.h>
#include <rspf/imaging/rspfGeoAnnotationObject.h>
#include <rspf/imaging/rspfAnnotationPolyObject.h>
 
class RSPFDLLEXPORT rspfGeoAnnotationPolyObject : public rspfGeoAnnotationObject
{
public:
   enum rspfPolyType
   {
     RSPF_POLY_EXTERIOR_RING  = 0,
     RSPF_POLY_INTERIOR_RING = 1
   };

   rspfGeoAnnotationPolyObject(bool enableFill = false,
                                rspf_uint8 r=255,
                                rspf_uint8 g=255,
                                rspf_uint8 b=255,
                                rspf_uint8 thickness=1);

   rspfGeoAnnotationPolyObject(const std::vector<rspfGpt>& groundPts,
                                bool enableFill = false,
                                rspf_uint8 r=255,
                                rspf_uint8 g=255,
                                rspf_uint8 b=255,
                                rspf_uint8 thickness=1);

   rspfGeoAnnotationPolyObject(const rspfGeoAnnotationPolyObject& rhs);


   virtual rspfObject* dup()const;

   virtual void applyScale(double x, double y);

   virtual void transform(rspfImageGeometry* projection);

   virtual std::ostream& print(std::ostream& out)const;

   virtual void draw(rspfRgbImage& anImage)const;

   virtual rspfAnnotationObject* getNewClippedObject(
      const rspfDrect& rect)const;

   virtual bool intersects(const rspfDrect& rect)const;
   
   virtual void getBoundingRect(rspfDrect& rect)const;

   virtual void computeBoundingRect();

   virtual const std::vector<rspfGpt>& getPolygon()const;
   virtual void setPolygon(const std::vector<rspfGpt>& poly);

   virtual void setFillFlag(bool flag);

   virtual void setColor(rspf_uint8 r,
                         rspf_uint8 g,
                         rspf_uint8 b);
   
   virtual void setThickness(rspf_uint8 thickness);

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

   virtual std::vector<rspfGpt> getPoints() { return thePolygon; }

   virtual rspfPolyType getPolyType () { return m_PolyType; }

   virtual void setPolyType(rspfPolyType polyType) { m_PolyType = polyType; }
   
protected:
   virtual ~rspfGeoAnnotationPolyObject();

   std::vector<rspfGpt>      thePolygon;
   rspfRefPtr<rspfAnnotationPolyObject> theProjectedPolyObject;
   rspfPolyType              m_PolyType;

TYPE_DATA
};

#endif
