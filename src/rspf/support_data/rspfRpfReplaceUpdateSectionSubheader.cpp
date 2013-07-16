//----------------------------------------------------------------------------
//
// File:     rspfRpfReplaceUpdateSectionSubheader.cpp
// 
// License:  See top level LICENSE.txt file.
// 
// Author:   David Burken
//
// Description:
//
// RPF replace/update section subheader record.
//
// See MIL-STD-2411 for detailed information.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfReplaceUpdateSectionSubheader.cpp 20324 2011-12-06 22:25:23Z dburken $

#include <rspf/support_data/rspfRpfReplaceUpdateSectionSubheader.h>
#include <rspf/base/rspfCommon.h> /* rspf::byteOrder() */
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfTrace.h>
#include <istream>
#include <ostream>

// Static trace for debugging
static rspfTrace traceDebug("rspfRpfReplaceUpdateSectionSubheader:debug");

std::ostream& operator <<(std::ostream& out, const rspfRpfReplaceUpdateSectionSubheader& data)
{
   return data.print(out);
}

rspfRpfReplaceUpdateSectionSubheader::rspfRpfReplaceUpdateSectionSubheader()
   :
   m_tableOffset(0),
   m_numberOfRecords(0),
   m_recordLength(0)
{
}

rspfRpfReplaceUpdateSectionSubheader::rspfRpfReplaceUpdateSectionSubheader(
   const rspfRpfReplaceUpdateSectionSubheader& obj)
   :
   m_tableOffset(obj.m_tableOffset),
   m_numberOfRecords(obj.m_numberOfRecords),
   m_recordLength(obj.m_recordLength)
{
}

const rspfRpfReplaceUpdateSectionSubheader& rspfRpfReplaceUpdateSectionSubheader::operator=(
   const rspfRpfReplaceUpdateSectionSubheader& rhs)
{
   if ( this != &rhs )
   {
      m_tableOffset     = rhs.m_tableOffset;
      m_numberOfRecords = rhs.m_numberOfRecords;
      m_recordLength    = rhs.m_recordLength;
   }
   return *this;
}

rspfRpfReplaceUpdateSectionSubheader::~rspfRpfReplaceUpdateSectionSubheader()
{
}

rspfErrorCode rspfRpfReplaceUpdateSectionSubheader::parseStream(std::istream& in,
                                                                  rspfByteOrder byteOrder)
{
   if(in)
   {
      in.read((char*)&m_tableOffset, 4);
      in.read((char*)&m_numberOfRecords, 2);
      in.read((char*)&m_recordLength, 2);

      if( rspf::byteOrder() != byteOrder )
      {
         rspfEndian anEndian;
         anEndian.swap(m_tableOffset);
         anEndian.swap(m_numberOfRecords);
         anEndian.swap(m_recordLength);
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

void rspfRpfReplaceUpdateSectionSubheader::writeStream(std::ostream& out)
{
   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      // Swap to big endian.
      rspfEndian anEndian;
      anEndian.swap(m_tableOffset);
      anEndian.swap(m_numberOfRecords);
      anEndian.swap(m_recordLength);
   } 

   out.write((char*)&m_tableOffset, 4);
   out.write((char*)&m_numberOfRecords, 2);
   out.write((char*)&m_recordLength, 2);
   
   if( rspf::byteOrder() != RSPF_BIG_ENDIAN)
   {
      // Swap back to native byte order.
      rspfEndian anEndian;
      anEndian.swap(m_tableOffset);
      anEndian.swap(m_numberOfRecords);
      anEndian.swap(m_recordLength);
   }
}
std::ostream& rspfRpfReplaceUpdateSectionSubheader::print(std::ostream& out) const
{
   out << "rspfRpfReplaceUpdateSectionSubheader DEBUG:"
       << "\ntable_offset:      " << m_tableOffset
       << "\nnumber_of_records: " << m_numberOfRecords
       << "\nrecord_length:     " << m_recordLength
       << std::endl;

   return out;
}

rspf_uint32 rspfRpfReplaceUpdateSectionSubheader::getOffset() const
{
   return m_tableOffset;
}

rspf_uint16 rspfRpfReplaceUpdateSectionSubheader::getNumberOfRecords() const
{
   return m_numberOfRecords;
}

rspf_uint16 rspfRpfReplaceUpdateSectionSubheader::getRecordLength() const
{
   return m_recordLength;
}

void rspfRpfReplaceUpdateSectionSubheader::setNumberOfRecords(rspf_uint16 count)
{
   m_numberOfRecords = count;
}

void rspfRpfReplaceUpdateSectionSubheader::setRecordLength(rspf_uint16 length)
{
   m_recordLength = length;
}

void rspfRpfReplaceUpdateSectionSubheader::clearFields()
{
   m_tableOffset     = 0;
   m_numberOfRecords = 0;
   m_recordLength    = 0;
}


