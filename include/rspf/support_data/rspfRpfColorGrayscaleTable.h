#ifndef rspfRpfColorGrayscaleTable_HEADER
#define rspfRpfColorGrayscaleTable_HEADER
#include <iostream>
using namespace std;
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfColorGrayscaleTable
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfRpfColorGrayscaleTable& data);
   rspfRpfColorGrayscaleTable();
   rspfRpfColorGrayscaleTable(const rspfRpfColorGrayscaleTable& rhs);
   virtual ~rspfRpfColorGrayscaleTable();

   rspfErrorCode parseStream(istream& in, rspfByteOrder byteOrder);
   
   void setTableData(unsigned short id,
                     unsigned long  numberOfElements);
   
   const rspfRpfColorGrayscaleTable& operator =(const rspfRpfColorGrayscaleTable&);

   const unsigned char* getData()const{return theData;}
   const unsigned char* getStartOfData(unsigned long entry)const;
   unsigned short getTableId()const{return theTableId;}
   unsigned long getNumberOfElements()const{return theNumberOfElements;}
   
private:   
   unsigned long theNumberOfElements;

   unsigned long theTotalNumberOfBytes;
   
   /*!
    * This will not be parsed from the stream.  This is set when
    * reading the offset record for this table.  The id defines
    * the format of the data buffer. if the id is :
    *
    * 1           then the buffer is in the format of RGB
    * 2                     ""                        RGBM
    * 3                     ""                        M
    * 4                     ""                        CMYK
    */
   unsigned short theTableId;

   

   unsigned char* theData;
};

#endif
