#ifndef rspfUtmpt_HEADER
#define rspfUtmpt_HEADER
#include <rspf/base/rspfConstants.h>
class rspfGpt;
class rspfEcefPoint;
class rspfUpspt;
class rspfDatum;
class RSPFDLLEXPORT rspfUtmpt
{
public:
   rspfUtmpt(const rspfGpt &aPt);
   rspfUtmpt(const rspfEcefPoint &aPt);
   rspfUtmpt(long         zone,
              char         hemisphere,  // N or S
              double       easting,
              double       northing,
              const rspfDatum* datum);
   
   /**
    * Return the zone for this utm point.
    */
   long zone() const { return theZone; }
   /**
    * Return the hemisphere ( Northern or Southern).
    * We store this as a char value N or S.  We store it
    * like this because we are interfacing to geotrans
    * and geotrans stores their values in this manner.
    */
   char hemisphere() const { return theHemisphere; }
   /**
    * return the easting value in meters.
    */
   double easting() const { return theEasting; }
   /**
    * return the northing value in meters.
    */
   double northing() const { return theNorthing; }
   /**
    * return the reference datum for this point.
    */
   const rspfDatum* datum() const { return theDatum; }
   /**
    * Initializes this point to utm coordinates for the ground point.
    *
    * @param aPt Ground point to initialize from.
    */
   void convertFromGround(const rspfGpt &aPt);
   /**
    * Initializes the ground point from the utm coordinates of this point.
    *
    * @param aPt Ground point to initialize.
    */
   void convertToGround(rspfGpt &aPt);
   
private:
   long               theZone;
   char               theHemisphere;
   double             theEasting;
   double             theNorthing;
   const rspfDatum*  theDatum;
};
#endif
