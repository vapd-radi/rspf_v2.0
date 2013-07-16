//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfPropertyInterface.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/base/rspfPropertyInterface.h>
#include <rspf/base/rspfStringProperty.h>

RTTI_DEF(rspfPropertyInterface, "rspfPropertyInterface");

void rspfPropertyInterface::setProperty(const rspfString& name,
                                         const rspfString& value)
{
   rspfRefPtr<rspfProperty> property = new rspfStringProperty(name, value);

   setProperty(property);
}

void rspfPropertyInterface::setProperty(rspfRefPtr<rspfProperty> /* property */ )
{
}

rspfRefPtr<rspfProperty> rspfPropertyInterface::getProperty(const rspfString& /* name */)const
{
   return (rspfProperty*)0;
}

void rspfPropertyInterface::getPropertyNames(std::vector<rspfString>& /* propertyNames */)const
{
}

rspfString rspfPropertyInterface::getPropertyValueAsString(const rspfString& name)const
{
	rspfRefPtr<rspfProperty> prop = getProperty(name);
	if(prop.valid())
	{
		return prop->valueToString();
	}
	
	return rspfString("");
}


void rspfPropertyInterface::getPropertyList(std::vector<rspfRefPtr<rspfProperty> >& propertyList)const
{
   std::vector<rspfString> propertyNames;
   int idx = 0;
   getPropertyNames(propertyNames);

   for(idx = 0; idx < (int)propertyNames.size();++idx)
   {
      rspfRefPtr<rspfProperty> prop = getProperty(propertyNames[idx]);
      if(prop.valid())
      {
         propertyList.push_back(prop);  
      }
   }
}

void rspfPropertyInterface::setProperties(std::vector<rspfRefPtr<rspfProperty> >& propertyList)
{
   rspf_uint32 idx = 0;

   for(idx = 0; idx < propertyList.size(); ++idx)
   {
      if(propertyList[idx].valid())
      {
         setProperty(propertyList[idx]);
      }
   }
}
