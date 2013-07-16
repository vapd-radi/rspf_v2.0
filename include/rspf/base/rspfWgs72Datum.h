//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
// Description:
//
// Wgs72Datum.  Special hardcoded datum.  It will create a static
// instance of a Wgs72Ellipsoid and set the initial defaults for
// that are specific to a Wgs72Datum
//*******************************************************************
//  $ID$
#ifndef rspfWgs72Datum_HEADER
#define rspfWgs72Datum_HEADER
#include <rspf/base/rspfThreeParamDatum.h>

class RSPFDLLEXPORT rspfWgs72Datum : public rspfThreeParamDatum
{
public:
   rspfWgs72Datum();

   virtual ~rspfWgs72Datum(){}
   virtual rspfGpt    shift(const rspfGpt    &aPt)const;
   virtual rspfGpt    shiftToWgs84(const rspfGpt &aPt)const;
   virtual rspfGpt    shiftFromWgs84(const rspfGpt &aPt)const;

   TYPE_DATA;   
};

#endif
