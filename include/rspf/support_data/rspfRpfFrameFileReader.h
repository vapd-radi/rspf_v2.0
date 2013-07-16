#ifndef rspfRpfFrameFileReader_HEADER
#define rspfRpfFrameFileReader_HEADER

#include <iosfwd>

#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/support_data/rspfRpfHeader.h>

class rspfRpfFrameFileReader
{
public:
   friend std::ostream& operator<<(std::ostream& out, const rspfRpfFrameFileReader& data);
   
   rspfRpfFrameFileReader();
   ~rspfRpfFrameFileReader();
   
   rspfErrorCode parseFile(const rspfFilename& fileName);
   std::ostream& print(std::ostream& out) const;
   const rspfRpfHeader* getRpfHeader()const;
   
private:
   void clearAll();
   
   rspfRefPtr<rspfRpfHeader> theRpfHeader;
   rspfFilename               theFilename;
};

#endif
