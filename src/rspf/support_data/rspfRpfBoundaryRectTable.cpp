//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class
// 
//********************************************************************
// $Id: rspfRpfBoundaryRectTable.cpp 16997 2010-04-12 18:53:48Z dburken $


#include <istream>
#include <iterator>
#include <ostream>

#include <rspf/support_data/rspfRpfBoundaryRectTable.h>
#include <rspf/base/rspfEndian.h>

std::ostream& operator <<(std::ostream& out, const rspfRpfBoundaryRectTable& data)
{
   return data.print(out);
}

rspfRpfBoundaryRectTable::rspfRpfBoundaryRectTable(rspf_uint32 numberOfEntries)
   :
   m_table(numberOfEntries)
{
}

rspfRpfBoundaryRectTable::rspfRpfBoundaryRectTable(const rspfRpfBoundaryRectTable& obj)
   :
   m_table(obj.m_table)
{
}

const rspfRpfBoundaryRectTable& rspfRpfBoundaryRectTable::operator=(
   const rspfRpfBoundaryRectTable& rhs)
{
   if ( this != &rhs )
   {
      m_table = rhs.m_table;
   }
   return *this;
}

rspfRpfBoundaryRectTable::~rspfRpfBoundaryRectTable()
{}

rspfErrorCode rspfRpfBoundaryRectTable::parseStream(std::istream& in,
                                                      rspfByteOrder byteOrder)
{
   if(in)
   {
      std::vector<rspfRpfBoundaryRectRecord>::iterator entry = m_table.begin();
      
      while(entry != m_table.end())
      {
         (*entry).parseStream(in, byteOrder);
         ++entry;
      }
   }
   else
   {
      return rspfErrorCodes::RSPF_ERROR;
   }

   return rspfErrorCodes::RSPF_OK;
}

void rspfRpfBoundaryRectTable::writeStream(std::ostream& out)
{
   std::vector<rspfRpfBoundaryRectRecord>::iterator entry = m_table.begin();
   while ( entry != m_table.end() )
   {
      (*entry).writeStream(out);
      ++entry;
   }
}

void rspfRpfBoundaryRectTable::setNumberOfEntries(rspf_uint32 numberOfEntries)
{
   m_table.resize(numberOfEntries);
}

rspf_uint32 rspfRpfBoundaryRectTable::getNumberOfEntries() const
{
   return static_cast<rspf_uint32>(m_table.size());
}

bool rspfRpfBoundaryRectTable::getEntry(
   rspf_uint32 entry, rspfRpfBoundaryRectRecord& record) const
{
   bool result = true;
   if ( entry < m_table.size() )
   {
      record = m_table[entry];
   }
   else
   {
      result = false;
   }
   return result;
}

std::ostream& rspfRpfBoundaryRectTable::print(std::ostream& out)const
{
   std::copy(m_table.begin(), m_table.end(),
             std::ostream_iterator<rspfRpfBoundaryRectRecord>(out, "\n"));
   return out;
}
