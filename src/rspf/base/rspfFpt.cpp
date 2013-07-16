//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
// Description:
//
// Contains class definitions for rspfFpt.
// 
//*******************************************************************
//  $Id: rspfFpt.cpp 11399 2007-07-26 13:39:54Z dburken $

#include <iostream>
#include <iomanip>

#include <rspf/base/rspfFpt.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfFpt::rspfFpt(const rspfIpt& pt)
   :
      x(pt.x), y(pt.y)
{
   if(pt.hasNans())
   {
      makeNan();
   }
}

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfFpt::rspfFpt(const rspfDpt& pt)
   :
      x(pt.x), y(pt.y)
{
   if(pt.hasNans())
   {
      makeNan();
   }
}

//*******************************************************************
// Public Method:
//*******************************************************************
const rspfFpt& rspfFpt::operator=(const rspfDpt& pt)
{
   x = pt.x;
   y = pt.y;
   
   if(pt.hasNans())
   {
      makeNan();
   }
   return *this;
}

//*******************************************************************
// Public Method:
//*******************************************************************
const rspfFpt& rspfFpt::operator=(const rspfIpt& pt)
{
   x = pt.x;
   y = pt.y;
   if(pt.hasNans())
   {
      makeNan();
   }
   
   return *this;
}

//*******************************************************************
// Public Method:
//*******************************************************************
void rspfFpt::print(std::ostream& os) const
{
   os << std::setiosflags(std::ios::fixed) << std::setprecision(15)
      << "( " << x << " " << y << " )";
}

//*******************************************************************
// friend function:
//*******************************************************************
std::ostream& operator<<(std::ostream& os, const rspfFpt& pt)
{
   pt.print(os);
   
   return os;
}
