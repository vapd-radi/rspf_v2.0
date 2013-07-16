//*******************************************************************
// Copyright (C) 2004 Garrett Potts
//
// LICENSE: LGPL see top level LICENSE.txt for more details
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfCompressionHeader.cpp 18413 2010-11-11 19:56:22Z gpotts $
#include <rspf/support_data/rspfNitfCompressionHeader.h>
#include <rspf/base/rspfKeywordlist.h>
#include <sstream>

RTTI_DEF1(rspfNitfCompressionHeader, "rspfNitfCompressionHeader", rspfObject);

bool rspfNitfCompressionHeader::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   return rspfObject::saveState(kwl, prefix);
}
