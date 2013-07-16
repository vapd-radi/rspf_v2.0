#ifndef rspfSpaceObliqueMercatorProjection_HEADER
#define rspfSpaceObliqueMercatorProjection_HEADER
#include <rspf/projection/rspfMapProjection.h>
/*!
 * This class implments the formulas for the ellipsoid and
 * circular orbit.  All formulas were directly taken from
 * proj 4.  Proj 4 implementation is from the following
 * manual reference:
 *
 * Map Projections - A Working Manual
 * by John Snyder
 * Space Oblique Mercator p. 221.
 *
 * 
 */
class rspfSpaceObliqueMercatorProjection : public rspfMapProjection
{
public:
   enum rspfSatelliteType
   {
      SOM_TYPE_LANDSAT_1 = 0,
      SOM_TYPE_LANDSAT_2 = 1,
      SOM_TYPE_LANDSAT_3 = 2,
      SOM_TYPE_LANDSAT_4 = 3, 
      SOM_TYPE_LANDSAT_5 = 4,
      SOM_TYPE_LANDSAT_7 = 5
   };
   
   rspfSpaceObliqueMercatorProjection(rspfSatelliteType type=SOM_TYPE_LANDSAT_7,
                                       double pathNumber=34,
                                       const rspfEllipsoid& ellipsoid = rspfEllipsoid());
   virtual rspfObject* dup()const{return new rspfSpaceObliqueMercatorProjection(*this);}
   virtual rspfDpt forward(const rspfGpt &worldPoint)    const;
   virtual rspfGpt inverse(const rspfDpt &projectedPoint)const;
    virtual rspfGpt inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const;
     /*!
    * Method to save the state of an object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;
   /*!
    * Method to the load (recreate) the state of an object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
  virtual void setParameters(rspfSatelliteType type,
                              double path);
   
   virtual void update();
   virtual bool operator==(const rspfProjection& projection) const;
protected:
   double a2, a4, b, c1, c3; 
   double q, t, u, w, p22, sa, ca, xj, rlm, rlm2;
   double lam0;
   double es;
   double e;
   double one_es;
   double rone_es;
   double a;
   
   void seraz0(double lam, double mult);
   
   /*!
    * Is the p variable in Map Projections a Working Manual.
    */
   double thePath;
   rspfSatelliteType theSatelliteType;
   
TYPE_DATA
};
#endif
