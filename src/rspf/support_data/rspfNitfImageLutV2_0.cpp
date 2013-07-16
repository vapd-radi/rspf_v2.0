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
// $Id: rspfNitfImageLutV2_0.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <cstring> // memcpy
#include <istream>
#include <ostream>

#include <rspf/support_data/rspfNitfImageLutV2_0.h>

rspfNitfImageLutV2_0::rspfNitfImageLutV2_0()
{
}

rspfNitfImageLutV2_0::~rspfNitfImageLutV2_0()
{
}

void rspfNitfImageLutV2_0::parseStream(std::istream &in)
{
   if (!in) return;

   rspf_uint32 count = 0;
   while(count < theNumberOfEntries)
   {
      in.read((char*)&theData[count], 1);
      
      ++count;
   }
}

std::ostream& rspfNitfImageLutV2_0::print(std::ostream& out)const
{
   out << "theNumberOfEntries:          " << theNumberOfEntries;

   return out;
}

rspf_uint32 rspfNitfImageLutV2_0::getNumberOfEntries()const
{
   return theNumberOfEntries;
}

void rspfNitfImageLutV2_0::setNumberOfEntries(rspf_uint32 numberOfEntries)
{
   theData.resize(numberOfEntries);
   theNumberOfEntries = numberOfEntries;
}

rspf_uint8 rspfNitfImageLutV2_0::getValue(rspf_uint32 index)const
{
   return theData[index];
}
