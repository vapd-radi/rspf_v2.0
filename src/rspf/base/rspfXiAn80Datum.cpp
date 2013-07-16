#include <rspf/base/rspfXiAn80Datum.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfEllipsoidFactory.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfNotifyContext.h>
rspfXiAn80Datum::rspfXiAn80Datum()
   :rspfThreeParamDatum("XiAn80",
                         "XiAn System 1980",
                         rspfEllipsoidFactory::instance()->xian80(),
						 -1, -1, -1,-M_PI, M_PI,
                         M_PI*36/180.0, M_PI*89/180.0, 
                         28, -130, -95)
{
   if(!ellipsoid())
   {
   }
}
