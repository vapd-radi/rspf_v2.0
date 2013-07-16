#ifndef rspfSrtmFilename_HEADER
#define rspfSrtmFilename_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGpt.h>

class rspfSrtmFilename
{
public:
   rspfSrtmFilename(const rspfFilename& srtmFilename="");

   bool setFilename(const rspfFilename& srtmFilename);
   rspfGpt ul()const;
   rspfGpt ur()const;
   rspfGpt lr()const;
   rspfGpt ll()const;
   
protected:
   rspfFilename theFilename;

   rspf_float64 theSouthwestLongitude;
   rspf_float64 theSouthwestLatitude;
};

#endif
