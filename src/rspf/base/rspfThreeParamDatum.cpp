//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//*******************************************************************
//  $Id: rspfThreeParamDatum.cpp 19795 2011-06-30 15:04:48Z gpotts $

#include <cmath>

#include <rspf/base/rspfThreeParamDatum.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfWgs84Datum.h>
#include <rspf/base/rspfbj54Datum.h>
#include <rspf/base/rspfXiAn80Datum.h>

RTTI_DEF1(rspfThreeParamDatum, "rspfThreeParamDatum", rspfDatum);
rspfGpt rspfThreeParamDatum::shiftToWgs84(const rspfGpt &aPt)const
{
   
   if(rspf::almostEqual(param1(),  0.0)&&
      rspf::almostEqual(param2(), 0.0)&&
      rspf::almostEqual(param3(), 0.0))
   {
      return rspfGpt(aPt.latd(),
                      aPt.lond(),
                      aPt.latd(),
                      rspfGpt().datum());
   }
      
   rspfEcefPoint p1 = aPt;
   rspfEcefPoint p2;
 
   
   if(withinMolodenskyRange(aPt.latd()))
   {
      rspfWgs84Datum wgs84;
      double latin, lonin, hgtin;
      double latout, lonout, hgtout;
      
      double da = wgs84.ellipsoid()->getA() - ellipsoid()->getA();
      double df = wgs84.ellipsoid()->getFlattening() - ellipsoid()->getFlattening();

      latin = aPt.latr();
      lonin = aPt.lonr();
      hgtin = aPt.height();

      if(aPt.isHgtNan())
      {
         hgtin = 0.0;
      }
      molodenskyShift(ellipsoid()->getA(), da, ellipsoid()->getFlattening(), df, param1(), param2(), param3(),
                      latin, lonin, hgtin,
                      latout, lonout, hgtout);
      rspfGpt g;

      g.latr(latout);
      g.lonr(lonout);
      g.height(hgtout);
      g.datum(this);
      return g;
   }
   else
     {
       p2 = rspfEcefPoint(p1.x() + theParam1,
			   p1.y() + theParam2,
			   p1.z() + theParam3);
     }
   
   return rspfGpt(p2); // defaults to WGS84
}

rspfGpt rspfThreeParamDatum::shiftFromWgs84(const rspfGpt &aPt)const
{
   if(rspf::almostEqual(param1(), 0.0)&&
      rspf::almostEqual(param2(), 0.0)&&
      rspf::almostEqual(param3(), 0.0))
   {
      return rspfGpt(aPt.latd(),
                      aPt.lond(),
                      aPt.latd(),
                      this);
   }
   rspfEcefPoint p1=aPt;
   rspfEcefPoint p2=aPt;
   
   if(withinMolodenskyRange(aPt.latd()))
   {
      rspfWgs84Datum wgs84;
      double latin, lonin, hgtin;
      double latout, lonout, hgtout;
      
      double da = ellipsoid()->getA() -  wgs84.ellipsoid()->getA();
      double df = ellipsoid()->getFlattening() - wgs84.ellipsoid()->getFlattening();

      latin = aPt.latr();
      lonin = aPt.lonr();
      hgtin = aPt.height();

      if(aPt.isHgtNan())
      {
         hgtin = 0.0;
      }
      molodenskyShift(wgs84.ellipsoid()->getA(), da, wgs84.ellipsoid()->getFlattening(), df, -param1(), -param2(), -param3(),
                      latin, lonin, hgtin,
                      latout, lonout, hgtout);
      rspfGpt g;

      g.latr(latout);
      g.lonr(lonout);
      g.height(hgtout);
      g.datum(this);
      return g;
   }
   else
   {
      p2 = rspfEcefPoint(p1.x() - theParam1,
                          p1.y() - theParam2,
                          p1.z() - theParam3);
   }
   
   
   return rspfGpt(p2, this);
}


rspfGpt rspfThreeParamDatum::shift(const rspfGpt &aPt)const
{
   const rspfDatum *aDatum = aPt.datum();

   if( code() == aDatum->code())
   {
      return rspfGpt(aPt.latd(), aPt.lond(), aPt.height(), this);
   }
   
   if(aDatum)
   {
      return shiftFromWgs84(aDatum->shiftToWgs84(aPt));
   }

   return aPt;
}

bool rspfThreeParamDatum::isEqualTo(const rspfObject& obj, rspfCompareType compareType)const
{
   const rspfThreeParamDatum* datum = dynamic_cast<const rspfThreeParamDatum*> (&obj);
   
   bool result = datum&&rspfDatum::isEqualTo(obj, compareType);
   
   if(result)
   {
      result = (rspf::almostEqual(theParam1, datum->theParam1)&&
                rspf::almostEqual(theParam2, datum->theParam2)&&
                rspf::almostEqual(theParam3, datum->theParam3));
   }
   
   return result;
}