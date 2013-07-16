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
// $Id: rspfString.h 20491 2012-01-23 20:10:06Z dburken $
#ifndef rspfString_HEADER
#define rspfString_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <string>
#include <vector>
#include <iostream>


class RSPF_DLL rspfString
{
public:

   typedef std::string::const_iterator const_iterator;
   typedef std::string::iterator       iterator;
   typedef std::string::size_type      size_type;

   /** @brief default constructor */
   rspfString(): m_str() {}

   /** @brief constructor that takes a std::string */
   rspfString(const std::string& s):m_str(s) {}

   /**
    * @brief constructor that takes char*
    * NOTE: std::string construtor throws exception given null pointer;
    * hence, the null check.
    */
   rspfString(const char *aString):m_str( aString?aString:"" ){}

   /** @brief copy constructor */
   rspfString(const rspfString& aString):m_str(aString.m_str){}

   /** @brief constructor - constructs with n copies of c */
   rspfString(std::string::size_type n, char c):m_str(n,c){}

   /** @brief constructor - constructs with 1 c. */
   rspfString(char aChar):m_str(1, aChar){}

   template <class Iter>
   rspfString(Iter startIter, Iter endIter):m_str(startIter, endIter){}

   /** @brief destructor */
   ~rspfString(){}

   bool contains(const rspfString& aString) const
   { return m_str.find(aString.m_str)!=std::string::npos; }

   bool contains(const char* aString) const
   { return m_str.find(aString)!=std::string::npos; }

   /** @brief Type conversion operator to a const std::string&. */
   operator const std::string&() const { return m_str; }

   /** @brief Type conversion operator to a std::string&. */
   operator std::string&() { return m_str; }

   /** @brief Type conversion operator to a const char*. */
   operator const char*() const { return m_str.c_str(); }

   /** @brief For backward compatibility. */
   const char* chars() const { return m_str.c_str(); }

   /** @brief Reads s from the input stream is. */
   friend RSPF_DLL std::istream& operator>>(std::istream&  is, rspfString& s);

   /** @brief Writes s to the output stream os. */
   friend RSPF_DLL std::ostream& operator<<(std::ostream& os,
                                             const rspfString& s);

   /**
    * @brief Reads a string from the input stream is, stopping when it
    * reaches delim.
    */
   friend RSPF_DLL std::istream& getline( std::istream& is,
                                           rspfString& str,
                                           char delim );

   /** @brief Reads a single line from the input stream is. */
   friend RSPF_DLL std::istream& getline( std::istream& is, rspfString& str );

   const rspfString& operator=(const std::string& s)
   {
      m_str = s;
      return *this;
   }

   const rspfString& operator=(const char* s)
   {
      if (s) // std::string::operator= throws exception given null pointer.
      {
         m_str = s;
      }
      else
      {
         m_str = "";
      }
      return *this;
   }

   const rspfString& operator=(char c)
   {
      m_str = c;
      return *this;
   }

   const rspfString& operator=(const rspfString& s)
   {
      if ( this != &s )
      {
         m_str = s.m_str;
      }
      return *this;
   }

   const rspfString& operator+=(const rspfString& s)
   {
      m_str.append(s.m_str);
      return *this;
   }

   const rspfString& operator+=(const std::string& s)
   {
      m_str.append(s);
      return *this;
   }

   const rspfString& operator+=(const char* s)
   {
      if ( s ) m_str.append(s);
      return *this;
   }

   const rspfString& operator+=(char c)
   {
      m_str.append(1, c);
      return *this;
   }

   const rspfString& append(const rspfString& s);

   const rspfString& append(const std::string& s);

   const rspfString& append(const char* s);

   /**
    *  @brief  Provides access to the data contained in the %string.
    *  @param n The index of the character to access.
    *  @return  Read-only (const) reference to the character.
    *  @throw  std::out_of_range  If @a n is an invalid index.
    *
    *  This function provides for safer data access.  The parameter is
    *  first checked that it is in the range of the string.  The function
    *  throws out_of_range if the check fails.
    */
   const char& at(std::string::size_type n) const;

