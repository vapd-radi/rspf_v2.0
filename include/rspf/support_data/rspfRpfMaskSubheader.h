#ifndef rspfRpfMaskSubheader_HEADER
#define rspfRpfMaskSubheader_HEADER
#include <iostream>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfString.h>

class rspfRpfMaskSubheader
{
public:
   friend std::ostream& operator <<(std::ostream& out,
                                    const rspfRpfMaskSubheader& data);
   rspfRpfMaskSubheader();

   virtual ~rspfRpfMaskSubheader();

   rspfErrorCode parseStream(std::istream& in,
                              rspfByteOrder byteOrder);
   void print(std::ostream& out)const;
   
private:
   void clearFields();

   unsigned short theSubframeSequenceRecordLength;
   unsigned short theTransparencySequenceRecordLength;
   unsigned short theTransparentOutputPixelCodeLength;
   unsigned char* theOutputPixelCodeBitString;

   /*!
    * This is just a working variable and is not
    * part of the Rpf fields. this is the number of bytes
    * of the output pixel code length.  It divides by
    * 8 and then does the ceiling of the
    * theTransparentOutputPixelCodeLength.
    */
   long theNumberOfBytes;
};
#endif
