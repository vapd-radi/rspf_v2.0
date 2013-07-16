#include <math.h>
#include <rspf/projection/rspfMollweidProjection.h>
#include <rspf/base/rspfKeywordNames.h>
RTTI_DEF1(rspfMollweidProjection, "rspfMollweidProjection", rspfMapProjection)
/***************************************************************************/
/*
 *                              DEFINES
 */
#ifndef PI_OVER_2
#  define PI_OVER_2  ( M_PI / 2.0)
#endif
#ifndef TWO_PI
#  define TWO_PI     (2.0 * M_PI)
#endif
#define MAX_LAT    ( (M_PI * 90.0) / 180.0 ) /* 90 degrees in radians    */
#define MOLL_NO_ERROR           0x0000
#define MOLL_LAT_ERROR          0x0001
#define MOLL_LON_ERROR          0x0002
#define MOLL_EASTING_ERROR      0x0004
#define MOLL_NORTHING_ERROR     0x0008
#define MOLL_CENT_MER_ERROR     0x0020
#define MOLL_A_ERROR            0x0040
#define MOLL_B_ERROR            0x0080
#define MOLL_A_LESS_B_ERROR     0x0100
rspfMollweidProjection::rspfMollweidProjection(const rspfEllipsoid& ellipsoid,
                                                 const rspfGpt& origin)
   :rspfMapProjection(ellipsoid, origin)
{
   setDefaults();
   update();
}
rspfMollweidProjection::rspfMollweidProjection(const rspfEllipsoid& ellipsoid,
                                                 const rspfGpt& origin,
                                                 double falseEasting,
                                                 double falseNorthing)
   :rspfMapProjection(ellipsoid, origin)
{
   Moll_False_Easting  = falseEasting;
   Moll_False_Northing = falseNorthing;
   Moll_Delta_Northing = 9009965.0;
   Moll_Max_Easting    = 18019930.0;
   Moll_Min_Easting    = -18019930.0;
   update();
}
void rspfMollweidProjection::update()
{   
   Set_Mollweide_Parameters(theEllipsoid.getA(),
                            theEllipsoid.getFlattening(),
                            theOrigin.lonr(),
                            Moll_False_Easting,
                            Moll_False_Northing);
   theFalseEastingNorthing.x = Moll_False_Easting;
   theFalseEastingNorthing.y = Moll_False_Northing;
   rspfMapProjection::update();
}
void rspfMollweidProjection::setFalseEasting(double falseEasting)
{
   Moll_False_Easting = falseEasting;
   update();
}
void rspfMollweidProjection::setFalseNorthing(double falseNorthing)
{
   Moll_False_Northing = falseNorthing;
   update();
}
void rspfMollweidProjection::setDefaults()
{
   Moll_False_Easting  = 0.0;
   Moll_False_Northing = 0.0;
   Moll_Delta_Northing = 9009965.0;
   Moll_Max_Easting    = 18019930.0;
   Moll_Min_Easting    = -18019930.0;
}
void rspfMollweidProjection::setFalseEastingNorthing(double falseEasting,
                                                      double falseNorthing)
{
   Moll_False_Easting  = falseEasting;
   Moll_False_Northing = falseNorthing;
   
   update();
}
void rspfMollweidProjection::setCentralMeridian(double centralMeridian)
{
  Moll_Origin_Long = centralMeridian;
  update();
}
rspfGpt rspfMollweidProjection::inverse(const rspfDpt &eastingNorthing)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Mollweide_To_Geodetic(eastingNorthing.x,
                                 eastingNorthing.y,
                                 &lat,
                                 &lon);
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}
rspfGpt rspfMollweidProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Mollweide_To_Geodetic(eastingNorthing.x,
                                 eastingNorthing.y,
                                 &lat,
                                 &lon);
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}

