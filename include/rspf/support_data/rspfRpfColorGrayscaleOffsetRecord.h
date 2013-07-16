#ifndef rspfRpfColorGrayscaleOffsetRecord_HEADER
#define rspfRpfColorGrayscaleOffsetRecord_HEADER
#include <iostream>
using namespace std;

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfColorGrayscaleOffsetRecord
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfRpfColorGrayscaleOffsetRecord& data);
   
   rspfRpfColorGrayscaleOffsetRecord();

   rspfErrorCode parseStream(istream& in,
                              rspfByteOrder byteOrder);
   
   void print(ostream& out)const;

   void clearFields();
   rspf_uint16 getColorGrayscaleTableId()const
      {
         return theColorGrayscaleTableId;
      }
   rspf_uint32 getNumberOfColorGrayscaleRecords()const
      {
         return theNumberOfColorGrayscaleRecords;
      }
   rspf_uint32 getColorGrayscaleTableOffset()const
      {
         return theColorGrayscaleTableOffset;
      }
private:
   rspf_uint16  theColorGrayscaleTableId;
   rspf_uint32  theNumberOfColorGrayscaleRecords;
   unsigned char theColorGrayscaleElementLength;
   rspf_uint16  theHistogramRecordLength;
   rspf_uint32  theColorGrayscaleTableOffset;
   rspf_uint32  theHistogramTableOffset;
};

#endif