   /**
    *  @brief  Provides access to the data contained in the %string.
    *  @param n The index of the character to access.
    *  @return  Read/write reference to the character.
    *  @throw  std::out_of_range  If @a n is an invalid index.
    *
    *  This function provides for safer data access.  The parameter is
    *  first checked that it is in the range of the string.  The function
    *  throws out_of_range if the check fails.  Success results in
    *  unsharing the string.
    */
   char& at(std::string::size_type n);

   /** @brief Append n copies of c to *this. */
   const rspfString& append(std::string::size_type n, char c);

   rspfString operator+(const rspfString& s)const
   {
      rspfString returnS(*this);
      returnS.m_str.append(s.m_str);
      return returnS;
   }

   rspfString operator+(const std::string& s)const
   {
      rspfString returnS(*this);
      returnS.m_str.append(s);
      return returnS;
   }

   rspfString operator+(const char* s)const
   {
      rspfString returnS(*this);
      if ( s ) returnS.m_str.append(s);
      return returnS;
   }

   rspfString operator+(char c)const
   {
      rspfString returnS(*this);
      returnS.m_str.append(1, c);
      return returnS;
   }

   friend RSPF_DLL rspfString operator+(const char* s1, const rspfString& s2);

   friend RSPF_DLL rspfString operator+(const std::string s1, const rspfString& s2);

   friend RSPF_DLL rspfString operator+(char c, const rspfString& s2);
   friend RSPF_DLL bool operator==(const char* lhs, const rspfString& rhs);
   friend RSPF_DLL bool operator==(const std::string& lhs, const rspfString& rhs);

   /**
    *  @brief  Test if this rspfString is equal to another rspfString.
    *  @param rhs rspfString to compare.
    *  @return  True if strings are equal.  False otherwise.
    */
   bool operator==(const rspfString& rhs) const
   {
      return (m_str.compare( rhs.m_str ) == 0);
   }

   /**
    *  @brief  Test if this rspfString is equal to a C sting.
    *  @param rhs C string to compare.
    *  @return  True if strings are equal.
    *  False if rhs is not equal null or null.
    */
   bool operator==(const char* rhs) const
   {
      bool result = false;
      if (rhs)
      {
         result = (m_str.compare(std::string(rhs)) == 0);
      }
      return result;
   }

   /**
    *  @brief  Test if this rspfString is not equal to another rspfString.
    *  @param rhs rspfString to compare.
    *  @return  True if strings are not equal.  False otherwise.
    */
   bool operator!=(const rspfString& rhs) const
   {
      return !(m_str.compare(rhs.m_str) == 0);
   }

   /**
    *  @brief  Test if this rspfString is not equal to a C sting.
    *  @param rhs C string to compare.
    *  @return  True if strings are not equal or rhs is null.
    *  False if rhs equal to this string.
    */
   bool operator!=(const char* rhs) const
   {
      bool result = true;
      if (rhs)
      {
         result = !(m_str.compare(std::string(rhs)) == 0);
      }
      return result;
   }

   char& operator[](std::string::size_type i)
   {
      return m_str[i];
      // return *( const_cast<char*>(m_strc_str())+i);
   }

   const char& operator[](std::string::size_type i)const
   {
      return m_str[i];
   }

   /**
    * @brief If pos > size(), throws out_of_range. Otherwise, equivalent to
    * insert(begin() + pos, s, s + traits::length(s)).
    */
   std::string& insert(std::string::size_type pos, const char* s);

   /** @brief Equivalent to insert(end(), c). */
   void push_back(char c);

   /** @brief Equivalent to basic_string(*this, pos, n). */
   std::string substr(std::string::size_type pos = 0,
                      std::string::size_type n = std::string::npos) const;

   /**
    * this will strip lead and trailing character passed in.
    * So if you want to remove blanks:
    *    rspfString temp("       asdfasdf      ");
    *    rspfString trimmedString = temp.trim(" \n\t\r");
    *
    *    this will now contain "asdfasdf" without the blanks.
    *
    */
   rspfString trim(const rspfString& valueToTrim= rspfString(" \t\n\r"))const;
   rspfString& trim(const rspfString& valueToTrim= rspfString(" \t\n\r"));

   rspfString beforePos(std::string::size_type pos)const;
    rspfString afterPos(std::string::size_type pos)const;

   /**
    *  Substitutes searchKey string with replacementValue and returns a
    *  string.  Will replace all occurrences found if "replaceAll" is true.
    */
   rspfString substitute(const rspfString &searchKey,
                                  const rspfString &replacementValue,
                                  bool replaceAll=false)const;

