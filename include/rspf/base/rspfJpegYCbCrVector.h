//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfJpegYCbCrVector.h 14789 2009-06-29 16:48:14Z dburken $
#ifndef rspfJpegYCbCrVector_HEADER
#define rspfJpegYCbCrVector_HEADER
#include <iostream>
using namespace std;
#include <rspf/base/rspfConstants.h>

class rspfRgbVector;

class RSPFDLLEXPORT rspfJpegYCbCrVector
{
public:
   friend ostream& operator << (ostream& out, const rspfJpegYCbCrVector& data)
      {
         out << "<" << (long)data.theBuf[0] << ", "
             << (long)data.theBuf[1] << ", "
             << (long)data.theBuf[2] << ">";

         return out;
      }
   
   rspfJpegYCbCrVector(unsigned char Y=0,
                        unsigned char Cb=0,
                        unsigned char Cr=0)
      {
         theBuf[0] = Y;
         theBuf[1] = Cb;
         theBuf[2] = Cr;
      }
   
   rspfJpegYCbCrVector(const rspfRgbVector&);
   
   long clamp(long colorValue, unsigned char min=0, unsigned char max=255)const
      {
         colorValue = colorValue > max? max:colorValue;
         colorValue = colorValue < min? min:colorValue;

         return colorValue;
      }

   
   rspfJpegYCbCrVector& operator =(const rspfRgbVector&);
   
   unsigned char getY() const { return theBuf[0]; }
   unsigned char getCb()const { return theBuf[1]; }
   unsigned char getCr()const { return theBuf[2]; }
   void setY(unsigned char  Y)  { theBuf[0] = Y; }
   void setCb(unsigned char Cb) { theBuf[1] = Cb; }
   void setCr(unsigned char Cr) { theBuf[2] = Cr; }
   
protected:
   /*!
    * JpegYCbCr is an 8-bit YCbCr color model.  We
    * will let:
    * buf[0] = Y
    * buf[1] = Cb
    * buf[2] = Cr
    */
   unsigned char theBuf[3];
};

#endif
