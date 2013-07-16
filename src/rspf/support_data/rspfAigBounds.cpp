//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts  (gpotts@imagelinks.com)
//
//*******************************************************************
// $Id: rspfAigBounds.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <ostream>
#include <rspf/support_data/rspfAigBounds.h>
#include <rspf/base/rspfEndian.h>

std::ostream& operator <<(std::ostream& out, const rspfAigBounds& data)
{
   out << "ll_x:  " << data.ll_x << std::endl
       << "ll_y:  " << data.ll_y << std::endl
       << "ur_x:  " << data.ur_x << std::endl
       << "ur_y:  " << data.ur_y;
   
   return out;
}

rspfAigBounds::rspfAigBounds()
{
   reset();
}

void rspfAigBounds::reset()
{
   ll_x = 0.0;
   ll_y = 0.0;
   ur_x = 0.0;
   ur_y = 0.0;
}

bool rspfAigBounds::writeStream(std::ostream& out)
{
   rspfEndian endian;
   double tempDouble;
   
   if(endian.getSystemEndianType() == RSPF_LITTLE_ENDIAN)
   {
      tempDouble = ll_x;
      endian.swap(tempDouble);
      out.write((char*)(&tempDouble), 8);

      tempDouble = ll_y;
      endian.swap(tempDouble);
      out.write((char*)(&tempDouble), 8);
      
      tempDouble = ur_x;
      endian.swap(tempDouble);
      out.write((char*)(&tempDouble), 8);
      
      tempDouble = ur_y;
      endian.swap(tempDouble);
      out.write((char*)(&tempDouble), 8);
   }
   else
   {
      out.write((char*)(&ll_x), 8);
      out.write((char*)(&ll_y), 8);
      out.write((char*)(&ur_x), 8);
      out.write((char*)(&ur_y), 8);
   }

   return out.good();
}
