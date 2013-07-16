#include <cmath>
using namespace std;
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/base/rspfKeywordNames.h>
RTTI_DEF1(rspfTransMercatorProjection, "rspfTransMercatorProjection", rspfMapProjection)
/******************************* DEFINES *********************************/
#define TRANMERC_NO_ERROR           0x0000
#define TRANMERC_LAT_ERROR          0x0001
#define TRANMERC_LON_ERROR          0x0002
#define TRANMERC_EASTING_ERROR      0x0004
#define TRANMERC_NORTHING_ERROR     0x0008
#define TRANMERC_ORIGIN_LAT_ERROR   0x0010
#define TRANMERC_CENT_MER_ERROR     0x0020
#define TRANMERC_A_ERROR            0x0040
#define TRANMERC_B_ERROR            0x0080
#define TRANMERC_A_LESS_B_ERROR     0x0100
#define TRANMERC_SCALE_FACTOR_ERROR 0x0200
#define TRANMERC_LON_WARNING        0x0400
#define MAX_LAT         ((M_PI * 90.0)/180.0)    /* 90 degrees in radians */
#define MAX_DELTA_LONG  ((M_PI * 90.0)/180.0)    /* 90 degrees in radians */
#define MIN_SCALE_FACTOR  0.3
#define MAX_SCALE_FACTOR  3.0
#define SPHTMD(Latitude) ((double) (TranMerc_ap * Latitude \
      - TranMerc_bp * sin(2.e0 * Latitude) + TranMerc_cp * sin(4.e0 * Latitude) \
      - TranMerc_dp * sin(6.e0 * Latitude) + TranMerc_ep * sin(8.e0 * Latitude) ) )
#define SPHSN(Latitude) ((double) (getA() / sqrt( 1.e0 - TranMerc_es * \
      pow(sin(Latitude), 2))))
#define SPHSR(Latitude) ((double) (getA() * (1.e0 - TranMerc_es) / \
    pow(DENOM(Latitude), 3)))
