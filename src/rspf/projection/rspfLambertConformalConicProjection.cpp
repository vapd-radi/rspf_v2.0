#include <iostream>
#include <iomanip>
#include <rspf/projection/rspfLambertConformalConicProjection.h>
#include <rspf/base/rspfKeywordNames.h>
RTTI_DEF1(rspfLambertConformalConicProjection, "rspfLambertConformalConicProjection", rspfMapProjection)
/***************************************************************************/
/*                               DEFINES
 *
 */
#ifndef PI_OVER_2
#  define PI_OVER_2  ( M_PI / 2.0)
#endif
#ifndef TWO_PI
#  define TWO_PI     (2.0 * M_PI)
#endif
#define MAX_LAT    (( M_PI *  89.99972222222222) / 180.0)  /* 89 59 59.0 degrees in radians */
#define LAMBERT_m(clat,essin)                  (clat / sqrt(1.0 - essin * essin))
#define LAMBERT_t(lat,essin)                   tan(PI_OVER_4 - lat / 2) /				\
										            pow((1.0 - essin) / (1.0 + essin), es_OVER_2)
#define ES_SIN(sinlat)                         (es * sinlat)
/**************************************************************************/
/*                               GLOBAL DECLARATIONS
 *
 */
const double PI_OVER_4 = (M_PI / 4.0);
#define LAMBERT_NO_ERROR           0x0000
#define LAMBERT_LAT_ERROR          0x0001
#define LAMBERT_LON_ERROR          0x0002
#define LAMBERT_EASTING_ERROR      0x0004
#define LAMBERT_NORTHING_ERROR     0x0008
#define LAMBERT_FIRST_STDP_ERROR   0x0010
#define LAMBERT_SECOND_STDP_ERROR  0x0020
#define LAMBERT_ORIGIN_LAT_ERROR   0x0040
#define LAMBERT_CENT_MER_ERROR     0x0080
#define LAMBERT_A_ERROR            0x0100
#define LAMBERT_B_ERROR            0x0200
#define LAMBERT_A_LESS_B_ERROR     0x0400
#define LAMBERT_HEMISPHERE_ERROR   0x0800
#define LAMBERT_FIRST_SECOND_ERROR 0x1000
rspfLambertConformalConicProjection::rspfLambertConformalConicProjection(const rspfEllipsoid& ellipsoid,
                                                                           const rspfGpt& origin)
   :rspfMapProjection(ellipsoid, origin)
{
   setDefaults();
   update();
}
    