   /**
    *  Substitutes searchKey string with replacementValue and returns a
    *  reference to *this.  Will replace all occurrences found if
    *  "replaceAll" is true.  (like substitute only works on "this")
    */
   rspfString& gsub(const rspfString &searchKey,
                             const rspfString &replacementValue,
                             bool replaceAll=false);

   std::vector<rspfString> explode(const rspfString& delimeter) const;

   /**
    * If the variable "$(env_var_name)" is found in the string, where
    * "env_var_name" is any system environment variable resolvable by
    * the getenv() function, the variable is expanded in place and the
    * result returned.
    */
   rspfString expandEnvironmentVariable() const;

   /**
    * @param aString String to make an upcased copy of.
    *
    * @return An upcased version of aString.
    */
   static rspfString upcase(const rspfString& aString);

   /**
    * @param aString String to make an downcased copy of.
    *
    * @return A downcased version of aString.
    */
   static rspfString downcase(const rspfString& aString);

   /**
    * Upcases this string.
    *
    * @return Reference to this.
    */
   rspfString& upcase();
   rspfString upcase()const;

   /**
    * Downcases this string.
    *
    * @return Reference to this.
    */
   rspfString& downcase();
   rspfString downcase()const;

   /**
    * @brief Returns a pointer to a null-terminated array of characters
    * representing the string's contents.
    */
   const char* c_str() const { return m_str.c_str(); }

   /**
    * @brief Returns a pointer to an array of characters (not necessarily
    * null-terminated) representing the string's contents.
    */
   const char* data() const { return m_str.data(); }

   /** @return The size of the string. */
   std::string::size_type size() const { return m_str.size(); }

   /** @return The size of the string. */
   std::string::size_type length() const { return m_str.size(); }

   /** @return True if size is 0. */
   bool empty() const { return m_str.empty(); }

   /** @return The underlying std::string container. */
   const std::string& string() const { return m_str; }

   /** @return The underlying std::string container. */
   std::string& string() { return m_str; }

   /** @return An iterator pointing to the beginning of the string. */
   std::string::iterator begin() { return m_str.begin(); }

   /** @return An iterator pointing to the end of the string. */
   std::string::iterator end() { return m_str.end(); }

   /** @return An const_iterator pointing to the beginning of the string. */
   std::string::const_iterator begin() const { return m_str.begin(); }

   /** @return An const_iterator pointing to the end of the string. */
   std::string::const_iterator end() const { return m_str.end(); }

   /** @brief Erases the entire container. */
   void clear() { m_str.clear(); }

   /** @brief Erases the character at position p. */
   std::string::iterator erase(std::string::iterator p);

   /** @brief Erases the range [first, last). */
   std::string::iterator erase(std::string::iterator first,
                               std::string::iterator last);

   /** @brief Erases a range. */
   std::string& erase(std::string::size_type pos = 0,
                      std::string::size_type n = std::string::npos);

   /**
    * @brief Searches for s as a substring of *this, beginning at character
    * pos of *this.
    */
   std::string::size_type find(const std::string& s,
                               std::string::size_type pos = 0) const;

   /**
    * @brief Searches for the first n characters of s as a substring of *this,
    * beginning at character pos of *this.
    */
   std::string::size_type find(const char* s,
                               std::string::size_type pos,
                               std::string::size_type n) const;

   /**
    * @brief Searches for a null-terminated character array as a substring
    * of *this, beginning at character pos of *this.
    */
   std::string::size_type find(const char* s,
                               std::string::size_type pos = 0) const;

   /**
    * @brief Searches for the character c, beginning at character position
    * pos.
    */
   std::string::size_type find(char c, std::string::size_type pos = 0) const;

    /**
    * @brief Searches backward for s as a substring of *this, beginning at
    * character position min(pos, size()).
    */
   std::string::size_type rfind(const std::string& s,
                                std::string::size_type pos = 0) const;

   /**
    * @brief Searches backward for the first n characters of s as a substring
    * of *this, beginning at character position min(pos, size()).
    */
   std::string::size_type rfind(const char* s,
                                std::string::size_type pos,
                                std::string::size_type n) const;

