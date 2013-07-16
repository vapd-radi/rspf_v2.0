//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Oscar Kramer
// 
//********************************************************************
// $Id$
#ifndef rspfSupportFilesList_HEADER
#define rspfSupportFilesList_HEADER

#include <vector>
#include <rspf/base/rspfFilename.h>

class rspfKeywordlist;

//*************************************************************************************************
//! Singleton class for logging all support data files opened during a session.
//*************************************************************************************************
class RSPFDLLEXPORT rspfSupportFilesList
{
public:
   static rspfSupportFilesList* instance();

   //! Add support data filename to the list:
   void add(const rspfFilename& f) { m_list.push_back(f); }

   //! Output list to the kwl.
   void save(rspfKeywordlist& kwl, const char* prefix) const;
   
   //! Clears the list to ready for new accumulation:
   void clear() { m_list.clear(); }

private:
   rspfSupportFilesList()  { }
   ~rspfSupportFilesList() { m_instance=0; }

   std::vector<rspfFilename>       m_list;
   static rspfSupportFilesList*    m_instance;
};

#endif
