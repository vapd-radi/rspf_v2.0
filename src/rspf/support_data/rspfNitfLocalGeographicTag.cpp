//*******************************************************************
// Copyright (C) 2004 Intelligence Data Systems. 
//
// LICENSE: LGPL
//
// see top level LICENSE.txt
// 
// Author: Garrett Potts
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfLocalGeographicTag.cpp 22013 2012-12-19 17:37:20Z dburken $
#include <rspf/support_data/rspfNitfLocalGeographicTag.h>
#include <iomanip>
#include <sstream>

RTTI_DEF1(rspfNitfLocalGeographicTag, "rspfNitfLocalGeographicTag", rspfNitfRegisteredTag);

rspfNitfLocalGeographicTag::rspfNitfLocalGeographicTag()
   : rspfNitfRegisteredTag(std::string("GEOLOB"), 48)
{
   clearFields();
}

rspfNitfLocalGeographicTag::~rspfNitfLocalGeographicTag()
{
}

void rspfNitfLocalGeographicTag::parseStream(std::istream& in)
{
   in.read(theLonDensity, 9);
   in.read(theLatDensity, 9);
   in.read(theLonOrigin, 15);
   in.read(theLatOrigin, 15);
}

void rspfNitfLocalGeographicTag::writeStream(std::ostream& out)
{
   out.write(theLonDensity, 9);
   out.write(theLatDensity, 9);
   out.write(theLonOrigin, 15);
   out.write(theLatOrigin, 15);
}

void rspfNitfLocalGeographicTag::clearFields()
{
   memset(theLonDensity, ' ', 9);
   memset(theLatDensity, ' ', 9);
   memset(theLonOrigin, '0', 15);
   memset(theLatOrigin, '0', 15);

   
   
   theLonDensity[9] = '\0';
   theLatDensity[9] = '\0';
   theLonOrigin[15] = '\0';
   theLatOrigin[14] = '\0';
}

void rspfNitfLocalGeographicTag::setDegreesPerPixelLat(double deltaLat)
{
   rspf_uint32 pixels = 0;
   if(deltaLat > 0.0)
   {
      pixels = (rspf_uint32)((1.0/deltaLat)*360.0 + .5);
   }
   std::ostringstream out;

   out << std::setw(9)
       << std::setfill('0')
       << pixels;

   memcpy(theLatDensity, out.str().c_str(), 9);
}

void rspfNitfLocalGeographicTag::setDegreesPerPixelLon(double deltaLon)
{
   rspf_uint32 pixels = 0;
   if(deltaLon > 0.0)
   {
      pixels = (rspf_uint32)((1.0/deltaLon)*360.0 + .5);
   }
   std::ostringstream out;

   out << std::setw(9)
       << std::setfill('0')
       << pixels;

   memcpy(theLonDensity, out.str().c_str(), 9);
}

void rspfNitfLocalGeographicTag::setLatOrigin(double origin)
{
   std::ostringstream out;

   out << std::setw(15)
       << std::setfill('0')
       << origin;
   memcpy(theLatOrigin, out.str().c_str(), 15);
}

void rspfNitfLocalGeographicTag::setLonOrigin(double origin)
{
   std::ostringstream out;

   out << std::setw(15)
       << std::setfill('0')
       << origin;
   memcpy(theLonOrigin, out.str().c_str(), 15);
}
