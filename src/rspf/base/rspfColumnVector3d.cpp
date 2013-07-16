//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description: This is a 3-D vector without the homogeneous
//              coordinate.
//
//*******************************************************************
//  $Id: rspfColumnVector3d.cpp 15766 2009-10-20 12:37:09Z gpotts $

#include <sstream>
#include <rspf/base/rspfColumnVector3d.h>
#include <rspf/base/rspfCommon.h>

rspfString rspfColumnVector3d::toString(rspf_uint32 precision) const
{
   std::ostringstream os;
   os << setprecision(precision);
   
   os << "(";
   if ( rspf::isnan(data[0]) == false)
   {
      os << data[0];
   }
   else
   {
      os << "nan";
   }
   os << ",";
   if ( rspf::isnan(data[1]) == false )
   {
      os << data[1];
   }
   else
   {
      os << "nan";
   }
   os << ",";
   if ( rspf::isnan(data[2]) == false )
   {
      os << data[2];
   }
   else
   {
      os << "nan";
   }
   os << ")";
   
   return rspfString(os.str());
}

void rspfColumnVector3d::toPoint(const std::string& s)
{
   // Nan out the column vector for starters.
   data[0] = rspf::nan();
   data[1] = rspf::nan();
   data[2] = rspf::nan();
  
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
      data[0] = os.toFloat64();
   }
   else
   {
      data[0] = rspf::nan();
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
      data[1] = os.toFloat64();
   }
   else
   {
      data[1] = rspf::nan();
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
      data[2] = os.toFloat64();
   }
   else
   {
      data[2] = rspf::nan();
   }
}
