//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: This class extends the stl's string class.
// 
//********************************************************************
// $Id: rspfString.cpp 22160 2013-02-25 12:09:35Z gpotts $

#include <cctype> /* for toupper */
#include <cstdlib> /* for getenv() */
#include <stack> 
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfRegExp.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfHexString.h>

static rspfTrace traceDebug("rspfString:debug");

#ifdef RSPF_ID_ENABLED
static char RSPF_ID[] = "$Id: rspfString.cpp 22160 2013-02-25 12:09:35Z gpotts $";
#endif

rspfString rspfString::upcase(const rspfString& aString)
{
   std::string s = aString.string();
   
   std::string::iterator eachCharacter = s.begin();
   while(eachCharacter != s.end())
   {
      *eachCharacter = toupper(*eachCharacter);
      eachCharacter++;
   }

   return s;
}

rspfString rspfString::downcase(const rspfString& aString)
{
   std::string s = aString.m_str;

   std::string::iterator eachCharacter = s.begin();
   while(eachCharacter != s.end())
   {
      *eachCharacter = tolower(*eachCharacter);
      ++eachCharacter;
   }

   return rspfString(s);
}

rspfString& rspfString::upcase()
{
   std::string::iterator eachCharacter = m_str.begin();
   while(eachCharacter != m_str.end())
   {
      *eachCharacter = toupper(*eachCharacter);
      ++eachCharacter;
   }

   return *this;
}

rspfString rspfString::upcase()const
{
   rspfString result(*this);
   result.upcase();
   return result;
}

rspfString& rspfString::downcase()
{
   std::string::iterator eachCharacter = m_str.begin();
   while(eachCharacter != m_str.end())
   {
      *eachCharacter = tolower(*eachCharacter);
      ++eachCharacter;
   }
   
   return *this;
}

rspfString rspfString::downcase()const
{
   rspfString result(*this);
   result.downcase();
   return result;
}

char* rspfString::stringDup()const
{
   char *result = 0;

   if(length() == 0)
   {
      result = new char[1];
      result[0] = '\0';
   }
   else
   {  
      rspf_uint32 index = 0;
      rspf_uint32 len = (rspf_uint32)m_str.length();
      result = new char[len+1];
      const char* sourceString = m_str.c_str();

      while(index < len) 
      {
         result[index] = sourceString[index];
         ++index;
      }
      result[len] = '\0';
   }
   return result;
}

rspfString rspfString::stripLeading(const rspfString &value, char characterToStrip)
{
   std::string s;
   rspfString::const_iterator stringIter = value.m_str.begin();
   
   while((*stringIter == characterToStrip)&&(stringIter!=value.m_str.end()))
   {
      ++stringIter;
   }

   while(stringIter != value.m_str.end())
   {
      s += *stringIter;
      ++stringIter;
   }
   
   return rspfString(s);
}

rspfString rspfString::substitute(const rspfString &searchKey,
                                    const rspfString &replacementValue,
                                    bool replaceAll)const
{
   std::string result = m_str;

   size_type pos = result.find(searchKey.m_str);
   
   if (pos == std::string::npos) return result;  // Search key not found.
   
   if(replaceAll)
   {
      while(pos != std::string::npos)
      {
         result.replace(pos, searchKey.m_str.size(), replacementValue.m_str.c_str());
         pos = result.find(searchKey.m_str, pos+replacementValue.m_str.size());
      }
   }
   else  // Replace only the first instance.
   {
      result.replace(pos, searchKey.m_str.size(), replacementValue.m_str.c_str());
   }

   return rspfString(result);
}

rspfString& rspfString::gsub(const rspfString &searchKey,
                               const rspfString &replacementValue,
                               bool replaceAll)
{
   size_type pos = m_str.find(searchKey.m_str);
   
   if (pos == std::string::npos) return *this;  // Search key not found.
   
   if(replaceAll)
   {
      while(pos < m_str.size())
      {
         m_str.replace(pos, searchKey.m_str.size(), replacementValue.m_str.c_str());
         pos = find(searchKey.m_str, pos+replacementValue.m_str.size());
      }
   }
   else  // Replace only the first instance.
   {
      m_str.replace(pos, searchKey.m_str.size(), replacementValue.m_str.c_str());
   }

   return *this;
}

rspfString  rspfString::trim(const rspfString& valueToTrim) const
{
   rspfString tempString(*this);
   
   return tempString.trim(valueToTrim);
}

