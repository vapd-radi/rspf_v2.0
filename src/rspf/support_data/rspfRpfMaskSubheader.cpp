#include <rspf/support_data/rspfRpfMaskSubheader.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>
#include <cmath>

std::ostream& operator <<(std::ostream& out,
                          const rspfRpfMaskSubheader& data)
{
   data.print(out);
   return out;
}

rspfRpfMaskSubheader::rspfRpfMaskSubheader()
   :theSubframeSequenceRecordLength(0),
    theTransparencySequenceRecordLength(0),
    theTransparentOutputPixelCodeLength(0),
    theOutputPixelCodeBitString(0),
    theNumberOfBytes(0)
{
   clearFields();
}

rspfRpfMaskSubheader::~rspfRpfMaskSubheader()
{
   if (theOutputPixelCodeBitString)
   {
      delete [] theOutputPixelCodeBitString;
      theOutputPixelCodeBitString = 0;
   }
}

rspfErrorCode rspfRpfMaskSubheader::parseStream(std::istream& in,
                                        rspfByteOrder byteOrder)
{
   if(in)
   {
      clearFields();
      
      in.read((char*)&theSubframeSequenceRecordLength, 2);
      in.read((char*)&theTransparencySequenceRecordLength, 2);
      in.read((char*)&theTransparentOutputPixelCodeLength, 2);
      rspfEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theSubframeSequenceRecordLength);
         anEndian.swap(theTransparencySequenceRecordLength);
         anEndian.swap(theTransparentOutputPixelCodeLength);

      }
      if(theTransparentOutputPixelCodeLength>0)
      {
         theNumberOfBytes = (long)std::ceil(theTransparentOutputPixelCodeLength/8.0);
         theOutputPixelCodeBitString = new unsigned char[theNumberOfBytes];
         in.read((char*)theOutputPixelCodeBitString, theNumberOfBytes);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfMaskSubheader::print(std::ostream& out)const
{
   out << "theSubframeSequenceRecordLength:     "
       << theSubframeSequenceRecordLength
       << "\ntheTransparencySequenceRecordLength: "
       << theTransparencySequenceRecordLength
       << "\ntheTransparentOutputPixelCodeLength: "
       << theTransparentOutputPixelCodeLength
       << "\ntheNumberOfBytes:                    "
       << theNumberOfBytes
       << "\ntheOutputPixelCodeBitString:         ";

   // display the pixel code as a bit pattern.
   for(long index = theNumberOfBytes-1;
       index >=0;
       --index)
   {
      unsigned long temp = theOutputPixelCodeBitString[index];
      out << ((temp>>7)&1)
          << ((temp>>6)&1)
          << ((temp>>5)&1)
          << ((temp>>4)&1)
          << ((temp>>3)&1)
          << ((temp>>2)&1)
          << ((temp>>1)&1)
          << (temp&1);
   }
   out << std::endl;
}

void rspfRpfMaskSubheader::clearFields()
{
   theSubframeSequenceRecordLength     = 0;
   theTransparencySequenceRecordLength = 0;
   theTransparentOutputPixelCodeLength = 0;
   theNumberOfBytes                    = 0;

   if(theOutputPixelCodeBitString)
   {
      delete [] theOutputPixelCodeBitString;
      theOutputPixelCodeBitString = 0;
   }
}
