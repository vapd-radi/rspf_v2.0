#ifndef rspfRpfColorGrayscaleSubheader_HEADER
#define rspfRpfColorGrayscaleSubheader_HEADER
#include <iostream>
using namespace std;
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfConstants.h>

class rspfRpfColorGrayscaleSubheader
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfRpfColorGrayscaleSubheader& data);
   
   rspfRpfColorGrayscaleSubheader();
   ~rspfRpfColorGrayscaleSubheader(){}
   
   rspfErrorCode parseStream(istream& in,
                            rspfByteOrder byteOrder);
   void print(ostream& out)const;
   unsigned long getStartOffset()const
      {
         return theStartOffset;
      }
   unsigned long getEndOffset()const
      {
         return theEndOffset;
      }
   unsigned long getNumberOfColorGreyscaleOffsetRecords()const
      {
         return theNumberOfColorGreyscaleOffsetRecords;
      }
   unsigned long getNumberOfColorConverterOffsetRecords()const
      {
         return theNumberOfColorConverterOffsetRecords;
      }
   
private:
   void clearFields();

   unsigned long theStartOffset;
   unsigned long theEndOffset;
   
   unsigned char theNumberOfColorGreyscaleOffsetRecords;
   unsigned char theNumberOfColorConverterOffsetRecords;

   /*!
    * 12 byte field.
    */
   rspfString         theColorGrayscaleFilename;
};

#endif
