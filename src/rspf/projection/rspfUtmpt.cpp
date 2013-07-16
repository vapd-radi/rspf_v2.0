#include <rspf/projection/rspfUtmpt.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfEcefPoint.h>
#include <rspf/base/rspfDatum.h>
#include <rspf/base/rspfEllipsoid.h>
#include <rspf/projection/rspfUtm.h>
rspfUtmpt::rspfUtmpt(const rspfGpt &aPt)
{
   convertFromGround(aPt);
}
rspfUtmpt::rspfUtmpt(const rspfEcefPoint &aPt)
{
   convertFromGround(rspfGpt(aPt));
}
rspfUtmpt::rspfUtmpt(long         zone,
                       char         hemisphere,  // N or S
                       double       easting,
                       double       northing,
                       const rspfDatum* datum)
   :
      theZone       (zone),
      theHemisphere (hemisphere),
      theEasting    (easting),
      theNorthing   (northing),
      theDatum      (datum)
{}
void rspfUtmpt::convertFromGround(const rspfGpt &aPt)
{
   const rspfDatum *aDatum = aPt.datum();
   if(aDatum)
   {
      Set_UTM_Parameters(aDatum->ellipsoid()->a(), aDatum->ellipsoid()->flattening(), 0);
      Convert_Geodetic_To_UTM(aPt.latr(),
                              aPt.lonr(),
                              &theZone,
                              &theHemisphere,
                              &theEasting,
                              &theNorthing);
      theDatum = aDatum;
   }
   else
   {
   }
}
void rspfUtmpt::convertToGround(rspfGpt &aPt)
{
   Set_UTM_Parameters( theDatum->ellipsoid()->a(),
                       theDatum->ellipsoid()->flattening(),
                       0 );
   
   double latitude  = 0.0;
   double longitude = 0.0;
   Convert_UTM_To_Geodetic( zone(),
                            hemisphere(),
                            easting(),
                            northing(),
                            &latitude,
                            &longitude );
   aPt.latr(latitude);
   aPt.lonr(longitude);
   aPt.datum(theDatum);
}
