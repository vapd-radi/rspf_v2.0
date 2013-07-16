//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfFrameFileIndexSectionSubheader.cpp 16997 2010-04-12 18:53:48Z dburken $

#include <istream>
#include <ostream>

#include <rspf/support_data/rspfRpfFrameFileIndexSectionSubheader.h>
#include <rspf/base/rspfCommon.h> /* rspf::byteOrder() */
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfTrace.h>

// Static trace for debugging
static rspfTrace traceDebug("rspfRpfFrameFileIndexSectionSubheader:debug");

std::ostream& operator <<(std::ostream& out, const rspfRpfFrameFileIndexSectionSubheader& data)
{
   return data.print(out);
}

rspfRpfFrameFileIndexSectionSubheader::rspfRpfFrameFileIndexSectionSubheader()
   :
   m_highestSecurityClassification(' '),
   m_indexTableOffset(0),
   m_numberOfIndexRecords(0),
   m_numberOfPathnameRecords(0),
   m_indexRecordLength(0)
{
}

rspfRpfFrameFileIndexSectionSubheader::rspfRpfFrameFileIndexSectionSubheader(
   const rspfRpfFrameFileIndexSectionSubheader& obj)
   :
   m_highestSecurityClassification(obj.m_highestSecurityClassification),
   m_indexTableOffset(obj.m_indexTableOffset),
   m_numberOfIndexRecords(obj.m_numberOfIndexRecords),
   m_numberOfPathnameRecords(obj.m_numberOfPathnameRecords),
   m_indexRecordLength(obj.m_indexRecordLength)
{
}

const rspfRpfFrameFileIndexSectionSubheader& rspfRpfFrameFileIndexSectionSubheader::operator=(
   const rspfRpfFrameFileIndexSectionSubheader& rhs)
{
   if ( this != &rhs )
   {
      m_highestSecurityClassification = rhs.m_highestSecurityClassification;
      m_indexTableOffset = rhs.m_indexTableOffset;
      m_numberOfIndexRecords = rhs.m_numberOfIndexRecords;
      m_numberOfPathnameRecords = rhs.m_numberOfPathnameRecords;
      m_indexRecordLength = rhs.m_indexRecordLength;
   }
   return *this;
}

rspfRpfFrameFileIndexSectionSubheader::~rspfRpfFrameFileIndexSectionSubheader()
{
}

rspfErrorCode rspfRpfFrameFileIndexSectionSubheader::parseStream(std::istream& in,
                                                                   rspfByteOrder byteOrder)
{
   if(in)
   {
      in.read((char*)&m_highestSecurityClassification, 1);
      in.read((char*)&m_indexTableOffset, 4);
      in.read((char*)&m_numberOfIndexRecords, 4);
      in.read((char*)&m_numberOfPathnameRecords, 2);
      in.read((char*)&m_indexRecordLength, 2);

      if( rspf::byteOrder() != byteOrder )
      {
         rspfEndian anEndian;
         anEndian.swap(m_indexTableOffset);
         anEndian.swap(m_numberOfIndexRecords);
         anEndian.swap(m_numberOfPathnameRecords);
         anEndian.swap(m_indexRecordLength);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   if ( traceDebug() )
   {
      print( rspfNotify(rspfNotifyLevel_NOTICE) );
   }
   
   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfFrameFileIndexSectionSubheader::writeStream(std::ostream& out)
{
   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      // Swap to big endian.
      rspfEndian anEndian;
      anEndian.swap(m_indexTableOffset);
      anEndian.swap(m_numberOfIndexRecords);
      anEndian.swap(m_numberOfPathnameRecords);
      anEndian.swap(m_indexRecordLength);
   } 

   out.write((char*)&m_highestSecurityClassification, 1);
   out.write((char*)&m_indexTableOffset, 4);
   out.write((char*)&m_numberOfIndexRecords, 4);
   out.write((char*)&m_numberOfPathnameRecords, 2);
   out.write((char*)&m_indexRecordLength, 2);
   
   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      // Swap back to native byte order.
      rspfEndian anEndian;
      anEndian.swap(m_indexTableOffset);
      anEndian.swap(m_numberOfIndexRecords);
      anEndian.swap(m_numberOfPathnameRecords);
      anEndian.swap(m_indexRecordLength);
   }
}
std::ostream& rspfRpfFrameFileIndexSectionSubheader::print(std::ostream& out) const
{
   out << "rspfRpfFrameFileIndexSectionSubheader DEBUG:"
       << "\nHighestSecurityClassification:  " << m_highestSecurityClassification
       << "\nIndexTableOffset:               " << m_indexTableOffset
       << "\nNumberOfIndexRecords:           " << m_numberOfIndexRecords
       << "\nNumberOfPathnameRecords:        " << m_numberOfPathnameRecords
       << "\nIndexRecordLength:              " << m_indexRecordLength
       << std::endl;

   return out;
}

rspf_uint32 rspfRpfFrameFileIndexSectionSubheader::getOffset() const
{
   return m_indexTableOffset;
}

rspf_uint32 rspfRpfFrameFileIndexSectionSubheader::getNumberOfIndexRecords() const
{
   return m_numberOfIndexRecords;
}

rspf_uint16 rspfRpfFrameFileIndexSectionSubheader::getNumberOfPathnameRecords() const
{
   return m_numberOfPathnameRecords;
}

rspf_uint16 rspfRpfFrameFileIndexSectionSubheader::getIndexRecordLength() const
{
   return m_indexRecordLength;
}

void rspfRpfFrameFileIndexSectionSubheader::setNumberOfIndexRecords(rspf_uint32 count)
{
   m_numberOfIndexRecords = count;
}

void rspfRpfFrameFileIndexSectionSubheader::setNumberOfPathnameRecords(rspf_uint16 count)
{
   m_numberOfPathnameRecords = count;
}

void rspfRpfFrameFileIndexSectionSubheader::setIndexRecordLength(rspf_uint16 length)
{
   m_indexRecordLength = length;
}

void rspfRpfFrameFileIndexSectionSubheader::clearFields()
{
   m_highestSecurityClassification = ' ';
   m_indexTableOffset              = 0;
   m_numberOfIndexRecords          = 0;
   m_numberOfPathnameRecords       = 0;
   m_indexRecordLength             = 0;
}
