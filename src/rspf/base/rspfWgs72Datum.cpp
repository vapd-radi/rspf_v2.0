//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// Wgs72Datum.  Special hardcoded datum.  It will create a static
// instance of a Wgs72Ellipsoid and set the initial defaults for
// that are specific to a Wgs72Datum
//*******************************************************************
//  $Id: rspfWgs72Datum.cpp 20096 2011-09-14 16:44:20Z dburken $

#include <iostream>
using namespace std;
#include <rspf/base/rspfWgs72Datum.h>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfNotifyContext.h>

/***************************************************************************/
/*
 *                               DEFINES FROM GEOTRANS
 */

RTTI_DEF1(rspfWgs72Datum, "rspfWgs72Datum", rspfThreeParamDatum);
rspfWgs72Datum::rspfWgs72Datum()
   :rspfThreeParamDatum("WGD",
                         "World Geodetic System 1972",
                         rspfEllipsoidFactory::instance()->wgs72(),
                         0.0,
                         0.0,
                         0.0,
                         -M_PI/2.0,
                         M_PI/2.0,
                         -M_PI,
                         M_PI,
                         0.0,
                         0.0,
                         0.0)
{
   if(!ellipsoid())
   {
      //ERROR
   }
}

rspfGpt rspfWgs72Datum::shift(const rspfGpt &aPt)const
{
   const rspfDatum *aDatum = aPt.datum();
   
   if( (ellipsoid()->getA()== aPt.datum()->ellipsoid()->getA())&&
       (ellipsoid()->getB()== aPt.datum()->ellipsoid()->getB()))
   {
      return rspfGpt(aPt.latd(), aPt.lond(), aPt.height(), this);
   }
   
   if(aDatum)
   {  
      return shiftFromWgs84(aDatum->shiftToWgs84(aPt));
   }

   return aPt;
}

rspfGpt rspfWgs72Datum::shiftToWgs84(const rspfGpt &aPt)const
{
/* Begin Geodetic_Shift_WGS72_To_WGS84 */
  /*
   *  This function shifts a geodetic coordinate (latitude, longitude in radians
   *  and height in meters) relative to WGS72 to a geodetic coordinate 
   *  (latitude, longitude in radians and height in meters) relative to WGS84.
   *
   *  WGS72_Lat : Latitude in radians relative to WGS72     (input)
   *  WGS72_Lon : Longitude in radians relative to WGS72    (input)
   *  WGS72_Hgt : Height in meters relative to WGS72        (input)
   *  WGS84_Lat : Latitude in radians relative to WGS84     (output)
   *  WGS84_Lon : Longitude in radians relative to WGS84    (output)
   *  WGS84_Hgt : Height in meters  relative to WGS84       (output)
   */
   double Delta_Lat;
   double Delta_Lon;
   double Delta_Hgt;
   double WGS84_a;       /* Semi-major axis of WGS84 ellipsoid               */
   double WGS84_f;       /* Flattening of WGS84 ellipsoid                    */
   double WGS72_a;       /* Semi-major axis of WGS72 ellipsoid               */
   double WGS72_f;       /* Flattening of WGS72 ellipsoid                    */
   double da;            /* WGS84_a - WGS72_a                                */
   double df;            /* WGS84_f - WGS72_f                                */
   double Q;
   double sin_Lat;
   double sin2_Lat;
   
   const rspfDatum *wgs84 = rspfDatumFactory::instance()->wgs84();
   const rspfDatum *wgs72 = rspfDatumFactory::instance()->wgs72();
   const rspfEllipsoid *wgs84Ellipsoid = rspfEllipsoidFactory::instance()->wgs84();
   const rspfEllipsoid *wgs72Ellipsoid = rspfEllipsoidFactory::instance()->wgs72();

   if(!wgs84 || !wgs72 || !wgs72Ellipsoid || !wgs84Ellipsoid)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfWgs72Datum::shiftToWgs84, NULL pointer found and no shift will be performed\n";
      return (aPt);
   }
   
   WGS84_a = wgs84Ellipsoid->a();
   WGS84_f = wgs84Ellipsoid->flattening();
   WGS72_a = wgs72Ellipsoid->a();
   WGS72_f = wgs72Ellipsoid->flattening();
   da = WGS84_a - WGS72_a;
   df = WGS84_f - WGS72_f;
   Q = M_PI /  648000;
   sin_Lat = sin(aPt.latr());
   sin2_Lat = sin_Lat * sin_Lat;
   
   Delta_Lat = (4.5 * cos(aPt.latr())) / (WGS72_a*Q) + (df * sin(2*aPt.latr())) / Q;
   Delta_Lat /= SEC_PER_RAD;
   Delta_Lon = 0.554 / SEC_PER_RAD;
   Delta_Hgt = 4.5 * sin_Lat + WGS72_a * df * sin2_Lat - da + 1.4;

   if(aPt.isHgtNan())
   {
      return rspfGpt(aPt.latd() + Delta_Lat*DEG_PER_RAD,
                      aPt.lond() + Delta_Lon*DEG_PER_RAD,
                      Delta_Hgt,
                      wgs84);
   }
   return rspfGpt(aPt.latd() + Delta_Lat*DEG_PER_RAD,
                   aPt.lond() + Delta_Lon*DEG_PER_RAD,
                   aPt.height() + Delta_Hgt,
                   wgs84);
   /* End Geodetic_Shift_WGS72_To_WGS84 */
}

