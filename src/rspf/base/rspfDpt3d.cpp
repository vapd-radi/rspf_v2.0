//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id: rspfDpt3d.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <sstream>
#include <rspf/base/rspfDpt3d.h>
#include <rspf/base/rspfIpt.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfString.h>


rspfDpt3d::rspfDpt3d(const rspfDpt &aPt)
   :x(aPt.x),
    y(aPt.y),
    z(0)
{
   if(aPt.isNan())
   {
      makeNan();
   }
}

rspfDpt3d::rspfDpt3d(const rspfIpt &aPt)
   :x(aPt.x),
    y(aPt.y),
    z(0)
{
   if(aPt.isNan())
   {
      makeNan();
   }
}

std::string rspfDpt3d::toString(rspf_uint32 precision) const
{
   std::ostringstream os;
   os <<  setprecision(precision);
   
   os << "(";
   if ( rspf::isnan(x) == false)
   {
      os << x;
   }
   else
   {
      os << "nan";
   }
   os << ",";
   if ( rspf::isnan(y) == false )
   {
      os << y;
   }
   else
   {
      os << "nan";
   }
   os << ",";
   if ( rspf::isnan(z) == false )
   {
      os << z;
   }
   else
   {
      os << "nan";
   }
   os << ")";
   
   return os.str();
}

void rspfDpt3d::toPoint(const std::string& s)
{
   // Nan out the column vector for starters.
   x = rspf::nan();
   y = rspf::nan();
   z = rspf::nan();
  
   std::istringstream is(s);

   // Check the stream.
   if (!is) return;

   //---
   // Expected input format:
   // ( 0.0000000, 0.0000000, 0.00000000 )
   //   -----x---- -----y---- -----z----
   //---
   
   const int SZ = 64; // Handle real big number...
   rspfString os;
   char buf[SZ];
   char c = 0;

   //---
   // X SECTION:
   //---
   
   // Grab data up to the first comma.
   is.get(buf, SZ, ',');

   if (!is) return;

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
      x = os.toFloat64();
   }
   else
   {
      x = rspf::nan();
   }

   // Eat the comma that we stopped at.
   while (c != ',')
   {
      is.get(c);
      if (!is) break;
   }
   
   //---
   // Y SECTION:
   //---
   
   // Grab the data up to the next ','
   is.get(buf, SZ, ',');

   if (!is) return;
   
   // Copy to rspf string.
   os = buf;
   
   if (os.contains("nan") == false)
   {
      y = os.toFloat64();
   }
   else
   {
      y = rspf::nan();
   }
   
   // Eat the comma that we stopped at.
   c = 0;
   while (c != ',')
   {
      is.get(c);
      if (!is) break;
   }

   //---
   // Z SECTION:
   //---
   
   // Grab the data up to the ')'
   is.get(buf, SZ, ')');
   
   if (!is) return;
   
   // Copy to rspf string.
   os = buf;
   
   if (os.contains("nan") == false)
   {
      z = os.toFloat64();
   }
   else
   {
      z = rspf::nan();
   }
}

std::ostream& operator<< (std::ostream& out, const rspfDpt3d &rhs)
{
   std::string s = rhs.toString(15);
   out << s;
   return out;
}
