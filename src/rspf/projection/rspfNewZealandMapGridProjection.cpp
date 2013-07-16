#include <rspf/projection/rspfNewZealandMapGridProjection.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
RTTI_DEF1(rspfNewZealandMapGridProjection, "rspfNewZealandMapGridProjection", rspfMapProjection)
#ifndef PI_OVER_2
#  define PI_OVER_2  ( M_PI / 2.0)
#endif
#ifndef TWO_PI
#  define TWO_PI     (2.0 * M_PI)
#endif
#define MAX_LAT         (-33.5 * M_PI / 180.0)    /* -33.5 degrees */
#define MIN_LAT         (-48.5 * M_PI / 180.0)    /* -48.5 degrees */
#define MAX_LON         (180.0 * M_PI / 180.0)    /* 180 degrees */
#define MIN_LON         (165.5 * M_PI / 180.0)    /* 165.5 degrees */
#define NZMG_NO_ERROR           0x0000
#define NZMG_LAT_ERROR          0x0001
#define NZMG_LON_ERROR          0x0002
#define NZMG_EASTING_ERROR      0x0004
#define NZMG_NORTHING_ERROR     0x0008
#define NZMG_ELLIPSOID_ERROR    0x0010
typedef struct ComplexNumber
{
  double real;
  double imag;
} Complex;
static double A[] = { 0.6399175073, -0.1358797613, 0.063294409,
                      -0.02526853, 0.0117879, -0.0055161,
                      0.0026906, -0.001333, 0.00067, -0.00034 };
static Complex B[] = { { 0.7557853228, 0.0 },
                       { 0.249204646, 0.003371507 },
                       { -0.001541739, 0.041058560 },
                       { -0.10162907, 0.01727609 },
                       { -0.26623489, -0.36249218 },
                       { -0.6870983, -1.1651967 } };
static Complex C[] = { { 1.3231270439, 0.0 },
                       { -0.577245789, -0.007809598 },
                       { 0.508307513, -0.112208952 },
                       { -0.15094762, 0.18200602 },
                       { 1.01418179, 1.64497696 },
                       { 1.9660549, 2.5127645 } };
static double D[] = { 1.5627014243, 0.5185406398, -0.03333098,
                      -0.1052906, -0.0368594, 0.007317,
                      0.01220, 0.00394, -0.0013 };   
                    
/* These state variables are for optimization purposes. The only function
 * that should modify them is Set_NZMG_Parameters.         */
/************************************************************************/
/*                              FUNCTIONS
 *
 */
