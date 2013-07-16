#include <rspf/support_data/rspfRpfAttributeOffsetRecord.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfAttributeOffsetRecord& data)
{
   data.print(out);
   
   return out;
}

rspfRpfAttributeOffsetRecord::rspfRpfAttributeOffsetRecord()
{
   clearFields();
}

rspfErrorCode rspfRpfAttributeOffsetRecord::parseStream(istream& in,
                                                          rspfByteOrder byteOrder)
{
   if(in)
   {
      in.read((char*)&theAttributeId, 2);
      in.read((char*)&theParameterId, 1);
      in.read((char*)&theArealCoverageSequenceNumber, 1);
      in.read((char*)&theAttributeRecordOffset, 4);

      rspfEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theAttributeId);
         anEndian.swap(theAttributeRecordOffset);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfAttributeOffsetRecord::print(ostream& out)const
{
   out << "theAttributeId:                  " << theAttributeId << endl
       << "theParameterId:                  " << theParameterId << endl
       << "theArealCoverageSequenceNumber:  " << theArealCoverageSequenceNumber << endl
       << "theAttributeRecordOffset:        " << theAttributeRecordOffset;
}

void rspfRpfAttributeOffsetRecord::clearFields()
{
   theAttributeId                 = 0;
   theParameterId                 = 0;
   theArealCoverageSequenceNumber = 0;
   theAttributeRecordOffset       = 0;
}
