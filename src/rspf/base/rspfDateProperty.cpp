//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL see top level license.txt
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfDateProperty.cpp 9094 2006-06-13 19:12:40Z dburken $
//

#include <iostream>
#include <iomanip>
#include <sstream>
#include <rspf/base/rspfDateProperty.h>


RTTI_DEF1(rspfDateProperty, "rspfDateProperty", rspfProperty);

rspfDateProperty::rspfDateProperty()
      :rspfProperty("")
{
   setDate(rspfDate());
}

rspfDateProperty::rspfDateProperty(const rspfString& name,
                                     const rspfString& value)
      :rspfProperty(name)
{
   setValue(value);
}

rspfDateProperty::rspfDateProperty(const rspfString& name,
                                     const rspfLocalTm& value)
      :rspfProperty(name),
       theValue(value)
{
}

rspfDateProperty::rspfDateProperty(const rspfDateProperty& src)
   :rspfProperty(src),
    theValue(src.theValue)
{
}

rspfObject* rspfDateProperty::dup()const
{
   return new rspfDateProperty(*this);
}

void rspfDateProperty::setDate(const rspfLocalTm& localTm)
{
   theValue = localTm;
}

const rspfLocalTm& rspfDateProperty::getDate()const
{
   return theValue;
}

bool rspfDateProperty::setValue(const rspfString& value)
{
   if(value.trim() == "")
   {
      theValue = rspfDate();
      return true;
   }
   bool result = value.size() == 14;
   
   rspfString year;
   rspfString month;
   rspfString day;
   rspfString hour;
   rspfString min;
   rspfString sec;

   if(value.size() == 14)
   {
      year = rspfString(value.begin(),
                         value.begin()+4);
      month = rspfString(value.begin()+4,
                          value.begin()+6);
      day = rspfString(value.begin()+6,
                        value.begin()+8);
      hour = rspfString(value.begin()+8,
                        value.begin()+10);
      min = rspfString(value.begin()+10,
                        value.begin()+12);
      sec = rspfString(value.begin()+12,
                        value.begin()+14);

      theValue.setYear(year.toUInt32());
      theValue.setMonth(month.toUInt32());
      theValue.setDay(day.toUInt32());
      theValue.setHour(hour.toUInt32());
      theValue.setMin(min.toUInt32());
      theValue.setSec(sec.toUInt32());
   }

   return result;
}

void rspfDateProperty::valueToString(rspfString& valueResult)const
{
    std::ostringstream out;

   out << std::setw(4)
       << std::setfill('0')
       << theValue.getYear()
       << std::setw(2)
       << std::setfill('0')
       << theValue.getMonth()
       << std::setw(2)
       << std::setfill('0')
       << theValue.getDay()
       << std::setw(2)
       << std::setfill('0')
       << theValue.getHour()
       << std::setw(2)
       << std::setfill('0')
       << theValue.getMin()
       << std::setw(2)
       << std::setfill('0')
       << theValue.getSec();
   
   valueResult =  out.str();
  
}

const rspfProperty& rspfDateProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);

   rspfDateProperty* rhsPtr = PTR_CAST(rspfDateProperty, &rhs);

   if(rhsPtr)
   {
      theValue = rhsPtr->theValue;
   }
   else
   {
      setValue(rhs.valueToString());
   }

   return *this;
}
