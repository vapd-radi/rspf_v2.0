//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfBoundaryRectSectionSubheader.cpp 16997 2010-04-12 18:53:48Z dburken $

#include <istream>
#include <ostream>

#include <rspf/support_data/rspfRpfBoundaryRectSectionSubheader.h>
#include <rspf/base/rspfCommon.h> /* rspf::byteOrder() */
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>

std::ostream& operator <<(std::ostream& out,
                          const rspfRpfBoundaryRectSectionSubheader &data)
{
   return data.print(out);
}

rspfRpfBoundaryRectSectionSubheader::rspfRpfBoundaryRectSectionSubheader()
   :
   m_rectangleTableOffset(0),
   m_numberOfEntries(0),
   m_lengthOfEachEntry(0)
{
}

rspfRpfBoundaryRectSectionSubheader::rspfRpfBoundaryRectSectionSubheader(
   const rspfRpfBoundaryRectSectionSubheader& obj)
   :
   m_rectangleTableOffset(obj.m_rectangleTableOffset),
   m_numberOfEntries(obj.m_numberOfEntries),
   m_lengthOfEachEntry(obj.m_lengthOfEachEntry)
{
}
const rspfRpfBoundaryRectSectionSubheader& rspfRpfBoundaryRectSectionSubheader::operator=(
   const rspfRpfBoundaryRectSectionSubheader& rhs)
{
   if ( this != &rhs )
   {
      m_rectangleTableOffset = rhs.m_rectangleTableOffset;
      m_numberOfEntries = rhs.m_numberOfEntries;
      m_lengthOfEachEntry = rhs.m_lengthOfEachEntry;
   }
   return *this;
}

rspfRpfBoundaryRectSectionSubheader::~rspfRpfBoundaryRectSectionSubheader()
{
}

rspfErrorCode rspfRpfBoundaryRectSectionSubheader::parseStream(std::istream& in,
                                                                 rspfByteOrder byteOrder)
{
   if(in)
   {
      clearFields();
      
      in.read((char*)&m_rectangleTableOffset, 4);
      in.read((char*)&m_numberOfEntries, 2);
      in.read((char*)&m_lengthOfEachEntry, 2);

      if( rspf::byteOrder() != byteOrder )
      {
         rspfEndian anEndian;
         anEndian.swap(m_rectangleTableOffset);
         anEndian.swap(m_numberOfEntries);
         anEndian.swap(m_lengthOfEachEntry);
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }
   
   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfBoundaryRectSectionSubheader::writeStream(std::ostream& out)
{
   rspfEndian anEndian;
   
   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Always write out big endian.
      anEndian.swap(m_rectangleTableOffset);
      anEndian.swap(m_numberOfEntries);
      anEndian.swap(m_lengthOfEachEntry);
   }

   out.write((char*)&m_rectangleTableOffset, 4);
   out.write((char*)&m_numberOfEntries, 2);
   out.write((char*)&m_lengthOfEachEntry, 2);

   if( anEndian.getSystemEndianType() != RSPF_BIG_ENDIAN )
   {
      // Swap back to native.
      anEndian.swap(m_rectangleTableOffset);
      anEndian.swap(m_numberOfEntries);
      anEndian.swap(m_lengthOfEachEntry);
   }
}
   
std::ostream& rspfRpfBoundaryRectSectionSubheader::print(std::ostream& out) const
{
   out << "RectangleTableOffset:      " << m_rectangleTableOffset
       << "\nNumberOfEntries:           " << m_numberOfEntries
       << "\nLengthOfEachEntry:         " << m_lengthOfEachEntry << std::endl;
   return out;
}

rspf_uint16 rspfRpfBoundaryRectSectionSubheader::getNumberOfEntries() const
{
   return m_numberOfEntries;
}

rspf_uint32 rspfRpfBoundaryRectSectionSubheader::getTableOffset() const
{
   return m_rectangleTableOffset;
}

rspf_uint16 rspfRpfBoundaryRectSectionSubheader::getLengthOfEachEntry() const
{
   return m_lengthOfEachEntry;
}

void rspfRpfBoundaryRectSectionSubheader::setNumberOfEntries(rspf_uint16 entries)
{
   m_numberOfEntries = entries;
}

void rspfRpfBoundaryRectSectionSubheader::setTableOffset(rspf_uint32 offset)
{
   m_rectangleTableOffset = offset;
}

void rspfRpfBoundaryRectSectionSubheader::setLengthOfEachEntry(rspf_uint16 length)
{
   m_lengthOfEachEntry = length;
}

void rspfRpfBoundaryRectSectionSubheader::clearFields()
{
   m_rectangleTableOffset = 0;
   m_numberOfEntries      = 0;
   m_lengthOfEachEntry    = 0;
}
