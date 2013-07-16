//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL see top level license.txt
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfStringProperty.cpp 9094 2006-06-13 19:12:40Z dburken $
#include <rspf/base/rspfStringProperty.h>
#include <algorithm>
RTTI_DEF1(rspfStringProperty, "rspfStringProperty", rspfProperty);

rspfStringProperty::rspfStringProperty(const rspfString& name,
                                         const rspfString& value,
                                         bool editableFlag,
                                         const std::vector<rspfString>& constraintList)
   :rspfProperty(name),
    theValue(value),
    theEditableFlag(editableFlag),
    theConstraints(constraintList)
{
}

rspfStringProperty::rspfStringProperty(const rspfStringProperty& rhs)
   :rspfProperty(rhs),
    theValue(rhs.theValue),
    theEditableFlag(rhs.theEditableFlag),
    theConstraints(rhs.theConstraints)
{
}

rspfObject* rspfStringProperty::dup()const
{
   return new rspfStringProperty(*this);
}

const rspfProperty& rspfStringProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);
   
   rspfStringProperty* rhsPtr = PTR_CAST(rspfStringProperty,
					  &rhs);
   
   theValue        = rhs.valueToString();
   
   if(rhsPtr)
     {
       theEditableFlag = rhsPtr->theEditableFlag;
       theConstraints  = rhsPtr->theConstraints;
     }
   
   return *this;
}


void rspfStringProperty::setEditableFlag(bool flag)
{
   theEditableFlag = flag;
}

bool rspfStringProperty::getEditableFlag()const
{
   return theEditableFlag;
}

bool rspfStringProperty::isEditable()const
{
   return (getEditableFlag() == true);
}


void rspfStringProperty::clearConstraints()
{
   theConstraints.clear();
}

void rspfStringProperty::setConstraints(const std::vector<rspfString>& constraintList)
{
   theConstraints = constraintList;
}

void rspfStringProperty::addConstraint(const rspfString& value)
{
   theConstraints.push_back(value);
}

const std::vector<rspfString>& rspfStringProperty::getConstraints()const
{
   return theConstraints;
}

bool rspfStringProperty::hasConstraints()const
{
   return (theConstraints.size() > 0);
}

bool rspfStringProperty::setValue(const rspfString& value)
{
   bool result = true;
   
   if(theConstraints.size() > 0)
   {
      if(std::find(theConstraints.begin(),
                   theConstraints.end(),
                   value)
                    != theConstraints.end())
      {
         theValue = value;
      }
      else
      {
         result = false;
      }
   }
   else
   {
      theValue = value;
   }

   return result;
}

void rspfStringProperty::valueToString(rspfString& valueResult)const
{
   valueResult = theValue;
}
