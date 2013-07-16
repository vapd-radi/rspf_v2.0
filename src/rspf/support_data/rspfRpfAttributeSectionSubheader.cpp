//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfAttributeSectionSubheader.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/support_data/rspfRpfAttributeSectionSubheader.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

ostream& operator <<(ostream& out,
                     const rspfRpfAttributeSectionSubheader& data)
{
   data.print(out);
   
   return out;
}

rspfRpfAttributeSectionSubheader::rspfRpfAttributeSectionSubheader()
{
   clearFields();
}

rspfErrorCode rspfRpfAttributeSectionSubheader::parseStream(istream& in,
                                                              rspfByteOrder byteOrder)
{
   theAttributeSectionSubheaderStart = 0;
   theAttributeSectionSubheaderEnd = 0;
   if(in)
   {
      theAttributeSectionSubheaderStart = in.tellg();
      in.read((char*)&theNumberOfAttributeOffsetRecords, 2);
      in.read((char*)&theNumberOfExplicitArealCoverageRecords, 2);
      in.read((char*)&theAttributeOffsetTableOffset, 4);
      in.read((char*)&theAttribteOffsetRecordLength, 2);
      rspfEndian anEndian;

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(theNumberOfAttributeOffsetRecords);
         anEndian.swap(theNumberOfExplicitArealCoverageRecords);
         anEndian.swap(theAttributeOffsetTableOffset);
         anEndian.swap(theAttribteOffsetRecordLength);
      }
      theAttributeSectionSubheaderEnd = in.tellg();
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfAttributeSectionSubheader::print(ostream& out)const
{
   out << "theNumberOfAttributeOffsetRecords:         " << theNumberOfAttributeOffsetRecords << endl
       << "theNumberOfExplicitArealCoverageRecords:   " << theNumberOfExplicitArealCoverageRecords << endl
       << "theAttributeOffsetTableOffset:             " << theAttributeOffsetTableOffset << endl
       << "theAttribteOffsetRecordLength:             " << theAttribteOffsetRecordLength;
}

void rspfRpfAttributeSectionSubheader::clearFields()
{
   theNumberOfAttributeOffsetRecords       = 0;
   theNumberOfExplicitArealCoverageRecords = 0;
   theAttributeOffsetTableOffset           = 0;
   theAttribteOffsetRecordLength           = 0;
   
   theAttributeSectionSubheaderStart       = 0;
   theAttributeSectionSubheaderEnd         = 0;
}

rspf_uint64 rspfRpfAttributeSectionSubheader::getSubheaderStart()const
{
   return theAttributeSectionSubheaderStart;
}

rspf_uint64 rspfRpfAttributeSectionSubheader::getSubheaderEnd()const
{
   return theAttributeSectionSubheaderEnd;
}
