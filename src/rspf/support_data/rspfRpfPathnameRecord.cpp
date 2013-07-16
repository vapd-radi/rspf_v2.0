//*******************************************************************
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Rpf support class.
//
//********************************************************************
// $Id: rspfRpfPathnameRecord.cpp 16997 2010-04-12 18:53:48Z dburken $

#include <istream>
#include <ostream>
#include <rspf/support_data/rspfRpfPathnameRecord.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

std::ostream& operator <<(std::ostream& out, const rspfRpfPathnameRecord& data)
{
   return data.print(out);
}

rspfRpfPathnameRecord::rspfRpfPathnameRecord()
   :
   m_length(0),
   m_pathname("")
{   
}

rspfRpfPathnameRecord::rspfRpfPathnameRecord(const rspfRpfPathnameRecord& obj)
   :
   m_length(obj.m_length),
   m_pathname(obj.m_pathname)
{
}

const rspfRpfPathnameRecord& rspfRpfPathnameRecord::operator=(const rspfRpfPathnameRecord& rhs)
{
   if ( this != &rhs )
   {
      m_length   = rhs.m_length;
      m_pathname = rhs.m_pathname;
   }
   return *this;
}

void rspfRpfPathnameRecord::clearFields()
{
   m_length   = 0;
   m_pathname = "";
}

rspfErrorCode rspfRpfPathnameRecord::parseStream(std::istream& in, rspfByteOrder byteOrder)
{
   if(in)
   {
      rspfEndian anEndian;
      
      clearFields();
      in.read((char*)&m_length, 2);

      if(anEndian.getSystemEndianType() != byteOrder)
      {
         anEndian.swap(m_length);
      }

      char *temp = new char[m_length+1];
      in.read((char*)temp, m_length);
      temp[m_length] = '\0';
      m_pathname = temp;
      
      delete [] temp;
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfPathnameRecord::writeStream(std::ostream& out)
{
   rspfEndian anEndian;
   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Always write big endian.
      anEndian.swap(m_length);
   }
   
   out.write((char*)&m_length, 2);

   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Swap back to native.
      anEndian.swap(m_length);
   }   

   if(m_pathname.size() >= m_length)
   {
      out.write(m_pathname.c_str(), m_length);
   }
}

std::ostream& rspfRpfPathnameRecord::print(std::ostream& out)const
{
   out << "length:        " << m_length
       << "\npathname:      " << m_pathname << std::endl;
   return out;
}

rspfString rspfRpfPathnameRecord::getPathname() const
{
   return m_pathname;
}

void rspfRpfPathnameRecord::setPathName(const rspfString& path)
{
   m_pathname = path;
   m_length = static_cast<rspf_uint16>( path.size() );
}
