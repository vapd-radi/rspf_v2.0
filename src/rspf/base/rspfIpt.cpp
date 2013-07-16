//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
// 
// Description:
//
// Contains class definitions for ipt.
//
//*******************************************************************
//  $Id: rspfIpt.cpp 20070 2011-09-07 18:48:35Z dburken $

#include <iostream>
#include <sstream>

#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfFpt.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfString.h>

//*******************************************************************
// Public constructor:
//*******************************************************************
rspfIpt::rspfIpt(const rspfDpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = rspf::round<int>(pt.x);
      y = rspf::round<int>(pt.y);
   }
}

//*******************************************************************
// Public constructor:
//*******************************************************************
rspfIpt::rspfIpt(const rspfFpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = rspf::round<int>(pt.x);
      y = rspf::round<int>(pt.y);
   }
}

rspfIpt::rspfIpt(const rspfDpt3d &pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = rspf::round<int>(pt.x);
      y = rspf::round<int>(pt.y);
   }
}
//*******************************************************************
// Public method:
//*******************************************************************
const rspfIpt& rspfIpt::operator=(const rspfDpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = rspf::round<int>(pt.x);
      y = rspf::round<int>(pt.y);
   }
   
   return *this;
}

//*******************************************************************
// Public method:
//*******************************************************************
const rspfIpt& rspfIpt::operator=(const rspfFpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = rspf::round<int>(pt.x);
      y = rspf::round<int>(pt.y);
   }
   
   return *this;
}

std::ostream& rspfIpt::print(std::ostream& os) const
{
   os << "( ";

   if (x != RSPF_INT_NAN)
   {
      os << x;
   }
   else
   {
      os << "nan";
   }
   
   os << ", ";

   if (y != RSPF_INT_NAN)
   {
      os << y;
   }
   else
   {
      os << "nan";
   }

   os << " )";

   return os;
}

std::ostream& operator<<(std::ostream& os, const rspfIpt& pt)
{
   return pt.print(os);
}

rspfString rspfIpt::toString() const
{
   std::ostringstream os;
   os << "(";
   
   if (x != RSPF_INT_NAN)
   {
      os << x;
   }
   else
   {
      os << "nan";
   }
   
   os << ",";
   
   if (y != RSPF_INT_NAN)
   {
      os << y;
   }
   else
   {
      os << "nan";
   }
   
   os << ")";
   
   return rspfString(os.str());
}


void rspfIpt::toPoint(const std::string& s)
{
   std::istringstream is(s);
   is >> *this;
}

std::istream& operator>>(std::istream& is, rspfIpt &pt)
{
   //---
   // Expected input format:
   // ( 30, -90 )
   //   -x- -y-
   //---

   // Start with a nan point.
   pt.makeNan();

   // Check the stream.
   if (!is) return is;
   
   const int SZ = 64; // Handle real big number...
   rspfString os;
   char buf[SZ];

   //---
   // X SECTION:
   //---
   
   // Grab data up to the first comma.
   is.get(buf, SZ, ',');

   if (!is) return is;

   // Copy to rspf string.
   os = buf;

   // Get rid of the '(' if there is any.
   std::string::size_type pos = os.find('(');
   if (pos != std::string::npos)
   {
      os.erase(pos, 1);
   }   
   if (os.contains("nan") == false)
   {
      pt.x = os.toInt32();
   }
   else
   {
      pt.x = RSPF_INT_NAN;
   }

   //---
   // Y SECTION:
   //---
   
   // Grab the data up to the ')'
   is.get(buf, SZ, ')');

   if (!is) return is;

   // Copy to rspf string.
   os = buf;

   // Get rid of the ',' if there is any.
   pos = os.find(',');
   if (pos != std::string::npos)
   {
      os.erase(pos, 1);
   }
   
   if (os.contains("nan") == false)
   {
      pt.y = os.toInt32();
   }
   else
   {
      pt.y = RSPF_INT_NAN;
   }

   // Gobble the trailing ")".
   char c = 0;
   while (c != ')')
   {
      is.get(c);
      if (!is) break;
   }

   // Finished
   return is;
}

bool rspfIpt::isEqualTo(const rspfIpt& rhs, rspfCompareType /* compareType */)const
{
   return ((x==rhs.x)&&
           (y==rhs.y));
}