#define DENOM(Latitude) ((double) (sqrt(1.e0 - TranMerc_es * pow(sin(Latitude),2))))
rspfTransMercatorProjection::rspfTransMercatorProjection(const rspfEllipsoid& ellipsoid,
                                                           const rspfGpt& origin)
   :
      rspfMapProjection(ellipsoid, origin),
      TranMerc_a(6378137.0),
      TranMerc_f(1.0/298.257223563),
      TranMerc_es(0.0066943799901413800),
      TranMerc_ebs(0.0067394967565869),
      TranMerc_Origin_Lat(origin.latr()),
      TranMerc_Origin_Long(origin.lonr()),
      TranMerc_False_Northing(0.0),
      TranMerc_False_Easting(0.0),
      TranMerc_Scale_Factor(1.0),
      TranMerc_ap(6367449.1458008),
      TranMerc_bp(16038.508696861),
      TranMerc_cp(16.832613334334),
      TranMerc_dp(0.021984404273757),
      TranMerc_Delta_Easting(40000000.0),
      TranMerc_Delta_Northing(40000000.0)
{
   update();
}
rspfTransMercatorProjection::rspfTransMercatorProjection(const rspfEllipsoid& ellipsoid,
                                                           const rspfGpt& origin,
                                                           double falseEasting,
                                                           double falseNorthing,
                                                           double scaleFactor)
   :
      rspfMapProjection(ellipsoid, origin),
      TranMerc_a(6378137.0),
      TranMerc_f(1.0/298.257223563),
      TranMerc_es(0.0066943799901413800),
      TranMerc_ebs(0.0067394967565869),
      TranMerc_Origin_Lat(origin.latr()),
      TranMerc_Origin_Long(origin.lonr()),
      TranMerc_False_Northing(falseNorthing),
      TranMerc_False_Easting(falseEasting),
      TranMerc_Scale_Factor(scaleFactor),
      TranMerc_ap(6367449.1458008),
      TranMerc_bp(16038.508696861),
      TranMerc_cp(16.832613334334),
      TranMerc_dp(0.021984404273757),
      TranMerc_Delta_Easting(40000000.0),
      TranMerc_Delta_Northing(40000000.0)
{
   update();
}
void rspfTransMercatorProjection::update()
{
   Set_Transverse_Mercator_Parameters(theEllipsoid.getA(),
                                      theEllipsoid.getFlattening(),
                                      theOrigin.latr(),
                                      theOrigin.lonr(),
                                      TranMerc_False_Easting,
                                      TranMerc_False_Northing,
                                      TranMerc_Scale_Factor);
   theFalseEastingNorthing.x = TranMerc_False_Easting;
   theFalseEastingNorthing.y = TranMerc_False_Northing;
   rspfMapProjection::update();
}
void rspfTransMercatorProjection::setFalseEasting(double falseEasting)
{
   TranMerc_False_Easting  = falseEasting;
   
   update();
}
void rspfTransMercatorProjection::setFalseNorthing(double falseNorthing)
{
   TranMerc_False_Northing = falseNorthing;
   
   update();
}
void rspfTransMercatorProjection::setFalseEastingNorthing(double falseEasting,
                                                      double falseNorthing)
{
   TranMerc_False_Easting  = falseEasting;
   TranMerc_False_Northing = falseNorthing;
   
   update();
}
void rspfTransMercatorProjection::setScaleFactor(double scaleFactor)
{
   TranMerc_Scale_Factor = scaleFactor;
   
   update();
}
void rspfTransMercatorProjection::setParameters(double falseEasting,
                                                   double falseNorthing,
                                                   double scaleFactor)
{
   TranMerc_False_Easting  = falseEasting;
   TranMerc_False_Northing = falseNorthing;
   TranMerc_Scale_Factor   = scaleFactor;
   
   update();
}
void rspfTransMercatorProjection::setDefaults()
{
   TranMerc_False_Easting  = 0.0;
   TranMerc_False_Northing = 0.0;
   TranMerc_Scale_Factor   = 1.0;
   TranMerc_Delta_Easting  = 40000000.0;
   TranMerc_Delta_Northing = 40000000.0;
}
rspfGpt rspfTransMercatorProjection::inverse(const rspfDpt &eastingNorthing)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Transverse_Mercator_To_Geodetic(eastingNorthing.x,
                                           eastingNorthing.y,
                                           &lat,
                                           &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);
}
rspfGpt rspfTransMercatorProjection::inverse_do(const rspfDpt &eastingNorthing,const rspfGpt &latLon)const
{
   double lat = 0.0;
   double lon = 0.0;
   
   Convert_Transverse_Mercator_To_Geodetic(eastingNorthing.x,
                                           eastingNorthing.y,
                                           &lat,
                                           &lon);
   
   return rspfGpt(lat*DEG_PER_RAD, lon*DEG_PER_RAD, 0.0, theDatum);
}

rspfDpt rspfTransMercatorProjection::forward(const rspfGpt &latLon)const
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
   Convert_Geodetic_To_Transverse_Mercator(gpt.latr(),
                                           gpt.lonr(),
                                           &easting,
                                           &northing);
   
   return rspfDpt(easting, northing);
}
bool rspfTransMercatorProjection::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   kwl.add(prefix,
           rspfKeywordNames::SCALE_FACTOR_KW,
           TranMerc_Scale_Factor,
           true);
   
   return rspfMapProjection::saveState(kwl, prefix);
}
bool rspfTransMercatorProjection::loadState(const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   bool flag = rspfMapProjection::loadState(kwl, prefix);
   const char* type          = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   const char* scaleFactor   = kwl.find(prefix, rspfKeywordNames::SCALE_FACTOR_KW);
   
   setDefaults();
   if(rspfString(type) == STATIC_TYPE_NAME(rspfTransMercatorProjection))
   {
      TranMerc_False_Easting  = theFalseEastingNorthing.x;
      TranMerc_False_Northing = theFalseEastingNorthing.y;
      if(scaleFactor)
      {
         double d = rspfString(scaleFactor).toDouble();
         if (d > 0.0) // Check to avoid divide by zero.
         {
            TranMerc_Scale_Factor = d;
         }
      }
   }
   update();
   return flag;
}
/************************************************************************/
/*                              FUNCTIONS     
 *
 */
