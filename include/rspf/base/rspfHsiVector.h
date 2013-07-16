//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description:
//
//*************************************************************************
// $Id: rspfHsiVector.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfHsiVector_HEADER
#define rspfHsiVector_HEADER
#include <iostream>
#include <rspf/base/rspfConstants.h>

class rspfRgbVector;
class rspfNormRgbVector;

class RSPFDLLEXPORT rspfHsiVector
{
public:
   friend std::ostream& operator << (std::ostream& out, const rspfHsiVector & data)
   {
      out << "<" << data.theBuf[0] << ", "
          << data.theBuf[1] << ", "
          << data.theBuf[2] << ">";
      
      return out;
   }
   // assumed normalized rspf_float64s
   //
   //
   rspfHsiVector(rspf_float64 h=0, rspf_float64 s=0, rspf_float64 i=0)
   {
      theBuf[0] = h;
      theBuf[1] = s;
      theBuf[2] = i;
   }
   rspfHsiVector(const rspfRgbVector& rgb);
   rspfHsiVector(const rspfNormRgbVector& rgb);

   rspfHsiVector& operator =(const rspfRgbVector& rgb);
   rspfHsiVector& operator =(const rspfNormRgbVector& rgb);

   void setFromRgb(rspf_float64 r, rspf_float64 g, rspf_float64 b);
   
   rspf_float64 getH()const { return theBuf[0]; }
   rspf_float64 getS()const { return theBuf[1]; }
   rspf_float64 getI()const { return theBuf[2]; }
   void setH(rspf_float64 H) { theBuf[0] = H; }
   void setS(rspf_float64 S) { theBuf[1] = S; }
   void setI(rspf_float64 I) { theBuf[2] = I; }

   rspf_float64 clamp(rspf_float64 colorValue, rspf_float64 min=0, rspf_float64 max=1)const
      {
         colorValue = colorValue > max? max:colorValue;
         colorValue = colorValue < min? min:colorValue;
         
         return colorValue;
      }
   
protected:
   /*!
    * buf[0] = hue
    * buf[1] = saturation
    * buf[2] = intensity
    */
   rspf_float64 theBuf[3];
};

#endif