rspfLambertConformalConicProjection::rspfLambertConformalConicProjection(const rspfEllipsoid& ellipsoid,
                                                                           const rspfGpt& origin,
                                                                           double stdParallel1,
                                                                           double stdParallel2,
                                                                           double falseEasting,
                                                                           double falseNorthing)
   :rspfMapProjection(ellipsoid, origin)
{
   Lambert_Std_Parallel_1 = stdParallel1*RAD_PER_DEG;
   Lambert_Std_Parallel_2 = stdParallel2*RAD_PER_DEG;
   Lambert_False_Easting  = falseEasting;
   Lambert_False_Northing = falseNorthing;
   
   update();
}
rspfLambertConformalConicProjection::~rspfLambertConformalConicProjection()
{
}
rspfObject* rspfLambertConformalConicProjection::dup()const
{
   return new rspfLambertConformalConicProjection(*this);
}
void rspfLambertConformalConicProjection::update()
{   
   Set_Lambert_Parameters(theEllipsoid.getA(),
                          theEllipsoid.getFlattening(),
                          theOrigin.latr(),
                          theOrigin.lonr(),
                          Lambert_Std_Parallel_1,
                          Lambert_Std_Parallel_2,
                          Lambert_False_Easting,
                          Lambert_False_Northing);
   theFalseEastingNorthing.x = Lambert_False_Easting;
   theFalseEastingNorthing.y = Lambert_False_Northing;
   rspfMapProjection::update();
}
void rspfLambertConformalConicProjection::setStandardParallel1(double degree)
{
   Lambert_Std_Parallel_1 = degree*RAD_PER_DEG;
   update();
}
void rspfLambertConformalConicProjection::setStandardParallel2(double degree)
{
   Lambert_Std_Parallel_2 = degree*RAD_PER_DEG;
   update();
}
void rspfLambertConformalConicProjection::setStandardParallels(double parallel1Degree,
                                                                double parallel2Degree)
{
   Lambert_Std_Parallel_1 = parallel1Degree*RAD_PER_DEG;
   Lambert_Std_Parallel_2 = parallel2Degree*RAD_PER_DEG;
   update();
   
}
void rspfLambertConformalConicProjection::setFalseEasting(double falseEasting)
{
   Lambert_False_Easting = falseEasting;
   update();
}
void rspfLambertConformalConicProjection::setFalseNorthing(double falseNorthing)
{
   Lambert_False_Northing = falseNorthing;
   update();
}
void rspfLambertConformalConicProjection::setFalseEastingNorthing(double falseEasting,
                                                                   double falseNorthing)
{
   Lambert_False_Easting = falseEasting;
   Lambert_False_Northing = falseNorthing;  
   update();
}
void rspfLambertConformalConicProjection::setParameters(double parallel1,
                                                         double parallel2,
                                                         double falseEasting,
                                                         double falseNorthing)
{
   Lambert_False_Easting = falseEasting;
   Lambert_False_Northing = falseNorthing;  
   Lambert_Std_Parallel_1 = parallel1*RAD_PER_DEG;
   Lambert_Std_Parallel_2 = parallel2*RAD_PER_DEG;
   update(); 
}
void rspfLambertConformalConicProjection::setDefaults()
{
   Lambert_Std_Parallel_1 = 40*RAD_PER_DEG;
   Lambert_Std_Parallel_2 = 50*RAD_PER_DEG;
   Lambert_False_Northing = 0.0;
   Lambert_False_Easting  = 0.0;
}
rspfGpt rspfLambertConformalConicProjection::inverse(const rspfDpt &eastingNorthing)const
{
   double lat = 0.0;
   double lon = 0.0;
  
   Convert_Lambert_To_Geodetic(eastingNorthing.x,
                               eastingNorthing.y,
                               &lat,
                               &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);
}
rspfGpt rspfLambertConformalConicProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat = 0.0;
   double lon = 0.0;
  
   Convert_Lambert_To_Geodetic(eastingNorthing.x,
                               eastingNorthing.y,
                               &lat,
                               &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);
}

rspfDpt rspfLambertConformalConicProjection::forward(const rspfGpt &latLon)const
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
   Convert_Geodetic_To_Lambert(gpt.latr(),
                               gpt.lonr(),
                               &easting,
                               &northing);
   return rspfDpt(easting, northing);
}
bool rspfLambertConformalConicProjection::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   kwl.add(prefix,
           rspfKeywordNames::STD_PARALLEL_1_KW,
           Lambert_Std_Parallel_1*DEG_PER_RAD,
           true);
   kwl.add(prefix,
           rspfKeywordNames::STD_PARALLEL_2_KW,
           Lambert_Std_Parallel_2*DEG_PER_RAD,
           true);
   
   return rspfMapProjection::saveState(kwl, prefix);
}
bool rspfLambertConformalConicProjection::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   bool flag = rspfMapProjection::loadState(kwl, prefix);
   const char* type          = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   const char* stdParallel1  = kwl.find(prefix, rspfKeywordNames::STD_PARALLEL_1_KW);
   const char* stdParallel2  = kwl.find(prefix, rspfKeywordNames::STD_PARALLEL_2_KW);
   setDefaults();
   
   if(rspfString(type) == STATIC_TYPE_NAME(rspfLambertConformalConicProjection))
   {
      Lambert_False_Easting  = theFalseEastingNorthing.x;
      Lambert_False_Northing = theFalseEastingNorthing.y;
      if(stdParallel1)
      {
         Lambert_Std_Parallel_1 = rspfString(stdParallel1).toDouble()*RAD_PER_DEG;
      }
      if(stdParallel2)
      {
         Lambert_Std_Parallel_2 = rspfString(stdParallel2).toDouble()*RAD_PER_DEG;
      }
   }
   update();
   return flag;
}
std::ostream& rspfLambertConformalConicProjection::print(
   std::ostream& out) const
{
   std::ios_base::fmtflags f = out.flags();
   out << setiosflags(ios::fixed) << setprecision(15);
   out << "// rspfLambertConformalConicProjection::print\n"
       << rspfKeywordNames::STD_PARALLEL_1_KW << ": "
       << Lambert_Std_Parallel_1*DEG_PER_RAD << "\n"
       << rspfKeywordNames::STD_PARALLEL_2_KW << ": "
      << Lambert_Std_Parallel_2*DEG_PER_RAD << std::endl;
   out.setf(f);
   
   return rspfMapProjection::print(out);
}
/************************************************************************/
/*                              FUNCTIONS
 *
 */
