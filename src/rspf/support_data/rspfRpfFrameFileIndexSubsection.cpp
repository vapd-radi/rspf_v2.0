//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// 
// Description: Rpf support class.
// 
//********************************************************************
// $Id: rspfRpfFrameFileIndexSubsection.cpp 16997 2010-04-12 18:53:48Z dburken $

#include <cstring> /* for memset */
#include <istream>
#include <iterator>
#include <ostream>
#include <rspf/support_data/rspfRpfFrameFileIndexSubsection.h>
#include <rspf/base/rspfEndian.h>
#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfFilename.h>

std::ostream& operator <<(std::ostream& out, const rspfRpfFrameFileIndexSubsection& data)
{
   return data.print(out);
}

rspfRpfFrameFileIndexSubsection::rspfRpfFrameFileIndexSubsection()
   :
   m_indexTable(0),
   m_pathnameTable(0)
{
}

rspfRpfFrameFileIndexSubsection::rspfRpfFrameFileIndexSubsection(
   const rspfRpfFrameFileIndexSubsection& obj)
   :
   m_indexTable(obj.m_indexTable),
   m_pathnameTable(obj.m_pathnameTable)
{
}

const rspfRpfFrameFileIndexSubsection& rspfRpfFrameFileIndexSubsection::operator=(
   const rspfRpfFrameFileIndexSubsection& rhs)
{
   if ( this != &rhs )
   {
      m_indexTable = rhs.m_indexTable;
      m_pathnameTable = rhs.m_pathnameTable;
   }
   return *this;
}

rspfRpfFrameFileIndexSubsection::~rspfRpfFrameFileIndexSubsection()
{
}

rspfErrorCode rspfRpfFrameFileIndexSubsection::parseStream(std::istream &in,
                                                             rspfByteOrder byteOrder)
{
   rspfErrorCode result = rspfErrorCodes::RSPF_OK;
   
   if(in && (m_indexTable.size()>0))
   {
      rspf_uint32 index;
      for(index = 0;
          (index < m_indexTable.size()) && (result == rspfErrorCodes::RSPF_OK);
          ++index)
      {
         result = m_indexTable[index].parseStream(in, byteOrder);
      }
      for(index = 0;
          (index < m_pathnameTable.size()) && (result ==rspfErrorCodes::RSPF_OK);
          ++index)
      {
         result = m_pathnameTable[index].parseStream(in, byteOrder);
      }
   }
   else
   {
      result = rspfErrorCodes::RSPF_ERROR;
   }

   return result;
}

void rspfRpfFrameFileIndexSubsection::writeStream(std::ostream& out)
{
   std::vector<rspfRpfFrameFileIndexRecord>::iterator indexIter = m_indexTable.begin();
   while ( indexIter != m_indexTable.end() )
   {
      (*indexIter).writeStream(out);
      ++indexIter;
   }

   std::vector<rspfRpfPathnameRecord>::iterator pathIter = m_pathnameTable.begin();
   while ( pathIter != m_pathnameTable.end() )
   {
      (*pathIter).writeStream(out);
      ++pathIter;
   }
}

void rspfRpfFrameFileIndexSubsection::clearFields()
{
   m_indexTable.clear();
   m_pathnameTable.clear();
}

std::ostream& rspfRpfFrameFileIndexSubsection::print(std::ostream& out)const
{
   copy(m_indexTable.begin(),
        m_indexTable.end(),
        std::ostream_iterator<rspfRpfFrameFileIndexRecord>(out, "\n"));
   copy(m_pathnameTable.begin(),
        m_pathnameTable.end(),
        std::ostream_iterator<rspfRpfPathnameRecord>(out, "\n"));
   return out;
}

void rspfRpfFrameFileIndexSubsection::setNumberOfFileIndexRecords(
   rspf_uint32 numberOfIndexRecords)
{
   m_indexTable.resize(numberOfIndexRecords);
}

void rspfRpfFrameFileIndexSubsection::setNumberOfPathnames(rspf_uint32 numberOfPathnames)
{
   m_pathnameTable.resize(numberOfPathnames);
}

const std::vector<rspfRpfFrameFileIndexRecord>&
rspfRpfFrameFileIndexSubsection::getIndexTable()const
{
   return m_indexTable;
}

const std::vector<rspfRpfPathnameRecord>&
rspfRpfFrameFileIndexSubsection::getPathnameTable() const
{
   return m_pathnameTable;
}

bool rspfRpfFrameFileIndexSubsection::getFrameFileIndexRecordFromFile(
   const rspfFilename& file, rspfRpfFrameFileIndexRecord& record) const
{
   bool result = false;
   std::vector<rspfRpfFrameFileIndexRecord>::const_iterator i = m_indexTable.begin();
   while ( i != m_indexTable.end() )
   {
      if ( (*i).getFilename() == file )
      {
         record = (*i);
         result = true;
         break;
      }
      ++i;
   }
   return result;
}
