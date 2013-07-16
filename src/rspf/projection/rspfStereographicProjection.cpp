#include <rspf/projection/rspfStereographicProjection.h>
#include <rspf/base/rspfKeywordNames.h>
RTTI_DEF1(rspfStereographicProjection, "rspfStereographicProjection", rspfMapProjection)
#ifndef PI_OVER_2
#  define PI_OVER_2  ( M_PI / 2.0)
#endif
#ifndef PI_OVER_4
#  define PI_OVER_4  ( M_PI / 4.0)
#endif
#ifndef TWO_PI
#  define TWO_PI     (2.0 * M_PI)
#endif
#define ONE	   (1.0 * M_PI / 180.0)       /* 1 degree in radians*/
#define STEREO_NO_ERROR                0x0000
#define STEREO_LAT_ERROR               0x0001
#define STEREO_LON_ERROR               0x0002
#define STEREO_ORIGIN_LAT_ERROR        0x0004
#define STEREO_CENT_MER_ERROR          0x0008
#define STEREO_EASTING_ERROR			     0x0010
#define STEREO_NORTHING_ERROR		       0x0020
#define STEREO_A_ERROR                 0x0040
#define STEREO_INV_F_ERROR             0x0080
rspfStereographicProjection::rspfStereographicProjection(const rspfEllipsoid& ellipsoid,
                                                       const rspfGpt& origin)
   :rspfMapProjection(ellipsoid, origin)
{
   setDefaults();
   update();
}
rspfStereographicProjection::rspfStereographicProjection(const rspfEllipsoid& ellipsoid,
                                                   const rspfGpt& origin,
                                                   double falseEasting,
                                                   double falseNorthing)
   :rspfMapProjection(ellipsoid, origin)
{
   Stereo_False_Easting  = falseEasting;
   Stereo_False_Northing = falseNorthing;
   Stereo_Delta_Easting  = 1460090226.0;
   Stereo_Delta_Northing = 1460090226.0;
   
   update();
}
void rspfStereographicProjection::update()
{   
   Set_Stereographic_Parameters(theEllipsoid.getA(),
                            theEllipsoid.getFlattening(),
                            theOrigin.latr(),
                            theOrigin.lonr(),
                            Stereo_False_Easting,
                            Stereo_False_Northing);
   theFalseEastingNorthing.x = Stereo_False_Easting;
   theFalseEastingNorthing.y = Stereo_False_Northing;
   rspfMapProjection::update();
}
void rspfStereographicProjection::setFalseEasting(double falseEasting)
{
   Stereo_False_Easting = falseEasting;
   
   update();
}
void rspfStereographicProjection::setFalseNorthing(double falseNorthing)
{
   Stereo_False_Northing = falseNorthing;
   
   update();
}
void rspfStereographicProjection::setDefaults()
{
   Stereo_False_Easting  = 0.0;
   Stereo_False_Northing = 0.0;
   Stereo_Delta_Easting  = 1460090226.0;
   Stereo_Delta_Northing = 1460090226.0;
}
void rspfStereographicProjection::setFalseEastingNorthing(double falseEasting,
                                                           double falseNorthing)
{
   Stereo_False_Easting  = falseEasting;
   Stereo_False_Northing = falseNorthing;
   
   update();
}
rspfGpt rspfStereographicProjection::inverse(const rspfDpt &eastingNorthing)const
{
   double lat = 0.0;
   double lon = 0.0;
   Convert_Stereographic_To_Geodetic(eastingNorthing.x,
                                 eastingNorthing.y,
                                 &lat,
                                 &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}

rspfGpt rspfStereographicProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat = 0.0;
   double lon = 0.0;
   Convert_Stereographic_To_Geodetic(eastingNorthing.x,
                                 eastingNorthing.y,
                                 &lat,
                                 &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);  
}

rspfDpt rspfStereographicProjection::forward(const rspfGpt &latLon)const
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
   Convert_Geodetic_To_Stereographic(gpt.latr(),
                                     gpt.lonr(),
                                     &easting,
                                     &northing);
   
   return rspfDpt(easting, northing);
}
bool rspfStereographicProjection::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   return rspfMapProjection::saveState(kwl, prefix);
}
bool rspfStereographicProjection::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   bool flag = rspfMapProjection::loadState(kwl, prefix);
   const char* type = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   
   setDefaults();
   if(rspfString(type) == STATIC_TYPE_NAME(rspfStereographicProjection))
   {
      Stereo_False_Easting  = theFalseEastingNorthing.x;
      Stereo_False_Northing = theFalseEastingNorthing.y;
   }
   
   update();
   return flag;
}
/************************************************************************/
/*                              FUNCTIONS
 *
 */
