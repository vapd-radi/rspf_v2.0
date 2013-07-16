//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// SevenParamDatum. This is a base class for all
// seven param datums.
//*******************************************************************
//  $Id: rspfSevenParamDatum.cpp 19795 2011-06-30 15:04:48Z gpotts $

#include <rspf/base/rspfSevenParamDatum.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfWgs84Datum.h>
#include <rspf/base/rspfbj54Datum.h>
#include <rspf/base/rspfXiAn80Datum.h>
RTTI_DEF1(rspfSevenParamDatum, "rspfSevenParamDatum", rspfDatum);
rspfGpt rspfSevenParamDatum::shift(const rspfGpt &aPt)const
{
   const rspfDatum* aDatum = aPt.datum();

   if( (ellipsoid()->getA()== aPt.datum()->ellipsoid()->getA())&&
       (ellipsoid()->getB()== aPt.datum()->ellipsoid()->getB()))
   {
      return rspfGpt(aPt.latd(), aPt.lond(), aPt.height(), this);
   }
   
   if(aDatum)
   {
      return shiftFromWgs84( aDatum->shiftToWgs84(aPt) );
   }

   return aPt;
}

rspfGpt rspfSevenParamDatum::shiftToWgs84(const rspfGpt &aPt)const
{
   rspfEcefPoint p1 = aPt;
   rspfEcefPoint p2;

//    if(aPt.isHgtNan())
//    {
//       p1 = rspfEcefPoint(rspfGpt(aPt.latd(),
//                                    aPt.lond(),
//                                    0,
//                                    aPt.datum()));
//    }
   
//    if(withinMolodenskyRange(aPt.latd()))
//    {
//       rspfWgs84Datum wgs84;
//       double latin, lonin, hgtin;
//       double latout, lonout, hgtout;
      
//       double a = wgs84.ellipsoid()->getA();
//       double f = wgs84.ellipsoid()->getFlattening();
//       double da = a - ellipsoid()->getA();
//       double df = f - ellipsoid()->getFlattening();
      
//       latin = aPt.latr();
//       lonin = aPt.lonr();
//       hgtin = aPt.height();

//       if(aPt.isHgtNan())
//       {
//          hgtin = 0.0;
//       }
//       molodenskyShift(a, da, f, df, param1(), param2(), param3(),
//                       latin, lonin, hgtin,
//                       latout, lonout, hgtout);
      
//       rspfGpt g;

//       g.latr(latout);
//       g.lonr(lonout);
//       g.height(hgtout);
//       if(aPt.isHgtNan())
//       {
//          g.height(RSPF_DBL_NAN);
//       }
//       return g;
//    }
//    else
   {
      //this is straight from Geotrans
      p2.x() = p1.x() + theParam1 + theParam6*p1.y() -
               theParam5*p1.z() + theParam7*p1.x();
      p2.y() = p1.y() + theParam2 - theParam6*p1.x()+
               theParam4*p1.z() + theParam7*p1.y();
      p2.z() = p1.z() + theParam3 + theParam5*p1.x() -
               theParam4*p1.y() + theParam7*p1.z();
   }
   
   return rspfGpt(p2); // defaults to WGS84
}

rspfGpt rspfSevenParamDatum::shiftFromWgs84(const rspfGpt &aPt)const
{      
   rspfEcefPoint p1;
   rspfEcefPoint p2;

   if(aPt.isHgtNan())
   {
      p1 = rspfEcefPoint(rspfGpt(aPt.latd(),
                                   aPt.lond(),
                                   0,
                                   aPt.datum()));
   }
   else
   {
      p1 = rspfEcefPoint(aPt);
   }
//    if(withinMolodenskyRange(aPt.latd()))
//    {
//       rspfWgs84Datum wgs84;
//       double latin, lonin, hgtin;
//       double latout, lonout, hgtout;
      
//       double a = wgs84.ellipsoid()->getA();
//       double f = wgs84.ellipsoid()->getFlattening();
//       double da = ellipsoid()->getA() - a;
//       double df = ellipsoid()->getFlattening() - f;

//       latin = aPt.latr();
//       lonin = aPt.lonr();
//       hgtin = aPt.height();

//       if(aPt.isHgtNan())
//       {
//          hgtin = 0.0;
//       }
//       molodenskyShift(a, da, f, df, -param1(), -param2(), -param3(),
//                       latin, lonin, hgtin,
//                       latout, lonout, hgtout);
//       rspfGpt g;

//       g.latr(latout);
//       g.lonr(lonout);
//       g.height(hgtout);
//       g.datum(this);
//       return g;
//    }
//    else
//    {
      p2.x() = p1.x() - theParam1 - theParam6 * p1.y()
               + theParam5 * p1.z() - theParam7 * p1.x();
      p2.y() = p1.y() - theParam2 + theParam6 * p1.x()
               - theParam4 *p1.z() - theParam7 * p1.y();

      p2.z() = p1.z() - theParam3 - theParam5 * p1.x()
               + theParam4 * p1.y() - theParam7 * p1.z();
//   }
   
   return rspfGpt(p2, this);
}

bool rspfSevenParamDatum::isEqualTo(const rspfObject& obj, rspfCompareType compareType)const
{
   const rspfSevenParamDatum* datum = dynamic_cast<const rspfSevenParamDatum*> (&obj);
   
   bool result = datum&&rspfDatum::isEqualTo(obj, compareType);
   
   if(result)
   {
      result = (rspf::almostEqual(theParam1, datum->theParam1)&&
                rspf::almostEqual(theParam2, datum->theParam2)&&
                rspf::almostEqual(theParam3, datum->theParam3)&&
                rspf::almostEqual(theParam4, datum->theParam4)&&
                rspf::almostEqual(theParam5, datum->theParam5)&&
                rspf::almostEqual(theParam6, datum->theParam6)&&
                rspf::almostEqual(theParam7, datum->theParam7)
                );
   }
   
   return result;
}
