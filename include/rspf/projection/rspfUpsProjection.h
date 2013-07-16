#ifndef rspfUpsProjection_HEADER
#define rspfUpsProjection_HEADER
#include <rspf/projection/rspfMapProjection.h>
class RSPFDLLEXPORT rspfUpsProjection : public rspfMapProjection
{
public:
   rspfUpsProjection(const rspfEllipsoid& ellipsoid = rspfEllipsoid(),
                      const rspfGpt& origin = rspfGpt());
   virtual rspfObject *dup()const{return new rspfUpsProjection(*this);}
   
   void setDefaults();
   virtual rspfGpt inverse(const rspfDpt &eastingNorthing)const;
   virtual rspfGpt inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const;
   virtual rspfDpt forward(const rspfGpt &latLon)const;
   virtual void update();
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
   double getFalseEasting()const{return UPS_False_Easting;}
   double getFalseNorthing()const{return UPS_False_Northing;}
   void setHemisphere(char hemisphere); 
   
   virtual bool operator==(const rspfProjection& projection) const;
protected:
   mutable char theHemisphere;
   mutable double UPS_a;          /* Semi-major axis of ellipsoid in meters   */
   mutable double UPS_f;  /* Flattening of ellipsoid  */
   mutable double UPS_Origin_Latitude;  /*set default = North Hemisphere */
   mutable double UPS_Origin_Longitude;
   mutable double false_easting;
   mutable double false_northing;
   mutable double UPS_Easting;
   mutable double UPS_Northing;
   double UPS_False_Easting;
   double UPS_False_Northing;
/*!
 * The function SET_UPS_PARAMETERS receives the ellipsoid parameters and sets
 * the corresponding state variables. If any errors occur, the error code(s)
 * are returned by the function, otherwise UPS_NO_ERROR is returned.
 *
 *   a     : Semi-major axis of ellipsoid in meters (input)
 *   f     : Flattening of ellipsoid                (input)
 */
   long Set_UPS_Parameters( double a,
                           double f);
/*!
 * The function Get_UPS_Parameters returns the current ellipsoid parameters.
 *
 *  a      : Semi-major axis of ellipsoid, in meters (output)
 *  f      : Flattening of ellipsoid                 (output)
 */
  void Get_UPS_Parameters( double *a,
                           double *f)const;
/*!
 *  The function Convert_Geodetic_To_UPS converts geodetic (latitude and
 *  longitude) coordinates to UPS (hemisphere, easting, and northing)
 *  coordinates, according to the current ellipsoid parameters. If any 
 *  errors occur, the error code(s) are returned by the function, 
 *  otherwide UPS_NO_ERROR is returned.
 *
 *    Latitude      : Latitude in radians                       (input)
 *    Longitude     : Longitude in radians                      (input)
 *    Hemisphere    : Hemisphere either 'N' or 'S'              (output)
 *    Easting       : Easting/X in meters                       (output)
 *    Northing      : Northing/Y in meters                      (output)
 */
  long Convert_Geodetic_To_UPS ( double Latitude,
                                 double Longitude,
                                 char   *Hemisphere,
                                 double *Easting,
                                 double *Northing)const;
/*!
 *  The function Convert_UPS_To_Geodetic converts UPS (hemisphere, easting, 
 *  and northing) coordinates to geodetic (latitude and longitude) coordinates
 *  according to the current ellipsoid parameters.  If any errors occur, the 
 *  error code(s) are returned by the function, otherwise UPS_NO_ERROR is 
 *  returned.
 *
 *    Hemisphere    : Hemisphere either 'N' or 'S'              (input)
 *    Easting       : Easting/X in meters                       (input)
 *    Northing      : Northing/Y in meters                      (input)
 *    Latitude      : Latitude in radians                       (output)
 *    Longitude     : Longitude in radians                      (output)
 */
  long Convert_UPS_To_Geodetic(char   Hemisphere,
                               double Easting,
                               double Northing,
                               double *Latitude,
                               double *Longitude)const;
TYPE_DATA  
};
#endif
