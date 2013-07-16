#include <rspf/support_data/rspfRpfColorConverterTable.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfColorConverterTable& data)
{
   data.print(out);
   
   return out;
}

rspfRpfColorConverterTable::rspfRpfColorConverterTable()
{
   theTableId                      = 0;
   theNumberOfEntries              = 0;
   theColorGrayscaleTableEntryList = NULL;
}

rspfRpfColorConverterTable::~rspfRpfColorConverterTable()
{
   if(theColorGrayscaleTableEntryList)
   {
      delete [] theColorGrayscaleTableEntryList;
      theColorGrayscaleTableEntryList = NULL;
   }
}
rspfErrorCode rspfRpfColorConverterTable::parseStream(istream& in,
                                                        rspfByteOrder byteOrder)
{
   if(in)
   {
      if(theNumberOfEntries > 0)
      {
         in.read((char*)theColorGrayscaleTableEntryList, 4*theNumberOfEntries);

         rspfEndian anEndian;
         if(anEndian.getSystemEndianType() != byteOrder)
         {
            for(rspf_uint32 index = 0;
                index < theNumberOfEntries;
                ++index)
            {
               anEndian.swap(theColorGrayscaleTableEntryList[index]);
            }
         }
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfColorConverterTable::print(ostream& out)const
{
   out << "theTableId:                   " << theTableId << endl
       << "theNumberOfEntries:           " << theNumberOfEntries << endl;

   if(theColorGrayscaleTableEntryList)
   {
      out << "Values: " << endl;
      for(rspf_uint32 index=0; index < theNumberOfEntries; index++)
      {
         out<< theColorGrayscaleTableEntryList[index] << endl;
      }
   }
   
}

void rspfRpfColorConverterTable::setNumberOfEntries(rspf_uint32 entries)
{
   if(theColorGrayscaleTableEntryList)
   {
      delete [] theColorGrayscaleTableEntryList;
      theColorGrayscaleTableEntryList = NULL;
   }
   theColorGrayscaleTableEntryList = new rspf_uint32[entries];
   theNumberOfEntries = entries;
   
   for(rspf_uint32 index=0;
       index < theNumberOfEntries;
       ++index)
   {
      theColorGrayscaleTableEntryList[index] = 0;
   }
}
void rspfRpfColorConverterTable::setTableId(rspf_uint16 id)
{
   theTableId = id;
}
