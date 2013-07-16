#ifndef rspfGeographicAnnotationGrid_HEADER
#define rspfGeographicAnnotationGrid_HEADER
#include <rspf/imaging/rspfAnnotationObject.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfGrect.h>
#include <rspf/base/rspfDrect.h>

class rspfMapProjection;

class rspfGeographicAnnotationGrid : public rspfAnnotationObject
{
public:
   rspfGeographicAnnotationGrid();

   virtual void draw(rspfRgbImage& anImage)const;

   /*!
    * We need to know what projection the view
    * and we need to know the bounding rect for
    * the view in geographic space.  The bounding
    * rect is there for an area of interest.
    */
   virtual void setViewProjectionInformation(rspfMapProjection* projection,
                                             const rspfGrect& boundingGroundRect);

   virtual void getBoundingRect(rspfDrect& rect)const;
   virtual void computeBoundingRect();
   virtual std::ostream& print(std::ostream& out)const;

protected:
   /*!
    * Holds the view projection.
    */
   rspfRefPtr<rspfMapProjection> theViewProjection;

   rspfGrect       theGroundRect;

   /*!
    * This will be computed based on the bounding
    * ground and projection.
    */
   rspfDrect       theBoundingRect;
   
   /*!
    * Will hold the spacing of the grid cells in
    * geographic lat lon change.
    */
   double theDeltaLatSpacing;
   double theDeltaLonSpacing;
};

#endif
