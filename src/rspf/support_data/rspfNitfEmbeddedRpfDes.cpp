//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfEmbeddedRpfDes.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <istream>
#include <ostream>
#include <rspf/support_data/rspfNitfEmbeddedRpfDes.h>


RTTI_DEF1(rspfNitfEmbeddedRpfDes, "rspfNitfEmbeddedRpfDes", rspfNitfRegisteredTag)


rspfNitfEmbeddedRpfDes::rspfNitfEmbeddedRpfDes()
   : rspfNitfRegisteredTag(std::string("RPFDES"), 0)
{
}

rspfNitfEmbeddedRpfDes::~rspfNitfEmbeddedRpfDes()
{
}

void rspfNitfEmbeddedRpfDes::parseStream(std::istream& /* in */ )
{
}

std::ostream& rspfNitfEmbeddedRpfDes::print(std::ostream& out, const std::string& /* prefix */)const
{
   return out;
}
