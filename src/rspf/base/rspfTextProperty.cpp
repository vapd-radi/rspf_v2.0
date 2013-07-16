//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfTextProperty.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <rspf/base/rspfTextProperty.h>

RTTI_DEF1(rspfTextProperty, "rspfTextProperty", rspfProperty);

rspfTextProperty::rspfTextProperty(const rspfString& name,
                                     const rspfString& value,
                                     bool multiLineFlag)
   :rspfProperty(name),
    theValue(value),
    theMultiLineFlag(multiLineFlag)
{
}

rspfTextProperty::rspfTextProperty(const rspfTextProperty& rhs)
   :rspfProperty(rhs),
    theValue(rhs.theValue),
    theMultiLineFlag(rhs.theMultiLineFlag)
{
}

rspfObject* rspfTextProperty::dup()const
{
   return new rspfTextProperty(*this);
}

bool rspfTextProperty::isMulitLine()const
{
   return theMultiLineFlag;
}

void rspfTextProperty::setMultiLineFlag(bool flag)
{
   theMultiLineFlag = flag;
}

bool rspfTextProperty::setValue(const rspfString& value)
{
   theValue = value;
   
   return true;
}

void rspfTextProperty::valueToString(rspfString& valueResult)const
{
   valueResult = theValue;
}

const rspfProperty& rspfTextProperty::assign(const rspfProperty& rhs)
{
   theValue = rhs.valueToString();

   return *this;
}
   
