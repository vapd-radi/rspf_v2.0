#ifndef rspfRpfColorConverterSubsection_HEADER
#define rspfRpfColorConverterSubsection_HEADER
#include <iostream>
#include <vector>
#include <iterator>
using namespace std;
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/support_data/rspfRpfColorConverterTable.h>

class rspfRpfColorConverterSubsection
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfRpfColorConverterSubsection& data);
   rspfRpfColorConverterSubsection();
   virtual ~rspfRpfColorConverterSubsection(){}

   rspfErrorCode parseStream(istream& in,
                              rspfByteOrder byteOrder);
   
   rspf_uint32 getStartOffset()const{return theStartOffset;}
   rspf_uint32 getEndOffset()const{return theEndOffset;}

   const vector<rspfRpfColorConverterTable>& getColorConversionTable()const
      {
         return theTableList;
      }
   const rspfRpfColorConverterTable* getColorConversionTable(rspf_uint32 givenThisNumberOfEntires)const;
   
   void setNumberOfColorConverterOffsetRecords(rspf_uint16 numberOfRecords);
   void print(ostream& out)const;

   void clearFields();
   
private:
   rspfRpfColorConverterSubsection(const rspfRpfColorConverterSubsection&){}//hide
   void operator =(const rspfRpfColorConverterSubsection&){}//hide

   rspf_uint32  theStartOffset;
   rspf_uint32  theEndOffset;
   
   rspf_uint16  theNumberOfColorConverterOffsetRecords;
   
   rspf_uint32  theColorConverterOffsetTableOffset;
   rspf_uint16  theColorConverterOffsetRecordLength;
   rspf_uint16  theConverterRecordLength;

   vector<rspfRpfColorConverterTable> theTableList;
};

#endif