long rspfTransMercatorProjection::Set_Transverse_Mercator_Parameters(double a,
                                                                      double f,
                                                                      double Origin_Latitude,
                                                                      double Central_Meridian,
                                                                      double False_Easting,
                                                                      double False_Northing,
                                                                      double Scale_Factor)
{ /* BEGIN Set_Tranverse_Mercator_Parameters */
  /*
   * The function Set_Tranverse_Mercator_Parameters receives the ellipsoid
   * parameters and Tranverse Mercator projection parameters as inputs, and
   * sets the corresponding state variables. If any errors occur, the error
   * code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    a                 : Semi-major axis of ellipsoid, in meters    (input)
   *    f                 : Flattening of ellipsoid						         (input)
   *    Origin_Latitude   : Latitude in radians at the origin of the   (input)
   *                         projection
   *    Central_Meridian  : Longitude in radians at the center of the  (input)
   *                         projection
   *    False_Easting     : Easting/X at the center of the projection  (input)
   *    False_Northing    : Northing/Y at the center of the projection (input)
   *    Scale_Factor      : Projection scale factor                    (input) 
   */
  double tn;        /* True Meridianal distance constant  */
  double tn2;
  double tn3;
  double tn4;
  double tn5;
  double dummy_northing;
  double TranMerc_b; /* Semi-minor axis of ellipsoid, in meters */
  long Error_Code = TRANMERC_NO_ERROR;
  if (!Error_Code)
  { /* no errors */
    TranMerc_a = a;
    TranMerc_f = f;
    TranMerc_Origin_Lat = 0;
    TranMerc_Origin_Long = 0;
    TranMerc_False_Northing = 0;
    TranMerc_False_Easting = 0; 
    TranMerc_Scale_Factor = 1;
    /* Eccentricity Squared */
    TranMerc_es = 2 * TranMerc_f - TranMerc_f * TranMerc_f;
    /* Second Eccentricity Squared */
    TranMerc_ebs = (1 / (1 - TranMerc_es)) - 1;
    TranMerc_b = TranMerc_a * (1 - TranMerc_f);    
    /*True meridianal constants  */
    tn = (TranMerc_a - TranMerc_b) / (TranMerc_a + TranMerc_b);
    tn2 = tn * tn;
    tn3 = tn2 * tn;
    tn4 = tn3 * tn;
    tn5 = tn4 * tn;
    TranMerc_ap = TranMerc_a * (1.e0 - tn + 5.e0 * (tn2 - tn3)/4.e0
                                + 81.e0 * (tn4 - tn5)/64.e0 );
    TranMerc_bp = 3.e0 * TranMerc_a * (tn - tn2 + 7.e0 * (tn3 - tn4)
                                       /8.e0 + 55.e0 * tn5/64.e0 )/2.e0;
    TranMerc_cp = 15.e0 * TranMerc_a * (tn2 - tn3 + 3.e0 * (tn4 - tn5 )/4.e0) /16.0;
    TranMerc_dp = 35.e0 * TranMerc_a * (tn3 - tn4 + 11.e0 * tn5 / 16.e0) / 48.e0;
    TranMerc_ep = 315.e0 * TranMerc_a * (tn4 - tn5) / 512.e0;
    Convert_Geodetic_To_Transverse_Mercator(MAX_LAT,
                                            MAX_DELTA_LONG,
                                            &TranMerc_Delta_Easting,
                                            &TranMerc_Delta_Northing);
    Convert_Geodetic_To_Transverse_Mercator(0,
                                            MAX_DELTA_LONG,
                                            &TranMerc_Delta_Easting,
                                            &dummy_northing);
    TranMerc_Origin_Lat = Origin_Latitude;
    if (Central_Meridian > M_PI)
      Central_Meridian -= TWO_PI;
    TranMerc_Origin_Long = Central_Meridian;
    TranMerc_False_Northing = False_Northing;
    TranMerc_False_Easting = False_Easting; 
    TranMerc_Scale_Factor = Scale_Factor;
  } /* END OF if(!Error_Code) */
  return (Error_Code);
}  /* END of Set_Transverse_Mercator_Parameters  */
void rspfTransMercatorProjection::Get_Transverse_Mercator_Parameters(double *a,
                                                                      double *f,
                                                                      double *Origin_Latitude,
                                                                      double *Central_Meridian,
                                                                      double *False_Easting,
                                                                      double *False_Northing,
                                                                      double *Scale_Factor)const
{ /* BEGIN Get_Tranverse_Mercator_Parameters  */
  /*
   * The function Get_Transverse_Mercator_Parameters returns the current
   * ellipsoid and Transverse Mercator projection parameters.
   *
   *    a                 : Semi-major axis of ellipsoid, in meters    (output)
   *    f                 : Flattening of ellipsoid						         (output)
   *    Origin_Latitude   : Latitude in radians at the origin of the   (output)
   *                         projection
   *    Central_Meridian  : Longitude in radians at the center of the  (output)
   *                         projection
   *    False_Easting     : Easting/X at the center of the projection  (output)
   *    False_Northing    : Northing/Y at the center of the projection (output)
   *    Scale_Factor      : Projection scale factor                    (output) 
   */
  *a = TranMerc_a;
  *f = TranMerc_f;
  *Origin_Latitude = TranMerc_Origin_Lat;
  *Central_Meridian = TranMerc_Origin_Long;
  *False_Easting = TranMerc_False_Easting;
  *False_Northing = TranMerc_False_Northing;
  *Scale_Factor = TranMerc_Scale_Factor;
  
  return;
} /* END OF Get_Tranverse_Mercator_Parameters */
long rspfTransMercatorProjection::Convert_Geodetic_To_Transverse_Mercator (double Latitude,
                                                                            double Longitude,
                                                                            double *Easting,
                                                                            double *Northing)const
   
