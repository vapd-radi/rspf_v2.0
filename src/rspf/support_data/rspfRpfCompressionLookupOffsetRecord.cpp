#include <rspf/support_data/rspfRpfCompressionLookupOffsetRecord.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream &out,
                     const rspfRpfCompressionLookupOffsetRecord& data)
{
   data.print(out);
   
   return out;
}

rspfRpfCompressionLookupOffsetRecord::rspfRpfCompressionLookupOffsetRecord()
{
   clearFields();
}

rspfErrorCode rspfRpfCompressionLookupOffsetRecord::parseStream(istream& in,
                                                                  rspfByteOrder byteOrder)
{
   if(in)
   {
      rspfEndian anEndian;

      in.read((char*)&theCompressionLookupTableId, 2);
      in.read((char*)&theNumberOfCompressionLookupRecords, 4);
      in.read((char*)&theNumberOfValuesPerCompressionLookupRecord, 2);
      in.read((char*)&theCompressionLookupValueBitLength, 2);
      in.read((char*)&theCompressionLookupTableOffset, 4);
      
      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theCompressionLookupTableId);
         anEndian.swap(theNumberOfCompressionLookupRecords);
         anEndian.swap(theNumberOfValuesPerCompressionLookupRecord);
         anEndian.swap(theCompressionLookupValueBitLength);
         anEndian.swap(theCompressionLookupTableOffset);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfCompressionLookupOffsetRecord::print(ostream& out)const
{
   out << "theCompressionLookupTableId:                 " << theCompressionLookupTableId << endl
       << "theNumberOfCompressionLookupRecords:         " << theNumberOfCompressionLookupRecords << endl
       << "theNumberOfValuesPerCompressionLookupRecord: " << theNumberOfValuesPerCompressionLookupRecord << endl
       << "theCompressionLookupValueBitLength:          " << theCompressionLookupValueBitLength << endl
       << "theCompressionLookupTableOffset:             " << theCompressionLookupTableOffset;
}

void rspfRpfCompressionLookupOffsetRecord::clearFields()
{
   theCompressionLookupTableId                  = 0;
   theNumberOfCompressionLookupRecords          = 0;
   theNumberOfValuesPerCompressionLookupRecord  = 0;
   theCompressionLookupValueBitLength           = 0;
   theCompressionLookupTableOffset              = 0;
}
