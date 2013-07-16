//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfBooleanProperty.cpp 19888 2011-08-03 11:30:22Z gpotts $
#include <rspf/base/rspfBooleanProperty.h>

RTTI_DEF1(rspfBooleanProperty, "rspfBooleanProperty", rspfProperty);

rspfBooleanProperty::rspfBooleanProperty(const rspfString& name,
                                           bool value)
   :rspfProperty(name),
    theValue(value)
{
}

rspfBooleanProperty::rspfBooleanProperty(const rspfBooleanProperty& rhs)
   :rspfProperty(rhs),
    theValue(rhs.theValue)
{
}

rspfObject* rspfBooleanProperty::dup()const
{
   return new rspfBooleanProperty(*this);
}

const rspfProperty& rspfBooleanProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);

   rspfBooleanProperty* property = PTR_CAST(rspfBooleanProperty,
                                             &rhs);
   if(property)
   {
      theValue = property->theValue;
   }
   
   return *this;
}

bool rspfBooleanProperty::setValue(const rspfString& value)
{
   theValue = value.toBool();
   
   return true;
}

bool rspfBooleanProperty::setBooleanValue(bool value,
					   rspfString& /* msg */)
{
   theValue = value;

   return true;
}

void rspfBooleanProperty::valueToString(rspfString& valueResult)const
{
   valueResult = theValue?"true":"false";
}

bool rspfBooleanProperty::getBoolean()const
{
   return theValue;
}
