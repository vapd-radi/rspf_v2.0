//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
// Description: This class provides manipulation of filenames.
//
//*************************************************************************
// $Id: rspfXmlString.h 9968 2006-11-29 14:01:53Z gpotts $

#ifndef rspfXmlString_HEADER
#define rspfXmlString_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfString.h>

class RSPF_DLL rspfXmlString : public rspfString
{
public:
   rspfXmlString()
      :
         rspfString()
      {}
   rspfXmlString(const rspfXmlString& src)
      :
         rspfString(src)
      {}
   rspfXmlString(const rspfString& src)
      :
         rspfString("")
      {
         assign(src);

      }
   rspfXmlString(const char* src)
      :
         rspfString("")
      {
         assign(src);
      }
   
   template <class Iter>
   rspfXmlString(Iter s, Iter e)
      :rspfString("")
      {
         assign(rspfString(s, e));
      }

   const rspfXmlString& operator =(const char* s)
      {
         return assign(rspfString(s));
      }
   	  const rspfXmlString& operator =(const rspfString& s)
      {
         return assign(rspfString(s));
      }
	  const rspfXmlString& operator =(const std::string& s)
      {
         return assign(rspfString(s));
      }

   const rspfXmlString& assign(const rspfString& s);
};

#endif
