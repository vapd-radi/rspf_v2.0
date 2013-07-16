#include <rspf/projection/rspfGnomonicProjection.h>
#include <rspf/base/rspfKeywordNames.h>
RTTI_DEF1(rspfGnomonicProjection, "rspfGnomonicProjection", rspfMapProjection)
#ifndef PI_OVER_2
#  define PI_OVER_2  ( M_PI / 2.0)
#endif
#ifndef TWO_PI
#  define TWO_PI     (2.0 * M_PI)
#endif
#define GNOM_NO_ERROR           0x0000
#define GNOM_LAT_ERROR          0x0001
#define GNOM_LON_ERROR          0x0002
#define GNOM_EASTING_ERROR      0x0004
#define GNOM_NORTHING_ERROR     0x0008
#define GNOM_ORIGIN_LAT_ERROR   0x0010
#define GNOM_CENT_MER_ERROR     0x0020
#define GNOM_A_ERROR            0x0040
#define GNOM_INV_F_ERROR        0x0080
rspfGnomonicProjection::rspfGnomonicProjection(const rspfEllipsoid& ellipsoid,
                                                 const rspfGpt& origin)
   :rspfMapProjection(ellipsoid, origin)
{
   setDefaults();
   update();
}
rspfGnomonicProjection::rspfGnomonicProjection(const rspfEllipsoid& ellipsoid,
                                                 const rspfGpt& origin,
                                                 double falseEasting,
                                                 double falseNorthing)
   :rspfMapProjection(ellipsoid, origin)
{
   Gnom_False_Easting  = falseEasting;
   Gnom_False_Northing = falseNorthing;
   
   update();
}
void rspfGnomonicProjection::update()
{   
   Set_Gnomonic_Parameters(theEllipsoid.getA(),
                           theEllipsoid.getFlattening(),
                           theOrigin.latr(),
                           theOrigin.lonr(),
                           Gnom_False_Easting,
                           Gnom_False_Northing);
   theFalseEastingNorthing.x = Gnom_False_Easting;
   theFalseEastingNorthing.y = Gnom_False_Northing;
   rspfMapProjection::update();
}
void rspfGnomonicProjection::setFalseEasting(double falseEasting)
{
   Gnom_False_Easting = falseEasting;
   
   update();
}
void rspfGnomonicProjection::setFalseNorthing(double falseNorthing)
{
   Gnom_False_Northing = falseNorthing;
   
   update();
}
void rspfGnomonicProjection::setDefaults()
{
   Gnom_False_Easting  = 0.0;
   Gnom_False_Northing = 0.0;
   Gnom_Delta_Northing = 40000000;
   Gnom_Delta_Easting  =  40000000;
}
void rspfGnomonicProjection::setFalseEastingNorthing(double falseEasting,
                                                           double falseNorthing)
{
   Gnom_False_Easting  = falseEasting;
   Gnom_False_Northing = falseNorthing;
   
   update();
}
rspfGpt rspfGnomonicProjection::inverse(const rspfDpt &eastingNorthing)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Gnomonic_To_Geodetic(eastingNorthing.x,
                                eastingNorthing.y,
                                &lat,
                                &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}
rspfGpt rspfGnomonicProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Gnomonic_To_Geodetic(eastingNorthing.x,
                                eastingNorthing.y,
                                &lat,
                                &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}

rspfDpt rspfGnomonicProjection::forward(const rspfGpt &latLon)const
{
   double easting  = 0.0;
   double northing = 0.0;
   rspfGpt gpt = latLon;
   
   if (theDatum)
   {
      if (theDatum->code() != latLon.datum()->code())
      {
         gpt.changeDatum(theDatum); // Shift to our datum.
      }
   }
   Convert_Geodetic_To_Gnomonic(gpt.latr(),
                                gpt.lonr(),
                                &easting,
                                &northing);
   
   return rspfDpt(easting, northing);
}
bool rspfGnomonicProjection::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   return rspfMapProjection::saveState(kwl, prefix);   
}
bool rspfGnomonicProjection::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   bool flag = rspfMapProjection::loadState(kwl, prefix);
   
   const char* type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   
   setDefaults();
   if(rspfString(type) == STATIC_TYPE_NAME(rspfGnomonicProjection))
   {
      Gnom_False_Easting  = theFalseEastingNorthing.x;
      Gnom_False_Northing = theFalseEastingNorthing.y;
   }
   
   update();
   return flag;
}
/***************************************************************************/
/*
 *                              FUNCTIONS     
 */
