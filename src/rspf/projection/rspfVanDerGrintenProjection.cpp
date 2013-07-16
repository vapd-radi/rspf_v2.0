#include <rspf/projection/rspfVanDerGrintenProjection.h>
#include <rspf/base/rspfKeywordNames.h>
RTTI_DEF1(rspfVanDerGrintenProjection, "rspfVanDerGrintenProjection", rspfMapProjection)
   
/***************************************************************************/
/*
 *                               DEFINES
 */
#ifndef PI_OVER_2
#  define PI_OVER_2  ( M_PI / 2.0)
#endif
#ifndef TWO_PI
#  define TWO_PI     (2.0 * M_PI)
#endif
#define MAX_LAT     ( 90.0 * (M_PI / 180.0) )  /* 90 degrees in radians   */
#define FLOAT_EQ(x,v,epsilon)   (((v - epsilon) < x) && (x < (v + epsilon)))
#define GRIN_NO_ERROR           0x0000
#define GRIN_LAT_ERROR          0x0001
#define GRIN_LON_ERROR          0x0002
#define GRIN_EASTING_ERROR      0x0004
#define GRIN_NORTHING_ERROR     0x0008
#define GRIN_CENT_MER_ERROR     0x0020
#define GRIN_A_ERROR            0x0040
#define GRIN_B_ERROR            0x0080
#define GRIN_A_LESS_B_ERROR     0x0100
#define GRIN_RADIUS_ERROR		  0x0200
/***************************************************************************/
/*
 *                               GLOBALS
 */
const double TWO_OVER_PI = (2.0 / M_PI);
const double PI_OVER_3 = (M_PI / 3.0);
const double ONE_THIRD  = (1.0 / 3.0);
rspfVanDerGrintenProjection::rspfVanDerGrintenProjection(const rspfEllipsoid& ellipsoid,
                                                           const rspfGpt& origin)
   :rspfMapProjection(ellipsoid, origin)
{
   setDefaults();
   update();
}
rspfVanDerGrintenProjection::rspfVanDerGrintenProjection(const rspfEllipsoid& ellipsoid,
                                                   const rspfGpt& origin,
                                                   double falseEasting,
                                                   double falseNorthing)
   :rspfMapProjection(ellipsoid, origin)
{
   Grin_False_Easting  = falseEasting;
   Grin_False_Northing = falseNorthing;
   
   update();
}
void rspfVanDerGrintenProjection::update()
{   
   Set_Van_der_Grinten_Parameters(theEllipsoid.getA(),
                                  theEllipsoid.getFlattening(),
                                  theOrigin.lonr(),
                                  Grin_False_Easting,
                                  Grin_False_Northing);
   theFalseEastingNorthing.x = Grin_False_Easting;
   theFalseEastingNorthing.y = Grin_False_Northing;
   rspfMapProjection::update();
}
void rspfVanDerGrintenProjection::setFalseEasting(double falseEasting)
{
   Grin_False_Easting = falseEasting;
   
   update();
}
void rspfVanDerGrintenProjection::setFalseNorthing(double falseNorthing)
{
   Grin_False_Northing = falseNorthing;
   
   update();
}
void rspfVanDerGrintenProjection::setDefaults()
{
   Grin_False_Easting  = 0.0;
   Grin_False_Northing = 0.0;
}
void rspfVanDerGrintenProjection::setCentralMeridian(double centralMeridian)
{
  Grin_Origin_Long = centralMeridian;
  update();
}
void rspfVanDerGrintenProjection::setFalseEastingNorthing(double falseEasting,
                                                           double falseNorthing)
{
   Grin_False_Easting  = falseEasting;
   Grin_False_Northing = falseNorthing;
   
   update();
}
rspfGpt rspfVanDerGrintenProjection::inverse(const rspfDpt &eastingNorthing)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Van_der_Grinten_To_Geodetic(eastingNorthing.x,
                                       eastingNorthing.y,
                                       &lat,
                                       &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}
rspfGpt rspfVanDerGrintenProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Van_der_Grinten_To_Geodetic(eastingNorthing.x,
                                       eastingNorthing.y,
                                       &lat,
                                       &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}

rspfDpt rspfVanDerGrintenProjection::forward(const rspfGpt &latLon)const
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
   Convert_Geodetic_To_Van_der_Grinten(gpt.latr(),
                                       gpt.lonr(),
                                       &easting,
                                       &northing);
   return rspfDpt(easting, northing);
}
bool rspfVanDerGrintenProjection::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   return rspfMapProjection::saveState(kwl, prefix);   
}
bool rspfVanDerGrintenProjection::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   bool flag = rspfMapProjection::loadState(kwl, prefix);
   const char* type          = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   
   setDefaults();
   if(rspfString(type) == STATIC_TYPE_NAME(rspfVanDerGrintenProjection))
   {
      Grin_False_Easting  = theFalseEastingNorthing.x;
      Grin_False_Northing = theFalseEastingNorthing.y;
   }
   
   update();
   return flag;
}
/***************************************************************************/
/*
 *                              FUNCTIONS
 */
