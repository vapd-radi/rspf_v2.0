//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class declaration for rspfLlxy.  This is a simple "latitude /
// longitude to x / y" projection.  Note that "origin" and "tie_point" are
// synonymous in this projection.
// 
//*******************************************************************
//  $Id: rspfLlxyProjection.h 19651 2011-05-25 18:49:22Z gpotts $

#ifndef rspfLlxyProjection_HEADER
#define rspfLlxyProjection_HEADER

#include <rspf/projection/rspfMapProjection.h>

class RSPFDLLEXPORT rspfLlxyProjection : public rspfMapProjection
{
public:
   rspfLlxyProjection();
   rspfLlxyProjection(const rspfLlxyProjection& rhs);
   rspfLlxyProjection(const rspfEllipsoid& ellipsoid,
                       const rspfGpt& origin);
   rspfLlxyProjection(const rspfGpt& origin,
                       double latSpacing,  // decimal degrees
                       double lonSpacing);
   
   virtual ~rspfLlxyProjection();
   
   virtual rspfObject *dup()const;
   
   virtual bool isGeographic()const;
   
   virtual rspfDpt forward(const rspfGpt &worldPoint) const;
   virtual rspfGpt inverse(const rspfDpt &projectedPoint) const;
   
   /*!
    *  METHOD:  getLatSpacing() 
    *  Returns the latitude spacing as a double representing decimal degrees.
    */
   virtual double getLatSpacing() const;

   /*!
    *  METHOD:  getLonSpacing() 
    *  Returns the longitude spacing as a double representing decimal degrees.
    */
   virtual double getLonSpacing() const;

   /*!
    *  METHOD:  setLatSpacing(double spacing)
    *  Set the latitude spacing as a double representing decimal degrees.
    */
   virtual void   setLatSpacing(double spacing);

    /*!
    *  METHOD:  setLonSpacing(double spacing)
    *  Set the latitude spacing as a double representing decimal degrees.
    */
   virtual void   setLonSpacing(double spacing);

   /*!
    *  METHOD: worldToLineSample()
    * Performs the forward projection from ground point to line, sample.
    */
   virtual void worldToLineSample(const rspfGpt& worldPoint,
                                  rspfDpt&       lineSampPt) const;

   /*!
    *  METHOD: lineSampleToWorld()
    * Performs the inverse projection from line, sample to ground (world):
    */
   virtual void lineSampleToWorld(const rspfDpt& lineSampPt,
                                  rspfGpt&       worldPt) const;

   
   /*!
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);

   virtual std::ostream& print(std::ostream& out) const;

   virtual bool operator==(const rspfProjection& projection) const;

  //   virtual rspfDpt getMetersPerPixel() const;
   virtual void setMetersPerPixel(const rspfDpt& pt);
   /*!
    * This will go from the ground point and give
    * you an approximate lat and lon per pixel. the Delta Lat
    * and delta lon will be in degrees.
    */
   virtual void computeDegreesPerPixel(const rspfGpt& ground,
                                       const rspfDpt& metersPerPixel,
                                       double &deltaLat,
                                       double &deltaLon);

   
   /*!
    * This will go from the ground point and give
    * you an approximate meters per pixel. the Delta Lat
    * and delta lon will be in degrees.
    */
   virtual void computeMetersPerPixel(const rspfGpt& center,
                                      double deltaDegreesPerPixelLat,
                                      double deltaDegreesPerPixelLon,
                                      rspfDpt &metersPerPixel);
   
private:

TYPE_DATA
};

#endif
