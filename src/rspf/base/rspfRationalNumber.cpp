//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
//*******************************************************************
// $Id: rspfRationalNumber.cpp 11347 2007-07-23 13:01:59Z gpotts $
#include <rspf/base/rspfRationalNumber.h>

// Normalisation
void rspfRationalNumber::normalize()
{
    if (theDen == 0)
    {
       return;
    }

    // Handle the case of zero separately, to avoid division by zero
    if (theNum == 0)
    {
       theDen = 1;
       return;
    }

    rspf_int32 g = rspf::gcd(theNum, theDen);
    
    theNum /= g;
    theDen /= g;

    // Ensure that the denominator is positive
    if (theDen < 0)
    {
       theNum = -theNum;
       theDen = -theDen;
    }
}

const rspfRationalNumber& rspfRationalNumber::assign(double value, long precision)
{
  rspf_sint32 s = 1;
  if(value <= 0.0)
    {
      s = -1;
      value *= -1.0;
    }
  rspf_int32 integerPart = (rspf_int32)std::floor(value);
  rspf_int32 decimalPart = (rspf_int32)((value - integerPart)*precision);
  rspfRationalNumber temp(integerPart);
  rspfRationalNumber temp2(decimalPart, precision);
  temp2.normalize();
  *this = (temp + temp2);
  theNum *= s; 
  normalize();
  return *this;
}

const rspfRationalNumber& rspfRationalNumber::operator+= (const rspfRationalNumber& r)
{
    // This calculation avoids overflow, and minimises the number of expensive
    // calculations. Thanks to Nickolay Mladenov for this algorithm.
    //
    // Proof:
    // We have to compute a/b + c/d, where gcd(a,b)=1 and gcd(b,c)=1.
    // Let g = gcd(b,d), and b = b1*g, d=d1*g. Then gcd(b1,d1)=1
    //
    // The result is (a*d1 + c*b1) / (b1*d1*g).
    // Now we have to normalize this ratio.
    // Let's assume h | gcd((a*d1 + c*b1), (b1*d1*g)), and h > 1
    // If h | b1 then gcd(h,d1)=1 and hence h|(a*d1+c*b1) => h|a.
    // But since gcd(a,b1)=1 we have h=1.
    // Similarly h|d1 leads to h=1.
    // So we have that h | gcd((a*d1 + c*b1) , (b1*d1*g)) => h|g
    // Finally we have gcd((a*d1 + c*b1), (b1*d1*g)) = gcd((a*d1 + c*b1), g)
    // Which proves that instead of normalizing the result, it is better to
    // divide num and den by gcd((a*d1 + c*b1), g)

    rspf_int32 g = rspf::gcd(theDen, r.theDen);
    theDen /= g;  // = b1 from the calculations above
    theNum = theNum * (r.theDen / g) + r.theNum * theDen;
    g = rspf::gcd(theNum, g);
    theNum /= g;
    theDen *= r.theDen/g;

    return *this;
}

const rspfRationalNumber& rspfRationalNumber::operator-= (const rspfRationalNumber& r)
{
    // This calculation avoids overflow, and minimises the number of expensive
    // calculations. It corresponds exactly to the += case above
    rspf_int32 g = rspf::gcd(theDen, r.theDen);
    theDen /= g;
    theNum = theNum * (r.theDen / g) - r.theNum * theDen;
    g = rspf::gcd(theNum, g);
    theNum /= g;
    theDen *= r.theDen/g;

    return *this;
}

const rspfRationalNumber& rspfRationalNumber::operator*= (const rspfRationalNumber& r)
{
    // Avoid overflow and preserve normalization
    rspf_int32 gcd1 = rspf::gcd(theNum, r.theDen);
    rspf_int32 gcd2 = rspf::gcd(r.theNum, theDen);
    theNum = (theNum/gcd1) * (r.theNum/gcd2);
    theDen = (theDen/gcd2) * (r.theDen/gcd1);
    
    return *this;
}