long rspfLambertConformalConicProjection::Set_Lambert_Parameters(double a,
                                                                  double f,
                                                                  double Origin_Latitude,
                                                                  double Central_Meridian,
                                                                  double Std_Parallel_1,
                                                                  double Std_Parallel_2,
                                                                  double False_Easting,
                                                                  double False_Northing)
{ /* BEGIN Set_Lambert_Parameters */
/*
 * The function Set_Lambert_Parameters receives the ellipsoid parameters and
 * Lambert Conformal Conic projection parameters as inputs, and sets the
 * corresponding state variables.  If any errors occur, the error code(s)
 * are returned by the function, otherwise LAMBERT_NO_ERROR is returned.
 *
 *   a                   : Semi-major axis of ellipsoid, in meters   (input)
 *   f                   : Flattening of ellipsoid						       (input)
 *   Origin_Latitude     : Latitude of origin, in radians            (input)
 *   Central_Meridian    : Longitude of origin, in radians           (input)
 *   Std_Parallel_1      : First standard parallel, in radians       (input)
 *   Std_Parallel_2      : Second standard parallel, in radians      (input)
 *   False_Easting       : False easting, in meters                  (input)
 *   False_Northing      : False northing, in meters                 (input)
 */
  double slat, slat1, clat;
  double es_sin;
  double t0, t1, t2;
  double m1, m2;
  long Error_Code = LAMBERT_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    Lambert_a = a;
    Lambert_f = f;
    Lambert_Origin_Lat = Origin_Latitude;
    Lambert_Std_Parallel_1 = Std_Parallel_1;
    Lambert_Std_Parallel_2 = Std_Parallel_2;
    Lambert_Origin_Long = Central_Meridian;
    Lambert_False_Easting = False_Easting;
    Lambert_False_Northing = False_Northing;
    es2 = 2 * Lambert_f - Lambert_f * Lambert_f;
    es = sqrt(es2);
    es_OVER_2 = es / 2.0;
    slat = sin(Lambert_Origin_Lat);
    es_sin = ES_SIN(slat);
    t0 = LAMBERT_t(Lambert_Origin_Lat, es_sin);
    slat1 = sin(Lambert_Std_Parallel_1);
    clat = cos(Lambert_Std_Parallel_1);
    es_sin = ES_SIN(slat1);
    m1 = LAMBERT_m(clat, es_sin);
    t1 = LAMBERT_t(Lambert_Std_Parallel_1, es_sin);
    if (fabs(Lambert_Std_Parallel_1 - Lambert_Std_Parallel_2) > 1.0e-10)
    {
      slat = sin(Lambert_Std_Parallel_2);
      clat = cos(Lambert_Std_Parallel_2);
      es_sin = ES_SIN(slat);
      m2 = LAMBERT_m(clat, es_sin);
      t2 = LAMBERT_t(Lambert_Std_Parallel_2, es_sin);
      n = log(m1 / m2) / log(t1 / t2);
    }
    else
      n = slat1;
    F = m1 / (n * pow(t1, n));
    Lambert_aF = Lambert_a * F;
    if ((t0 == 0) && (n < 0))
      rho0 = 0.0;
    else
      rho0 = Lambert_aF * pow(t0, n);
  }
  return (Error_Code);
} /* END OF Set_Lambert_Parameters */
void rspfLambertConformalConicProjection::Get_Lambert_Parameters(double *a,
                                                                  double *f,
                                                                  double *Origin_Latitude,
                                                                  double *Central_Meridian,
                                                                  double *Std_Parallel_1,
                                                                  double *Std_Parallel_2,
                                                                  double *False_Easting,
                                                                  double *False_Northing)const
{ /* BEGIN Get_Lambert_Parameters */
/*                         
 * The function Get_Lambert_Parameters returns the current ellipsoid
 * parameters and Lambert Conformal Conic projection parameters.
 *
 *   a                   : Semi-major axis of ellipsoid, in meters   (output)
 *   f                   : Flattening of ellipsoid					         (output)
 *   Origin_Latitude     : Latitude of origin, in radians            (output)
 *   Central_Meridian    : Longitude of origin, in radians           (output)
 *   Std_Parallel_1      : First standard parallel, in radians       (output)
 *   Std_Parallel_2      : Second standard parallel, in radians      (output)
 *   False_Easting       : False easting, in meters                  (output)
 *   False_Northing      : False northing, in meters                 (output)
 */
  *a = Lambert_a;
  *f = Lambert_f;
  *Std_Parallel_1 = Lambert_Std_Parallel_1;
  *Std_Parallel_2 = Lambert_Std_Parallel_2;
  *Origin_Latitude = Lambert_Origin_Lat;
  *Central_Meridian = Lambert_Origin_Long;
  *False_Easting = Lambert_False_Easting;
  *False_Northing = Lambert_False_Northing;
  
  return;
} /* END OF Get_Lambert_Parameters */
long rspfLambertConformalConicProjection::Convert_Geodetic_To_Lambert (double Latitude,
                                                                        double Longitude,
                                                                        double *Easting,
                                                                        double *Northing)const
   
