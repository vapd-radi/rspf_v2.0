//*******************************************************************
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//*************************************************************************
// $Id: rspfId.cpp 9430 2006-08-16 19:04:16Z gpotts $

#include <iostream>
#include <rspf/base/rspfId.h>

const rspf_int64 rspfId::INVALID_ID = -1;

std::ostream& operator <<(std::ostream &out, const rspfId& data)
{
   data.print(out);
   return out;
}

void rspfId::print(std::ostream& out)const
{
   out << "id: " << theId;
}
