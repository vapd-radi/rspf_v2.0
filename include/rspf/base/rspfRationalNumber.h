//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Garrett Potts (gpotts@imagelinks.com)
//
//*******************************************************************
//  $Id: rspfRationalNumber.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfRationalNumber_HEADER
#define rspfRationalNumber_HEADER
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfConstants.h>
#include <iostream>

class RSPFDLLEXPORT rspfRationalNumber
{
public:

   friend std::ostream& operator<<(std::ostream& out, const rspfRationalNumber& rhs)
      {
         out<<rhs.theNum << "/" << rhs.theDen;
         
         return out;
      }
   rspfRationalNumber()
      :theNum(1),
       theDen(1)
      {
      }
   rspfRationalNumber(rspf_int32 num,
                       rspf_int32 den)
      :theNum(num),
       theDen(den)
      {
      }
   rspfRationalNumber(rspf_int32 num)
      :theNum(num),
       theDen(1)
      {}
   double toDouble()const
      {
         return (static_cast<double>(theNum)/
                 static_cast<double>(theDen));
      }
   float toFloat()const
      {
         return (static_cast<float>(theNum)/
                 static_cast<float>(theDen));
      }

   void normalize();
   
   const rspfRationalNumber& operator=(rspf_int32 n)
      {
         return assign(n, 1);
      }

   const rspfRationalNumber& operator=(double n)
      {
         return assign(n);
      }

   rspfRationalNumber operator-()const
      {
         return rspfRationalNumber(-theNum, theDen);
      }
   const rspfRationalNumber& operator+= (const rspfRationalNumber& r);
   const rspfRationalNumber& operator-= (const rspfRationalNumber& r);
   const rspfRationalNumber& operator*= (const rspfRationalNumber& r);
   const rspfRationalNumber& operator/= (const rspfRationalNumber& r);
   
   inline const rspfRationalNumber& operator+= (rspf_int32 i);
   inline const rspfRationalNumber& operator-= (rspf_int32 i);
   inline const rspfRationalNumber& operator*= (rspf_int32 i);
   inline const rspfRationalNumber& operator/= (rspf_int32 i);

   rspfRationalNumber operator+ (const rspfRationalNumber& r)const;
   rspfRationalNumber operator- (const rspfRationalNumber& r)const;
   rspfRationalNumber operator* (const rspfRationalNumber& r)const;
   rspfRationalNumber operator/ (const rspfRationalNumber& r)const;
   
   inline rspfRationalNumber operator+ (rspf_int32 i)const;
   inline rspfRationalNumber operator- (rspf_int32 i)const;
   inline rspfRationalNumber operator* (rspf_int32 i)const;
   inline rspfRationalNumber operator/ (rspf_int32 i)const;

   friend inline rspfRationalNumber operator+ (rspf_int32 i, rspfRationalNumber& r);
   friend inline rspfRationalNumber operator- (rspf_int32 i, rspfRationalNumber& r);
   friend inline rspfRationalNumber operator* (rspf_int32 i, rspfRationalNumber& r);
   friend inline rspfRationalNumber operator/ (rspf_int32 i, rspfRationalNumber& r);
   
     // Comparison operators
//   inline bool operator< (const rspfRationalNumber& r) const;
   inline bool operator== (const rspfRationalNumber& r) const;
   
   inline bool operator== (rspf_int32 i) const;
   
   // Increment and decrement
   inline const rspfRationalNumber& operator++();
   inline const rspfRationalNumber& operator--();
   
   // Assign in place
   inline const rspfRationalNumber& assign(rspf_int32 n, rspf_int32 d);
  /*!
   * default estimation is out to the 10000 place.  Will set this 
   * rational to a value that is close to the passed in number.
   */
  const rspfRationalNumber& assign(double value, long precision=10000);

   /*!
    * Holds the value of the numberator.
    */
   rspf_int32 theNum;

   /*!
    * Holds the value of the denominator.
    */
   rspf_int32 theDen;

};

inline const rspfRationalNumber& rspfRationalNumber::assign(rspf_int32 n, rspf_int32 d)
{
   theNum = n;
   theDen = d;
   normalize();
   
   return *this;  
}

// Mixed-mode operators
inline rspfRationalNumber rspfRationalNumber::operator+(rspf_int32 i)const
{
    return operator + (rspfRationalNumber(i));
}

inline rspfRationalNumber rspfRationalNumber::operator-(rspf_int32 i)const
{
    return operator - (rspfRationalNumber(i));
}

inline rspfRationalNumber rspfRationalNumber::operator*(rspf_int32 i)const
{
    return operator * (rspfRationalNumber(i));
}

inline rspfRationalNumber rspfRationalNumber::operator/(rspf_int32 i)const
{
    return operator / (rspfRationalNumber(i));
}

inline rspfRationalNumber operator+ (rspf_int32 i, rspfRationalNumber& r)
{
   return rspfRationalNumber(i)+r;
}

inline rspfRationalNumber operator- (rspf_int32 i, rspfRationalNumber& r)
{
   return rspfRationalNumber(i)-r;
}

inline rspfRationalNumber operator* (rspf_int32 i, rspfRationalNumber& r)
{
   return rspfRationalNumber(i)*r;
}

inline rspfRationalNumber operator/ (rspf_int32 i, rspfRationalNumber& r)
{
   return rspfRationalNumber(i)/r;
}

inline const rspfRationalNumber& rspfRationalNumber::operator++()
{
   theNum += theDen;
   
   return *this;
}

inline const rspfRationalNumber& rspfRationalNumber::operator--()
{
   theNum -= theDen;
   
   return *this;   
}

inline bool rspfRationalNumber::operator== (const rspfRationalNumber& r)const
{
    return ((theNum == r.theNum) && (theDen == r.theDen));
}

inline bool rspfRationalNumber::operator== (rspf_int32 i) const
{
    return ((theDen == rspf_int32(1)) && (theNum == i));
}

#endif
