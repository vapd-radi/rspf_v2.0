#include <rspf/support_data/rspfRpfColorGrayscaleOffsetRecord.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfColorGrayscaleOffsetRecord& data)
{
   data.print(out);
   
   return out;
}
   
rspfRpfColorGrayscaleOffsetRecord::rspfRpfColorGrayscaleOffsetRecord()
{
   clearFields();
}

rspfErrorCode rspfRpfColorGrayscaleOffsetRecord::parseStream(istream& in,
                                                               rspfByteOrder byteOrder)
{
   if(in)
   {
      clearFields();
      
      in.read((char*)&theColorGrayscaleTableId, 2);
      in.read((char*)&theNumberOfColorGrayscaleRecords, 4);
      in.read((char*)&theColorGrayscaleElementLength, 1);
      in.read((char*)&theHistogramRecordLength, 2);
      in.read((char*)&theColorGrayscaleTableOffset, 4);
      in.read((char*)&theHistogramTableOffset, 4);

      rspfEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theColorGrayscaleTableId);
         anEndian.swap(theNumberOfColorGrayscaleRecords);
         anEndian.swap(theHistogramRecordLength);
         anEndian.swap(theColorGrayscaleTableOffset);
         anEndian.swap(theHistogramTableOffset);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfColorGrayscaleOffsetRecord::print(ostream& out)const
{
   out << "theColorGrayscaleTableId:                 " << theColorGrayscaleTableId << endl
       << "theNumberOfColorGrayscaleRecords:         " << theNumberOfColorGrayscaleRecords << endl
       << "theColorGrayscaleElementLength:           " << (unsigned long)theColorGrayscaleElementLength << endl
       << "theHistogramRecordLength:                 " << theHistogramRecordLength << endl
       << "theColorGrayscaleTableOffset:             " << theColorGrayscaleTableOffset << endl
       << "theHistogramTableOffset:                  " << theHistogramTableOffset;
}

void rspfRpfColorGrayscaleOffsetRecord::clearFields()
{
   theColorGrayscaleTableId         = 0;
   theNumberOfColorGrayscaleRecords = 0;
   theColorGrayscaleElementLength   = 0;
   theHistogramRecordLength         = 0;
   theColorGrayscaleTableOffset     = 0;
   theHistogramTableOffset          = 0;
}