rspfString& rspfString::trim(const rspfString& valueToTrim)
{
   if(m_str.size() == 0) return *this;
   if(valueToTrim.empty()) return *this;
   iterator startPos = (*this).begin();
   iterator endPos   = (*this).begin() + ((*this).size()-1);

   while( ( startPos != (*this).end() ) &&
          (std::find(valueToTrim.begin(),
                     valueToTrim.end(),
                     *startPos)!=valueToTrim.end()) ) ++startPos;

   if(startPos == (*this).end())
   {
      *this = "";
      return *this;
   }
   
   while( (endPos!=startPos)&& (std::find(valueToTrim.begin(),
                                          valueToTrim.end(),
                                          *endPos)!=valueToTrim.end())) --endPos;

   *this = rspfString(startPos, endPos+1);

   return *this;
}

rspfString rspfString::beforePos(std::string::size_type pos)const
{
   rspfString result = *this;

   if(pos < length())
   {
      result.erase(pos, std::string::npos);
   }

   return result;
}

rspfString rspfString::afterPos(std::string::size_type pos)const
{
   rspfString result = *this;

   if(pos < length())
   {
      result.erase(0, pos+1);
   }
   else
   {
      return "";
   }

   return result;
   
}

std::vector<rspfString> rspfString::explode(const rspfString& delimeter) const
{
   rspfString exp_str = *this;
   std::vector<rspfString> result;
   char* tokenPtr;

   tokenPtr = strtok((char*)exp_str.c_str(), (char*)delimeter.c_str());

   while(tokenPtr != NULL)
   {
      result.push_back(tokenPtr);
      tokenPtr = strtok(NULL, delimeter.c_str());
   }

   return result;
}

rspfString rspfString::expandEnvironmentVariable() const
{
   rspfString result(*this);
   std::stack<rspf_uint32> startChars;
   rspfRegExp regExpStart("\\$\\(");

   if(regExpStart.find(result.c_str()))
   {
      startChars.push(regExpStart.start());
      while(!startChars.empty())
      {
         rspf_uint32 offset = startChars.top() + 2; // skip over the $(
         
         // We will replace like a stack by looking at the right most $(
         //
         if(regExpStart.find(result.c_str()+offset))
         {
            // maintain absolute offset to the original string
            startChars.push(regExpStart.start()+offset);
         }
         else 
         {
            // now look for a closing ) for the stating $(
            rspfRegExp regExpEnd("\\)");
            if(regExpEnd.find(result.c_str()+startChars.top()))
            {
               rspfString envVarStr(result.begin()+startChars.top()+2,
                                     result.begin()+startChars.top()+regExpEnd.start());
               const char* lookup = getenv( envVarStr.c_str() );
               if ( lookup )
               {
                  result.replace(result.begin()+startChars.top(),
                                 result.begin()+startChars.top()+regExpEnd.start()+1,
                                 rspfString(lookup));
               }
               else 
               {
                  if(traceDebug())
                  {
                     rspfNotify(rspfNotifyLevel_WARN)
                     << "In member function rspfString::expandEnvironmentVariable() "
                     << "\n\tERROR: Environment variable("
                     << envVarStr.c_str()
                     << ") not found!"
                     << std::endl;
                  }
                  result.replace(result.begin()+startChars.top(),
                                 result.begin()+startChars.top()+regExpEnd.start()+1,
                                 "");
               }
            }
            startChars.pop();
         }
      }
   }
   
   
   return result;
}

//---
// Regular expression pattern utilities
//---

rspfString rspfString::beforeRegExp(const char *regularExpressionPattern) const
{   
   rspfRegExp anExpression;
   
   anExpression.compile(regularExpressionPattern);

   if(anExpression.find(c_str()))
   {
      if (anExpression.start() > 0)
      {
         return substr(0, anExpression.start());
      }
   }
   
   return rspfString("");
}

rspfString rspfString::fromRegExp(const char *regularExpressionPattern) const
{   
   rspfRegExp anExpression;
   
   anExpression.compile(regularExpressionPattern);

   if(anExpression.find(c_str()))
   {
      if (anExpression.start() < size())
      {
         return substr(anExpression.start(), (size()-anExpression.start()));
      }
   }
   
   return rspfString("");
}

