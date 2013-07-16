//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfDataExtensionSegment.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <ostream>

#include <rspf/support_data/rspfNitfDataExtensionSegment.h>
RTTI_DEF1(rspfNitfDataExtensionSegment,
          "rspfNitfDataExtensionSegment",
          rspfObject)

std::ostream& operator <<(std::ostream &out,
                          const rspfNitfDataExtensionSegment &data)
{
   data.print(out);
   
   return out;
}
