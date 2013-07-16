//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author:  David Burken
// 
// Description:
// Contains class definitions for rspfDpt.
// 
//*******************************************************************
//  $Id: rspfDpt.cpp 20204 2011-11-04 15:12:28Z dburken $

#include <iostream>
#include <iomanip>
#include <sstream>

#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfFpt.h>
#include <rspf/base/rspfGpt.h>

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfDpt::rspfDpt(const rspfFpt& pt)
   :
      x(pt.x), y(pt.y)
{
   if(pt.hasNans())
   {
      makeNan();
   }
 }

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfDpt::rspfDpt(const rspfIpt& pt)
   :
      x(pt.x), y(pt.y)
{
   if(pt.hasNans())
   {
      makeNan();
   }
}

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfDpt::rspfDpt(const rspfDpt3d &pt)
   :
      x(pt.x), y(pt.y)
{
}

//*******************************************************************
// Public Constructor:
//*******************************************************************
rspfDpt::rspfDpt(const rspfGpt &pt)
   :
      x(pt.lon), y(pt.lat)
{
}

//*******************************************************************
// Public Method:
//*******************************************************************
const rspfDpt& rspfDpt::operator=(const rspfFpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = pt.x;
      y = pt.y;
   }
   return *this;
}

//*******************************************************************
// Public Method:
//*******************************************************************
const rspfDpt& rspfDpt::operator=(const rspfIpt& pt)
{
   if(pt.hasNans())
   {
      makeNan();
   }
   else
   {
      x = pt.x;
      y = pt.y;
   }
   return *this;
}

//*******************************************************************
// Public Method:
//*******************************************************************
const rspfDpt& rspfDpt::operator=(const rspfDpt3d& pt)
{
   x = pt.x;
   y = pt.y;
   return *this;
}

const rspfDpt& rspfDpt::operator=(const rspfGpt& pt)
{
   x = pt.lon;
   y = pt.lat;
   return *this;
}

std::ostream& rspfDpt::print(std::ostream& os, rspf_uint32 precision) const
{
   os << std::setiosflags(std::ios::fixed) << std::setprecision(precision)
      << "( ";

   if (rspf::isnan(x) == false)
   {
      os << x;
   }
   else
   {
      os << "nan";
   }
   
   os << ", ";

   if (rspf::isnan(y) == false)
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

std::ostream& operator<<(std::ostream& os, const rspfDpt& pt)
{
   return pt.print(os);
}

bool rspfDpt::isEqualTo(const rspfDpt& rhs, rspfCompareType /* compareType */)const
{
   if(rhs.isNan()&&isNan()) return true;
   return (rspf::almostEqual(x, rhs.x)&&
           rspf::almostEqual(y, rhs.y));
}

rspfString rspfDpt::toString(rspf_uint32 precision) const
{
   std::ostringstream os;
   os << std::setprecision(precision);

   os << "(";
   if (rspf::isnan(x) == false)
   {
      os << x;
   }
   else
   {
      os << "nan";
   }
   
   os << ",";
   
   if (rspf::isnan(y) == false)
   {
      os << y;
   }
   else
   {
      os << "nan";
   }
   
   os << ")";
   
   //print(os, precision);
   return rspfString(os.str());
}

void rspfDpt::toPoint(const std::string& s)
{
   std::istringstream is(s);
   is >> *this;
}

std::istream& operator>>(std::istream& is, rspfDpt &pt)
{
   //---
   // Expected input format:
   // ( 30.00000000000000, -90.00000000000000 )
   //   --------x--------  ---------y--------
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
      pt.x = os.toFloat64();
   }
   else
   {
      pt.x = rspf::nan();
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
      pt.y = os.toFloat64();
   }
   else
   {
      pt.y = rspf::nan();
   }

   // Gobble the trailing ")".
   char c = '\0';
   while (c != ')')
   {
      is.get(c);
      if (!is) break;
   }

   // Finished
   return is;
}