const rspfRationalNumber& rspfRationalNumber::operator/= (const rspfRationalNumber& r)
{
   rspf_int32 zero(0);
   
   if (r.theNum == zero)
   {
      theNum = RSPF_INT_NAN;
      theDen = RSPF_INT_NAN;
      
      return *this;
   }
   *this = (*this)*(rspfRationalNumber(r.theDen, r.theNum));

   return *this;
}

// Mixed-mode operators
const rspfRationalNumber& rspfRationalNumber::operator+= (rspf_int32 i)
{
    return operator += (rspfRationalNumber(i));
}

const rspfRationalNumber& rspfRationalNumber::operator-= (rspf_int32 i)
{
    return operator -= (rspfRationalNumber(i));
}

const rspfRationalNumber& rspfRationalNumber::operator*= (rspf_int32 i)
{
    return operator *= (rspfRationalNumber(i));
}

const rspfRationalNumber& rspfRationalNumber::operator/= (rspf_int32 i)
{
    return operator /= (rspfRationalNumber(i));
}

rspfRationalNumber rspfRationalNumber::operator+ (const rspfRationalNumber& r)const
{
    // This calculation avoids overflow, and minimises the number of expensive
    // calculations. Thanks to Nickolay Mladenov for this algorithm.
    //
    // Proof:
    // We have to compute a/b + c/d, where gcd(a,b)=1 and gcd(b,c)=1.
    // Let g = gcd(b,d), and b = b1*g, d=d1*g. Then gcd(b1,d1)=1
    //
    // The result is (a*d1 + c*b1) / (b1*d1*g).
    // Now we have to normalize this ratio.
    // Let's assume h | gcd((a*d1 + c*b1), (b1*d1*g)), and h > 1
    // If h | b1 then gcd(h,d1)=1 and hence h|(a*d1+c*b1) => h|a.
    // But since gcd(a,b1)=1 we have h=1.
    // Similarly h|d1 leads to h=1.
    // So we have that h | gcd((a*d1 + c*b1) , (b1*d1*g)) => h|g
    // Finally we have gcd((a*d1 + c*b1), (b1*d1*g)) = gcd((a*d1 + c*b1), g)
    // Which proves that instead of normalizing the result, it is better to
    // divide num and den by gcd((a*d1 + c*b1), g)

   rspf_int32 g = rspf::gcd(theDen, r.theDen);
   rspf_int32 den = theDen;
   rspf_int32 num = theNum;
   den /= g;  // = b1 from the calculations above
   num = num * (r.theDen / g) + r.theNum * den;
   g = rspf::gcd(num, g);
   num /= g;
   den *= r.theDen/g;

    return rspfRationalNumber(num, den);
}

rspfRationalNumber rspfRationalNumber::operator-(const rspfRationalNumber& r)const
{
   rspfRationalNumber result = *this;
    // This calculation avoids overflow, and minimises the number of expensive
    // calculations. It corresponds exactly to the += case above
    rspf_int32 g = rspf::gcd(result.theDen, r.theDen);
    result.theDen /= g;
    result.theNum = result.theNum * (r.theDen / g) - r.theNum * result.theDen;
    g = rspf::gcd(result.theNum, g);
    result.theNum /= g;
    result.theDen *= r.theDen/g;

    return result;
}

rspfRationalNumber rspfRationalNumber::operator*(const rspfRationalNumber& r)const
{
   rspfRationalNumber result = *this;
   // Avoid overflow and preserve normalization
   rspf_int32 gcd1 = rspf::gcd(result.theNum, r.theDen);
   rspf_int32 gcd2 = rspf::gcd(r.theNum, result.theDen);
   result.theNum = (result.theNum/gcd1) * (r.theNum/gcd2);
   result.theDen = (result.theDen/gcd2) * (r.theDen/gcd1);
   
   return result;
}

rspfRationalNumber rspfRationalNumber::operator/(const rspfRationalNumber& r)const
{
   rspf_int32 zero(0);
   
   if (r.theNum == zero)
   {
      return rspfRationalNumber(RSPF_INT_NAN, RSPF_INT_NAN);;
   }
   
   return (*this)*(rspfRationalNumber(r.theDen, r.theNum));
}
