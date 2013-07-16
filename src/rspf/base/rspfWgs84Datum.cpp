//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// Wgs84Datum.  Special hardcoded datum.  It will create a static
// instance of a Wgs84Ellipsoid and set the initial defaults for
// that are specific to a Wgs84Datum
//*******************************************************************
//  $Id: rspfWgs84Datum.cpp 19795 2011-06-30 15:04:48Z gpotts $

#include <rspf/base/rspfWgs84Datum.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <rspf/base/rspfEllipsoid.h>


RTTI_DEF1(rspfWgs84Datum, "rspfWgs84Datum", rspfThreeParamDatum);

rspfWgs84Datum::rspfWgs84Datum()
   :rspfThreeParamDatum("WGE",
                         "World Geodetic System 1984",
                         rspfEllipsoidFactory::instance()->wgs84(),
                         0.0, 0.0, 0.0,
                         -M_PI/2.0, M_PI/2.0, -M_PI, M_PI,
                         0.0, 0.0, 0.0)
{
   if(!ellipsoid())
   {
      //ERROR
   }
}


rspfGpt rspfWgs84Datum::shift(const rspfGpt &aPt)const
{
   const rspfDatum *aDatum = aPt.datum();
   
    if( aPt.datum()->code() == code())
    {
       return rspfGpt(aPt.latd(), aPt.lond(), aPt.height(), this);
    }
   
    if(aDatum)
    {
       return aDatum->shiftToWgs84(aPt);
    }

   return aPt;      
}

rspfGpt rspfWgs84Datum::shiftToWgs84(const rspfGpt &aPt)const
{
   return rspfGpt(aPt.latd(),
                   aPt.lond(),
                   aPt.height(),
                   rspfDatumFactory::instance()->wgs84());
}

rspfGpt rspfWgs84Datum::shiftFromWgs84(const rspfGpt &aPt)const
{
   return aPt;
}