{      /* BEGIN Convert_Geodetic_To_Transverse_Mercator */
  /*
   * The function Convert_Geodetic_To_Transverse_Mercator converts geodetic
   * (latitude and longitude) coordinates to Transverse Mercator projection
   * (easting and northing) coordinates, according to the current ellipsoid
   * and Transverse Mercator projection coordinates.  If any errors occur, the
   * error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    Latitude      : Latitude in radians                         (input)
   *    Longitude     : Longitude in radians                        (input)
   *    Easting       : Easting/X in meters                         (output)
   *    Northing      : Northing/Y in meters                        (output)
   */
  double c;       /* Cosine of latitude                          */
  double c2;
  double c3;
  double c5;
  double c7;
  double dlam;    /* Delta longitude - Difference in Longitude       */
  double eta;     /* constant - TranMerc_ebs *c *c                   */
  double eta2;
  double eta3;
  double eta4;
  double s;       /* Sine of latitude                        */
  double sn;      /* Radius of curvature in the prime vertical       */
  double t;       /* Tangent of latitude                             */
  double tan2;
  double tan3;
  double tan4;
  double tan5;
  double tan6;
  double t1;      /* Term in coordinate conversion formula - GP to Y */
  double t2;      /* Term in coordinate conversion formula - GP to Y */
  double t3;      /* Term in coordinate conversion formula - GP to Y */
  double t4;      /* Term in coordinate conversion formula - GP to Y */
  double t5;      /* Term in coordinate conversion formula - GP to Y */
  double t6;      /* Term in coordinate conversion formula - GP to Y */
  double t7;      /* Term in coordinate conversion formula - GP to Y */
  double t8;      /* Term in coordinate conversion formula - GP to Y */
  double t9;      /* Term in coordinate conversion formula - GP to Y */
  double tmd;     /* True Meridional distance                        */
  double tmdo;    /* True Meridional distance for latitude of origin */
  long    Error_Code = TRANMERC_NO_ERROR;
   if (Longitude > M_PI)
     Longitude -= TWO_PI;
  if (!Error_Code)
  { /* no errors */
    /* 
     *  Delta Longitude
     */
    dlam = Longitude - TranMerc_Origin_Long;
     if (dlam > M_PI)
       dlam -= TWO_PI;
     if (dlam < -M_PI)
       dlam += TWO_PI;
     if (fabs(dlam) < 2.e-10)
       dlam = 0.0;
    s = sin(Latitude);
    c = cos(Latitude);
    c2 = c * c;
    c3 = c2 * c;
    c5 = c3 * c2;
    c7 = c5 * c2;
    t = tan (Latitude);
    tan2 = t * t;
    tan3 = tan2 * t;
    tan4 = tan3 * t;
    tan5 = tan4 * t;
    tan6 = tan5 * t;
    eta = TranMerc_ebs * c2;
    eta2 = eta * eta;
    eta3 = eta2 * eta;
    eta4 = eta3 * eta;
    /* radius of curvature in prime vertical */
    sn = SPHSN(Latitude);
    /* True Meridianal Distances */
    tmd = SPHTMD(Latitude);
    /*  Origin  */
    tmdo = SPHTMD (TranMerc_Origin_Lat);
    /* northing */
    t1 = (tmd - tmdo) * TranMerc_Scale_Factor;
    t2 = sn * s * c * TranMerc_Scale_Factor/ 2.e0;
    t3 = sn * s * c3 * TranMerc_Scale_Factor * (5.e0 - tan2 + 9.e0 * eta 
                                                + 4.e0 * eta2) /24.e0; 
    t4 = sn * s * c5 * TranMerc_Scale_Factor * (61.e0 - 58.e0 * tan2
                                                + tan4 + 270.e0 * eta - 330.e0 * tan2 * eta + 445.e0 * eta2
                                                + 324.e0 * eta3 -680.e0 * tan2 * eta2 + 88.e0 * eta4 
                                                -600.e0 * tan2 * eta3 - 192.e0 * tan2 * eta4) / 720.e0;
    t5 = sn * s * c7 * TranMerc_Scale_Factor * (1385.e0 - 3111.e0 * 
                                                tan2 + 543.e0 * tan4 - tan6) / 40320.e0;
    *Northing = TranMerc_False_Northing + t1 + pow(dlam,2.e0) * t2
                + pow(dlam,4.e0) * t3 + pow(dlam,6.e0) * t4
                + pow(dlam,8.e0) * t5; 
    /* Easting */
    t6 = sn * c * TranMerc_Scale_Factor;
    t7 = sn * c3 * TranMerc_Scale_Factor * (1.e0 - tan2 + eta ) /6.e0;
    t8 = sn * c5 * TranMerc_Scale_Factor * (5.e0 - 18.e0 * tan2 + tan4
                                            + 14.e0 * eta - 58.e0 * tan2 * eta + 13.e0 * eta2 + 4.e0 * eta3 
                                            - 64.e0 * tan2 * eta2 - 24.e0 * tan2 * eta3 )/ 120.e0;
    t9 = sn * c7 * TranMerc_Scale_Factor * ( 61.e0 - 479.e0 * tan2
                                             + 179.e0 * tan4 - tan6 ) /5040.e0;
    *Easting = TranMerc_False_Easting + dlam * t6 + pow(dlam,3.e0) * t7 
               + pow(dlam,5.e0) * t8 + pow(dlam,7.e0) * t9;
  }
  return (Error_Code);
} /* END OF Convert_Geodetic_To_Transverse_Mercator */
long rspfTransMercatorProjection::Convert_Transverse_Mercator_To_Geodetic (double Easting,
                                                                            double Northing,
                                                                            double *Latitude,
                                                                            double *Longitude)const
{      /* BEGIN Convert_Transverse_Mercator_To_Geodetic */
  /*
   * The function Convert_Transverse_Mercator_To_Geodetic converts Transverse
   * Mercator projection (easting and northing) coordinates to geodetic
   * (latitude and longitude) coordinates, according to the current ellipsoid
   * and Transverse Mercator projection parameters.  If any errors occur, the
   * error code(s) are returned by the function, otherwise TRANMERC_NO_ERROR is
   * returned.
   *
   *    Easting       : Easting/X in meters                         (input)
   *    Northing      : Northing/Y in meters                        (input)
   *    Latitude      : Latitude in radians                         (output)
   *    Longitude     : Longitude in radians                        (output)
   */
  double c;       /* Cosine of latitude                          */
  double de;      /* Delta easting - Difference in Easting (Easting-Fe)    */
  double dlam;    /* Delta longitude - Difference in Longitude       */
  double eta;     /* constant - TranMerc_ebs *c *c                   */
  double eta2;
  double eta3;
  double eta4;
  double ftphi;   /* Footpoint latitude                              */
  int    i;       /* Loop iterator                   */
  double s;       /* Sine of latitude                        */
  double sn;      /* Radius of curvature in the prime vertical       */
  double sr;      /* Radius of curvature in the meridian             */
  double t;       /* Tangent of latitude                             */
  double tan2;
  double tan4;
  double t10;     /* Term in coordinate conversion formula - GP to Y */
  double t11;     /* Term in coordinate conversion formula - GP to Y */
  double t12;     /* Term in coordinate conversion formula - GP to Y */
  double t13;     /* Term in coordinate conversion formula - GP to Y */
  double t14;     /* Term in coordinate conversion formula - GP to Y */
  double t15;     /* Term in coordinate conversion formula - GP to Y */
  double t16;     /* Term in coordinate conversion formula - GP to Y */
  double t17;     /* Term in coordinate conversion formula - GP to Y */
  double tmd;     /* True Meridional distance                        */
  double tmdo;    /* True Meridional distance for latitude of origin */
  long Error_Code = TRANMERC_NO_ERROR;
  if (!Error_Code)
  {
    /* True Meridional Distances for latitude of origin */
    tmdo = SPHTMD(TranMerc_Origin_Lat);
    /*  Origin  */
    tmd = tmdo +  (Northing - TranMerc_False_Northing) / TranMerc_Scale_Factor; 
    /* First Estimate */
    sr = SPHSR(0.e0);
    ftphi = tmd/sr;
    for (i = 0; i < 5 ; i++)
    {
      t10 = SPHTMD (ftphi);
      sr = SPHSR(ftphi);
      ftphi = ftphi + (tmd - t10) / sr;
    }
    /* Radius of Curvature in the meridian */
    sr = SPHSR(ftphi);
    /* Radius of Curvature in the meridian */
    sn = SPHSN(ftphi);
    /* Sine Cosine terms */
    s = sin(ftphi);
    c = cos(ftphi);
    /* Tangent Value  */
    t = tan(ftphi);
    tan2 = t * t;
    tan4 = tan2 * tan2;
    eta = TranMerc_ebs * pow(c,2);
    eta2 = eta * eta;
    eta3 = eta2 * eta;
    eta4 = eta3 * eta;
    de = Easting - TranMerc_False_Easting;
    if (fabs(de) < 0.0001)
      de = 0.0;
    /* Latitude */
    t10 = t / (2.e0 * sr * sn * pow(TranMerc_Scale_Factor, 2));
    t11 = t * (5.e0  + 3.e0 * tan2 + eta - 4.e0 * pow(eta,2)
               - 9.e0 * tan2 * eta) / (24.e0 * sr * pow(sn,3) 
                                       * pow(TranMerc_Scale_Factor,4));
    t12 = t * (61.e0 + 90.e0 * tan2 + 46.e0 * eta + 45.E0 * tan4
               - 252.e0 * tan2 * eta  - 3.e0 * eta2 + 100.e0 
               * eta3 - 66.e0 * tan2 * eta2 - 90.e0 * tan4
               * eta + 88.e0 * eta4 + 225.e0 * tan4 * eta2
               + 84.e0 * tan2* eta3 - 192.e0 * tan2 * eta4)
          / ( 720.e0 * sr * pow(sn,5) * pow(TranMerc_Scale_Factor, 6) );
    t13 = t * ( 1385.e0 + 3633.e0 * tan2 + 4095.e0 * tan4 + 1575.e0 
                * pow(t,6))/ (40320.e0 * sr * pow(sn,7) * pow(TranMerc_Scale_Factor,8));
    *Latitude = ftphi - pow(de,2) * t10 + pow(de,4) * t11 - pow(de,6) * t12 
                + pow(de,8) * t13;
    t14 = 1.e0 / (sn * c * TranMerc_Scale_Factor);
    t15 = (1.e0 + 2.e0 * tan2 + eta) / (6.e0 * pow(sn,3) * c * 
                                        pow(TranMerc_Scale_Factor,3));
    t16 = (5.e0 + 6.e0 * eta + 28.e0 * tan2 - 3.e0 * eta2
           + 8.e0 * tan2 * eta + 24.e0 * tan4 - 4.e0 
           * eta3 + 4.e0 * tan2 * eta2 + 24.e0 
           * tan2 * eta3) / (120.e0 * pow(sn,5) * c  
                             * pow(TranMerc_Scale_Factor,5));
    t17 = (61.e0 +  662.e0 * tan2 + 1320.e0 * tan4 + 720.e0 
           * pow(t,6)) / (5040.e0 * pow(sn,7) * c 
                          * pow(TranMerc_Scale_Factor,7));
    /* Difference in Longitude */
    dlam = de * t14 - pow(de,3) * t15 + pow(de,5) * t16 - pow(de,7) * t17;
    /* Longitude */
    (*Longitude) = TranMerc_Origin_Long + dlam;
    while (*Latitude > (90.0 * RAD_PER_DEG))
    {
      *Latitude = M_PI - *Latitude;
      *Longitude += M_PI;
      if (*Longitude > M_PI)
        *Longitude -= TWO_PI;
    }
     while (*Latitude < (-90.0 * RAD_PER_DEG))
     {
       *Latitude = - (*Latitude + M_PI);
       *Longitude += M_PI;
       if (*Longitude > M_PI)
         *Longitude -= TWO_PI;
     }
     if (*Longitude > TWO_PI)
       *Longitude -= TWO_PI;
     if (*Longitude < -M_PI)
       *Longitude += TWO_PI;
  }
  return (Error_Code);
} /* END OF Convert_Transverse_Mercator_To_Geodetic */
std::ostream& rspfTransMercatorProjection::print(std::ostream& out) const
{
   out << setiosflags(ios::fixed) << setprecision(15)
       << "// rspfTransMercatorProjection::print\n"
       << rspfKeywordNames::SCALE_FACTOR_KW << ":  " << TranMerc_Scale_Factor
       << endl;
   return rspfMapProjection::print(out);
}
bool rspfTransMercatorProjection::operator==(const rspfProjection& proj) const
{
   if (!rspfMapProjection::operator==(proj))
      return false;
   rspfTransMercatorProjection* p = PTR_CAST(rspfTransMercatorProjection, &proj);
   if (!p) return false;
   if (!rspf::almostEqual(TranMerc_Scale_Factor,p->TranMerc_Scale_Factor)) return false;
   return true;
}

