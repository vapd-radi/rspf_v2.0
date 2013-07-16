//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfColorProperty.cpp 13667 2008-10-02 19:59:55Z gpotts $
#include <sstream>
#include <rspf/base/rspfColorProperty.h>

RTTI_DEF1(rspfColorProperty, "rspfColorProperty", rspfProperty);

rspfColorProperty::rspfColorProperty(const rspfString& name,
                                       const rspfRgbVector& value)
   :rspfProperty(name),
    theValue(value)
{
}

rspfColorProperty::rspfColorProperty(const rspfColorProperty& rhs)
   :rspfProperty(rhs),
    theValue(rhs.theValue)
{
}

rspfColorProperty::~rspfColorProperty()
{
}

rspfObject* rspfColorProperty::dup()const
{
   return new rspfColorProperty(*this);
}

const rspfProperty& rspfColorProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);

   rspfColorProperty* rhsPtr = PTR_CAST(rspfColorProperty, &rhs);

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

bool rspfColorProperty::setValue(const rspfString& value)
{
   bool result = false;
   std::vector<rspfString> splitArray;
   
   value.split(splitArray, " ");
   if(splitArray.size() == 3)
   {
      int r,g,b;
      r = splitArray[0].toInt32();
      g = splitArray[1].toInt32();
      b = splitArray[2].toInt32();
      result = true;
      
      theValue = rspfRgbVector(r,g,b);
   }
   
   return result;
}

void rspfColorProperty::valueToString(rspfString& valueResult)const
{
   ostringstream out;

   out << (int)theValue.getR() << " " << (int)theValue.getG() << " " << (int)theValue.getB() << endl;

   valueResult = out.str().c_str();
}

const rspfRgbVector& rspfColorProperty::getColor()const
{
   return theValue;
}

void rspfColorProperty::setColor(const rspfRgbVector& value)
{
   theValue = value;
}

rspf_uint8 rspfColorProperty::getRed()const
{
   return theValue.getR();
}

rspf_uint8 rspfColorProperty::getGreen()const
{
   return theValue.getG();
}

rspf_uint8 rspfColorProperty::getBlue()const
{
   return theValue.getB();
}

void rspfColorProperty::setRed(rspf_uint8 r)
{
   theValue.setR(r);
}

void rspfColorProperty::setGreen(rspf_uint8 g)
{
   theValue.setG(g);
}

void rspfColorProperty::setBlue(rspf_uint8 b)
{
   theValue.setB(b);
}
