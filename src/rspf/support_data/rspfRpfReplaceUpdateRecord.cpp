//----------------------------------------------------------------------------
//
// File:     rspfRpfReplaceUpdateRecord.cpp
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
// $Id: rspfRpfReplaceUpdateRecord.cpp 20324 2011-12-06 22:25:23Z dburken $

#include <rspf/support_data/rspfRpfReplaceUpdateRecord.h>
#include <rspf/support_data/rspfNitfCommon.h>
#include <cstring> /* for memset, memcpy */
#include <istream>
#include <ostream>

std::ostream& operator <<(std::ostream& out, const rspfRpfReplaceUpdateRecord& data)
{
   return data.print(out);
}

rspfRpfReplaceUpdateRecord::rspfRpfReplaceUpdateRecord()
   :
   m_updateStatus(0)
{
   clearFields();
}

rspfRpfReplaceUpdateRecord::rspfRpfReplaceUpdateRecord(const rspfRpfReplaceUpdateRecord& obj)
   :
   m_updateStatus(obj.m_updateStatus)
{
   memcpy(m_newFile, obj.m_newFile, 13);
   memcpy(m_oldFile, obj.m_oldFile, 13);
}

const rspfRpfReplaceUpdateRecord& rspfRpfReplaceUpdateRecord::operator=(
   const rspfRpfReplaceUpdateRecord& rhs)
{
   if ( this != &rhs )
   {
      memcpy(m_newFile, rhs.m_newFile, 13);
      memcpy(m_oldFile, rhs.m_oldFile, 13);
      m_updateStatus = rhs.m_updateStatus;      
   }
   return *this;
}

rspfErrorCode rspfRpfReplaceUpdateRecord::parseStream( std::istream& in )
{
   rspfErrorCode status = rspfErrorCodes::RSPF_OK;
   if( in.good() )
   {
      clearFields();
      in.read((char*)&m_newFile, 12);
      in.read((char*)&m_oldFile, 12);
      in.read((char*)&m_updateStatus, 1);
   }

   if ( in.fail() )
   {
      status = rspfErrorCodes::RSPF_ERROR;
   }
   
   return status;
}

void rspfRpfReplaceUpdateRecord::writeStream(std::ostream& out)
{
   out.write((char*)&m_newFile, 12);
   out.write((char*)&m_oldFile, 12);
   out.write((char*)&m_updateStatus, 1);
}

std::ostream& rspfRpfReplaceUpdateRecord::print(std::ostream& out,
                                                 const std::string& prefix,
                                                 rspf_uint32 recordNumber)const
{
   std::string pfx = prefix;
   pfx += "replace_update_record";
   pfx += rspfString::toString(recordNumber).string();
   pfx += ".";
   out << pfx << "new_file: "   << m_newFile << "\n"
       << pfx << "old_file: " << m_oldFile << "\n"
       << pfx << "update_status: " << int(m_updateStatus)
       << std::endl;
   return out;
}

void rspfRpfReplaceUpdateRecord::getNewFilename(std::string& file) const
{
   file = m_newFile;
}

void rspfRpfReplaceUpdateRecord::getOldFilename(std::string& file) const
{
   file = m_oldFile;
}

rspf_uint8 rspfRpfReplaceUpdateRecord::getUpdateStatus() const
{
   return m_updateStatus;
}

void rspfRpfReplaceUpdateRecord::setNewFilename(const std::string& file)
{
   rspfNitfCommon::setField( m_newFile, rspfString(file), 12 );
}

void rspfRpfReplaceUpdateRecord::setOldFilename(const std::string& file)
{
   rspfNitfCommon::setField( m_oldFile, rspfString(file), 12 );
}

void rspfRpfReplaceUpdateRecord::setUpdateStatus(rspf_uint8 status)
{
   m_updateStatus = status;
}

void rspfRpfReplaceUpdateRecord::clearFields()
{
   memset(m_newFile, ' ', 12);
   memset(m_oldFile, ' ', 12);   
   m_updateStatus = 0;

   m_newFile[12] = '\0';
   m_oldFile[12] = '\0';
}