   /**
    * @brief Searches backward for a null-terminated character array as a
    * substring of *this, beginning at character min(pos, size()).
    */
   std::string::size_type rfind(const char* s,
                                std::string::size_type pos = 0) const;

   /**
    * @brief Searches backward for the character c, beginning at character
    * position min(pos, size().
    */
   std::string::size_type rfind(char c, std::string::size_type pos = 0) const;

   /** @brief Equivalent to find(c, pos). */
   std::string::size_type find_first_of(
      char c, std::string::size_type pos = 0) const;

   /**
    * @brief Returns the smallest character position N such that
    * pos <= N < size(), and such that (*this)[N] does not compare equal to c.
    * Returns npos if no such character position exists.
    */
   std::string::size_type find_first_not_of(
      char c, std::string::size_type pos = 0) const;

   /**
    * @brief Equivalent to find_first_of(s, pos, traits::length(s)).
    */
   std::string::size_type find_first_of(
      const char* s, std::string::size_type pos = 0) const;

      /**
    * @brief Equivalent to find_first_not_of(s, pos, traits::length(s)).
    */
   std::string::size_type find_first_not_of(
      const char* s, std::string::size_type pos = 0) const;

   /** @brief Equivalent to rfind(c, pos). */
   std::string::size_type find_last_of(
      char c, std::string::size_type pos = std::string::npos) const;

   /**
    * @brief Erases the existing characters and replaces them by n copies
    * of c.
    */
   std::string& assign(std::string::size_type n, char c);

   /** @brief Replaces a substring of *this with the string s. */
   std::string& replace(std::string::size_type pos,
                        std::string::size_type n,
                        const std::string& s);

   /** brief Equivalent to insert(erase(first, last), s.begin(), s.end()). */
   std::string& replace(std::string::iterator first,
                        std::string::iterator last,
                        const std::string& s);

   /** @brief Replaces a substring of *this with a substring of s. */
   std::string& replace(std::string::size_type pos,
                        std::string::size_type n,
                        const std::string& s,
                        std::string::size_type pos1,
                        std::string::size_type n1);

   /** @brief Requests that the string's capacity be changed. */
   void reserve(std::string::size_type n);

   /**
    * METHOD: before(str, pos)
    * Returns string beginning at pos and ending one before the token str
    * If string not found or pos out of range the whole string will be
    * returned.
    */
   rspfString before(const rspfString& str, std::string::size_type pos=0)const;

   /**
    * METHOD: after(str, pos)
    * Returns string immediately after the token str. The search for str
    * begins at pos.  Returns an empty string if not found or pos out of
    * range.
    */
   rspfString after (const rspfString& str, std::string::size_type pos=0)const;

   char* stringDup()const;

   /**
    * String to numeric methods.
    */
   bool                 toBool()const;
   static bool          toBool(const rspfString& aString);

   rspf_uint8          toUInt8()const;
   static rspf_uint8   toUInt8(const rspfString& aString);

   int                  toInt()const;
   static int           toInt(const rspfString& aString);

   rspf_int16          toInt16()const;
   static rspf_int16   toInt16(const rspfString& aString);
   rspf_uint16         toUInt16()const;
   static rspf_uint16  toUInt16(const rspfString& aString);

   rspf_int32          toInt32()const;
   static rspf_int32   toInt32(const rspfString& aString);
   rspf_uint32         toUInt32()const;
   static rspf_uint32  toUInt32(const rspfString& aString);

   rspf_int64          toInt64()const;
   static rspf_int64   toInt64(const rspfString& aString);
   rspf_uint64         toUInt64()const;
   static rspf_uint64  toUInt64(const rspfString& aString);

   /**
    * toLong's deprecated, please use the toInts...
    */
   long                 toLong()const;
   static long          toLong(const rspfString& aString);
   unsigned long        toULong()const;
   static unsigned long toULong(const rspfString& aString);

   rspf_float32        toFloat32()const;
   static rspf_float32 toFloat32(const rspfString& aString);
   rspf_float64        toFloat64()const;
   static rspf_float64 toFloat64(const rspfString& aString);
   double               toDouble()const;
   static double        toDouble(const rspfString& aString);