rspfString rspfString::afterRegExp(const char *regularExpressionPattern) const
{   
   rspfRegExp anExpression;
   
   anExpression.compile(regularExpressionPattern);

   if(anExpression.find(c_str()))
   {
      if (anExpression.end() < size())
      {
         return substr(anExpression.end(), (size()-anExpression.end()));
      }
   }
   
   return rspfString("");
}

rspfString rspfString::match(const char *regularExpressionPattern) const
{   
   rspfRegExp anExpression;
   
   anExpression.compile(regularExpressionPattern);

   if((anExpression.find(this->c_str())) &&
      (anExpression.start() !=anExpression.end()))
   {
      return this->substr(anExpression.start(),
                          anExpression.end() - anExpression.start() );
   }

   
   return rspfString("");
}

rspfString rspfString::replaceAllThatMatch(const char *regularExpressionPattern,
                                             const char *value) const
{
   rspfString result = *this;
   rspfRegExp anExpression;
   std::string::size_type offset     = 0;
   std::string::size_type valueLength = rspfString(value).length();
   anExpression.compile(regularExpressionPattern);
   if(!anExpression.is_valid())
   {
      return *this;
   }
   while(anExpression.find(result.c_str()+offset))
   {
      if(anExpression.start() < anExpression.end())
      {
         result.replace(anExpression.start() + offset,
                        anExpression.end()-anExpression.start(),
                        value);
         offset += anExpression.start() + valueLength;
      }
      else
      {
         break;
      }
   }

   return result;
}

rspfString rspfString::replaceStrThatMatch(const char *regularExpressionPattern,
                                             const char *value) const
{
   rspfString result = *this;
   rspfRegExp anExpression;
   anExpression.compile(regularExpressionPattern);
   if(!anExpression.is_valid())
   {
      return *this;
   }
   if(anExpression.find(result.c_str()))
   {
      if(anExpression.start() < anExpression.end())
      {
         result.replace(anExpression.start(),
                        anExpression.end()-anExpression.start(),
                        value);
      }
   }

   return result;
}

bool rspfString::toBool()const
{
   rspfString s = c_str();
   if (s.empty())
   {
      return false;
   }
   
   s = s.downcase();
   if ( (s == "true") ||
        (s == "yes")  ||
        (s == "y")    ||
        (s == "1") )
   {
      return true;
   }
   else if ( (s == "false") ||
             (s == "no")    ||
             (s == "n")     ||
             (s == "0") )
   {
      return false;
      
   }
   else if (toInt32())
   {
      return true;
   }

   return false;
}

bool rspfString::toBool(const rspfString& aString)
{
   // Check for true or false, yes or no, y or n, and 1 or 0...
   rspfString s = aString;
   if (s.empty())
   {
      return false;
   }
   s.downcase();
   if ( (s == "true") ||
        (s == "yes")  ||
        (s == "y")    ||
        (s == "1") )
   {
      return true;
   }
   else if ( (s == "false") ||
             (s == "no")    ||
             (s == "n")     ||
             (s == "0") )
   {
      return false;
      
   }
   else if (aString.toInt32())
   {
      return true;
   }

   return false;
}

rspf_uint8 rspfString::toUInt8()const
{
   // Note the std::istringstream::operator>> does not work with unsigned 8 bit.
   rspf_uint16 i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
        i = 0;
      }
   }
   return static_cast<rspf_uint8>(i);
}

rspf_uint8 rspfString::toUInt8(const rspfString& aString)
{
   return aString.toUInt8();
}

