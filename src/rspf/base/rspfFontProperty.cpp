//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfFontProperty.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <sstream>
#include <rspf/base/rspfFontProperty.h>
#include <rspf/base/rspfKeywordlist.h>


RTTI_DEF1(rspfFontProperty, "rspfFontProperty", rspfProperty);

rspfFontProperty::rspfFontProperty(const rspfString& name,
                                     const rspfFontInformation& value)
   :rspfProperty(name),
    theValue(value)
{
   
}

rspfFontProperty::rspfFontProperty(const rspfFontProperty& rhs)
   :rspfProperty(rhs),
    theValue(rhs.theValue)
{
}

rspfFontProperty::~rspfFontProperty()
{
}

rspfObject* rspfFontProperty::dup()const
{
   return new rspfFontProperty(*this);
}

const rspfProperty& rspfFontProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);

   rspfFontProperty* rhsPtr = PTR_CAST(rspfFontProperty,
                                        &rhs);

   if(rhsPtr)
   {
      theValue = rhsPtr->theValue;
   }
   
   return *this;
}

bool rspfFontProperty::setValue(const rspfString& value)
{
   bool result = true;
   rspfKeywordlist kwl;

   std::istringstream in(value);

   result = kwl.parseStream(in);
   if(result)
   {
      theValue.loadState(kwl);
   }

   return result;
}

void rspfFontProperty::valueToString(rspfString& valueResult)const
{
   std::ostringstream out;
   rspfKeywordlist kwl;

   theValue.saveState(kwl);
   
   kwl.writeToStream(out);
   valueResult = kwl.toString();
}

void rspfFontProperty::setFontInformation(rspfFontInformation& fontInfo)
{
   theValue = fontInfo;
}

const rspfFontInformation& rspfFontProperty::getFontInformation()const
{
   return theValue;
}
