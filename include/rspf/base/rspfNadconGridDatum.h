#ifndef rspfNadconGridDatum_HEADER
#define rspfNadconGridDatum_HEADER

#include <rspf/base/rspfThreeParamDatum.h>
#include <rspf/base/rspfNadconGridFile.h>
#include <rspf/base/rspfDrect.h>

class rspfNadconGridDatum : public rspfThreeParamDatum
{
public:
  rspfNadconGridDatum(const rspfFilename& datumDirectory,
		       const rspfString &code, 
		       const rspfString &name,
		       const rspfEllipsoid* anEllipsoid,
		       double sigmaX,
		       double sigmaY,
		       double sigmaZ,
		       double westLongitude,
		       double eastLongitude,
		       double southLatitude,
		       double northLatitude,
		       double param1,
		       double param2,
		       double param3);

protected:
  mutable rspfDrect theConusRect;
  mutable rspfDrect theHawaiiRect;
  mutable rspfDrect theAlaskaRect;
  mutable rspfDrect theStGeorgeRect;
  mutable rspfDrect theLatStLaurenceRect;
  mutable rspfDrect theStPaulRect;
  mutable rspfDrect thePrviRect;
  
  mutable rspfNadconGridFile theLatGrid;
  mutable rspfNadconGridFile theLonGrid;
  
  mutable rspfDrect    theCurrentGridRect;
  rspfFilename theDatumDirectory;
  
  void checkGrid(const rspfDpt& latLon)const;
   
   TYPE_DATA;   
};

#endif