int rspfString::toInt()const
{
   int i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

int rspfString::toInt(const rspfString& aString)
{
   return aString.toInt();
}

rspf_int16 rspfString::toInt16()const
{
   rspf_int16 i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

rspf_int16 rspfString::toInt16(const rspfString& aString)
{
   return aString.toInt16();
}

rspf_uint16 rspfString::toUInt16()const
{
   rspf_uint16 i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

rspf_uint16 rspfString::toUInt16(const rspfString& aString)
{
   return aString.toUInt16();
}

rspf_int32 rspfString::toInt32()const
{
   rspf_int32 i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

rspf_int32 rspfString::toInt32(const rspfString& aString)
{
   return aString.toInt32();
}

rspf_uint32 rspfString::toUInt32()const
{
   rspf_uint32 i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

rspf_uint32 rspfString::toUInt32(const rspfString& aString)
{
   return aString.toUInt32();
}

rspf_int64 rspfString::toInt64()const
{
   rspf_int64 i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

rspf_int64 rspfString::toInt64(const rspfString& aString)
{
   return aString.toInt64();
}

rspf_uint64 rspfString::toUInt64()const
{
   rspf_uint64 i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

rspf_uint64 rspfString::toUInt64(const rspfString& aString)
{
   return aString.toUInt64();
}

long rspfString::toLong()const
{
  long i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

long rspfString::toLong(const rspfString& aString)
{
   return aString.toLong();
}

unsigned long  rspfString::toULong()const
{
   unsigned long i = 0;
   if (!empty())
   {
      std::istringstream is(m_str);
      is >> i;
      if(is.fail())
      {
         i = 0;
      }
   }
   return i;
}

unsigned long rspfString::toULong(const rspfString& aString)
{
   return aString.toULong();
}

rspf_float32 rspfString::toFloat32()const
{
   if(contains("nan"))
   {
      return rspf::nan();
   }
   rspf_float32 d = 0.0;
   // this part is core dumping under mingw in rspfPlanet.
   // There is a possibility that this isn't a thread safe implementation
   // in mingw stl.  Let's resort back to atof for now

#if 0
   d = (rspf_float32)atof(c_str());
#endif
#if 1
   if (!empty())
   {
      std::istringstream is(c_str());
      is >> d;
      if(is.fail())
      {
         d = 0.0;
      }
   }
#endif
   return d;
}

rspf_float32 rspfString::toFloat32(const rspfString& aString)
{
   return aString.toFloat32();
}

rspf_float64 rspfString::toFloat64()const
{
   if(contains("nan"))
   {
      return rspf::nan();
   }
   rspf_float64 d = 0.0;
   // this part is core dumping under mingw in rspfPlanet.
   // There is a possibility that this isn't a thread safe implementation
   // in mingw stl.  Let's resort back to atof for now

#if 0
   d = (rspf_float64)atof(c_str());
#endif
#if 1
   if (!empty())
   {
      std::istringstream is(c_str());
      is >> d;
      if(is.fail())
      {
         d = 0.0;
      }
   }
#endif
   return d;
}

rspf_float64 rspfString::toFloat64(const rspfString& aString)
{
   return aString.toFloat64();
}

double rspfString::toDouble()const
{
   if(contains("nan"))
   {
      return rspf::nan();
   }
   double d = 0.0;

   // this part is core dumping under mingw in rspfPlanet.
   // There is a possibility that this isn't a thread safe implementation
   // in mingw stl.  Let's resort back to atof for now

   if (!empty())
   {
#if 0
      d = atof(c_str());
#endif
#if 1
      if(!empty())
      {
         std::istringstream is(c_str());
         is >> d;
         if(is.fail())
         {
            d = 0.0;
         }
      }
#endif
   }
   return d;
}

double rspfString::toDouble(const rspfString& aString)
{
   return aString.toDouble();
}

rspfString rspfString::toString(bool aValue)
{
   std::ostringstream s;
   s << aValue;
   rspfString result(s.str());
   return result;
}

rspfString rspfString::toString(rspf_int16 aValue)
{
   std::ostringstream s;
   s << aValue;
   rspfString result(s.str());
   return result;
}

rspfString rspfString::toString(rspf_uint16 aValue)
{
   std::ostringstream s;
   s << aValue;
   rspfString result(s.str());
   return result;
}

rspfString rspfString::toString(rspf_int32 aValue)
{
   std::ostringstream s;
   s << aValue;
   rspfString result(s.str());
   return result;
}

rspfString rspfString::toString(rspf_uint32 aValue)
{
   std::ostringstream s;
   s << aValue;
   rspfString result(s.str());
   return result;
}

rspfString rspfString::toString(rspf_int64 aValue)
{
   std::ostringstream s;
   s << aValue;
   rspfString result(s.str());
   return result;
}

rspfString rspfString::toString(rspf_uint64 aValue)
{
   std::ostringstream s;
   s << aValue;
   rspfString result(s.str());
   return result;
}

rspfString rspfString::toString(rspf_float32 aValue,
                                  rspf_int32 precision,
                                  bool fixed)
{
   if ( rspf::isnan(aValue) )
   {
      return rspfString("nan");
   }

   std::ostringstream s;
   s << std::setprecision(precision);
   
   if (fixed)
   {
      s << std::setiosflags(std::ios::fixed); 
   }
   
   s << aValue;
   
   return rspfString(s.str());
}

rspfString rspfString::toString(rspf_float64 aValue,
                                  rspf_int32 precision,
                                  bool fixed)
{
   if ( rspf::isnan(aValue) )
   {
      return rspfString("nan");
   }
   
   std::ostringstream s;
   s << std::setprecision(precision);
   
   if (fixed)
   {
      s << std::setiosflags(std::ios::fixed); 
   }
   
   s << aValue;
   
   return rspfString(s.str());
}

rspfString rspfString::before(const rspfString& str,
                                std::string::size_type pos)const
{
   if(*this == "") return rspfString();

   size_type last = find(str.c_str(), pos);
   if(last >= std::string::npos) return *this;
   
   return rspfString( substr(0, last) );
}

rspfString rspfString::after(const rspfString& str,
                               std::string::size_type pos)const
{
   size_type last = find(str.c_str(), pos);
   if (last >= std::string::npos) return rspfString();
   
   return rspfString( substr(last+str.length()) );
}

//*************************************************************************************************
// Splits this string into a vector of strings (fields) using the delimiter list specified.
// If a delimiter is encountered at the beginning or the end of this, or two delimiters are 
// contiguous, a blank field is inserted in the vector, unless skipBlankFields is true.
//*************************************************************************************************
void rspfString::split(std::vector<rspfString>& result,
                        const rspfString& separatorList,
                        bool skipBlankFields)const
{
	if(this->empty()) return;
   
   std::string::const_iterator iterStart = m_str.begin();
   std::string::const_iterator iterCurrent = m_str.begin();
   
   while(iterCurrent != m_str.end())
   {
      if(std::find(separatorList.m_str.begin(), separatorList.m_str.end(), *iterCurrent) != separatorList.m_str.end())
      {
         if(iterStart == iterCurrent)
         {
            if(!skipBlankFields)
            {
               result.push_back(rspfString());
            }
         }
         else 
         {
            result.push_back(rspfString(iterStart, iterCurrent));
         }

         ++iterCurrent;
         iterStart = iterCurrent;
      }
      else 
      {
         ++iterCurrent;
      }
   }
   if(iterStart!=iterCurrent)
   {
      result.push_back(rspfString(iterStart, iterCurrent));
   }
   
#if 0   
//   result = split(separatorList);
   rspfString copyString = *this;

   char* s = strtok(const_cast<char*>(copyString.c_str()),
                    separatorList.c_str());

   for(std::string::size_type i = 0; i < separatorList.size(); ++ i)
   {
      if (((*(this->begin())) == separatorList.c_str()[i]) && !skipBlankFields)
         result.push_back("");
   }
   while(s)
   {
      result.push_back(rspfString(s));
      if ((*s != 0) || !skipBlankFields)
         s = strtok(NULL, separatorList.c_str());
   }
   for(std::string::size_type i = 0; i < separatorList.size(); ++ i)
   {
      if (((*(this->end()-1)) == separatorList.c_str()[i]) && !skipBlankFields)
         result.push_back("");
   }
#endif
}

std::vector<rspfString> rspfString:: split(const rspfString& separatorList,
                                             bool skipBlankFields)const
{
   std::vector<rspfString> result;

   split(result, separatorList, skipBlankFields);
   
   return result;
}

const rspfString& rspfString::join(const std::vector<rspfString>& stringList,
                                     const rspfString& separator)
{
   *this = "";
   if(stringList.size())
   {
      for(long i = 0; i < ((long)stringList.size()-1); ++i)
      {
         *this += stringList[i];
         *this += separator;
      }
      *this += stringList[stringList.size()-1];
   }

   return *this;
}

rspfString rspfString::urlEncode()const
{
   rspfString::const_iterator iter = begin();
   rspfString result;
   while(iter != end())
   {
      rspf_uint8 c = *iter;
      
      if(c > 47 && c < 58)
      {
         result += c;
      }
      else if(c > 64 && c < 91)
      {
         result += c;
      }
      else if(c > 96 && c < 123)
      {
         result+=c;
      }
      else if (c == 32)
      {
         result+="+";
      }
      else
      {
         result += ("%" + rspfHexString(c));
      }
      
      ++iter;
   }

  return result;
   
}

rspfString rspfString::getOssimId() const
{
#ifdef RSPF_ID_ENABLED
   return rspfString(RSPF_ID);
#endif
   return rspfString("");
}

