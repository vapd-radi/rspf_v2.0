//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfStringListProperty.cpp 19682 2011-05-31 14:21:20Z dburken $
//
#include <sstream>
#include <algorithm>
#include <rspf/base/rspfStringListProperty.h>
#include <rspf/base/rspfKeywordlist.h>
RTTI_DEF1(rspfStringListProperty, "rspfStringListProperty", rspfProperty);

rspfStringListProperty::rspfStringListProperty(const rspfString& name,
                                                 const std::vector<rspfString>& value)
   :rspfProperty(name),
    theValueList(value),
    theUniqueFlag(false),
    theOrderMattersFlag(false)
{
}

rspfStringListProperty::rspfStringListProperty(const rspfStringListProperty& rhs)
   :rspfProperty(rhs),
    theValueList(rhs.theValueList),
    theConstraintList(rhs.theConstraintList),
    theUniqueFlag(rhs.theUniqueFlag),
    theOrderMattersFlag(rhs.theOrderMattersFlag),
    theMinNumberOfValues(rhs.theMinNumberOfValues),
    theMaxNumberOfValues(rhs.theMaxNumberOfValues)
{
}
   
rspfObject* rspfStringListProperty::dup()const
{
   return new rspfStringListProperty(*this);
}

const rspfProperty& rspfStringListProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);

   rspfStringListProperty *rhsPtr = PTR_CAST(rspfStringListProperty,
                                              &rhs);

   if(rhsPtr)
   {
      theValueList         = rhsPtr->theValueList;
      theConstraintList    = rhsPtr->theConstraintList;
      theUniqueFlag        = rhsPtr->theUniqueFlag;
      theOrderMattersFlag  = rhsPtr->theOrderMattersFlag;
      theMinNumberOfValues = rhsPtr->theMinNumberOfValues;
      theMaxNumberOfValues = rhsPtr->theMaxNumberOfValues;
   }

   return *this;
}

bool rspfStringListProperty::setValue(const rspfString& value)
{
   rspfKeywordlist kwl;
   std::istringstream in(value);
   bool result = true;
   
   if(kwl.parseStream(in))
   {
      int idx = 0;
      std::vector<rspfString> keys =
         kwl.getSubstringKeyList( "^([0-9]*" );
      
      std::vector<int> theNumberList(keys.size());
      for(idx = 0; idx < (int)theNumberList.size();++idx)
      {
         theNumberList[idx] = keys[idx].toInt();
      }
      std::sort(theNumberList.begin(), theNumberList.end());
      clearValueList();
      for(idx = 0; idx < (int)theNumberList.size(); ++idx)
      {
         const char* temp = kwl.find(rspfString::toString(theNumberList[idx]));

         if(temp)
         {
            addValue(temp);
         }
      }
   }
   else
   {
      result = false;
   }

   return result;
}

void rspfStringListProperty::valueToString(rspfString& valueResult)const
{
   rspfKeywordlist kwl;
   int idx = 0;

   for(idx = 0; idx < (int)theValueList.size(); ++idx)
   {
      kwl.add(rspfString::toString(idx).c_str(),
              theValueList[idx],
              true);
   }
   
   valueResult = kwl.toString();
}


void rspfStringListProperty::clearValueList()
{
   theValueList.clear();
}

rspfString rspfStringListProperty::getValueAt(int idx)const
{
   if((idx >= 0)&&
      (idx < (int)getNumberOfValues()))
   {
      return theValueList[idx];
   }
   
   return rspfString("");;
}

bool rspfStringListProperty::setValueAt(int idx,
                                         const rspfString& value)
{
   bool result = true;
   
   if(canAddValue(value))
   {
      if((idx < (int)getNumberOfValues())&&
         (idx >= 0))
      {
         theValueList[idx] = value;
      }
   }
   else
   {
      result = false;
   }
   return result;
   
}

bool rspfStringListProperty::addValue(const rspfString& value)
{
   bool result = true;
   
   if(canAddValue(value))
   {
      theValueList.push_back(value);
   }
   else
   {
      result = false;
   }

   return result;
}
 
rspf_uint32 rspfStringListProperty::getNumberOfValues()const
{
   return (int)theValueList.size();
}

rspf_uint32 rspfStringListProperty::getNumberOfContraints()const
{
   return (rspf_uint32)theConstraintList.size();
}

rspfString rspfStringListProperty::getConstraintAt(rspf_uint32 idx)const
{
   if(idx < theConstraintList.size())
   {
      return theConstraintList[(int)idx];
   }

   return rspfString("");
}

void rspfStringListProperty::setConstraints(const std::vector<rspfString>& constraints)
{
   theConstraintList =  constraints;
}

bool rspfStringListProperty::hasConstraints()const
{
   return (theConstraintList.size()>0);
}

void rspfStringListProperty::setUniqueFlag(bool flag)
{
   theUniqueFlag = flag;
}

void rspfStringListProperty::setOrderMattersFlag(bool flag)
{
   theOrderMattersFlag = flag;
}

void rspfStringListProperty::setNumberOfValuesBounds(int minNumber,
                                                      int maxNumber)
{
   theMinNumberOfValues = minNumber;
   theMaxNumberOfValues = maxNumber;
}

void rspfStringListProperty::getNumberofValuesBounds(int& minNumber,
                                                      int& maxNumber)const
{
   minNumber = theMinNumberOfValues;
   maxNumber = theMaxNumberOfValues;
}

bool rspfStringListProperty::findValueInConstraintList(const rspfString& value)const
{
   return (std::find(theConstraintList.begin(),
                     theConstraintList.end(),
                     value)!=theConstraintList.end());
}

bool rspfStringListProperty::findValueInValueList(const rspfString& value)const
{
   return (std::find(theValueList.begin(),
                     theValueList.end(),
                     value)!=theConstraintList.end());
}

bool rspfStringListProperty::canAddValue(const rspfString& value)const
{
   bool result = true;

   if(hasConstraints())
   {
      if(findValueInConstraintList(value))
      {
         if(theUniqueFlag)
         {
            if(findValueInValueList(value))
            {
               result = false;
            }
         }
      }
      else
      {
         result = false;
      }
   }

   return result;
}
