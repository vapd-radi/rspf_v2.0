#ifndef rspfRpfCompressionLookupOffsetRecord_HEADER
#define rspfRpfCompressionLookupOffsetRecord_HEADER
#include <iostream>
using namespace std;
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfCompressionLookupOffsetRecord
{
public:
   friend ostream& operator <<(ostream &out,
                               const rspfRpfCompressionLookupOffsetRecord& data);
   rspfRpfCompressionLookupOffsetRecord();
   rspfErrorCode parseStream(istream& in,
                              rspfByteOrder byteOrder);
   void print(ostream& out)const;
   rspf_uint16 getCompressionLookupTableId()const
      {
         return theCompressionLookupTableId;
      }
   rspf_uint32 getNumberOfCompressionLookupRecords()const
      {
         return theNumberOfCompressionLookupRecords;
      }
   rspf_uint16 getNumberOfValuesPerCompressionLookupRecord()const
      {
         return theNumberOfValuesPerCompressionLookupRecord;
      }
   rspf_uint16 getCompressionLookupValueBitLength()const
      {
         return theCompressionLookupValueBitLength;
      }
   rspf_uint32 getCompressionLookupTableOffset()const
      {
         return theCompressionLookupTableOffset;
      }
   
private:
   void clearFields();
   
   rspf_uint16 theCompressionLookupTableId;
   rspf_uint32 theNumberOfCompressionLookupRecords;
   rspf_uint16 theNumberOfValuesPerCompressionLookupRecord;
   rspf_uint16 theCompressionLookupValueBitLength;
   rspf_uint32 theCompressionLookupTableOffset;
};

#endif