long rspfVanDerGrintenProjection::Set_Van_der_Grinten_Parameters(double a,
                                                                  double f,
                                                                  double Central_Meridian,
                                                                  double False_Easting,
                                                                  double False_Northing)
{ /* BEGIN Set_Van_der_Grinten_Parameters */
/*
 * The function Set_Van_der_Grinten_Parameters receives the ellipsoid parameters and
 * projection parameters as inputs, and sets the corresponding state
 * variables.  If any errors occur, the error code(s) are returned by the function, 
 * otherwise Grin_NO_ERROR is returned.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (input)
 *    f                 : Flattening of ellipsoid							      (input)
 *    Central_Meridian  : Longitude in radians at the center of     (input)
 *                          the projection
 *    False_Easting     : A coordinate value in meters assigned to the
 *                          central meridian of the projection.     (input)
 *    False_Northing    : A coordinate value in meters assigned to the
 *                          origin latitude of the projection       (input)
 */
  long Error_Code = GRIN_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    Grin_a = a;
    Grin_f = f;
    es2 = 2 * Grin_f - Grin_f * Grin_f;
    es4 = es2 * es2;
    es6 = es4 * es2;
    /* spherical radius */
    Ra = Grin_a * (1.0 - es2 / 6.0 - 17.0 * es4 / 360.0 - 67.0 * es6 /3024.0);
    PI_Ra = M_PI * Ra;
    Grin_Origin_Long    = Central_Meridian;
    Grin_False_Easting  = False_Easting;
    Grin_False_Northing = False_Northing;
  } /* END OF if(!Error_Code) */
  return (Error_Code);
} /* END OF Set_Van_der_Grinten_Parameters */
void rspfVanDerGrintenProjection::Get_Van_der_Grinten_Parameters(double *a,
                                                                  double *f,
                                                                  double *Central_Meridian,
                                                                  double *False_Easting,
                                                                  double *False_Northing)const
{ /* BEGIN Get_Van_der_Grinten_Parameters */
/*
 * The function Get_Van_der_Grinten_Parameters returns the current ellipsoid
 * parameters, and Van Der Grinten projection parameters.
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
  *a = Grin_a;
  *f = Grin_f;
  *Central_Meridian = Grin_Origin_Long;
  *False_Easting = Grin_False_Easting;
  *False_Northing = Grin_False_Northing;
  
  return;
} /* END OF Get_Van_der_Grinten_Parameters */
long rspfVanDerGrintenProjection::Convert_Geodetic_To_Van_der_Grinten (double Latitude,
                                                                        double Longitude,
                                                                        double *Easting,
                                                                        double *Northing)const
{ /* BEGIN Convert_Geodetic_To_Van_der_Grinten */
/*
 * The function Convert_Geodetic_To_Van_der_Grinten converts geodetic (latitude and
 * longitude) coordinates to Van Der Grinten projection (easting and northing)
 * coordinates, according to the current ellipsoid and Van Der Grinten projection
 * parameters.  If any errors occur, the error code(s) are returned by the
 * function, otherwise GRIN_NO_ERROR is returned.
 *
 *    Latitude          : Latitude (phi) in radians           (input)
 *    Longitude         : Longitude (lambda) in radians       (input)
 *    Easting           : Easting (X) in meters               (output)
 *    Northing          : Northing (Y) in meters              (output)
 */
  double dlam;                      /* Longitude - Central Meridan */
  double aa, aasqr;
  double gg;
  double pp, ppsqr;
  double gg_MINUS_ppsqr, ppsqr_PLUS_aasqr;
  double in_theta;
  double theta;
  double sin_theta, cos_theta;
  double qq;
  long   Error_Code = GRIN_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    dlam = Longitude - Grin_Origin_Long;
    if (Latitude == 0.0)
    {
      *Easting = Ra * dlam + Grin_False_Easting;
      *Northing = 0.0;
    }
    else if (dlam == 0.0 || FLOAT_EQ(Latitude,MAX_LAT,.00001)  || FLOAT_EQ(Latitude,-MAX_LAT,.00001))
    {
      in_theta = fabs(TWO_OVER_PI * Latitude);
      if (in_theta > 1.0)
        in_theta = 1.0;
      else if (in_theta < -1.0)
        in_theta = -1.0;
      theta = asin(in_theta);
      *Easting = 0.0;
      *Northing = PI_Ra * tan(theta / 2) + Grin_False_Northing;
      if (Latitude < 0.0)
        *Northing *= -1.0;
    }
    else
    {
      aa = 0.5 * fabs(M_PI / dlam - dlam / M_PI);
      in_theta = fabs(TWO_OVER_PI * Latitude);
      if (in_theta > 1.0)
        in_theta = 1.0;
      else if (in_theta < -1.0)
        in_theta = -1.0;
      theta = asin(in_theta);
      sin_theta = sin(theta);
      cos_theta = cos(theta);
      gg = cos_theta / (sin_theta + cos_theta - 1);
      pp = gg * (2 / sin_theta - 1);
      aasqr = aa * aa;
      ppsqr = pp * pp;
      gg_MINUS_ppsqr = gg - ppsqr;
      ppsqr_PLUS_aasqr = ppsqr + aasqr;
      qq = aasqr + gg;
      *Easting = PI_Ra * (aa * (gg_MINUS_ppsqr) +
                          sqrt(aasqr * (gg_MINUS_ppsqr) * (gg_MINUS_ppsqr) -
                               (ppsqr_PLUS_aasqr) * (gg * gg - ppsqr))) / 
                 (ppsqr_PLUS_aasqr) + Grin_False_Easting;
      if (dlam < 0.0)
        *Easting *= -1.0;
      *Northing = PI_Ra * (pp * qq - aa * sqrt ((aasqr + 1) * (ppsqr_PLUS_aasqr) - qq * qq)) /
                  (ppsqr_PLUS_aasqr) + Grin_False_Northing;
      if (Latitude < 0.0)
        *Northing *= -1.0;
    }
  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Van_der_Grinten */
long rspfVanDerGrintenProjection::Convert_Van_der_Grinten_To_Geodetic(double Easting,
                                                                       double Northing,
                                                                       double *Latitude,
                                                                       double *Longitude)const
{ /* BEGIN Convert_Van_der_Grinten_To_Geodetic */
/*
 * The function Convert_Van_der_Grinten_To_Geodetic converts Grinten projection
 * (easting and northing) coordinates to geodetic (latitude and longitude)
 * coordinates, according to the current ellipsoid and Grinten projection
 * coordinates.  If any errors occur, the error code(s) are returned by the
 * function, otherwise GRIN_NO_ERROR is returned.
 *
 *    Easting           : Easting (X) in meters                  (input)
 *    Northing          : Northing (Y) in meters                 (input)
 *    Latitude          : Latitude (phi) in radians              (output)
 *    Longitude         : Longitude (lambda) in radians          (output)
 */
  double dx, dy;
  double xx, xxsqr;
  double yy, yysqr, two_yysqr;
  double xxsqr_PLUS_yysqr;
  double c1;
  double c2;
  double c3, c3sqr;
  double c2_OVER_3c3;
  double dd;
  double a1;
  double m1;
  double i;
  double theta1;
  long Error_Code = GRIN_NO_ERROR;
  if (!Error_Code)
  {
    dy = Northing - Grin_False_Northing;
    dx = Easting - Grin_False_Easting;
    xx = dx / PI_Ra;
    yy = dy / PI_Ra;
    xxsqr = xx * xx;
    yysqr = yy * yy;
    xxsqr_PLUS_yysqr = xxsqr + yysqr;
    two_yysqr = 2 * yysqr;
    if (Northing == 0.0)
      *Latitude = 0.0;
    else
    {
      c1 = - fabs(yy) * (1 + xxsqr_PLUS_yysqr);
      c2 = c1 - two_yysqr + xxsqr;
      c3 = - 2 * c1 + 1 + two_yysqr + (xxsqr_PLUS_yysqr) * (xxsqr_PLUS_yysqr);
      c2_OVER_3c3 = c2 / (3.0 * c3);
      c3sqr = c3 * c3;
      dd = yysqr / c3 + ((2 * c2 * c2 * c2) / (c3sqr * c3) - (9 * c1 * c2) / (c3sqr)) / 27;
      a1 = (c1 - c2 * c2_OVER_3c3) /c3;
      m1 = 2 * sqrt(-ONE_THIRD * a1);
      i = 3 * dd/ (a1 * m1);
      if ((i > 1.0)||(i < -1.0))
        *Latitude = MAX_LAT;
      else
      {
        theta1 = ONE_THIRD * acos(3 * dd / (a1 * m1));
        *Latitude = M_PI * (-m1 * cos(theta1 + PI_OVER_3) - c2_OVER_3c3);
      }
    }
    if (Northing < 0.0)
      *Latitude *= -1.0;
    if (xx == 0.0)
      *Longitude = Grin_Origin_Long;
    else
    {
      *Longitude = M_PI * (xxsqr_PLUS_yysqr - 1 + 
                         sqrt(1 + (2 * xxsqr - two_yysqr) + (xxsqr_PLUS_yysqr) * (xxsqr_PLUS_yysqr))) / 
                   (2 * xx) + Grin_Origin_Long;
    }
  }
  return (Error_Code);
} /* END OF Convert_Van_der_Grinten_To_Geodetic */
bool rspfVanDerGrintenProjection::operator==(const rspfProjection& proj) const
{
   if (!rspfMapProjection::operator==(proj))
      return false;
   rspfVanDerGrintenProjection* p = PTR_CAST(rspfVanDerGrintenProjection, &proj);
   if (!p) return false;
   if (!rspf::almostEqual(Grin_Origin_Long,p->Grin_Origin_Long)) return false;
   return true;
}
