//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// rspfWgs84Datum.  Special hardcoded datum.  It will create a static
// instance of a Wgs84Ellipsoid and set the initial defaults for
// that are specific to a rspfWgs84Datum
//*******************************************************************
//  $Id: rspfWgs84Datum.h 19795 2011-06-30 15:04:48Z gpotts $

#ifndef rspfWgs84Datum_HEADER
#define rspfWgs84Datum_HEADER
#include <rspf/base/rspfThreeParamDatum.h>

class RSPFDLLEXPORT rspfWgs84Datum : public rspfThreeParamDatum
{
public:
   rspfWgs84Datum();
   virtual ~rspfWgs84Datum(){}
   
   virtual rspfGpt    shift(const rspfGpt    &aPt)const;

   virtual rspfGpt    shiftToWgs84(const rspfGpt &aPt)const;
   virtual rspfGpt    shiftFromWgs84(const rspfGpt&aPt)const;

protected:
 
   TYPE_DATA;
};

#endif
