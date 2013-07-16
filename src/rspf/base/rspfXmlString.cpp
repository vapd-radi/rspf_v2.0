//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Ken Melero (kmelero@remotesensing.org)
// Description: This class provides manipulation of filenames.
//
//*************************************************************************
// $Id: rspfXmlString.cpp 19682 2011-05-31 14:21:20Z dburken $
#include <rspf/base/rspfXmlString.h>

const rspfXmlString& rspfXmlString::assign(const rspfString& s)
{
   rspfString result = s;
   
   std::string::size_type pos = 0;

   pos = result.find("&");
   // do ampersand first
   //
   while(pos != std::string::npos)
   {
      rspfString s1(result.begin() + pos,
                     result.begin()+pos+5);
      rspfString s2(result.begin() + pos,
                     result.begin()+pos+4);
      rspfString s3(result.begin() + pos,
                     result.begin()+pos+3);
      
      if((s2 != "&amp")&&
         (s1 != "&quot")&&
         (s3 != "&gt")&&
         (s3 != "&lt")&&
         (s1 != "&apos"))
      {
         result.replace(pos, 1, "&amp");
      }
      
      pos = result.find("&", pos+4);
   }

   result = result.substitute("\"",
                              "&quot",
                              true);
   result = result.substitute("<",
                              "&lt",
                              true);
   result = result.substitute(">",
                              "&gt",
                              true);
   result = result.substitute("'",
                              "&apos",
                              true);
                     
   *((rspfString*)this) = result;

   return *this;
}
