#include <rspf/base/rspfNadconNarDatum.h>

RTTI_DEF1(rspfNadconNarDatum, "rspfNadconNarDatum", rspfNadconGridDatum);
rspfNadconNarDatum::rspfNadconNarDatum(const rspfFilename& nadconDirectory)
   :
      rspfNadconGridDatum(nadconDirectory,
                           rspfString("NAR"),
                           rspfString("NORTH AMERICAN 1983 NADCON, CONUS GRID"),
                           rspfEllipsoidFactory::instance()->create(rspfString("RF")),
                           2.000000000000000, 2.000000000000000, 2.000000000000000, 
                           -135.0, -60.0, 15.0, 60.0,
                           0.000000000000000, 0.000000000000000, 0.000000000000000)
{
}

rspfGpt rspfNadconNarDatum::shift(const rspfGpt    &aPt)const
{
  const rspfDatum* datum = aPt.datum();
  rspfString code = datum->code();
  rspfString subCode(code.begin(),
		      code.begin() + 3);
  if(subCode == "NAR")
  {
     return aPt;
  }
  else
  {
     if(subCode == "NAS")
     {
	checkGrid(aPt);
	if(!theLatGrid.getFileOkFlag()||
	   !theLonGrid.getFileOkFlag())
        {
           return rspfThreeParamDatum::shift(aPt);
        }
	
        double shiftLat = theLatGrid.getShiftAtLatLon(aPt.latd(), aPt.lond());
        double shiftLon = theLonGrid.getShiftAtLatLon(aPt.latd(), aPt.lond());
        
        if( (rspf::isnan(shiftLat)) || (rspf::isnan(shiftLon)) )
        {
           return rspfThreeParamDatum::shift(aPt);
        }
        else
        {
           // Note the shifts are stored in the file
           // as seconds.
           //
           // convert the seconds into decimal degrees.  
           //
           shiftLat /= 3600.0;
           shiftLon /= 3600.0;
           return rspfGpt(aPt.latd() + shiftLat,
                           aPt.lond() - shiftLon,
                           aPt.height(),
                           this);
        }
     }
     else
     {
        return rspfThreeParamDatum::shift(aPt);
     }
  }
  
  return rspfThreeParamDatum::shift(aPt);
}