/* Add two complex numbers */
static Complex Add(Complex z1, Complex z2)
{
  Complex z;
  z.real = z1.real + z2.real;
  z.imag = z1.imag + z2.imag;
  return z;
}
/* Multiply two complex numbers */
static Complex Multiply(Complex z1, Complex z2)
{
  Complex z;
  z.real = z1.real * z2.real - z1.imag * z2.imag;
  z.imag = z1.imag * z2.real + z1.real * z2.imag;
  return z;
}
/* Divide two complex numbers */
static Complex Divide(Complex z1, Complex z2)
{
  Complex z;
  double denom;
  denom = z2.real * z2.real + z2.imag * z2.imag;
  z.real = (z1.real * z2.real + z1.imag * z2.imag) / denom;
  z.imag = (z1.imag * z2.real - z1.real * z2.imag) / denom;
  return z;
}
rspfNewZealandMapGridProjection::rspfNewZealandMapGridProjection()
   :rspfMapProjection(*rspfEllipsoidFactory::instance()->create("IN"),
                       rspfGpt(-41, 173.0))
{
   setDefaults();
   update();
}
rspfNewZealandMapGridProjection::~rspfNewZealandMapGridProjection()
{
}
void rspfNewZealandMapGridProjection::setFalseEasting(double falseEasting)
{
   NZMG_False_Easting  = falseEasting;
   update();
}
void rspfNewZealandMapGridProjection::setFalseNorthing(double falseNorthing)
{
   NZMG_False_Northing = falseNorthing;
   update();
}
void rspfNewZealandMapGridProjection::setFalseEastingNorthing(double falseEasting, double falseNorthing)
{
   NZMG_False_Easting  = falseEasting;
   NZMG_False_Northing = falseNorthing;
   
   update();   
}
void rspfNewZealandMapGridProjection::setDefaults()
{
   theOrigin.latd(-41.0);
   theOrigin.lond(173.0);
   NZMG_Origin_Lat     = theOrigin.latd();
   NZMG_Origin_Long    = theOrigin.lonr();
   NZMG_Max_Easting    = 3170000.0;
   NZMG_Max_Northing   = 6900000.0;
   NZMG_Min_Easting    = 1810000.0;
   NZMG_Min_Northing   = 5160000.0;
   NZMG_False_Easting  = 2510000.0;
   NZMG_False_Northing = 6023150.0;
}
void rspfNewZealandMapGridProjection::update()
{
   theOrigin = rspfGpt(-41, 173);
   theDatum     = rspfDatumFactory::instance()->create(rspfString("GEO"));
   if(theDatum)
   {
      theEllipsoid = *(theDatum->ellipsoid());
   }
   else
   {
      rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfNewZealandMapGridProjection::update(): Was unable to locate the newzealand datum!" << std::endl;
   }
   
   NZMG_Origin_Lat   = theOrigin.latd();
   NZMG_Origin_Long  = theOrigin.lonr();
   NZMG_a            = theEllipsoid.getA();
   theFalseEastingNorthing.x = NZMG_False_Easting;
   theFalseEastingNorthing.y = NZMG_False_Northing;
   rspfMapProjection::update();
}
rspfGpt rspfNewZealandMapGridProjection::inverse(const rspfDpt &eastingNorthing)const
{
   double lat = 0.0;
   double lon = 0.0;
   Convert_NZMG_To_Geodetic(eastingNorthing.x,
                            eastingNorthing.y,
                            &lat,
                            &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);
}
rspfGpt rspfNewZealandMapGridProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat = 0.0;
   double lon = 0.0;
   Convert_NZMG_To_Geodetic(eastingNorthing.x,
                            eastingNorthing.y,
                            &lat,
                            &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);
}

