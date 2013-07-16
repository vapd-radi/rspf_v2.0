#include <rspf/base/rspfBj54Datum.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfNotifyContext.h>
rspfBj54Datum::rspfBj54Datum()
   :rspfThreeParamDatum("BJ54",
                         "BeiJing System 1954",
                         rspfEllipsoidFactory::instance()->bj54(),
						 -1, -1, -1,-M_PI, M_PI,
                         M_PI*36/180.0, M_PI*89/180.0, 
                         28, -130, -95)
{
   if(!ellipsoid())
   {
   }
}