rspfGpt rspfWgs72Datum::shiftFromWgs84(const rspfGpt &aPt)const
{
 /* Begin Geodetic_Shift_WGS84_To_WGS72 */
  /*
   *  This function shifts a geodetic coordinate (latitude, longitude in radians
   *  and height in meters) relative to WGS84 to a geodetic coordinate 
   *  (latitude, longitude in radians and height in meters) relative to WGS72.
   *
   *  WGS84_Lat : Latitude in radians relative to WGS84     (input)
   *  WGS84_Lon : Longitude in radians relative to WGS84    (input)
   *  WGS84_Hgt : Height in meters  relative to WGS84       (input)
   *  WGS72_Lat : Latitude in radians relative to WGS72     (output)
   *  WGS72_Lon : Longitude in radians relative to WGS72    (output)
   *  WGS72_Hgt : Height in meters relative to WGS72        (output)
   */
   double Delta_Lat;
   double Delta_Lon;
   double Delta_Hgt;
   double WGS84_a;       /* Semi-major axis of WGS84 ellipsoid               */
   double WGS84_f;       /* Flattening of WGS84 ellipsoid                    */
   double WGS72_a;       /* Semi-major axis of WGS72 ellipsoid               */
   double WGS72_f;       /* Flattening of WGS72 ellipsoid                    */
   double da;            /* WGS72_a - WGS84_a                                */
   double df;            /* WGS72_f - WGS84_f                                */
   double Q;
   double sin_Lat;
   double sin2_Lat;
   const rspfDatum *wgs84 = rspfDatumFactory::instance()->wgs84();
   const rspfDatum *wgs72 = rspfDatumFactory::instance()->wgs72();
   const rspfEllipsoid *wgs84Ellipsoid = rspfEllipsoidFactory::instance()->wgs84();
   const rspfEllipsoid *wgs72Ellipsoid = rspfEllipsoidFactory::instance()->wgs72();
   
   if(!wgs84 || !wgs72 || !wgs72Ellipsoid || !wgs84Ellipsoid)
   {
      rspfNotify(rspfNotifyLevel_WARN) << "rspfWgs72Datum::shiftFromWgs84, NULL pointer found and no shift will be performed\n";
      
      return (aPt);
   }
   
   WGS84_a = wgs84Ellipsoid->a();
   WGS84_f = wgs84Ellipsoid->flattening();
   WGS72_a = wgs72Ellipsoid->a();
   WGS72_f = wgs72Ellipsoid->flattening();
   
   da = WGS72_a - WGS84_a;
   df = WGS72_f - WGS84_f;
   Q = M_PI / 648000;
   sin_Lat = sin(aPt.latr());
   sin2_Lat = sin_Lat * sin_Lat;

   Delta_Lat = (-4.5 * cos(aPt.latr())) / (WGS84_a*Q)
               + (df * sin(2*aPt.latr())) / Q;
   Delta_Lat /= SEC_PER_RAD;
   Delta_Lon = -0.554 / SEC_PER_RAD;
   Delta_Hgt = -4.5 * sin_Lat + WGS84_a * df * sin2_Lat - da - 1.4;

   if(aPt.isHgtNan())
   {
      
      return rspfGpt(aPt.latd() + Delta_Lat*DEG_PER_RAD,
                      aPt.lond() + Delta_Lon*DEG_PER_RAD,
                      Delta_Hgt,
                      this);
   }
   return rspfGpt(aPt.latd() + Delta_Lat*DEG_PER_RAD,
                   aPt.lond() + Delta_Lon*DEG_PER_RAD,
                   aPt.height() + Delta_Hgt,
                   this);
}
