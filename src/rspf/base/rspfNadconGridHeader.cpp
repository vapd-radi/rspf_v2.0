//**************************************************************************
// Copyright (C) 2003 Storage Area Networks, Inc.
//
// Written by:   Kenneth Melero  <kmelero@sanz.com>
//
//**************************************************************************

#include <fstream>
#include <iostream>
#include <rspf/base/rspfNadconGridHeader.h>
#include <rspf/base/rspfEndian.h>

std::ostream& operator <<(std::ostream& out, const rspfNadconGridHeader& rhs)
{
   out << "theCols:    " << rhs.theCols
       << "\ntheRows:    " << rhs.theRows
       << "\ntheZ:       " << rhs.theZ
       << "\ntheMinX:    " << rhs.theMinX
       << "\ntheDx:      " << rhs.theDx
       << "\ntheMinY:    " << rhs.theMinY
       << "\ntheDy:      " << rhs.theDy
       << "\nBounds:     " << rhs.getBoundingRect() << std::endl;
   return out;
}

bool rspfNadconGridHeader::readHeader(const rspfFilename& file)
{
   std::ifstream in(file.c_str(), std::ios::in|std::ios::binary);

   if(in)
   {
      return readHeader(in);
   }

   return false;
}

bool rspfNadconGridHeader::readHeader(std::istream& in)
{
   in.read((char*)theCharBuf, 64);
   in.read((char*)&theCols, 4);
   in.read((char*)&theRows, 4);
   in.read((char*)&theZ,   4);
   in.read((char*)&theMinX, 4);
   in.read((char*)&theDx, 4);
   in.read((char*)&theMinY, 4);
   in.read((char*)&theDy, 4);
   rspfEndian anEndian;
   if(anEndian.getSystemEndianType() != RSPF_LITTLE_ENDIAN)
     {
       anEndian.swap(theCols);
       anEndian.swap(theRows);
       anEndian.swap(theZ);
       anEndian.swap(theMinX);
       anEndian.swap(theDx);
       anEndian.swap(theMinY);
       anEndian.swap(theDy);
     }

   return in.good();
}
