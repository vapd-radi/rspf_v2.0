//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfFilenameProperty.cpp 15833 2009-10-29 01:41:53Z eshirschorn $
#include <rspf/base/rspfFilenameProperty.h>

RTTI_DEF1(rspfFilenameProperty, "rspfFilenameProperty", rspfProperty);

rspfFilenameProperty::rspfFilenameProperty(const rspfString& name,
                                             const rspfFilename& value,
                                             const std::vector<rspfString>& filterList)
   :rspfProperty(name),
    theValue(value),
    theFilterList(filterList),
    theIoType(rspfFilenamePropertyIoType_NOT_SPECIFIED)
{
}

rspfFilenameProperty::rspfFilenameProperty(const rspfFilenameProperty& rhs)
   :rspfProperty(rhs),
    theValue(rhs.theValue),
    theFilterList(rhs.theFilterList),
    theIoType(rhs.theIoType)
{
}

rspfFilenameProperty::~rspfFilenameProperty()
{
}

rspfObject* rspfFilenameProperty::dup()const
{
   return new rspfFilenameProperty(*this);
}

void rspfFilenameProperty::valueToString(rspfString& valueResult)const
{
   valueResult = theValue;
}

bool rspfFilenameProperty::setValue(const rspfString& value)
{
   theValue = value;

   return true;
}

const rspfFilenameProperty& rspfFilenameProperty::operator = (rspfFilenameProperty& rhs)
{
   assign(rhs);

   return *this;
}

const rspfProperty& rspfFilenameProperty::assign(const rspfProperty& rhs)
{
   rspfProperty::assign(rhs);
   theValue = rhs.valueToString();

   rspfFilenameProperty* rhsPtr = PTR_CAST(rspfFilenameProperty,
                                            &rhs);

   if(rhsPtr)
   {
      theFilterList = rhsPtr->theFilterList;
      theIoType     = rhsPtr->theIoType;
   }

   return *this;
}

void rspfFilenameProperty::clearFilterList()
{
   theFilterList.clear();
}

rspf_uint32 rspfFilenameProperty::getNumberOfFilters()const
{
   return (rspf_uint32)theFilterList.size();
}

void rspfFilenameProperty::setFilter(rspf_uint32 idx,
                                      const rspfString& filterValue)
{
   if(idx < getNumberOfFilters())
   {
      theFilterList[idx] = filterValue;
   }
}

rspfString rspfFilenameProperty::getFilter(rspf_uint32 idx)const
{
   if(idx < getNumberOfFilters())
   {
      return theFilterList[idx];
   }

   return rspfString("");
}


rspfString rspfFilenameProperty::getFilterListAsString(const rspfString& separator)const
{
   rspfString result;
   rspf_uint32 idx = 0;

   if(getNumberOfFilters() > 0)
   {
      for(idx = 0; idx < getNumberOfFilters()-1;++idx)
      {
         result += getFilter(idx);
         result += separator;
      }
      result += getFilter(getNumberOfFilters()-1);
   }

   return result;
}


const std::vector<rspfString>& rspfFilenameProperty::getFilterList()const
{
   return theFilterList;
}

void rspfFilenameProperty::addFilter(const rspfString& filter)
{
   theFilterList.push_back(filter);
}


void rspfFilenameProperty::setIoType(rspfFilenamePropertyIoType ioType)
{
   theIoType = ioType;
}

rspfFilenameProperty::rspfFilenamePropertyIoType rspfFilenameProperty::getIoType()const
{
   return theIoType;
}

bool rspfFilenameProperty::isIoTypeInput()const
{
   return (theIoType==rspfFilenamePropertyIoType_INPUT);
}

bool rspfFilenameProperty::isIoTypeOutput()const
{
   return (theIoType==rspfFilenamePropertyIoType_OUTPUT);
}