rspfDpt rspfNewZealandMapGridProjection::forward(const rspfGpt &latLon)const
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
   Convert_Geodetic_To_NZMG(gpt.latr(),
                            gpt.lonr(),
                            &easting,
                            &northing);
   
   return rspfDpt(easting, northing); 
}
bool rspfNewZealandMapGridProjection::saveState(rspfKeywordlist& kwl,
                                                 const char* prefix)const
{
   rspfMapProjection::saveState(kwl, prefix);
   return true;
}
bool rspfNewZealandMapGridProjection::loadState(const rspfKeywordlist& kwl,
                                                 const char* prefix)
{
   rspfKeywordlist newKwl = kwl;
   newKwl.add(prefix,
              rspfKeywordNames::DATUM_KW,
              "GEO",
              true);
   
   newKwl.add(prefix,
              rspfKeywordNames::ORIGIN_LATITUDE_KW,
              -41,
              true);
   
   newKwl.add(prefix,
              rspfKeywordNames::CENTRAL_MERIDIAN_KW,
              173.0,
              true);
   
   setDefaults();
   
   rspfMapProjection::loadState(newKwl, prefix);
   update();
   return true;
}
long rspfNewZealandMapGridProjection::Convert_Geodetic_To_NZMG (double Latitude,
                                                                 double Longitude,
                                                                 double *Easting,
                                                                 double *Northing)const
{ /* BEGIN Convert_Geodetic_To_NZMG */
/*
 * The function Convert_Geodetic_To_NZMG converts geodetic (latitude and
 * longitude) coordinates to New Zealand Map Grid projection (easting and northing)
 * coordinates, according to the current ellipsoid and New Zealand Map Grid 
 * projection parameters.  If any errors occur, the error code(s) are returned 
 * by the function, otherwise NZMG_NO_ERROR is returned.
 *
 *    Latitude          : Latitude (phi), in radians           (input)
 *    Longitude         : Longitude (lambda), in radians       (input)
 *    Easting           : Easting (X), in meters               (output)
 *    Northing          : Northing (Y), in meters              (output)
 */
  Complex Zeta, z;
  int n;
  double dphi;
  double du, dlam;
  long Error_Code = NZMG_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    dphi = (Latitude * (180.0 / M_PI) - NZMG_Origin_Lat) * 3600.0 * 1.0e-5;
    du = A[9];
    for (n = 8; n >= 0; n--)
      du = du * dphi + A[n];
    du *= dphi;
    dlam = Longitude - NZMG_Origin_Long;
    Zeta.real = du;
    Zeta.imag = dlam;
    z.real = B[5].real;
    z.imag = B[5].imag;
    for (n = 4; n >= 0; n--)
    {
      z = Multiply(z, Zeta);
      z = Add(B[n], z);
    }
    z = Multiply(z, Zeta);
    *Easting = (z.imag * NZMG_a) + NZMG_False_Easting;
    *Northing = (z.real * NZMG_a) + NZMG_False_Northing;
  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_NZMG */
long rspfNewZealandMapGridProjection::Convert_NZMG_To_Geodetic(double Easting,
                                                                double Northing,
                                                                double *Latitude,
                                                                double *Longitude)const
{ /* Begin Convert_NZMG_To_Geodetic */
/*
 * The function Convert_NZMG_To_Geodetic converts New Zealand Map Grid projection
 * (easting and northing) coordinates to geodetic (latitude and longitude)
 * coordinates, according to the current ellipsoid and New Zealand Map Grid projection
 * coordinates.  If any errors occur, the error code(s) are returned by the
 * function, otherwise NZMG_NO_ERROR is returned.
 *
 *    Easting           : Easting (X), in meters                  (input)
 *    Northing          : Northing (Y), in meters                 (input)
 *    Latitude          : Latitude (phi), in radians              (output)
 *    Longitude         : Longitude (lambda), in radians          (output)
 */
  int i, n;
  Complex coeff;
  Complex z, Zeta, Zeta_Numer, Zeta_Denom, Zeta_sqr;
  double dphi;
  long Error_Code = NZMG_NO_ERROR; 
  if (!Error_Code)
  { /* no errors */
    z.real = (Northing - NZMG_False_Northing) / NZMG_a;
    z.imag = (Easting - NZMG_False_Easting) / NZMG_a;
    Zeta.real = C[5].real;
    Zeta.imag = C[5].imag;
    for (n = 4; n >= 0; n--)
    {
      Zeta = Multiply(Zeta, z);
      Zeta = Add(C[n], Zeta);
    }
    Zeta = Multiply(Zeta, z);
    for (i = 0; i < 2; i++)
    {
      Zeta_Numer.real = 5.0 * B[5].real;
      Zeta_Numer.imag = 5.0 * B[5].imag;
      Zeta_Denom.real = 6.0 * B[5].real;
      Zeta_Denom.imag = 6.0 * B[5].imag;
      for (n = 4; n >= 1; n--)
      {
        Zeta_Numer = Multiply(Zeta_Numer, Zeta);
        coeff.real = n * B[n].real;
        coeff.imag = n * B[n].imag;
        Zeta_Numer = Add(coeff, Zeta_Numer); 
        Zeta_Denom = Multiply(Zeta_Denom, Zeta);
        coeff.real = (n+1) * B[n].real;
        coeff.imag = (n+1) * B[n].imag;
        Zeta_Denom = Add(coeff, Zeta_Denom);
      }
      Zeta_sqr = Multiply(Zeta, Zeta);
      Zeta_Numer = Multiply(Zeta_Numer, Zeta_sqr);
      Zeta_Numer = Add(z, Zeta_Numer);
      Zeta_Denom = Multiply(Zeta_Denom, Zeta);
      Zeta_Denom = Add(B[0], Zeta_Denom);
      Zeta = Divide(Zeta_Numer, Zeta_Denom);  
    }
    dphi = D[8];
    for (n = 7; n >= 0; n--)
      dphi = dphi * Zeta.real + D[n];
    dphi *= Zeta.real;
      
    *Latitude = NZMG_Origin_Lat + (dphi * 1.0e5 / 3600.0);
    *Latitude *= M_PI / 180.0;
    *Longitude = NZMG_Origin_Long + Zeta.imag;
  }
  return (Error_Code);
} /* END OF Convert_NZMG_To_Geodetic */
