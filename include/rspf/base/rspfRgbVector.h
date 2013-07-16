//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfRgbVector.h 14789 2009-06-29 16:48:14Z dburken $
#ifndef rspfRgbVector_HEADER
#define rspfRgbVector_HEADER
#include <iostream>
using namespace std;
#include <rspf/base/rspfConstants.h>

class rspfJpegYCbCrVector;
class rspfHsiVector;
class rspfHsvVector;
class rspfCmyVector;

class RSPFDLLEXPORT rspfRgbVector
{
public:
   friend ostream& operator <<(ostream& out, const rspfRgbVector& data)
      {
         out << "<" << (long)data.theBuf[0] << ", "
             << (long)data.theBuf[1] << ", "
             << (long)data.theBuf[2] << ">";

         return out;
      }
   rspfRgbVector(unsigned char r=0, unsigned char g=0, unsigned char b=0)
      {
         theBuf[0] = r;
         theBuf[1] = g;
         theBuf[2] = b;
      }
   rspfRgbVector(unsigned char grey)
      {
         theBuf[0] = grey;
         theBuf[1] = grey;
         theBuf[2] = grey;
      }
   rspfRgbVector(unsigned char colorData[3])
      {
         theBuf[0] = colorData[0];
         theBuf[1] = colorData[1];
         theBuf[2] = colorData[2];
      }

   rspfRgbVector(const rspfJpegYCbCrVector& YCbCr);
   rspfRgbVector(const rspfHsiVector& hsi);
   rspfRgbVector(const rspfHsvVector& hsv);
   rspfRgbVector(const rspfCmyVector& cmy);

   rspfRgbVector operator -(const rspfRgbVector& rgb)const
      {
         return rspfRgbVector((unsigned char)clamp((long)theBuf[0] - (long)rgb.theBuf[0]),
                               (unsigned char)clamp((long)theBuf[1] - (long)rgb.theBuf[1]),
                               (unsigned char)clamp((long)theBuf[2] - (long)rgb.theBuf[2]));
      }

   rspfRgbVector operator +(const rspfRgbVector& rgb)const
      {
         return rspfRgbVector((unsigned char)clamp((long)theBuf[0] + (long)rgb.theBuf[0]),
                               (unsigned char)clamp((long)theBuf[1] + (long)rgb.theBuf[1]),
                               (unsigned char)clamp((long)theBuf[2] + (long)rgb.theBuf[2]));
      }
   rspfRgbVector operator *(double t)const
      {
         return rspfRgbVector((unsigned char)clamp((long)(theBuf[0]*t)),
                               (unsigned char)clamp((long)(theBuf[1]*t)),
                               (unsigned char)clamp((long)(theBuf[2]*t)));
      }
   const rspfRgbVector& operator =(const rspfJpegYCbCrVector& data);
   const rspfRgbVector& operator =(const rspfHsiVector& hsi);
   const rspfRgbVector& operator =(const rspfHsvVector& hsv);
   const rspfRgbVector& operator =(const rspfCmyVector& cmy);
   bool operator ==(const rspfRgbVector& rgb)
      {
         return ( (rgb.theBuf[0] == theBuf[0])&&
                  (rgb.theBuf[1] == theBuf[1])&&
                  (rgb.theBuf[2] == theBuf[2]));
      }
   bool operator !=(const rspfRgbVector& rgb)
      {
         return ( (rgb.theBuf[0] != theBuf[0])||
                  (rgb.theBuf[1] != theBuf[1])||
                  (rgb.theBuf[2] != theBuf[2]));
      }
   static long clamp(long colorValue, unsigned char min=0, unsigned char max=255)
      {
         colorValue = colorValue > (long)max? (long)max:colorValue;
         colorValue = colorValue < (long)min? (long)min:colorValue;
         
         return colorValue;
      }
   unsigned char getR() const { return theBuf[0]; }
   unsigned char getG()const  { return theBuf[1]; }
   unsigned char getB()const  { return theBuf[2]; }
   void setR(unsigned char  R) { theBuf[0] = R; }
   void setG(unsigned char G)  { theBuf[1] = G; }
   void setB(unsigned char B)  { theBuf[2] = B; }
   
protected:
   unsigned char theBuf[3];
};

#endif