void rspfTransMercatorProjection::setZone(const rspfGpt& ground)
{
   theZone = computeZone(ground);
   theOrigin.lond(computeZoneMeridian(theZone));
   theOrigin.latd(0.0);
   TranMerc_Origin_Long = theOrigin.lonr();
}
void rspfTransMercatorProjection::setZone(rspf_int32 zone)
{
   if( (zone < 1) || (zone > 60) )
   {
      theZone = computeZone(theOrigin);
   }
   else
   {
      theZone = zone;
   }
   theOrigin.lond(computeZoneMeridian(theZone));
   theOrigin.latd(0);
   TranMerc_Origin_Long = theOrigin.lonr();
}
rspf_int32 rspfTransMercatorProjection::computeZone(const rspfGpt& ground)
{
   rspf_int32 result = 0;
   double longitude = ground.lonr();
   double lat_Degrees  = (rspf_int32)( (ground.latd()) + 0.00000005);
   double long_Degrees = (rspf_int32)( (ground.lond()) + 0.00000005);
   
   if (longitude < M_PI)
      result = (rspf_int32)( (31 + ((180 * longitude) / (6 * M_PI)) ) + 0.00000005);
   else
      result = (rspf_int32)( (((180 * longitude) / (6 * M_PI)) - 29) + 0.00000005);
   if (result > 60)
      result = 1;
    /* UTM special cases */
   if ((lat_Degrees > 55) && (lat_Degrees < 64) && (long_Degrees > -1)
       && (long_Degrees < 3))
      result = 31;
   if ((lat_Degrees > 55) && (lat_Degrees < 64) && (long_Degrees > 2)
       && (long_Degrees < 12))
      result = 32;
   if ((lat_Degrees > 71) && (long_Degrees > -1) && (long_Degrees < 9))
      result = 31;
   if ((lat_Degrees > 71) && (long_Degrees > 8) && (long_Degrees < 21))
      result = 33;
   if ((lat_Degrees > 71) && (long_Degrees > 20) && (long_Degrees < 33))
      result = 35;
   if ((lat_Degrees > 71) && (long_Degrees > 32) && (long_Degrees < 42))
      result = 37;
   return result-30;
}
double rspfTransMercatorProjection::computeZoneMeridian(rspf_int32 zone)
{
   return (6.0 * zone - 3.0);;
}

