//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: ENVI Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/support_data/rspfEnviInfo.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/support_data/rspfEnviHeader.h>
#include <iostream>

rspfEnviInfo::rspfEnviInfo()
   : rspfInfoBase(),
     m_file()
{
}

rspfEnviInfo::~rspfEnviInfo()
{
}

bool rspfEnviInfo::open( const rspfFilename& file )
{
   bool result = false;

   m_file = file;

   if ( file.ext().downcase() != "hdr" )
   {
      // Typical case, we were fed the image file.  Look for a header file beside image.
      m_file.setExtension("hdr"); // image.hdr
      if ( !m_file.exists() )
      {
         m_file.setExtension("HDR"); // image.HDR
         if ( !m_file.exists() )
         {
            m_file = file;
            m_file.string() += ".hdr"; // image.ras.hdr
         }
      }
   }

   if ( m_file.exists() )
   {
      if ( rspfEnviHeader::isEnviHeader( m_file ) )
      {
         result = true;
      }
   }

   if ( !result ) 
   {
      m_file.clear();
   }

   return result;
}

std::ostream& rspfEnviInfo::print( std::ostream& out ) const
{
   if ( m_file.size() )
   {
      rspfEnviHeader hdr;
      if ( hdr.open( m_file ) )
      {
         hdr.getMap().addPrefixToAll( rspfString( "envi." ) );
         out << hdr.getMap() << std::endl;
      }
   }
   return out; 
}
