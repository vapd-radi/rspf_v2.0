//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author:  David Burken
//
// Description:
//
// Contains class definition for Keyword.h
//*******************************************************************
//  $Id: rspfKeyword.cpp 9963 2006-11-28 21:11:01Z gpotts $

#include <iostream>
#include <rspf/base/rspfKeyword.h>

rspfKeyword::rspfKeyword()
   :
      theKey(""),
      theDesc(""),
      theValue("")
{}

rspfKeyword::rspfKeyword(const char* key, const char* description)
   :
      theKey(key),
      theDesc(description),
      theValue("")
{}

rspfKeyword::rspfKeyword(const char* key,
                           const char* description,
                           const char* value)
   :
      theKey(key),
      theDesc(description),
      theValue(value)      
{}

rspfKeyword::rspfKeyword(const rspfKeyword& src)
   :
      theKey(src.key()),
      theDesc(src.description()),
      theValue(src.value())
{}

rspfKeyword::~rspfKeyword()
{}

rspfKeyword::operator const char*() const
{
   return theKey.c_str();
}

const char* rspfKeyword::key() const
{
   return theKey.c_str();
}

const char* rspfKeyword::description() const
{
   return theDesc.c_str();
}

const char* rspfKeyword::value() const
{
   return theValue.c_str();
}

rspfString rspfKeyword::keyString() const
{
   return theKey;
}

rspfString rspfKeyword::descriptionString() const
{
   return theDesc;
}

rspfString rspfKeyword::valueString() const
{
   return theValue;
}

void rspfKeyword::setValue(const char* val)
{
   theValue = val;
}

void rspfKeyword::setValue(const rspfString& val)
{
   theValue = val;
}

void rspfKeyword::setDescription(const char* desc)
{
   theDesc  = desc;
}

void rspfKeyword::setDescription(const rspfString& desc)
{
   theDesc  = desc;
}
   
const rspfKeyword& rspfKeyword::operator=(const rspfKeyword& kw)
{
   if (this != &kw)
   {
      theKey = kw.key();
      theDesc = kw.description();
      theValue = kw.value();
   }
   
   return *this;
}

std::ostream& operator<<(std::ostream& os, const rspfKeyword& kw)
{
   os << "\n key   = " << kw.key()
      << "\n desc  = " << kw.description()
      << "\n value = " << kw.value()
      << std::endl;
   return os;
}
