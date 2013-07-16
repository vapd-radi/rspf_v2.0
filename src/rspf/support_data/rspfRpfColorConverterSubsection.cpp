//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//********************************************************************
// $Id: rspfRpfColorConverterSubsection.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/support_data/rspfRpfColorConverterSubsection.h>
#include <rspf/support_data/rspfRpfColorConverterOffsetRecord.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfColorConverterSubsection& data)
{
   data.print(out);
   
   return out;
}

rspfRpfColorConverterSubsection::rspfRpfColorConverterSubsection()
{
   theNumberOfColorConverterOffsetRecords = 0;
   clearFields();
}

void rspfRpfColorConverterSubsection::clearFields()
{
   theColorConverterOffsetTableOffset     = 0;
   theColorConverterOffsetRecordLength    = 0;
   theConverterRecordLength               = 0;

   theTableList.clear();
}

const rspfRpfColorConverterTable* rspfRpfColorConverterSubsection::getColorConversionTable(rspf_uint32 givenThisNumberOfEntires)const
{
   vector<rspfRpfColorConverterTable>::const_iterator listElement = theTableList.begin();

   while(listElement != theTableList.end())
   {
      if((*listElement).getNumberOfEntries() == givenThisNumberOfEntires)
      {
         return &(*listElement);
      }
   }

   return NULL;
}

rspfErrorCode rspfRpfColorConverterSubsection::parseStream(istream& in,
                                                             rspfByteOrder byteOrder)
{
   if(in)
   {
      clearFields();

      // this is the start of the subsection
      theStartOffset = in.tellg();
      in.read((char*)&theColorConverterOffsetTableOffset, 4);
      in.read((char*)&theColorConverterOffsetRecordLength, 2);
      in.read((char*)&theConverterRecordLength, 2);

      // this grabs the end of the subsection
      theEndOffset   = in.tellg();

      rspfEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theColorConverterOffsetTableOffset);
         anEndian.swap(theColorConverterOffsetRecordLength);
         anEndian.swap(theConverterRecordLength);

      }
      theTableList.resize(theNumberOfColorConverterOffsetRecords);
      for(unsigned long index = 0;
          index < theNumberOfColorConverterOffsetRecords;
          ++index)
      {
         rspfRpfColorConverterOffsetRecord recordInfo;
         
         if(recordInfo.parseStream(in, byteOrder) ==
            rspfErrorCodes::RSPF_OK)
         {
            unsigned long rememberGet = in.tellg();

            theTableList[index].setNumberOfEntries(recordInfo.theNumberOfColorConverterRecords);
            theTableList[index].setTableId(recordInfo.theColorConverterTableId);
            in.seekg(theStartOffset + recordInfo.theColorConverterTableOffset, ios::beg);
            theTableList[index].parseStream(in, byteOrder);
            
            in.seekg(rememberGet, ios::beg);
         }
         else
         {
            return rspfErrorCodes::RSPF_ERROR;
         }
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfColorConverterSubsection::setNumberOfColorConverterOffsetRecords(rspf_uint16 numberOfRecords)
{
   theNumberOfColorConverterOffsetRecords = numberOfRecords;
}

void rspfRpfColorConverterSubsection::print(ostream& out)const
{
   out << "theColorConverterOffsetTableOffset:      "
       << theColorConverterOffsetTableOffset << endl
       << "theColorConverterOffsetRecordLength:     "
       << theColorConverterOffsetRecordLength << endl
       << "theConverterRecordLength:                "
       << theConverterRecordLength << endl;

   copy(theTableList.begin(),
        theTableList.end(),
        ostream_iterator<rspfRpfColorConverterTable>(out, "\n"));
}
