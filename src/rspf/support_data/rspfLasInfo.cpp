//----------------------------------------------------------------------------
//
// File: rspfLibLasInfo.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: LAS LIDAR info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/support_data/rspfLasInfo.h>
#include <rspf/support_data/rspfLasHdr.h>

#include <fstream>

rspfLasInfo::rspfLasInfo()
   : m_file()
{
}

rspfLasInfo::~rspfLasInfo()
{
}

bool rspfLasInfo::open(const rspfFilename& file)
{
   bool result = false;
   std::ifstream istr;
   istr.open(file.c_str(), std::ios_base::in | std::ios_base::binary);
   if ( istr.is_open() )
   {
      rspfLasHdr hdr;
      result = hdr.checkSignature(istr);
      if (result) m_file = file;
   }
   return result;
}

std::ostream& rspfLasInfo::print(std::ostream& out) const
{
   std::ifstream istr;
   istr.open(m_file.c_str(), std::ios_base::in | std::ios_base::binary);
   if ( istr.is_open() )
   {
      rspfLasHdr hdr;
      if ( hdr.checkSignature(istr) )
      {
         hdr.readStream(istr);
         hdr.print(out);
      }
   }
   return out;
}
