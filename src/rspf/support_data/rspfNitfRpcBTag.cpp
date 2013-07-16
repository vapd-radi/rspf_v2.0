//*******************************************************************
//
// LICENSE: LGPL
//
// see top level LICENSE.txt
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfRpcBTag.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <rspf/support_data/rspfNitfRpcBTag.h>

RTTI_DEF1(rspfNitfRpcBTag, "rspfNitfRpcBTag", rspfNitfRpcBase);

rspfNitfRpcBTag::rspfNitfRpcBTag()
   : rspfNitfRpcBase()
{
   // Set the tag name in base.
   setTagName(std::string("RPC00B"));
}
