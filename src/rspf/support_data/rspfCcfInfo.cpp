//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: CCF Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>
#include <rspf/base/rspfFilename.h>
#include <rspf/support_data/rspfCcfInfo.h>
#include <rspf/imaging/rspfCcfHead.h>

rspfCcfInfo::rspfCcfInfo()
   : theFile()
{
}

rspfCcfInfo::~rspfCcfInfo()
{
}

bool rspfCcfInfo::open(const rspfFilename& file)
{
   bool result = false;

   rspfString extension = file.ext();

   extension.downcase();

   if (extension == "ccf")
   {
      theFile = file;
      result = true;
   }
   else
   {
      theFile.clear();
   }

   return result;
}

std::ostream& rspfCcfInfo::print(std::ostream& out) const
{
   
   if ( theFile.size() )
   {
      rspfCcfHead ccf(theFile);
      out << "File:  " << theFile << "\n" << ccf;
   }
   return out;
}
