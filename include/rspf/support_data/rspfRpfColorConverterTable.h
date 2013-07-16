#ifndef rspfRpfColorConverterTable_HEADER
#define rspfRpfColorConverterTable_HEADER
#include <iostream>
using namespace std;

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfErrorContext.h>

class rspfRpfColorConverterTable
{
public:
   friend ostream& operator <<(ostream& out,
                               const rspfRpfColorConverterTable& data);
   
   rspfRpfColorConverterTable();
   virtual ~rspfRpfColorConverterTable();

   rspfErrorCode parseStream(istream& in,
                              rspfByteOrder byteOrder);
   void print(ostream& out)const;
   rspf_uint32 getEntry(rspf_uint32 entry){return theColorGrayscaleTableEntryList[entry];}
   void setNumberOfEntries(rspf_uint32 entries);
   void setTableId(rspf_uint16 id);
   const rspf_uint32* getEntryList()const{return theColorGrayscaleTableEntryList;}
   rspf_uint32 getNumberOfEntries()const{return theNumberOfEntries;}
   
private:
   /*!
    * This will not be a field read from the stream this is
    * the value in the color converter offset table.
    */
   rspf_uint16 theTableId;

   /*!
    * Holds the number of entries
    */ 
   rspf_uint32  theNumberOfEntries;

   /*!
    * This will hold the lookup table values.
    */
   rspf_uint32* theColorGrayscaleTableEntryList;
};

#endif
