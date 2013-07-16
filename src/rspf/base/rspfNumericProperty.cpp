//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfNumericProperty.cpp 9963 2006-11-28 21:11:01Z gpotts $
#include <algorithm>
#include <rspf/base/rspfNumericProperty.h>


RTTI_DEF1(rspfNumericProperty, "rspfNumericProperty", rspfProperty);

rspfNumericProperty::rspfNumericProperty(const rspfString& name,
                                           const rspfString& value)
   :rspfProperty(name),
    theValue(value),
    theType(rspfNumericPropertyType_FLOAT64)
{
}

rspfNumericProperty::rspfNumericProperty(const rspfString& name,
                                           const rspfString& value,
                                           double minValue,
                                           double maxValue)
   :rspfProperty(name),
    theValue(value),
    theType(rspfNumericPropertyType_FLOAT64)
{
   setConstraints(minValue,
                  maxValue);
}

rspfNumericProperty::rspfNumericProperty(const rspfNumericProperty& rhs)
   :rspfProperty(rhs),
    theValue(rhs.theValue),
    theType(rhs.theType),
    theRangeConstraint(rhs.theRangeConstraint)
{
}

rspfObject* rspfNumericProperty::dup()const
{
   return new rspfNumericProperty(*this);
}

const rspfProperty& rspfNumericProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);

   rspfNumericProperty* numericProperty = PTR_CAST(rspfNumericProperty,
                                                   &rhs);
   if(numericProperty)
   {
      theValue           = numericProperty->theValue;
      theType            = numericProperty->theType;
      theRangeConstraint = numericProperty->theRangeConstraint;
   }
   else
   {
      rspfString value;
      rhs.valueToString(value);
      setValue(value);
   }
   return *this;
}

bool rspfNumericProperty::hasConstraints()const
{
   return (theRangeConstraint.size() == 2);
}

double rspfNumericProperty::getMinValue()const
{
   if(hasConstraints())
   {
      return theRangeConstraint[0];
   }

   return 0.0;
}

double rspfNumericProperty::getMaxValue()const
{
   if(hasConstraints())
   {
      return theRangeConstraint[1];
   }

   return 0.0;
}

void rspfNumericProperty::clearConstraints()
{
   theRangeConstraint.clear();
}

void rspfNumericProperty::setConstraints(double minValue,
                                          double maxValue)
{
   theRangeConstraint.resize(2);
   theRangeConstraint[0] = minValue;
   theRangeConstraint[1] = maxValue;

   if(minValue > maxValue)
   {
      std::swap(theRangeConstraint[0],
                theRangeConstraint[1]);
   }
}

bool rspfNumericProperty::setValue(const rspfString& value)
{
   bool result = true;
   if(hasConstraints())
   {
      rspf_float64 tempV = (rspf_float64)value.toDouble();
      if((tempV >= theRangeConstraint[0])&&
         (tempV <= theRangeConstraint[1]))
      {
         theValue = value;
      }
   }
   else
   {
     theValue = value;
   }

   return result;
}

void rspfNumericProperty::valueToString(rspfString& valueResult)const
{
   switch(theType)
   {
   case rspfNumericPropertyType_INT:
   {
      valueResult = rspfString::toString(asInt32());
      break;
   }
   case rspfNumericPropertyType_UINT:
   {
      valueResult = rspfString::toString(asUInt32());
      break;
   }
   case rspfNumericPropertyType_FLOAT32:
   {
      valueResult = rspfString::toString(asFloat32());
      
      break;
   }
   case rspfNumericPropertyType_FLOAT64:
   {
      valueResult = rspfString::toString(asFloat64());
      break;
   }
   };
}


rspfNumericProperty::rspfNumericPropertyType rspfNumericProperty::getNumericType()const
{
   return theType;
}

void rspfNumericProperty::setNumericType(rspfNumericPropertyType type)
{
   theType = type;
}

rspf_float64 rspfNumericProperty::asFloat64()const
{
   return (rspf_float64)theValue.toDouble();
}

rspf_float32 rspfNumericProperty::asFloat32()const
{
   return (rspf_float32)theValue.toDouble();
}

rspf_uint32  rspfNumericProperty::asUInt32()const
{
   return theValue.toUInt32();
}

rspf_uint16 rspfNumericProperty::asUInt16()const
{
   return (rspf_uint16)theValue.toUInt32();
}

rspf_uint8 rspfNumericProperty::asUInt8()const
{
   return (rspf_uint8)theValue.toUInt32();
}

rspf_sint32 rspfNumericProperty::asInt32()const
{
   return (rspf_int32)theValue.toInt32();
}

rspf_sint16 rspfNumericProperty::asInt16()const
{
   return (rspf_int16)theValue.toInt32();
}

rspf_sint8 rspfNumericProperty::asInt8()const
{
   return (rspf_int8)theValue.toInt32();
}
