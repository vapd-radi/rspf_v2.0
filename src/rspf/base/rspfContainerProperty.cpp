//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfContainerProperty.cpp 19920 2011-08-09 12:04:27Z gpotts $
#include <rspf/base/rspfContainerProperty.h>
#include <rspf/base/rspfStringProperty.h>
#include <rspf/base/rspfVisitor.h>

RTTI_DEF1(rspfContainerProperty, "rspfContainerProperty", rspfProperty);

rspfContainerProperty::rspfContainerProperty(const rspfString& name)
   :rspfProperty(name)
{
}

rspfContainerProperty::rspfContainerProperty(const rspfContainerProperty& rhs)
   :rspfProperty(rhs)
{
   copyList(rhs);
}

rspfContainerProperty::~rspfContainerProperty()
{
   deleteChildren();
}

rspfObject* rspfContainerProperty::dup()const
{
   return new rspfContainerProperty(*this);
}

const rspfProperty& rspfContainerProperty::assign(const rspfProperty& rhs)
{
   rspfContainerProperty* rhsContainer = PTR_CAST(rspfContainerProperty,
                                                   &rhs);
   rspfProperty::assign(rhs);

   if(rhsContainer)
   {
      copyList(*rhsContainer);
   }

   return *this;
}

void rspfContainerProperty::copyList(const rspfContainerProperty& rhs)
{
   deleteChildren();

   for(rspf_uint32 idx = 0; idx < rhs.theChildPropertyList.size();++idx)
   {
      if(rhs.theChildPropertyList[idx].valid())
      {
         theChildPropertyList.push_back((rspfProperty*)(rhs.theChildPropertyList[idx]->dup()));
      }
      else
      {
         theChildPropertyList.push_back((rspfProperty*)0);
      }
   }
}

void rspfContainerProperty::addChildren(std::vector<rspfRefPtr<rspfProperty> >& propertyList)
{
   for(rspf_uint32 idx = 0; idx < propertyList.size(); ++idx)
   {
      if(propertyList[idx].valid())
      {
         theChildPropertyList.push_back(propertyList[idx]);
      }
   }
}

void rspfContainerProperty::addChild(rspfProperty* property)
{
   theChildPropertyList.push_back(property);
}

void rspfContainerProperty::addStringProperty(const rspfString& name,
                                               const rspfString& value,
                                               bool readOnlyFlag)
{
   rspfProperty* prop = new rspfStringProperty(name, value);
   
   prop->setReadOnlyFlag(readOnlyFlag);
   addChild(prop);
}

const rspfContainerProperty* rspfContainerProperty::asContainer()const
{
   return this;
}

rspfContainerProperty* rspfContainerProperty::asContainer()
{
   return this;
}

rspfRefPtr<rspfProperty> rspfContainerProperty::getProperty(const rspfString& name,
                                                               bool recurse)
{
   rspf_uint32 idx = 0;
   std::vector<rspfRefPtr<rspfContainerProperty> > containers;
   if(name == getName())
   {
      return this;
   }
   for(idx = 0; idx < theChildPropertyList.size(); ++idx)
   {
      if(theChildPropertyList[idx].valid())
      {
         if(theChildPropertyList[idx]->getName() == name)
         {
            return theChildPropertyList[idx];
         }
         if(PTR_CAST(rspfContainerProperty, theChildPropertyList[idx].get())&&recurse)
         {
            containers.push_back(PTR_CAST(rspfContainerProperty, theChildPropertyList[idx].get()));
         }
      }
   }
   
   if(containers.size())
   {
      for(idx = 0; idx < containers.size();++idx)
      {
         rspfRefPtr<rspfProperty> prop = containers[idx]->getProperty(name, recurse);
         if(prop.valid())
         {
            return prop;
         }
      }
   }
   
   return (rspfProperty*)0;
}

void rspfContainerProperty::deleteChildren()
{
//    for(rspf_uint32 idx = 0; idx < theChildPropertyList.size(); ++idx)
//    {
//       if(theChildPropertyList[idx])
//       {
//          delete theChildPropertyList[idx];
//          theChildPropertyList[idx] = (rspfProperty*)0;
//       }
//    }
   theChildPropertyList.clear();
}


bool rspfContainerProperty::setValue(const rspfString& /* value */ )
{
   return false;
}

void rspfContainerProperty::valueToString(rspfString& /* valueResult */ )const
{
}

rspf_uint32 rspfContainerProperty::getNumberOfProperties()const
{
   return (rspf_uint32)theChildPropertyList.size();
}

rspfRefPtr<rspfProperty> rspfContainerProperty::getProperty(rspf_uint32 idx)
{
   if(idx < theChildPropertyList.size())
   {
      return theChildPropertyList[idx];
   }

   return (rspfProperty*)0;
}

void rspfContainerProperty::getPropertyList(
   std::vector<rspfRefPtr<rspfProperty> >& children) const
{
   children = theChildPropertyList;
}

rspfRefPtr<rspfXmlNode> rspfContainerProperty::toXml()const
{
   rspfXmlNode* result = new rspfXmlNode;

   result->setTag(getName());
   
   rspf_uint32 idx = 0;
   for(idx = 0; idx < theChildPropertyList.size(); ++idx)
   {
      rspfRefPtr<rspfXmlNode> child = theChildPropertyList[idx]->toXml();

      result->addChildNode(child.get());
   }

   return result;
}

void rspfContainerProperty::setReadOnlyFlag(bool flag)
{
   rspfProperty::setReadOnlyFlag(flag);
   rspf_uint32 idx = 0;
   for(idx = 0; idx < theChildPropertyList.size(); ++idx)
   {
      theChildPropertyList[idx]->setReadOnlyFlag(flag);
   }
}

void rspfContainerProperty::accept(rspfVisitor& visitor)
{
   rspf_uint32 idx = 0;
   if(!visitor.hasVisited(this))
   {
      rspfProperty::accept(visitor);
      if(visitor.getVisitorType()&rspfVisitor::VISIT_CHILDREN)
      {
         for(idx = 0; idx < theChildPropertyList.size(); ++idx)
         {
            theChildPropertyList[idx]->accept(visitor);
         }
      }
   }
}

