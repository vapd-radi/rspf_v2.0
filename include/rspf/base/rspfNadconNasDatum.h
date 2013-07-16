#ifndef rspfNadconNasDatum_HEADER
#define rspfNadconNasDatum_HEADER

#include <rspf/base/rspfNadconGridDatum.h>
#include <rspf/base/rspfNadconGridFile.h>

class rspfNadconNasDatum : public rspfNadconGridDatum
{
public:
   rspfNadconNasDatum(const rspfFilename& nadconDirectory);

   // Argument holds the source point and datum.  Returns another
   // point with this datum.
   //
   virtual rspfGpt shift(const rspfGpt    &aPt)const;
   TYPE_DATA;
};

#endif
