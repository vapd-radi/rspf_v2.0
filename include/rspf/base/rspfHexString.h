//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
// 
//********************************************************************
// $Id: rspfHexString.h 12952 2008-06-01 16:23:19Z dburken $

#include <cstring>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfConstants.h>

class RSPFDLLEXPORT rspfHexString : public rspfString
{
public:
   rspfHexString():rspfString()
      {}
   template <class Iter>
      rspfHexString(Iter start, Iter end):rspfString(start, end){}
   rspfHexString(const std::string& aString):rspfString(aString)
      {}
   rspfHexString(const char *aString):rspfString(aString?aString:"")
      {}
   rspfHexString(const rspfString& aString):rspfString(aString.c_str())
      {}
   
   rspfHexString(char value):rspfString()
      {
         assign(value);
      }
   rspfHexString(rspf_int16 value):rspfString()
      {
         assign(value);
      }
   rspfHexString(rspf_int32 value):rspfString()
      {
         assign(value);
      }
   rspfHexString(rspf_uint8 value):rspfString()
      {
         assign(value);
      }
   rspfHexString(rspf_uint16 value):rspfString()
      {
         assign(value);
      }
   rspfHexString(rspf_uint32 value):rspfString()
      {
         assign(value);
      }

   bool operator==(const rspfHexString& s) const {return (std::strcmp(this->c_str(),s.c_str())==0);}
   bool operator==(const char* s) const {return (std::strcmp(this->c_str(),s)==0);}
   bool operator!=(const rspfHexString& s) const {return (std::strcmp(this->c_str(),s.c_str())!=0);}
   bool operator!=(const char* s) const {return (std::strcmp(this->c_str(),s)!=0);}
   operator const char*()const{return c_str();}
   const char* chars()const{return c_str();}
   char& operator[](int i)
      {
         return *( const_cast<char*>(c_str())+i);
      }
   const char& operator[](int i)const
      {
         return *(c_str()+i);
      }
   const rspfHexString& operator =(rspf_int16 value)
      {
         assign(value);
         return *this;
      }
   const rspfHexString& operator =(rspf_int32 value)
      {
         assign(value);
         return *this;
      }
   const rspfHexString& operator =(rspf_uint16 value)
      {
         assign(value);
         return *this;
      }
   const rspfHexString& operator =(rspf_uint32 value)
      {
         assign(value);
         return *this;
      }
   const rspfHexString& operator =(rspf_int8 value)
      {
         assign(value);
         return *this;
      }
   const rspfHexString& operator =(rspf_uint8 value)
      {
         assign(value);
         return *this;
      }
   const rspfHexString& operator =(const rspf_int8* value)
      {
         rspfString::operator=(value);
         return *this;
      }
   const rspfHexString& operator =(const rspfString& value)
      {
         rspfString::operator=(value);
         return *this;
      }

   const rspfHexString& operator =(const rspfHexString& value)
      {
         rspfString::operator=(value);
         return *this;
      }

   const rspfHexString& operator =(const std::string& value)
      {
         rspfString::operator=(value);
         return *this;
      }
   
   rspf_int32  toInt32()const;
   rspf_int16  toInt16()const;
   rspf_int8   toChar()const;
   rspf_uint8  toUchar()const;
   rspf_uint32 toUint32()const;
   rspf_uint16 toUint16()const;

   void assign(rspf_int16 value)
      {
         assign((rspf_uint16)value);
      }
   void assign(rspf_int32 value)
      {
         assign((rspf_uint32)value);
      }
   void assign(rspf_int8 value)
      {
         assign((rspf_uint8)value);
      }
   void assign(rspf_uint16 value);
   void assign(rspf_uint32 value);
   void assign(rspf_uint8  value);
};
