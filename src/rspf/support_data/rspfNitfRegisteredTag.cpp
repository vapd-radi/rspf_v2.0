//*******************************************************************
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
// Description: Nitf support class
// 
//********************************************************************
// $Id: rspfNitfRegisteredTag.cpp 22013 2012-12-19 17:37:20Z dburken $

#include <iostream>
#include <sstream>
#include <string>
#include <sstream>
#include <rspf/support_data/rspfNitfRegisteredTag.h>

RTTI_DEF2(rspfNitfRegisteredTag, "rspfNitfRegisteredTag", rspfObject, rspfPropertyInterface)

rspfNitfRegisteredTag::rspfNitfRegisteredTag()
   : 
   rspfObject(),
   rspfPropertyInterface(),
   m_tagName(),
   m_tagLength(0)
{}
rspfNitfRegisteredTag::rspfNitfRegisteredTag(const std::string& tagName, rspf_uint32 tagLength)
   : 
   rspfObject(),
   rspfPropertyInterface(),
   m_tagName(tagName),
   m_tagLength(tagLength)
{}

rspfNitfRegisteredTag::~rspfNitfRegisteredTag()
{}

std::string rspfNitfRegisteredTag::getRegisterTagName() const
{
   // Deprecated:
   return m_tagName;
}

const std::string& rspfNitfRegisteredTag::getTagName() const
{
   return m_tagName;
}

void rspfNitfRegisteredTag::setTagName(const std::string& tagName)
{
   m_tagName = tagName;
}

rspf_uint32 rspfNitfRegisteredTag::getSizeInBytes() const
{
   // Deprecated:
   return getTagLength();
}

rspf_uint32 rspfNitfRegisteredTag::getTagLength() const
{
   return m_tagLength;
}

void rspfNitfRegisteredTag::setTagLength(rspf_uint32 length)
{
   m_tagLength = length;
}

void rspfNitfRegisteredTag::setProperty(rspfRefPtr<rspfProperty> /* property */)
{
}

rspfRefPtr<rspfProperty> rspfNitfRegisteredTag::getProperty(const rspfString& /* name */)const
{
   return 0;
}

void rspfNitfRegisteredTag::getPropertyNames(std::vector<rspfString>& /* propertyNames */)const
{
}

void rspfNitfRegisteredTag::getMetadata(rspfKeywordlist& /* kwl */,
                                         const char* /* prefix */ ) const
{
}

std::ostream& rspfNitfRegisteredTag::print(
   std::ostream& out, const std::string& /* prefix */) const
{
   // return print(out);
   return out;
}

bool rspfNitfRegisteredTag::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   bool result = false;
   // by default we will use the already implemented print method as backup
   //
   std::ostringstream out;
   
   print(out);
   
   rspfKeywordlist kwlTemp;
   
   std::istringstream in(out.str());
   if(kwlTemp.parseStream(in))
   {
      result = true;
      kwl.add(prefix, kwlTemp);
   }
   
   return result;
}
