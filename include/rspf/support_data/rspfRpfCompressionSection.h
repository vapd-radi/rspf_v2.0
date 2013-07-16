#ifndef rspfRpfCompressionSection_HEADER
#define rspfRpfCompressionSection_HEADER
#include <iostream>
#include <vector>
#include <iterator>
using namespace std;
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfConstants.h>

class rspfRpfCompressionSectionSubheader;

struct rspfRpfCompressionOffsetTableData
{
   friend ostream& operator<<(ostream& out,
                              const rspfRpfCompressionOffsetTableData& data);
   rspfRpfCompressionOffsetTableData();
   rspfRpfCompressionOffsetTableData(const rspfRpfCompressionOffsetTableData& rhs);
   ~rspfRpfCompressionOffsetTableData();
   const rspfRpfCompressionOffsetTableData& operator =(const rspfRpfCompressionOffsetTableData& rhs);
   
   rspf_uint16 theTableId;
   rspf_uint32 theNumberOfLookupValues;
   rspf_uint16 theCompressionLookupValueBitLength;
   rspf_uint16 theNumberOfValuesPerLookup;
   rspf_uint8* theData;
};

class rspfRpfCompressionSection
{
public:
   friend ostream& operator << (ostream& out,
                                const rspfRpfCompressionSection& data);
   rspfRpfCompressionSection();
   virtual ~rspfRpfCompressionSection();
   rspfErrorCode parseStream(istream& in,
                              rspfByteOrder byteOrder);
   void print(ostream& out)const;
   const vector<rspfRpfCompressionOffsetTableData>& getTable()const
      {
         return theTable;
      }
   const rspfRpfCompressionSectionSubheader* getSubheader()const
   {
      return theSubheader;
   }
private:
   void clearTable();
   
   rspfRpfCompressionSectionSubheader* theSubheader;
   
   rspf_uint32 theCompressionLookupOffsetTableOffset;
   rspf_uint16 theCompressionLookupTableOffsetRecordLength;

   vector<rspfRpfCompressionOffsetTableData> theTable;
};

#endif
