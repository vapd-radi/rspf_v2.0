//----------------------------------------------------------------------------
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: USGS DEM Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <iostream>

#include <rspf/support_data/rspfDemInfo.h>

#include <rspf/base/rspfErrorCodes.h>
#include <rspf/base/rspfFilename.h>

#include <rspf/support_data/rspfDemHeader.h>
#include <rspf/support_data/rspfDemUtil.h>

rspfDemInfo::rspfDemInfo()
   : theFile()
{
}

rspfDemInfo::~rspfDemInfo()
{
}

bool rspfDemInfo::open(const rspfFilename& file)
{
   bool result = rspfDemUtil::isUsgsDem(file);

   if ( result )
   {
      theFile = file;
   }
   else
   {
      theFile = rspfFilename::NIL;
   }
   
   return result;
}

std::ostream& rspfDemInfo::print(std::ostream& out) const
{
   if ( theFile.exists() )
   {
      rspfDemHeader hdr;
      if ( hdr.open(theFile) )
      {
         // std::string prefix;
         hdr.print(std::cout);
      }
   }
   return out;
}
