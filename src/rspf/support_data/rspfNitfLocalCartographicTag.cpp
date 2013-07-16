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
// $Id: rspfNitfLocalCartographicTag.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <sstream>
#include <iostream>
#include <iomanip>
#include <rspf/support_data/rspfNitfLocalCartographicTag.h>

RTTI_DEF1(rspfNitfLocalCartographicTag, "rspfNitfLocalCartographicTag", rspfNitfRegisteredTag);

rspfNitfLocalCartographicTag::rspfNitfLocalCartographicTag()
   : rspfNitfRegisteredTag(std::string("MAPLOB"), 43)
{
}

rspfNitfLocalCartographicTag::~rspfNitfLocalCartographicTag()
{
}

void rspfNitfLocalCartographicTag::parseStream(std::istream& in)
{
   clearFields();
   
   in.read(theLengthUnits, 3);
   in.read(theEastingInterval, 5);
   in.read(theNorthingInterval, 5);
   in.read(theEastingReferenceOrigin, 15);
   in.read(theNorthingReferenceOrigin, 15);
}

void rspfNitfLocalCartographicTag::writeStream(std::ostream& out)
{
   out.write(theLengthUnits, 3);
   out.write(theEastingInterval, 5);
   out.write(theNorthingInterval, 5);
   out.write(theEastingReferenceOrigin, 15);
   out.write(theNorthingReferenceOrigin, 15);
}

void rspfNitfLocalCartographicTag::clearFields()
{
   memset(theLengthUnits, ' ', 3);
   memset(theEastingInterval, '0', 5);
   memset(theNorthingInterval, '0', 5);
   memset(theEastingReferenceOrigin, '0', 15);
   memset(theNorthingReferenceOrigin, '0', 15);

   theLengthUnits[3]              = '\0';
   theEastingInterval[5]          = '\0';
   theNorthingInterval[5]         = '\0';
   theEastingReferenceOrigin[15]  = '\0';
   theNorthingReferenceOrigin[15] = '\0';
}

void rspfNitfLocalCartographicTag::setLengthUnits(const rspfString& units)
{
   rspfString temp = rspfString(units).trim().upcase();
   std::ostringstream out;

   out << std::setw(3)
       << std::setfill(' ')
       << temp.c_str();
   
   memcpy(theLengthUnits, out.str().c_str(), 3);
}

void rspfNitfLocalCartographicTag::setEastingInterval(rspf_uint32 eastingInterval)
{
   rspf_uint32 temp = eastingInterval;

   if(temp > 99999)
   {
      temp = 99999;
   }
   
   std::ostringstream out;

   out << std::setw(5)
       << std::setfill('0')
       << temp;
   
   memcpy(theEastingInterval, out.str().c_str(), 5);
}

void rspfNitfLocalCartographicTag::setNorthingInterval(rspf_uint32 northingInterval)
{
   rspf_uint32 temp = northingInterval;

   if(temp > 99999)
   {
      temp = 99999;
   }
   
   std::ostringstream out;

   out << std::setw(5)
       << std::setfill('0')
       << temp;
   
   memcpy(theNorthingInterval, out.str().c_str(), 5);
}

void rspfNitfLocalCartographicTag::setEastingReferenceOrigin(rspf_float64 east)
{
   std::ostringstream out;

   out << std::setw(15)
       << std::setfill(' ')
       << std::setprecision(1)
       << std::setiosflags(std::ios::fixed)
       << east;
   
   memcpy(theEastingReferenceOrigin, out.str().c_str(), 15);
   
}

void rspfNitfLocalCartographicTag::setNorthingReferenceOrigin(rspf_float64 north)
{
   std::ostringstream out;

   out << std::setw(15)
       << std::setfill(' ')
       << std::setprecision(1)
       << std::setiosflags(std::ios::fixed)
       << north;
   
   memcpy(theNorthingReferenceOrigin, out.str().c_str(), 15);
}