   /**
    * Numeric to string methods.
    */
   static rspfString toString(bool aValue);
   static rspfString toString(rspf_int16   aValue);
   static rspfString toString(rspf_uint16  aValue);
   static rspfString toString(rspf_int32   aValue);
   static rspfString toString(rspf_uint32  aValue);
   static rspfString toString(rspf_int64   aValue);
   static rspfString toString(rspf_uint64  aValue);

   /**
    * @param aValue Value to convert to string.
    *
    * @param precision Decimal point precision of the output.
    *
    * @param fixed If true setiosflags(std::ios::fixed) will be called.
    */
   static rspfString toString(rspf_float32 aValue,
                               rspf_int32 precision = 8,
                               bool fixed = false);

   /**
    * @param aValue Value to convert to string.
    *
    * @param precision Decimal point precision of the output.
    *
    * @param fixed If true setiosflags(std::ios::fixed) will be called.
    */
   static rspfString toString(rspf_float64 aValue,
                               rspf_int32 precision = 15,
                               bool fixed = false);

   static rspfString stripLeading(const rspfString &value,
                                   char characterToStrip);


   /**
    * Splits this string into a vector of strings (fields) using the delimiter list specified.
    * If a delimiter is encountered at the beginning or the end of this, or two delimiters are
    * contiguous, a blank field is inserted in the vector, unless skipBlankFields is true.
    */
   void split(std::vector<rspfString>& result,
              const rspfString& separatorList,
              bool skipBlankFields=false)const;

   std::vector<rspfString> split(const rspfString& separatorList,
                                  bool skipBlankFields=false)const;

   const rspfString& join(const std::vector<rspfString>& stringList,
                           const rspfString& separator);

   //---
   // Regular expression pattern utilities
   //---

   /**
    * Returns from start of string up to but not including found pattern.
    * Returns "" if pattern not found.
    */
   rspfString beforeRegExp(const char *regularExpressionPattern) const;

   /**
    * Returns from position of found pattern to end of string.
    * Returns "" if pattern not found.
    */
   rspfString fromRegExp(const char *regularExpressionPattern) const;

   /**
    * Returns from position after found pattern to end of string.
    * Returns "" if pattern not found.
    */
   rspfString afterRegExp(const char *regularExpressionPattern) const;

   /**
    * Returns from found pattern to end of pattern.
    * Returns "" if pattern not found.
    */
   rspfString match(const char *regularExpressionPattern) const;

   rspfString replaceAllThatMatch(const char *regularExpressionPattern,
                                   const char *value="") const;

   rspfString replaceStrThatMatch(const char *regularExpressionPattern,
                                   const char *value="") const;

   rspfString urlEncode()const;

   /**
    * If RSPF_ID_ENABLED returns the RSPF_ID which currently is the
    * expanded cvs $Id: rspfString.h 20491 2012-01-23 20:10:06Z dburken $ macro; else, an empty string.
    */
   rspfString getOssimId() const;

protected:

   std::string m_str;
};

inline std::string::iterator rspfString::erase(std::string::iterator p)
{
   return m_str.erase(p);
}

inline std::string::iterator rspfString::erase(std::string::iterator first,
                                                std::string::iterator last)
{
   return m_str.erase(first, last);
}

inline std::string& rspfString::erase(std::string::size_type pos,
                                       std::string::size_type n)
{
   return m_str.erase(pos, n);
}

inline std::string::size_type rspfString::find(
   const std::string& s, std::string::size_type pos) const
{
   return m_str.find(s, pos);
}

inline std::string::size_type rspfString::find(
   const char* s, std::string::size_type pos, std::string::size_type n) const
{
   return m_str.find(s, pos, n);
}

inline std::string::size_type rspfString::find(
   const char* s, std::string::size_type pos) const
{
   return m_str.find(s, pos);
}

inline std::string::size_type rspfString::find(
   char c, std::string::size_type pos) const
{
   return m_str.find(c, pos);
}

inline std::string::size_type rspfString::rfind(
   const std::string& s, std::string::size_type pos) const
{
   return m_str.rfind(s, pos);
}

inline std::string::size_type rspfString::rfind(
   const char* s, std::string::size_type pos, std::string::size_type n) const
{
   return m_str.rfind(s, pos, n);
}

inline std::string::size_type rspfString::rfind(
   const char* s, std::string::size_type pos) const
{
   return m_str.rfind(s, pos);
}

inline std::string::size_type rspfString::rfind(
   char c, std::string::size_type pos) const
{
   return m_str.rfind(c, pos);
}

