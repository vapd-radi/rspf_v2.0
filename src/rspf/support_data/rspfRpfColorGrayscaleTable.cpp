#include <rspf/support_data/rspfRpfColorGrayscaleTable.h>
#include <string.h> // for memcpy
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfColorGrayscaleTable& data)
{
   out << "theNumberOfElements:                   " << data.theNumberOfElements << endl
       << "theTotalNumberOfBytes:                 " << data.theTotalNumberOfBytes << endl
       << "theTableId:                            " << data.theTableId;

   return out;
}

rspfRpfColorGrayscaleTable::rspfRpfColorGrayscaleTable()
   :theNumberOfElements(0),
    theTotalNumberOfBytes(0),
    theTableId(0),
    theData(NULL)
{
}

rspfRpfColorGrayscaleTable::rspfRpfColorGrayscaleTable(const rspfRpfColorGrayscaleTable& rhs)
   :theNumberOfElements(0),
    theTotalNumberOfBytes(0),
    theTableId(0),
    theData(NULL)
{
   *this = rhs;
}


rspfRpfColorGrayscaleTable::~rspfRpfColorGrayscaleTable()
{
   if(theData)
   {
      delete [] theData;
      theData = NULL;
   }
}

rspfErrorCode rspfRpfColorGrayscaleTable::parseStream(istream& in,
                                                        rspfByteOrder /* byteOrder */)
{
   if(in)
   {
      if((theTotalNumberOfBytes > 0)&&theData)
      {
         in.read((char*)theData, theTotalNumberOfBytes);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

const unsigned char* rspfRpfColorGrayscaleTable::getStartOfData(unsigned long entry)const
{
   unsigned long offset = 0;

   switch(theTableId)
   {
   case 1: // this is in format RGB
   {
      offset = 3*entry;
      break;
   }
   case 2: // format RGBM
   case 4: // format CMYK
   {
      offset = 4*entry;
      break;
   }
   case 3: // format M
   {
      offset = entry;
      break;
   }
   }

   return (const unsigned char*)(theData + offset);
}

void rspfRpfColorGrayscaleTable::setTableData(unsigned short id,
                                               unsigned long  numberOfElements)
{
   if(theData)
   {
      delete theData;
      theData = NULL;
   }

   unsigned long totalBytes = 0;
   
   switch(id)
   {
   case 1: // this is in format RGB
   {
      totalBytes = 3*numberOfElements;
      break;
   }
   case 2: // format RGBM
   case 4: // format CMYK
   {
      totalBytes = 4*numberOfElements;
      break;
   }
   case 3: // format M
   {
      totalBytes = numberOfElements;
      break;
   }
   }

   if(totalBytes > 0)
   {
      theTableId            = id;
      theNumberOfElements   = numberOfElements;
      theTotalNumberOfBytes = totalBytes;

      theData = new unsigned char[totalBytes];
   }
}



const rspfRpfColorGrayscaleTable& rspfRpfColorGrayscaleTable::operator =(const rspfRpfColorGrayscaleTable& rhs)
{
   
   if(&rhs != this)
   {
      theNumberOfElements   = rhs.theNumberOfElements;
      theTotalNumberOfBytes = rhs.theTotalNumberOfBytes;
      theTableId            = rhs.theTableId;
      if(theData)
      {
         delete theData;
         theData = NULL;
      }
      if(theTotalNumberOfBytes&&rhs.theData)
      {
         theData = new unsigned char[theTotalNumberOfBytes];
         
         memcpy(theData, rhs.theData, theTotalNumberOfBytes);
      }
   }
   
   return *this;
}
