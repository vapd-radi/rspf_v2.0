//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
// Base class for all rspf streams...  Derives from rspfReferenced.
//
//*******************************************************************
//  $Id: rspfStreamBase.cpp 11177 2007-06-07 19:47:04Z dburken $

#include <rspf/base/rspfStreamBase.h>

rspfStreamBase::rspfStreamBase()
   : rspfReferenced()
{
}

rspfStreamBase::~rspfStreamBase()
{
}

bool rspfStreamBase::isCompressed() const
{
   return false;
}
