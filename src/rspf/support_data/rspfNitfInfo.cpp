//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Description: NITF Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>

#include <rspf/support_data/rspfNitfInfo.h>

rspfNitfInfo::rspfNitfInfo()
   : m_nitfFile(0)
{
}

rspfNitfInfo::~rspfNitfInfo()
{
   m_nitfFile = 0;
}

bool rspfNitfInfo::open(const rspfFilename& file)
{
   m_nitfFile = new rspfNitfFile();

   bool result = m_nitfFile->parseFile(file);

   if (result == false)
   {
      m_nitfFile = 0;
   }

   return result;
}

std::ostream& rspfNitfInfo::print(std::ostream& out) const
{
   if ( m_nitfFile.valid() )
   {
      std::string prefix;
      m_nitfFile->print(out, prefix, getProcessOverviewFlag());
   }
   return out;
}

bool rspfNitfInfo::getKeywordlist(rspfKeywordlist& kwl)const
{
   bool result = false;
   if ( m_nitfFile.valid() )
   {
      m_nitfFile->saveState(kwl, "nitf.");
   }
   
   return result;
}
