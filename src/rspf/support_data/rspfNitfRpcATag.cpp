//*******************************************************************
//
// LICENSE: LGPL
//
// see top level LICENSE.txt
// 
// Author: Garrett Potts
//
// Description: Nitf support class for RPC00A -
// Rational Polynomial Coefficient extension.
//
//********************************************************************
// $Id: rspfNitfRpcATag.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <rspf/support_data/rspfNitfRpcATag.h>

RTTI_DEF1(rspfNitfRpcATag, "rspfNitfRpcATag", rspfNitfRpcBase);

rspfNitfRpcATag::rspfNitfRpcATag()
   : rspfNitfRpcBase()
{
   // Set the tag name in base.
   setTagName(std::string("RPC00A"));
}
