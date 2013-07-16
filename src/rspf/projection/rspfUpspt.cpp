#include <rspf/projection/rspfUpspt.h>
#include <rspf/projection/rspfUtmpt.h>
#include <rspf/projection/rspfUps.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/base/rspfDatum.h>
rspfUpspt::rspfUpspt(const rspfGpt &aPt)
{
   convertFromGeodetic(aPt);
}
rspfUpspt::rspfUpspt(const rspfEcefPoint &aPt)
{
   convertFromGeodetic(rspfGpt(aPt));
}
void rspfUpspt::convertFromGeodetic(const rspfGpt &aPt)
{
   const rspfDatum *aDatum = aPt.datum();
   if(aDatum)
   {
      Set_UPS_Parameters(aDatum->ellipsoid()->a(),
                         aDatum->ellipsoid()->b());
      
      Convert_Geodetic_To_UPS(aPt.latr(),
                              aPt.lonr(),
                              &theHemisphere,
                              &theEasting,
                              &theNorthing);
      theDatum = const_cast<rspfDatum*>(aDatum);
   }
   else
   {
   }
}
