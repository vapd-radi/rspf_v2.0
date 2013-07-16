#include <rspf/support_data/rspfRpfCompressionSection.h>
#include <rspf/support_data/rspfRpfCompressionSectionSubheader.h>
#include <rspf/support_data/rspfRpfCompressionLookupOffsetRecord.h>
#include <string.h> // for memset
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

#ifndef NULL
#include <stddef.h>
#endif

ostream& operator<<(ostream& out,
                    const rspfRpfCompressionOffsetTableData& data)
{
   unsigned long size = (data.theNumberOfValuesPerLookup*data.theNumberOfLookupValues*
                         data.theCompressionLookupValueBitLength)/8;
   
   out << "theTableId:                         " << data.theTableId << endl
       << "theNumberOfLookupValues:            " << data.theNumberOfLookupValues << endl
       << "theCompressionLookupValueBitLength: " << data.theCompressionLookupValueBitLength << endl
       << "theNumberOfValuesPerLookup:         " << data.theNumberOfValuesPerLookup << endl
       << "Data Size in bytes:                 "
       << size;

   return out;
}


rspfRpfCompressionOffsetTableData::rspfRpfCompressionOffsetTableData()
   :theTableId(0),
    theNumberOfLookupValues(0),
    theCompressionLookupValueBitLength(0),
    theNumberOfValuesPerLookup(0),
    theData(NULL)
{
}

rspfRpfCompressionOffsetTableData::rspfRpfCompressionOffsetTableData(
   const rspfRpfCompressionOffsetTableData& rhs)
   :theNumberOfLookupValues(0),
    theCompressionLookupValueBitLength(0),
    theNumberOfValuesPerLookup(0),
    theData(NULL)
{
   *this = rhs;
}

const rspfRpfCompressionOffsetTableData& rspfRpfCompressionOffsetTableData::operator = (
   const rspfRpfCompressionOffsetTableData& rhs)
{
   if(this != &rhs)
   {
      if(theData)
      {
         delete [] theData;
         theData = 0;
      }

      if(rhs.theData)
      {
         theTableId                         = rhs.theTableId;
         theNumberOfLookupValues            = rhs.theNumberOfLookupValues;
         theCompressionLookupValueBitLength = rhs.theCompressionLookupValueBitLength;
         theNumberOfValuesPerLookup         = rhs.theNumberOfValuesPerLookup;
         
         unsigned long size = (theNumberOfValuesPerLookup*
                               theNumberOfLookupValues*
                               theCompressionLookupValueBitLength)/8;
         if(size > 0)
         {
            theData = new unsigned char[size];
            memcpy(theData, rhs.theData, size);
         }
      }
      else
      {
         theTableId = 0;
         theNumberOfLookupValues = 0;
         theCompressionLookupValueBitLength = 0;
         theNumberOfValuesPerLookup = 0;
      }
   }
   
   return *this;
}

rspfRpfCompressionOffsetTableData::~rspfRpfCompressionOffsetTableData()
{
   if(theData)
   {
      delete [] theData;
      theData = 0;
   }
}


ostream& operator << (ostream& out,
                      const rspfRpfCompressionSection& data)
{
   data.print(out);
   
   return out;
}

rspfRpfCompressionSection::rspfRpfCompressionSection()
   :theSubheader(NULL)
{
   theSubheader = new rspfRpfCompressionSectionSubheader;
}

rspfRpfCompressionSection::~rspfRpfCompressionSection()
{
   if(theSubheader)
   {
      delete theSubheader;
      theSubheader = NULL;
   }
}

rspfErrorCode rspfRpfCompressionSection::parseStream(istream& in,
                                                       rspfByteOrder byteOrder)
{
   rspfErrorCode result = rspfErrorCodes::RSPF_OK;
   
   if(in&&theSubheader)
   {
      result = theSubheader->parseStream(in, byteOrder);

      if(result == rspfErrorCodes::RSPF_OK)
      {
         rspfEndian anEndian;
         rspfRpfCompressionLookupOffsetRecord record;
         
         in.read((char*)&theCompressionLookupOffsetTableOffset, 4);
         in.read((char*)&theCompressionLookupTableOffsetRecordLength, 2);
         
         if(byteOrder != anEndian.getSystemEndianType())
         {
            anEndian.swap(theCompressionLookupOffsetTableOffset);
            anEndian.swap(theCompressionLookupTableOffsetRecordLength);
         }

         if(theSubheader->getNumberOfCompressionLookupOffsetRecords() > 0)
         {
            // clear the table
            theTable.clear();

            // resize it
            theTable.resize(theSubheader->getNumberOfCompressionLookupOffsetRecords());
            
            // now populate it
            for(long index = 0;
                index < theSubheader->getNumberOfCompressionLookupOffsetRecords();
                ++index)
            {
               record.parseStream(in, byteOrder);
               unsigned long rememberLocation = in.tellg();
               in.seekg(record.getCompressionLookupTableOffset()+
                        theSubheader->getEndOffset(), ios::beg);

               theTable[index].theTableId                            = record.getCompressionLookupTableId();
               theTable[index].theNumberOfLookupValues               = record.getNumberOfCompressionLookupRecords();
               theTable[index].theCompressionLookupValueBitLength    = record.getCompressionLookupValueBitLength();
               theTable[index].theNumberOfValuesPerLookup            = record.getNumberOfValuesPerCompressionLookupRecord();
               
               // store the information about this compress/decompress algorithm
               // I am not sure but I will assume that the bit length can be
               // arbitrary.  In other words if someone says that the bit length of
               // the lookup value is 12 this should be handled ok.
               //
               unsigned long size = (theTable[index].theNumberOfValuesPerLookup*
                                     theTable[index].theNumberOfLookupValues*
                                     theTable[index].theCompressionLookupValueBitLength)/8;
               
               theTable[index].theData = new unsigned char[size];

               in.read( (char*)theTable[index].theData, size);
               
               in.seekg(rememberLocation, ios::beg);
            }
         }
      }
   }
   else
   {
      result = rspfErrorCodes::RSPF_ERROR;
   }

   return result;
}

void rspfRpfCompressionSection::print(ostream& out)const
{
   if(theSubheader)
   {
      out << *theSubheader << endl;
      copy(theTable.begin(),
           theTable.end(),
           ostream_iterator<rspfRpfCompressionOffsetTableData>(out, "\n"));
   }
}

