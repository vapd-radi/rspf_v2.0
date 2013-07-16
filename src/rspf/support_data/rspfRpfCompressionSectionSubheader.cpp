#include <rspf/support_data/rspfRpfCompressionSectionSubheader.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

rspfRpfCompressionSectionSubheader::rspfRpfCompressionSectionSubheader()
{
   clearFields();
}

ostream& operator<<(ostream& out,
                    const rspfRpfCompressionSectionSubheader& data)
{
   data.print(out);
   
   return out;
}

rspfErrorCode rspfRpfCompressionSectionSubheader::parseStream(istream& in,
                                                                rspfByteOrder byteOrder)
{
   if(in)
   {
      theStartOffset = in.tellg();
      
      in.read((char*)&theCompressionAlgorithmId, 2);
      in.read((char*)&theNumberOfCompressionLookupOffsetRecords, 2);
      in.read((char*)&theNumberOfCompressionParameterOffsetRecords, 2);

      theEndOffset = in.tellg();
      
      rspfEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theCompressionAlgorithmId);
         anEndian.swap(theNumberOfCompressionLookupOffsetRecords);
         anEndian.swap(theNumberOfCompressionParameterOffsetRecords);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}
   
void rspfRpfCompressionSectionSubheader::print(ostream& out)const
{
   out << "theCompressionAlgorithmId:                    " << theCompressionAlgorithmId << endl
       << "theNumberOfCompressionLookupOffsetRecords:    " << theNumberOfCompressionLookupOffsetRecords << endl
       << "theNumberOfCompressionParameterOffsetRecords: " << theNumberOfCompressionParameterOffsetRecords;
}

void rspfRpfCompressionSectionSubheader::clearFields()
{
   theStartOffset                               = 0;
   theEndOffset                                 = 0;
   theCompressionAlgorithmId                    = 0;
   theNumberOfCompressionLookupOffsetRecords    = 0;
   theNumberOfCompressionParameterOffsetRecords = 0;
}