long rspfGnomonicProjection::Set_Gnomonic_Parameters(double a,
                                                      double f,
                                                      double Origin_Latitude,
                                                      double Central_Meridian,
                                                      double False_Easting,
                                                      double False_Northing)
{ /* BEGIN Set_Gnomonic_Parameters */
/*
 * The function Set_Gnomonic_Parameters receives the ellipsoid parameters and
 * projection parameters as inputs, and sets the corresponding state
 * variables.  If any errors occur, the error code(s) are returned by the function, 
 * otherwise GNOM_NO_ERROR is returned.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (input)
 *    f                 : Flattening of ellipsoid						        (input)
 *    Origin_Latitude   : Latitude in radians at which the          (input)
 *                          point scale factor is 1.0
 *    Central_Meridian  : Longitude in radians at the center of     (input)
 *                          the projection
 *    False_Easting     : A coordinate value in meters assigned to the
 *                          central meridian of the projection.     (input)
 *    False_Northing    : A coordinate value in meters assigned to the
 *                          origin latitude of the projection       (input)
 */
  double es2, es4, es6;
  long Error_Code = GNOM_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    Gnom_a = a;
    Gnom_f = f;
    es2 = 2 * Gnom_f - Gnom_f * Gnom_f;
    es4 = es2 * es2;
    es6 = es4 * es2;
    /* spherical radius */
    Ra = Gnom_a * (1.0 - es2 / 6.0 - 17.0 * es4 / 360.0 - 67.0 * es6 / 3024.0);
    Gnom_Origin_Lat = Origin_Latitude;
    Sin_Gnom_Origin_Lat = sin(Gnom_Origin_Lat);
    Cos_Gnom_Origin_Lat = cos(Gnom_Origin_Lat);
    abs_Gnom_Origin_Lat = fabs(Gnom_Origin_Lat);
    Gnom_Origin_Long = Central_Meridian;
    Gnom_False_Northing = False_Northing;
    Gnom_False_Easting = False_Easting;
  } /* End if(!Error_Code) */
  return (Error_Code);
} /* End Set_Gnomonic_Parameters */
void rspfGnomonicProjection::Get_Gnomonic_Parameters(double *a,
                                                      double *f,
                                                      double *Origin_Latitude,
                                                      double *Central_Meridian,
                                                      double *False_Easting,
                                                      double *False_Northing)const
{ /* Begin Get_Gnomonic_Parameters */
/*
 * The function Get_Gnomonic_Parameters returns the current ellipsoid
 * parameters and Gnomonic projection parameters.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (output)
 *    f                 : Flattening of ellipsoid						        (output)
 *    Origin_Latitude   : Latitude in radians at which the          (output)
 *                          point scale factor is 1.0
 *    Central_Meridian  : Longitude in radians at the center of     (output)
 *                          the projection
 *    False_Easting     : A coordinate value in meters assigned to the
 *                          central meridian of the projection.     (output)
 *    False_Northing    : A coordinate value in meters assigned to the
 *                          origin latitude of the projection       (output)
 */
  *a = Gnom_a;
  *f = Gnom_f;
  *Origin_Latitude = Gnom_Origin_Lat;
  *Central_Meridian = Gnom_Origin_Long;
  *False_Easting = Gnom_False_Easting;
  *False_Northing = Gnom_False_Northing;
  
  return;
} /* End Get_Gnomonic_Parameters */
long rspfGnomonicProjection::Convert_Geodetic_To_Gnomonic (double Latitude,
                                                            double Longitude,
                                                            double *Easting,
                                                            double *Northing)const
{ /* Begin Convert_Geodetic_To_Gnomonic */
/*
 * The function Convert_Geodetic_To_Gnomonic converts geodetic (latitude and
 * longitude) coordinates to Gnomonic projection (easting and northing)
 * coordinates, according to the current ellipsoid and Gnomonic projection
 * parameters.  If any errors occur, the error code(s) are returned by the
 * function, otherwise GNOM_NO_ERROR is returned.
 *
 *    Latitude          : Latitude (phi) in radians           (input)
 *    Longitude         : Longitude (lambda) in radians       (input)
 *    Easting           : Easting (X) in meters               (output)
 *    Northing          : Northing (Y) in meters              (output)
 */
  double dlam;       /* Longitude - Central Meridan */
  double cos_c;      
  double k_prime;    /* scale factor */
  double Ra_kprime;
  double slat = sin(Latitude);
  double clat = cos(Latitude);
  double Ra_cotlat;
  double sin_dlam, cos_dlam;
  double temp_Easting, temp_Northing;
  long Error_Code = GNOM_NO_ERROR;
  dlam = Longitude - Gnom_Origin_Long;
  sin_dlam = sin(dlam);
  cos_dlam = cos(dlam);
  cos_c = Sin_Gnom_Origin_Lat * slat + Cos_Gnom_Origin_Lat * clat * cos_dlam;
  if (cos_c <= 1.0e-10)
  {  /* Point is out of view.  Will return longitude out of range message
    since no point out of view is implemented.  */
    Error_Code |= GNOM_LON_ERROR;
  }
  if (!Error_Code)
  { /* no errors */
    if (fabs(abs_Gnom_Origin_Lat - PI_OVER_2) < 1.0e-10)
    {
      Ra_cotlat = Ra * (clat / slat);
      temp_Easting = Ra_cotlat * sin_dlam;
      temp_Northing = Ra_cotlat * cos_dlam;
      if (Gnom_Origin_Lat >= 0.0)
      {
        *Easting = temp_Easting + Gnom_False_Easting;
        *Northing = -1.0 * temp_Northing + Gnom_False_Northing;
      }
      else
      {
        *Easting = -1.0 * temp_Easting + Gnom_False_Easting;
        *Northing = temp_Northing + Gnom_False_Northing;
      }
    }
    else if (abs_Gnom_Origin_Lat <= 1.0e-10)
    {
      *Easting = Ra * tan(dlam) + Gnom_False_Easting;
      *Northing = Ra * tan(Latitude) / cos_dlam + Gnom_False_Northing;
    }
    else
    {
      k_prime = 1 / cos_c;
      Ra_kprime = Ra * k_prime;
      *Easting = Ra_kprime * clat * sin_dlam + Gnom_False_Easting;
      *Northing = Ra_kprime * (Cos_Gnom_Origin_Lat * slat - Sin_Gnom_Origin_Lat * clat * cos_dlam) + Gnom_False_Northing;
    }
  }
  return (Error_Code);
} /* End Convert_Geodetic_To_Gnomonic */
long rspfGnomonicProjection::Convert_Gnomonic_To_Geodetic(double Easting,
                                                           double Northing,
                                                           double *Latitude,
                                                           double *Longitude)const
{ /* Begin Convert_Gnomonic_To_Geodetic */
/*
 * The function Convert_Gnomonic_To_Geodetic converts Gnomonic projection
 * (easting and northing) coordinates to geodetic (latitude and longitude)
 * coordinates, according to the current ellipsoid and Gnomonic projection
 * coordinates.  If any errors occur, the error code(s) are returned by the
 * function, otherwise GNOM_NO_ERROR is returned.
 *
 *    Easting           : Easting (X) in meters                  (input)
 *    Northing          : Northing (Y) in meters                 (input)
 *    Latitude          : Latitude (phi) in radians              (output)
 *    Longitude         : Longitude (lambda) in radians          (output)
 */
  double dx, dy;
  double rho;
  double c;
  double sin_c, cos_c;
  double dy_sinc;
  long Error_Code = GNOM_NO_ERROR; 
  if (!Error_Code)
  {
    dy = Northing - Gnom_False_Northing;
    dx = Easting - Gnom_False_Easting;
    rho = sqrt(dx * dx + dy * dy);
    if (fabs(rho) <= 1.0e-10)
    {
      *Latitude = Gnom_Origin_Lat;
      *Longitude = Gnom_Origin_Long;
    }
    else
    {
      c = atan(rho / Ra);
      sin_c = sin(c);
      cos_c = cos(c);
      dy_sinc = dy * sin_c;
      *Latitude = asin((cos_c * Sin_Gnom_Origin_Lat) + ((dy_sinc * Cos_Gnom_Origin_Lat) / rho));
      if (fabs(abs_Gnom_Origin_Lat - PI_OVER_2) < 1.0e-10)
      {
        if (Gnom_Origin_Lat >= 0.0)
          *Longitude = Gnom_Origin_Long + atan2(dx, -dy);
        else
          *Longitude = Gnom_Origin_Long + atan2(dx, dy);
      }
      else
        *Longitude = Gnom_Origin_Long + atan2((dx * sin_c), (rho * Cos_Gnom_Origin_Lat * cos_c - dy_sinc * Sin_Gnom_Origin_Lat));
    }
  }
  return (Error_Code);
} /* End Convert_Gnomonic_To_Geodetic */
