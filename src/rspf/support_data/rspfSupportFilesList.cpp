//**************************************************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
// 
// Description: Singleton class for logging all support data files opened during a session.
//
//**************************************************************************************************
// $Id$

#include <rspf/support_data/rspfSupportFilesList.h>
#include <rspf/base/rspfKeywordlist.h>

rspfSupportFilesList* rspfSupportFilesList::m_instance = 0;

//*************************************************************************************************
// Implements singleton pattern
//*************************************************************************************************
rspfSupportFilesList* rspfSupportFilesList::instance()
{
   if (!m_instance)
      m_instance = new rspfSupportFilesList;

   return m_instance;
}

//*************************************************************************************************
// Saves the list to a KWL
//*************************************************************************************************
void rspfSupportFilesList::save(rspfKeywordlist& kwl, const char* prefix) const
{
   rspfString baseName ("support_file");
   for (unsigned int i=0; i< (unsigned int) m_list.size(); i++)
   {
      rspfString key = baseName + rspfString::toString(i);
      kwl.add(prefix, key.chars(), m_list[i]);
   }
}