inline std::string::size_type rspfString::find_first_of(
   char c, std::string::size_type pos) const
{
   return m_str.find_first_of(c, pos);
}

inline std::string::size_type rspfString::find_first_of(
   const char* s, std::string::size_type pos) const
{
   return m_str.find_first_of(s, pos);
}

inline std::string::size_type rspfString::find_first_not_of(
   const char c, std::string::size_type pos) const
{
   return m_str.find_first_not_of(c, pos);
}

inline std::string::size_type rspfString::find_first_not_of(
   const char* s, std::string::size_type pos) const
{
   return m_str.find_first_not_of(s, pos);
}

inline std::string::size_type rspfString::find_last_of(
   char c, std::string::size_type pos) const
{
   return m_str.find_last_of(c, pos);
}

inline const rspfString& rspfString::append(const rspfString& s)
{
   m_str.append(s.m_str);
   return *this;
}

inline const rspfString& rspfString::append(const std::string& s)
{
   m_str.append(s);
   return *this;
}

inline const rspfString& rspfString::append(const char* s)
{
   if ( s ) m_str.append(s);
   return *this;
}

inline const rspfString& rspfString::append(std::string::size_type n, char c)
{
   m_str.append(n, c);
   return *this;
}

inline const char& rspfString::at(std::string::size_type n) const
{
   return m_str.at(n); 
}

inline char& rspfString::at(std::string::size_type n)
{
   return m_str.at(n);
}

inline std::string& rspfString::assign(std::string::size_type n, char c)
{
   return m_str.assign(n, c);
}

inline std::string& rspfString::replace(
   std::string::size_type pos, std::string::size_type n, const std::string& s)
{
   return m_str.replace(pos, n, s);
}

inline std::string& rspfString::replace(std::string::iterator first,
                                         std::string::iterator last,
                                         const std::string& s)
{
   return m_str.replace(first, last, s);
}

inline std::string& rspfString::replace(std::string::size_type pos,
                                         std::string::size_type n,
                                         const std::string& s,
                                         std::string::size_type pos1,
                                         std::string::size_type n1)
{
   return m_str.replace(pos, n, s, pos1, n1);
}

inline void rspfString::reserve(std::string::size_type n)
{
   m_str.reserve(n);
}


inline std::string& rspfString::insert(std::string::size_type pos,
                                        const char* s)
{
   if (s) m_str.insert(pos, s);
   return m_str;
}

inline void rspfString::push_back(char c)
{
   m_str.push_back(c);
}

inline std::string rspfString::substr(std::string::size_type pos,
                                       std::string::size_type n) const
{
   return m_str.substr(pos, n);
}

inline std::istream& getline( std::istream& is, rspfString& str, char delim )
{
   return std::getline( is, str.string(), delim );
}

inline std::istream& getline( std::istream& is, rspfString& str )
{
   // Not sure why getline( is, str.string()) doesn't work. (drb)
   return std::getline( is, str.string(), '\n' );
}

inline std::istream& operator>>(std::istream&  is, rspfString& s)
{
   return is >> s.string();
}

inline std::ostream& operator<<(std::ostream& os, const rspfString& s)
{
   return os << s.string();
}

inline rspfString operator+(const char* s1, const rspfString& s2)
{
   rspfString result;
   if ( s1 ) result = rspfString( s1 + s2.string() );
   else result = s2;
   return result;
}

inline rspfString operator+(const std::string s1, const rspfString& s2)
{
   return rspfString( s1 + s2.string() );
}

inline rspfString operator+(char c, const rspfString& s2)
{
   return rspfString( c + s2.string() );
}

inline bool operator==(const char* lhs, const rspfString& rhs)
{
   return (rhs.operator ==(lhs));
}

inline bool operator==(const std::string& lhs, const rspfString& rhs)
{
   return (rhs.operator ==(lhs));
}

/**
 * @brief For use with std::map as a compare operator:
 * std::map<rspfString, rspfString, rspfStringLtstr>
 *
 * This will allow the find method on map to work.
 */
struct rspfStringLtstr
{
   bool operator()(const rspfString& s1, const rspfString& s2) const
   {
      return s1.string().compare(s2.string()) < 0;
   }
};

#endif /* #ifndef rspfString_HEADER */
