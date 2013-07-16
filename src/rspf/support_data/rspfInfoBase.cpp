//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: Base class for Info object.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/support_data/rspfInfoBase.h>
#include <rspf/base/rspfKeywordlist.h>
#include <sstream>

rspfInfoBase::rspfInfoBase()
   : theOverviewFlag(true)
{}

rspfInfoBase::~rspfInfoBase()
{}

void rspfInfoBase::setProcessOverviewFlag(bool flag)
{
   theOverviewFlag = flag;
}

bool rspfInfoBase::getProcessOverviewFlag() const
{
   return theOverviewFlag;
}

bool rspfInfoBase::getKeywordlist(rspfKeywordlist& kwl)const
{  
   // Do a print to a memory stream.
   std::ostringstream out;
   print(out);

   std::istringstream in(out.str());
   // Give the result to the keyword list.
   return kwl.parseStream(in);
}
