#include <rspf/support_data/rspfRpfColorConverterOffsetRecord.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfColorConverterOffsetRecord& data)
{
   out << "theColorConverterTableId:                 " << data.theColorConverterTableId << endl
       << "theNumberOfColorConverterRecords:         " << data.theNumberOfColorConverterRecords << endl
       << "theColorConverterTableOffset:             " << data.theColorConverterTableOffset << endl
       << "theSourceColorGrayscaleOffsetTableOffset: " << data.theSourceColorGrayscaleOffsetTableOffset << endl
       << "theTargetColorGrayscaleOffsetTableOffset: " << data.theTargetColorGrayscaleOffsetTableOffset;

   return out;
}

rspfRpfColorConverterOffsetRecord::rspfRpfColorConverterOffsetRecord()
{
   clearFields();
}

rspfErrorCode rspfRpfColorConverterOffsetRecord::parseStream(istream& in,
                                                               rspfByteOrder byteOrder)
{
   if(in)
   {
      in.read((char*)&theColorConverterTableId, 2);
      in.read((char*)&theNumberOfColorConverterRecords, 4);
      in.read((char*)&theColorConverterTableOffset, 4);
      in.read((char*)&theSourceColorGrayscaleOffsetTableOffset, 4);
      in.read((char*)&theTargetColorGrayscaleOffsetTableOffset, 4);
      
      rspfEndian anEndian;
      
      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theColorConverterTableId);
         anEndian.swap(theNumberOfColorConverterRecords);
         anEndian.swap(theColorConverterTableOffset);
         anEndian.swap(theSourceColorGrayscaleOffsetTableOffset);
         anEndian.swap(theTargetColorGrayscaleOffsetTableOffset);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfColorConverterOffsetRecord::clearFields()
{
   theColorConverterTableId                 = 0;
   theNumberOfColorConverterRecords         = 0;
   theColorConverterTableOffset             = 0;
   theSourceColorGrayscaleOffsetTableOffset = 0;
   theTargetColorGrayscaleOffsetTableOffset = 0;
}