long rspfStereographicProjection::Set_Stereographic_Parameters (double a,
                                                                 double f,
                                                                 double Origin_Latitude,
                                                                 double Central_Meridian,
                                                                 double False_Easting,
                                                                 double False_Northing)
{  /* BEGIN Set_Stereographic_Parameters   */
/*  
 *  The function Set_Stereographic_Parameters receives the ellipsoid
 *  parameters and Stereograpic projection parameters as inputs, and
 *  sets the corresponding state variables.  If any errors occur, error
 *  code(s) are returned by the function, otherwise STEREO_NO_ERROR is returned.
 *
 *  a                : Semi-major axis of ellipsoid, in meters         (input)
 *  f                : Flattening of ellipsoid					               (input)
 *  Origin_Latitude  : Latitude, in radians, at the center of          (input)
 *                       the projection
 *  Central_Meridian : Longitude, in radians, at the center of         (input)
 *                       the projection
 *  False_Easting    : Easting (X) at center of projection, in meters  (input)
 *  False_Northing   : Northing (Y) at center of projection, in meters (input)
 */
  double es2, es4, es6;
  double temp = 0;
  long Error_Code = STEREO_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    Stereo_a = a;
    Stereo_f = f;
    es2 = 2 * Stereo_f - Stereo_f * Stereo_f;
    es4 = es2 * es2;
    es6 = es4 * es2;
    Stereo_Ra = Stereo_a * (1.0 - es2 / 6.0 - 17.0 * es4 / 360.0 - 67.0 * es6 /3024.0);
    Two_Stereo_Ra = 2.0 * Stereo_Ra;
    Stereo_Origin_Lat = Origin_Latitude;
    Sin_Stereo_Origin_Lat = sin(Stereo_Origin_Lat);
    Cos_Stereo_Origin_Lat = cos(Stereo_Origin_Lat);
    Stereo_Origin_Long = Central_Meridian;
    Stereo_False_Easting = False_Easting;
    Stereo_False_Northing = False_Northing;
    if(fabs(fabs(Stereo_Origin_Lat) - PI_OVER_2) < 1.0e-10)
      Stereo_At_Pole = 1;
    else
      Stereo_At_Pole = 0;
    if ((Stereo_At_Pole) || (fabs(Stereo_Origin_Lat) < 1.0e-10))
    {
      Stereo_Delta_Easting = 1460090226.0;
    }
    else
    {
      if (Stereo_Origin_Long <= 0)
        Convert_Geodetic_To_Stereographic(-Stereo_Origin_Lat, M_PI + Stereo_Origin_Long - ONE, &Stereo_Delta_Easting, &temp);
      else
        Convert_Geodetic_To_Stereographic(-Stereo_Origin_Lat, Stereo_Origin_Long - M_PI - ONE, &Stereo_Delta_Easting, &temp);
    }
  } /* END OF if(!Error_Code) */
  return (Error_Code);
} /* END OF Set_Stereographic_Parameters */
void rspfStereographicProjection::Get_Stereographic_Parameters(double *a,
                                                                double *f,
                                                                double *Origin_Latitude,
                                                                double *Central_Meridian,
                                                                double *False_Easting,
                                                                double *False_Northing)const
{ /* BEGIN Get_Stereographic_Parameters */
/*
 * The function Get_Stereographic_Parameters returns the current ellipsoid
 * parameters and Stereographic projection parameters.
 *
 *    a                 : Semi-major axis of ellipsoid, in meters   (output)
 *    f                 : Flattening of ellipsoid						        (output)
 *    Origin_Latitude   : Latitude, in radians, at the center of    (output)
 *                          the projection
 *    Central_Meridian  : Longitude, in radians, at the center of   (output)
 *                          the projection
 *    False_Easting     : A coordinate value, in meters, assigned to the
 *                          central meridian of the projection.     (output)
 *    False_Northing    : A coordinate value, in meters, assigned to the
 *                          origin latitude of the projection       (output) 
 */
  *a = Stereo_a;
  *f = Stereo_f;
  *Origin_Latitude = Stereo_Origin_Lat;
  *Central_Meridian = Stereo_Origin_Long;
  *False_Easting = Stereo_False_Easting;
  *False_Northing = Stereo_False_Northing;
  
  return;
} /* END OF Get_Stereographic_Parameters */
long rspfStereographicProjection::Convert_Geodetic_To_Stereographic (double Latitude,
                                                                      double Longitude,
                                                                      double *Easting,
                                                                      double *Northing)const
{  /* BEGIN Convert_Geodetic_To_Stereographic */
/*
 * The function Convert_Geodetic_To_Stereographic converts geodetic
 * coordinates (latitude and longitude) to Stereographic coordinates
 * (easting and northing), according to the current ellipsoid
 * and Stereographic projection parameters. If any errors occur, error
 * code(s) are returned by the function, otherwise STEREO_NO_ERROR is returned.
 *
 *    Latitude   :  Latitude, in radians                      (input)
 *    Longitude  :  Longitude, in radians                     (input)
 *    Easting    :  Easting (X), in meters                    (output)
 *    Northing   :  Northing (Y), in meters                   (output)
 */
  double g, k;
  double num = 0;
  double Ra_k = 0;
  double slat = sin(Latitude);
  double clat = cos(Latitude);
  double dlam;                        /* Longitude - Central Meridan */
  double cos_dlam;
  long   Error_Code = STEREO_NO_ERROR;
  if (!Error_Code)
  {  /* no errors */
    dlam = Longitude - Stereo_Origin_Long;
    cos_dlam = cos(dlam);
    g = 1.0 + Sin_Stereo_Origin_Lat * slat + Cos_Stereo_Origin_Lat * clat * cos_dlam;
    if (fabs(g) <= 1.0e-10)
    {  /* Point is out of view.  Will return longitude out of range message
          since no point out of view is implemented.  */
      Error_Code |= STEREO_LON_ERROR;
    }
    else
    {
      if (Stereo_At_Pole)
      {
        if (fabs(fabs(Latitude) - PI_OVER_2) < 1.0e-10)
        {
          *Easting = Stereo_False_Easting;
          *Northing = Stereo_False_Northing;
        }
        else
        {
          if (Stereo_Origin_Lat > 0)
          {
            num = Two_Stereo_Ra * tan(PI_OVER_4 - Latitude / 2.0);
            *Easting = Stereo_False_Easting + num * sin(dlam);
            *Northing = Stereo_False_Northing + (-num * cos_dlam);
          }
          else
          {
            num = Two_Stereo_Ra * tan(PI_OVER_4 + Latitude / 2.0);
            *Easting = Stereo_False_Easting + num * sin(dlam);
            *Northing = Stereo_False_Northing + num * cos_dlam;
          }
        }
      }
      else
      {
        if (fabs(Stereo_Origin_Lat) <= 1.0e-10)
        {
          k = 2.0 / (1.0 + clat * cos_dlam);
          Ra_k = Stereo_Ra * k;
          *Northing = Stereo_False_Northing + Ra_k * slat;
        }
        else
        {
          k = 2.0 / g;
          Ra_k = Stereo_Ra * k;
          *Northing = Stereo_False_Northing + Ra_k * (Cos_Stereo_Origin_Lat * slat - Sin_Stereo_Origin_Lat * clat * cos_dlam);
        }
        *Easting = Stereo_False_Easting + Ra_k * clat * sin(dlam);
      }
    }
  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Stereographic */
  
long rspfStereographicProjection::Convert_Stereographic_To_Geodetic(double Easting,
                                                                     double Northing,
                                                                     double *Latitude,
                                                                     double *Longitude)const
{ /* BEGIN Convert_Stereographic_To_Geodetic */
/*
 * The function Convert_Stereographic_To_Geodetic converts Stereographic projection
 * (easting and northing) coordinates to geodetic (latitude and longitude)
 * coordinates, according to the current ellipsoid and Stereographic projection
 * coordinates.  If any errors occur, the error code(s) are returned by the
 * function, otherwise STEREO_NO_ERROR is returned.
 *
 *    Easting           : Easting (X), in meters              (input)
 *    Northing          : Northing (Y), in meters             (input)
 *    Latitude          : Latitude (phi), in radians          (output)
 *    Longitude         : Longitude (lambda), in radians      (output)
 */
  double dx, dy;
  double rho, c;
  double sin_c, cos_c;
  double dy_sin_c;
  long Error_Code = STEREO_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    dy = Northing - Stereo_False_Northing;
    dx = Easting - Stereo_False_Easting;
    rho = sqrt(dx * dx + dy * dy);
    if (fabs(rho) <= 1.0e-10)
    {
      *Latitude = Stereo_Origin_Lat;
      *Longitude = Stereo_Origin_Long;
    }
    else
    {
      c = 2.0 * atan(rho / (Two_Stereo_Ra));
      sin_c = sin(c);
      cos_c = cos(c);
      dy_sin_c = dy * sin_c;
      if (Stereo_At_Pole)
      {
        if (Stereo_Origin_Lat > 0)
          *Longitude = Stereo_Origin_Long + atan2(dx, -dy);
        else
          *Longitude = Stereo_Origin_Long + atan2(dx, dy);
      }
      else
        *Longitude = Stereo_Origin_Long + atan2(dx * sin_c, (rho * Cos_Stereo_Origin_Lat * cos_c - dy_sin_c * Sin_Stereo_Origin_Lat));
      *Latitude = asin(cos_c * Sin_Stereo_Origin_Lat + ((dy_sin_c * Cos_Stereo_Origin_Lat) / rho));
    }
  }
  return (Error_Code);
} /* END OF Convert_Stereographic_To_Geodetic */
