#include <rspf/support_data/rspfRpfColorGrayscaleSubheader.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfColorGrayscaleSubheader& data)
{
   data.print(out);

   return out;
}

rspfRpfColorGrayscaleSubheader::rspfRpfColorGrayscaleSubheader()
{
   clearFields();
}

rspfErrorCode rspfRpfColorGrayscaleSubheader::parseStream(istream& in,
                                                            rspfByteOrder /* byteOrder */)
{
   if(in)
   {
      clearFields();
      
      theStartOffset = in.tellg();
      in.read((char*)&theNumberOfColorGreyscaleOffsetRecords, 1);
      in.read((char*)&theNumberOfColorConverterOffsetRecords, 1);

      char tempString[12];
      in.read(tempString, 12);
      theColorGrayscaleFilename = rspfString(tempString,
                                              (char*)(&tempString[12]));
      theEndOffset = in.tellg();
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfColorGrayscaleSubheader::print(ostream& out)const
{
   out << "theNumberOfColorGreyscaleOffsetRecords:    " << (int)theNumberOfColorGreyscaleOffsetRecords << endl
       << "theNumberOfColorConverterOffsetRecords:    " << (int)theNumberOfColorConverterOffsetRecords << endl
       << "theColorGrayscaleFilename:                 " << theColorGrayscaleFilename;
   
}

void rspfRpfColorGrayscaleSubheader::clearFields()
{
   theStartOffset = 0;
   theEndOffset   = 0;
   
   theNumberOfColorGreyscaleOffsetRecords = 0;
   theNumberOfColorConverterOffsetRecords = 0;
   theColorGrayscaleFilename              = "";
}

