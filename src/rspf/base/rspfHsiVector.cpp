//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@remotesensing.org)
// Description:
//
//*************************************************************************
// $Id: rspfHsiVector.cpp 11955 2007-10-31 16:10:22Z gpotts $
#include <rspf/base/rspfHsiVector.h>
#include <rspf/base/rspfRgbVector.h>
#include <rspf/base/rspfNormRgbVector.h>
#include <math.h>
#include <rspf/base/rspfCommon.h>

// nonstandard versions that use operator>, so they behave differently
// than std:::min/max and rspf::min/max.  kept here for now for that
// reason.
#ifndef MAX
#  define MAX(x,y) ((x)>(y)?(x):(y))
#  define MIN(x,y) ((x)>(y)?(y):(x))
#endif

rspfHsiVector::rspfHsiVector(const rspfRgbVector& rgb)
{
   setFromRgb(rgb.getR()/255.0, rgb.getG()/255.0, rgb.getB()/255.0);
}

rspfHsiVector::rspfHsiVector(const rspfNormRgbVector& rgb)
{
   setFromRgb(rgb.getR(), rgb.getG(), rgb.getB());
}

rspfHsiVector& rspfHsiVector::operator =(const rspfRgbVector& rgb)
{
   setFromRgb(rgb.getR()/255.0, rgb.getG()/255.0, rgb.getB()/255.0);
   
   return *this;
   
}

rspfHsiVector& rspfHsiVector::operator =(const rspfNormRgbVector& rgb)
{
   setFromRgb(rgb.getR(), rgb.getG(), rgb.getB());
   
   return *this;
   
}

void rspfHsiVector::setFromRgb(rspf_float64 r, rspf_float64 g, rspf_float64 b)
{
   rspf_float64 sum = r + g + b;
   
   theBuf[2] =  sum/3;

   if(theBuf[2] > FLT_EPSILON)
   {
      double deltaI1I2 = r - g;
      double root = deltaI1I2*deltaI1I2 +
                    ((r-b)*(g-b));

      // compute Saturation from RGB
      theBuf[1] = 1 - (3.0/sum)*(MIN(MIN(r,g),b));
      
      // compte Hue from Rgb.
      if(root >= FLT_EPSILON)
      {
         theBuf[0] = acos((.5*((r-g)+(r-b)))/
                          sqrt(root))*DEG_PER_RAD;

         if(b > g)
         {
            theBuf[0] = 360 - theBuf[0];
         }
      }
      else
      {
         theBuf[0] = b;
      }
   }
   else
   {
      theBuf[0] = 0;
      theBuf[1] = 0;
      theBuf[2] = 0;
   }

   setH(theBuf[0]);
   setS(theBuf[1]);
   setI(theBuf[2]);
}
