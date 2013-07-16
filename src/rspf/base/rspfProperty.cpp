//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfProperty.cpp 19917 2011-08-09 11:12:24Z gpotts $
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfKeywordlist.h>

RTTI_DEF1(rspfProperty, "rspfProperty", rspfObject);

const rspfProperty& rspfProperty::operator = (const rspfProperty& rhs)
{
   this->assign(rhs);
   
   return *this;
}

rspfProperty::rspfProperty(const rspfString& name)
   :theName(name),
    theReadOnlyFlag(false),
    theChangeType(rspfPropertyChangeType_NOTSPECIFIED),
    theModifiedFlag(false)
{
}

rspfProperty::rspfProperty(const rspfProperty& rhs)
   :theName(rhs.theName),
    theReadOnlyFlag(rhs.theReadOnlyFlag),
    theChangeType(rhs.theChangeType),
    theModifiedFlag(rhs.theModifiedFlag)
{
}

rspfProperty::~rspfProperty()
{
}

const rspfString& rspfProperty::getName()const
{
   return theName;
}

void rspfProperty::setName(const rspfString& name)
{
   theName = name;
}

const rspfProperty& rspfProperty::assign(const rspfProperty& rhs)
{
   theName         = rhs.theName;
   theReadOnlyFlag = rhs.theReadOnlyFlag;
   theChangeType   = rhs.theChangeType;
   theModifiedFlag = rhs.theModifiedFlag;
   
   return *this;
}

rspfString rspfProperty::valueToString()const
{
   rspfString result;

   valueToString(result);
   
   return result;
   
}

const rspfContainerProperty* rspfProperty::asContainer()const
{
   return 0;
}

rspfContainerProperty* rspfProperty::asContainer()
{
   return 0;
}

bool rspfProperty::isChangeTypeSet(int type)const
{
   return ((type&theChangeType)!=0);
}

void rspfProperty::clearChangeType()
{
   theChangeType = rspfPropertyChangeType_NOTSPECIFIED;
}

void rspfProperty::setChangeType(int type, bool on)
{
   if(on)
   {
      theChangeType = static_cast<rspfPropertyChangeType>(theChangeType | type);
   }
   else 
   {
      theChangeType = static_cast<rspfPropertyChangeType>((~type)&theChangeType);
   }
}

void rspfProperty::setFullRefreshBit()
{
   setChangeType(rspfPropertyChangeType_FULL_REFRESH);
}

void rspfProperty::setCacheRefreshBit()
{
   setChangeType(rspfPropertyChangeType_CACHE_REFRESH);
}

rspfProperty::rspfPropertyChangeType rspfProperty::getChangeType()const
{
   return theChangeType;
}

bool rspfProperty::isFullRefresh()const
{
   return isChangeTypeSet(rspfPropertyChangeType_FULL_REFRESH);
}

bool rspfProperty::isCacheRefresh()const
{
   return isChangeTypeSet(rspfPropertyChangeType_CACHE_REFRESH);
}

bool rspfProperty::isChangeTypeSpecified()const
{
   return (theChangeType != rspfPropertyChangeType_NOTSPECIFIED);
}

bool rspfProperty::affectsOthers()const
{
   return isChangeTypeSet(rspfPropertyChangeType_AFFECTS_OTHERS);
}

void rspfProperty::setReadOnlyFlag(bool flag)
{
   theReadOnlyFlag = flag;
}

bool rspfProperty::getReadOnlyFlag()const
{
   return theReadOnlyFlag;
}

bool rspfProperty::isReadOnly()const
{
   return (getReadOnlyFlag());
}

void rspfProperty::setModifiedFlag(bool flag)
{
   theModifiedFlag = flag;
}

bool rspfProperty::getModifiedFlag()const
{
   return theModifiedFlag;
}

bool rspfProperty::isModified()const
{
   return (theModifiedFlag == true);
}

void rspfProperty::clearModifiedFlag()
{
   theModifiedFlag = false;
}

void rspfProperty::setDescription(const rspfString& description)
{
   theDescription = description;
}

rspfString rspfProperty::getDescription()const
{
   return theDescription;
}

void rspfProperty::accept(rspfVisitor& visitor)
{
   rspfObject::accept(visitor);
}

rspfRefPtr<rspfXmlNode> rspfProperty::toXml()const
{
   rspfRefPtr<rspfXmlNode> result = new rspfXmlNode();
   
   result->setTag(getName());
   result->setText(valueToString());
   
   return result;
}

void rspfProperty::saveState(rspfKeywordlist& kwl, const rspfString& prefix)const
{
   const rspfContainerProperty* container = asContainer();
   if(container)
   {
      rspf_uint32 nproperties     = container->getNumberOfProperties();
      rspf_uint32 propertiesIndex = 0;
      for(propertiesIndex = 0; propertiesIndex < nproperties; ++propertiesIndex)
      {
         rspfString newPrefix = prefix + container->getName() + ".";
         rspfRefPtr<rspfProperty> prop = container->theChildPropertyList[propertiesIndex];
         if(prop.valid())
         {
            prop->saveState(kwl, newPrefix);
         }
      }
   }
   else 
   {
      kwl.add(prefix + getName(), valueToString(), true);
   }
}

