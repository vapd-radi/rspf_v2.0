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
// $Id: rspfNitfImageLut.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <cstring> // memcpy
#include <ostream>

#include <rspf/support_data/rspfNitfImageLut.h>

std::ostream& operator <<(std::ostream& out,
                          const rspfNitfImageLut &data)
{
   data.print(out);

   return out;
}

RTTI_DEF1(rspfNitfImageLut, "rspfNitfImageLut", rspfObject);
rspfNitfImageLut::rspfNitfImageLut()
{
}

rspfNitfImageLut::~rspfNitfImageLut()
{
}

