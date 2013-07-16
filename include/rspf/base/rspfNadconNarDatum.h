#ifndef rspfNadconNarDatum_HEADER
#define rspfNadconNarDatum_HEADER

#include <rspf/base/rspfNadconGridDatum.h>
#include <rspf/base/rspfNadconGridFile.h>
class rspfNadconNarDatum : public rspfNadconGridDatum
{
public:
   rspfNadconNarDatum(const rspfFilename& nadconDirectory);

   // Argument holds the source point and datum.  Returns another
   // point with this datum.
   //
   virtual rspfGpt shift(const rspfGpt    &aPt)const;
   //utility functions to shift to and from the standard coordinates.
   //Users should use the shift instead!!!
   //
   TYPE_DATA;
};

#endif
