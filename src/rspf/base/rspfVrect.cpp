//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
// Contains class declaration for vrect.
// Container class for four double points representing a rectangle
// where y is up
// 
//*******************************************************************
//  $Id: rspfVrect.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/base/rspfVrect.h>

//*******************************************************************
// Public Method:
//*******************************************************************
void rspfVrect::print(std::ostream& os) const
{
   os << theUlCorner << theLrCorner;
}

//*******************************************************************
// friend function:
//*******************************************************************
std::ostream& operator<<(std::ostream& os, const rspfVrect& rect)
{
   rect.print(os);

   return os;
}

rspfVrect::~rspfVrect()
{
}