{ /* BEGIN Convert_Geodetic_To_Lambert */
/*
 * The function Convert_Geodetic_To_Lambert converts Geodetic (latitude and
 * longitude) coordinates to Lambert Conformal Conic projection (easting
 * and northing) coordinates, according to the current ellipsoid and
 * Lambert Conformal Conic projection parameters.  If any errors occur, the
 * error code(s) are returned by the function, otherwise LAMBERT_NO_ERROR is
 * returned.
 *
 *    Latitude         : Latitude, in radians                         (input)
 *    Longitude        : Longitude, in radians                        (input)
 *    Easting          : Easting (X), in meters                       (output)
 *    Northing         : Northing (Y), in meters                      (output)
 */
  double slat;
  double es_sin;
  double t;
  double rho;
  double dlam;
  double theta;
  long  Error_Code = LAMBERT_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    if (fabs(fabs(Latitude) - PI_OVER_2) > 1.0e-10)
    {
      slat = sin(Latitude);
      es_sin = ES_SIN(slat);
      t = LAMBERT_t(Latitude, es_sin);
      rho = Lambert_aF * pow(t, n);
    }
    else
    {
      if ((Latitude * n) <= 0)
      { /* Point can not be projected */
        Error_Code |= LAMBERT_LAT_ERROR;
        return (Error_Code);
      }
      rho = 0.0;
    }
    dlam = Longitude - Lambert_Origin_Long;
    theta = n * dlam;
    *Easting = rho * sin(theta) + Lambert_False_Easting;
    *Northing = rho0 - rho * cos(theta) + Lambert_False_Northing;
  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Lambert */
long rspfLambertConformalConicProjection::Convert_Lambert_To_Geodetic (double Easting,
                                                                        double Northing,
                                                                        double *Latitude,
                                                                        double *Longitude)const
{ /* BEGIN Convert_Lambert_To_Geodetic */
/*
 * The function Convert_Lambert_To_Geodetic converts Lambert Conformal
 * Conic projection (easting and northing) coordinates to Geodetic
 * (latitude and longitude) coordinates, according to the current ellipsoid
 * and Lambert Conformal Conic projection parameters.  If any errors occur,
 * the error code(s) are returned by the function, otherwise LAMBERT_NO_ERROR
 * is returned.
 *
 *    Easting          : Easting (X), in meters                       (input)
 *    Northing         : Northing (Y), in meters                      (input)
 *    Latitude         : Latitude, in radians                         (output)
 *    Longitude        : Longitude, in radians                        (output)
 */
  double dy, dx;
  double rho, rho0_MINUS_dy;
  double t;
  double PHI;
  double tempPHI = 0.0;
  double sin_PHI;
  double es_sin;
  double theta = 0.0;
  double tolerance = 4.85e-10;
  long Error_Code = LAMBERT_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    dy = Northing - Lambert_False_Northing;
    dx = Easting - Lambert_False_Easting;
    rho0_MINUS_dy = rho0 - dy;
    rho = sqrt(dx * dx + (rho0_MINUS_dy) * (rho0_MINUS_dy));
    if (n < 0.0)
    {
      rho *= -1.0;
      dy *= -1.0;
      dx *= -1.0;
      rho0_MINUS_dy *= -1.0;
    }
    if (rho != 0.0)
    {
      theta = atan2(dx, rho0_MINUS_dy);
      t = pow(rho / (Lambert_aF) , 1.0 / n);
      PHI = PI_OVER_2 - 2.0 * atan(t);
      while (fabs(PHI - tempPHI) > tolerance)
      {
        tempPHI = PHI;
        sin_PHI = sin(PHI);
        es_sin = ES_SIN(sin_PHI);
        PHI = PI_OVER_2 - 2.0 * atan(t * pow((1.0 - es_sin) / (1.0 + es_sin), es_OVER_2));
      }
      *Latitude = PHI;
      *Longitude = theta / n + Lambert_Origin_Long;
      if (fabs(*Latitude) < 2.0e-7)  /* force lat to 0 to avoid -0 degrees */
        *Latitude = 0.0;
      if (*Latitude > PI_OVER_2)  /* force distorted values to 90, -90 degrees */
        *Latitude = PI_OVER_2;
      else if (*Latitude < -PI_OVER_2)
        *Latitude = -PI_OVER_2;
      if (*Longitude > M_PI)
      {
        if (*Longitude - M_PI < 3.5e-6) 
          *Longitude = M_PI;
      }
      if (*Longitude < -M_PI)
      {
        if (fabs(*Longitude + M_PI) < 3.5e-6)
          *Longitude = -M_PI;
      }
      if (fabs(*Longitude) < 2.0e-7)  /* force lon to 0 to avoid -0 degrees */
        *Longitude = 0.0;
      if (*Longitude > M_PI)  /* force distorted values to 180, -180 degrees */
        *Longitude = M_PI;
      else if (*Longitude < -M_PI)
        *Longitude = -M_PI;
    }
    else
    {
      if (n > 0.0)
        *Latitude = PI_OVER_2;
      else
        *Latitude = -PI_OVER_2;
      *Longitude = Lambert_Origin_Long;
    }
  }
  return (Error_Code);
} /* END OF Convert_Lambert_To_Geodetic */
double rspfLambertConformalConicProjection::getStandardParallel1()const
{
   return  Lambert_Std_Parallel_1/RAD_PER_DEG;
}
double rspfLambertConformalConicProjection::getStandardParallel2()const
{
   return  Lambert_Std_Parallel_2/RAD_PER_DEG;
}
bool rspfLambertConformalConicProjection::operator==(const rspfProjection& proj) const
{
   if (!rspfMapProjection::operator==(proj)) 
      return false;
   rspfLambertConformalConicProjection* p = PTR_CAST(rspfLambertConformalConicProjection, &proj);
   if (!p) return false;
   if (!rspf::almostEqual(Lambert_Std_Parallel_1,p->Lambert_Std_Parallel_1)) return false;
   if (!rspf::almostEqual(Lambert_Std_Parallel_2,p->Lambert_Std_Parallel_2)) return false;
   return true;
}
