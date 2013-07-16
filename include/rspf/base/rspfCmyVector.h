//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfCmyVector.h 14789 2009-06-29 16:48:14Z dburken $
#include <iostream>
#include <rspf/base/rspfConstants.h>

class rspfJpegYCbCrVector;
class rspfHsiVector;
class rspfHsvVector;
class rspfRgbVector;

class RSPFDLLEXPORT rspfCmyVector
{
public:
   friend std::ostream& operator <<(std::ostream& out, const rspfCmyVector& cmy)
      {
         out << "<" << cmy.theC << ", " << cmy.theM <<", " << cmy.theY << ">" << std::endl;
         
         return out;
      }
   rspfCmyVector(unsigned char c,
                  unsigned char m,
                  unsigned char y)
      :
         theC(c),
         theM(m),
         theY(y)
      {}
   rspfCmyVector(const rspfRgbVector& rgb);

   rspf_uint8 getC()const
      {
         return theC;
      }
   rspf_uint8 getM()const
      {
         return theC;
      }
   rspf_uint8 getY()const
      {
         return theC;
      }
   const rspfCmyVector& operator = (const rspfRgbVector& rgb);
protected:
   rspf_uint8 theC;
   rspf_uint8 theM;
   rspf_uint8 theY;
};