rspfDpt rspfMollweidProjection::forward(const rspfGpt &latLon)const
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
   Convert_Geodetic_To_Mollweide(gpt.latr(),
                                 gpt.lonr(),
                                 &easting,
                                 &northing);
   
   return rspfDpt(easting, northing);
}
bool rspfMollweidProjection::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   return rspfMapProjection::saveState(kwl, prefix);
}
bool rspfMollweidProjection::loadState(const rspfKeywordlist& kwl,
                                        const char* prefix)
{
   bool flag = rspfMapProjection::loadState(kwl, prefix);
   const char* type          = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   setDefaults();
   if(rspfString(type) == STATIC_TYPE_NAME(rspfMollweidProjection))
   {
      Moll_False_Easting  = theFalseEastingNorthing.x;
      Moll_False_Northing = theFalseEastingNorthing.y;
   }
   update();
   return flag;   
}
/***************************************************************************/
/*
 *                              FUNCTIONS
 */
long rspfMollweidProjection::Set_Mollweide_Parameters(double a,
                                                       double f,
                                                       double Central_Meridian,
                                                       double False_Easting,
                                                       double False_Northing)
{ /* Begin Set_Mollweide_Parameters */
/*
 * The function Set_Mollweide_Parameters receives the ellipsoid parameters and
 * Mollweide projcetion parameters as inputs, and sets the corresponding state
 * variables.  If any errors occur, the error code(s) are returned by the 
 * function, otherwise MOLL_NO_ERROR is returned.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (input)
 *    f                 : Flattening of ellipsoid						        (input)
 *    Central_Meridian  : Longitude in radians at the center of     (input)
 *                          the projection
 *    False_Easting     : A coordinate value in meters assigned to the
 *                          central meridian of the projection.     (input)
 *    False_Northing    : A coordinate value in meters assigned to the
 *                          origin latitude of the projection       (input)
 */
  double Ra;                       /* Spherical Radius */
  long Error_Code = MOLL_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    Moll_a = a;
    Moll_f = f;
    es2 = 2 * Moll_f - Moll_f * Moll_f;
    es4 = es2 * es2;
    es6 = es4 * es2;
    /* spherical radius */
    Ra = Moll_a * (1.0 - es2 / 6.0 - 17.0 * es4 / 360.0 - 67.0 * es6 / 3024.0);
    Sqrt2_Ra = sqrt(2.0) * Ra;
    Sqrt8_Ra = sqrt(8.0) * Ra;
    Moll_Origin_Long = Central_Meridian;
    Moll_False_Easting = False_Easting;
    Moll_False_Northing = False_Northing;
    if (Moll_Origin_Long > 0)
    {
      Moll_Max_Easting = 17919819.0;
      Moll_Min_Easting = -18019930.0;
    }
    else if (Moll_Origin_Long < 0)
    {
      Moll_Max_Easting = 18019930.0;
      Moll_Min_Easting = -17919819.0;
    }
    else
    {
      Moll_Max_Easting = 18019930.0;
      Moll_Min_Easting = -18019930.0;
    }
  } /* End if(!Error_Code) */
  return (Error_Code);
} /* End Set_Mollweide_Parameters */
void rspfMollweidProjection::Get_Mollweide_Parameters(double *a,
                                                       double *f,
                                                       double *Central_Meridian,
                                                       double *False_Easting,
                                                       double *False_Northing)const
{ /* Begin Get_Mollweide_Parameters */
/*
 * The function Get_Mollweide_Parameters returns the current ellipsoid
 * parameters and Mollweide projection parameters.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (output)
 *    f                 : Flattening of ellipsoid						        (output)
 *    Central_Meridian  : Longitude in radians at the center of     (output)
 *                          the projection
 *    False_Easting     : A coordinate value in meters assigned to the
 *                          central meridian of the projection.     (output)
 *    False_Northing    : A coordinate value in meters assigned to the
 *                          origin latitude of the projection       (output)
 */
  *a = Moll_a;
  *f = Moll_f;
  *Central_Meridian = Moll_Origin_Long;
  *False_Easting = Moll_False_Easting;
  *False_Northing = Moll_False_Northing;
  
  return;
} /* End Get_Mollweide_Parameters */
long rspfMollweidProjection::Convert_Geodetic_To_Mollweide (double Latitude,
                                                             double Longitude,
                                                             double *Easting,
                                                             double *Northing)const
{ /* Begin Convert_Geodetic_To_Mollweide */
/*
 * The function Convert_Geodetic_To_Mollweide converts geodetic (latitude and
 * longitude) coordinates to Mollweide projection (easting and northing)
 * coordinates, according to the current ellipsoid and Mollweide projection
 * parameters.  If any errors occur, the error code(s) are returned by the
 * function, otherwise MOLL_NO_ERROR is returned.
 *
 *    Latitude          : Latitude (phi) in radians           (input)
 *    Longitude         : Longitude (lambda) in radians       (input)
 *    Easting           : Easting (X) in meters               (output)
 *    Northing          : Northing (Y) in meters              (output)
 */
  double PI_Sin_Latitude = M_PI * sin(Latitude);
  double dlam;                                  /* Longitude - Central Meridan */
  double theta;
  double theta_primed = Latitude;
  double delta_theta_primed = 0.1745329;        /* arbitrarily initialized to 10 deg */
  double dtp_tolerance = 4.85e-10;              /* approximately 1/1000th of
                                                 an arc second or 1/10th meter */
  long   Error_Code = MOLL_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    dlam = Longitude - Moll_Origin_Long;
    while (fabs(delta_theta_primed) > dtp_tolerance)
    {
      delta_theta_primed = -(theta_primed + sin(theta_primed) -
                             PI_Sin_Latitude) / (1.0 + cos(theta_primed));
      theta_primed += delta_theta_primed;
    }
    theta = theta_primed / 2.0;
    *Easting = (Sqrt8_Ra / M_PI ) * dlam * cos(theta) +
               Moll_False_Easting;
    *Northing = Sqrt2_Ra * sin(theta) + Moll_False_Northing;
  }
  return (Error_Code);
} /* End Convert_Geodetic_To_Mollweide */
long rspfMollweidProjection::Convert_Mollweide_To_Geodetic(double Easting,
                                                            double Northing,
                                                            double *Latitude,
                                                            double *Longitude)const
{ /* Begin Convert_Mollweide_To_Geodetic */
/*
 * The function Convert_Mollweide_To_Geodetic converts Mollweide projection
 * (easting and northing) coordinates to geodetic (latitude and longitude)
 * coordinates, according to the current ellipsoid and Mollweide projection
 * coordinates.  If any errors occur, the error code(s) are returned by the
 * function, otherwise MOLL_NO_ERROR is returned.
 *
 *    Easting           : Easting (X) in meters                  (input)
 *    Northing          : Northing (Y) in meters                 (input)
 *    Latitude          : Latitude (phi) in radians              (output)
 *    Longitude         : Longitude (lambda) in radians          (output)
 */
  double dx, dy;
  double theta = 0.0;
  double two_theta;
  double i;
  long Error_Code = MOLL_NO_ERROR;
  if (!Error_Code)
  {
    dy = Northing - Moll_False_Northing;
    dx = Easting - Moll_False_Easting;
    i = dy / Sqrt2_Ra;
    if (fabs(i) > 1.0)
    {
      *Latitude = MAX_LAT;
      if (Northing < 0.0)
        *Latitude *= -1.0;
    }
    else
    {
      theta = asin(i);
      two_theta = 2.0 * theta;
      *Latitude = asin((two_theta + sin(two_theta)) / M_PI);
      if (*Latitude > PI_OVER_2)  /* force distorted values to 90, -90 degrees */
        *Latitude = PI_OVER_2;
      else if (*Latitude < -PI_OVER_2)
        *Latitude = -PI_OVER_2;
    }
    if (fabs(fabs(*Latitude) - MAX_LAT) < 1.0e-10)
      *Longitude = Moll_Origin_Long;
    else
      *Longitude = Moll_Origin_Long + M_PI * dx /
                   (Sqrt8_Ra * cos(theta));
  }
  return (Error_Code);
} /* End Convert_Mollweide_To_Geodetic */
bool rspfMollweidProjection::operator==(const rspfProjection& proj) const
{
   if (!rspfMapProjection::operator==(proj))
      return false;
   rspfMollweidProjection* p = PTR_CAST(rspfMollweidProjection, &proj);
   if (!p) return false;
   if (!rspf::almostEqual(Moll_Origin_Long,p->Moll_Origin_Long)) return false;
   return true;
}
