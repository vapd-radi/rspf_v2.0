//*******************************************************************
// Copyright (C) 2005 Garrett Potts
//
// License: LGPL
// Author: Garrett Potts 
// Description:
//
//*************************************************************************
// $Id: rspfNormRgbVector.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfNormRgbVector_HEADER
#define rspfNormRgbVector_HEADER
#include <iostream>
#include <rspf/base/rspfConstants.h>

class rspfJpegYCbCrVector;
class rspfHsiVector;
class rspfHsvVector;
class rspfCmyVector;
class rspfRgbVector;

class RSPF_DLL rspfNormRgbVector
{
public:
   friend std::ostream& operator <<(std::ostream& out, const rspfNormRgbVector& data)
   {
      out << "<" << data.theBuf[0] << ", "
          << data.theBuf[1] << ", "
          << data.theBuf[2] << ">";

         return out;
      }
   rspfNormRgbVector(rspf_float64 r=0, rspf_float64 g=0, rspf_float64 b=0)
      {
         theBuf[0] = r;
         theBuf[1] = g;
         theBuf[2] = b;
      }
   rspfNormRgbVector(rspf_float64 grey)
      {
         theBuf[0] = grey;
         theBuf[1] = grey;
         theBuf[2] = grey;
      }
   rspfNormRgbVector(rspf_float64 colorData[3])
      {
         theBuf[0] = colorData[0];
         theBuf[1] = colorData[1];
         theBuf[2] = colorData[2];
      }
   rspfNormRgbVector(const rspfNormRgbVector& rgb)
   {
      theBuf[0] = rgb.theBuf[0];
      theBuf[1] = rgb.theBuf[1];
      theBuf[2] = rgb.theBuf[2];
   }
   rspfNormRgbVector(const rspfRgbVector& rgb);
   rspfNormRgbVector(const rspfJpegYCbCrVector& YCbCr);
   rspfNormRgbVector(const rspfHsiVector& hsi);
   rspfNormRgbVector(const rspfHsvVector& hsv);
   rspfNormRgbVector(const rspfCmyVector& cmy);
   rspfNormRgbVector operator -(const rspfNormRgbVector& rgb)const
   {
      return rspfNormRgbVector(clamp(theBuf[0] - rgb.theBuf[0]),
                                clamp(theBuf[1] - rgb.theBuf[1]),
                                clamp(theBuf[2] - rgb.theBuf[2]));
   }
   rspfNormRgbVector operator *(rspf_float64 t)const
   {
      return rspfNormRgbVector(clamp(theBuf[0]*t),
                                clamp(theBuf[1]*t),
                                clamp(theBuf[2]*t));
   }
   
   const rspfNormRgbVector& operator =(const rspfJpegYCbCrVector& data);
   const rspfNormRgbVector& operator =(const rspfHsiVector& hsi);
   const rspfNormRgbVector& operator =(const rspfHsvVector& hsv);
   const rspfNormRgbVector& operator =(const rspfCmyVector& cmy);
   bool operator ==(const rspfNormRgbVector& rgb)
      {
         return ( (rgb.theBuf[0] == theBuf[0])&&
                  (rgb.theBuf[1] == theBuf[1])&&
                  (rgb.theBuf[2] == theBuf[2]));
      }
   bool operator !=(const rspfNormRgbVector& rgb)
      {
         return ( (rgb.theBuf[0] != theBuf[0])||
                  (rgb.theBuf[1] != theBuf[1])||
                  (rgb.theBuf[2] != theBuf[2]));
      }
   static rspf_float64 clamp(rspf_float64 colorValue, rspf_float64 min=0.0, rspf_float64 max=1.0)
      {
         colorValue = colorValue > max? max:colorValue;
         colorValue = colorValue < min? min:colorValue;
         
         return colorValue;
      }
   rspf_float64 getR() const { return theBuf[0]; }
   rspf_float64 getG()const  { return theBuf[1]; }
   rspf_float64 getB()const  { return theBuf[2]; }
   void setR(rspf_float64 r) { theBuf[0] = r; }
   void setG(rspf_float64 g) { theBuf[1] = g; }
   void setB(rspf_float64 b) { theBuf[2] = b; }
   
protected:
   rspf_float64 theBuf[3];
   
};

#endif
