//----------------------------------------------------------------------------
//
// File:     rspfRpfReplaceUpdateTable.cpp
// 
// License:  See top level LICENSE.txt file.
// 
// Author:   David Burken
//
// Description:
//
// Holds a table of RPF replace/update section subheader records.
//
// See MIL-STD-2411 for detailed information.
// 
//----------------------------------------------------------------------------
// $Id: rspfRpfReplaceUpdateTable.cpp 20324 2011-12-06 22:25:23Z dburken $

#include <rspf/support_data/rspfRpfReplaceUpdateTable.h>
#include <iostream>

rspfRpfReplaceUpdateTable::rspfRpfReplaceUpdateTable()
   :
   m_table(0)
{
}

rspfRpfReplaceUpdateTable::rspfRpfReplaceUpdateTable(const rspfRpfReplaceUpdateTable& obj)
   :
   m_table(obj.m_table)
{
}

const rspfRpfReplaceUpdateTable& rspfRpfReplaceUpdateTable::operator=(
   const rspfRpfReplaceUpdateTable& rhs)
{
   if ( this != &rhs )
   {
      m_table = rhs.m_table;
   }
   return *this;
}

void rspfRpfReplaceUpdateTable::addRecord(const rspfRpfReplaceUpdateRecord& record)
{
   m_table.push_back(record);
}

void rspfRpfReplaceUpdateTable::clear()
{
   m_table.clear();
}

std::ostream& rspfRpfReplaceUpdateTable::print( std::ostream& out,
                                                 const std::string& prefix ) const
{
   rspf_uint32 size = static_cast<rspf_uint32>( m_table.size() );
   for (rspf_uint32 i = 0; i < size; ++i)
   {
      m_table[i].print(out, prefix, i);
   }
   return out;
}
