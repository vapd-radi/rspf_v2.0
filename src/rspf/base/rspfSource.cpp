//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfSource.cpp 17195 2010-04-23 17:32:18Z dburken $
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfDataObject.h>
#include <rspf/base/rspfIdManager.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfBooleanProperty.h>

RTTI_DEF2(rspfSource, "rspfSource",
          rspfConnectableObject, rspfErrorStatusInterface)


rspfSource::rspfSource(rspfObject* owner)
   :
      rspfConnectableObject(owner),
      theEnableFlag(true),
      theInitializedFlag(false)
{
}

rspfSource::rspfSource(const rspfSource &rhs)
   :
      rspfConnectableObject(rhs),
      theEnableFlag(rhs.theEnableFlag),
      theInitializedFlag(rhs.theInitializedFlag)
{
}

rspfSource::rspfSource(rspfObject* owner,
                         rspf_uint32 inputListSize,
                         rspf_uint32 outputListSize,
                         bool inputListIsFixedFlag,
                         bool outputListIsFixedFlag)
   :
      rspfConnectableObject(owner, inputListSize, outputListSize,
                             inputListIsFixedFlag, outputListIsFixedFlag),
      theEnableFlag(true),
      theInitializedFlag(false)
{}

rspfSource::~rspfSource()   
{
}

bool rspfSource::saveState(rspfKeywordlist& kwl,
                            const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::ENABLED_KW,
           (int)theEnableFlag,
           true);

   return rspfConnectableObject::saveState(kwl, prefix);
}

bool rspfSource::loadState(const rspfKeywordlist& kwl,
                            const char* prefix)
{
   const char* lookup = kwl.find(prefix,
                                 rspfKeywordNames::ENABLED_KW);
   if(lookup)
   {
      theEnableFlag = rspfString(lookup).toBool();
   }

   return rspfConnectableObject::loadState(kwl, prefix);
}

bool rspfSource::isSourceEnabled()const
{
   return theEnableFlag;
}

void rspfSource::enableSource()
{
   setEnableFlag(true);
}

void rspfSource::disableSource()
{
   setEnableFlag(false);
}

bool rspfSource::getEnableFlag() const
{
   return theEnableFlag;
}

void rspfSource::setEnableFlag(bool flag)
{
   theEnableFlag = flag;
}

bool rspfSource::isInitialized() const
{
   return theInitializedFlag;
}

void rspfSource::setInitializedFlag(bool flag)
{
   theInitializedFlag = flag;
}

void rspfSource::initialize()
{
}

void rspfSource::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;

   rspfString name = property->getName();
   name = name.downcase();
   
   if(name == rspfKeywordNames::ENABLED_KW)
   {
      rspfString value;
      
      property->valueToString(value);
      setEnableFlag(value.toBool());
   }
   else
   {
      rspfConnectableObject::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfSource::getProperty(const rspfString& name)const
{
   if( (name == "Enabled") || (name == rspfKeywordNames::ENABLED_KW) )
   {
      return new rspfBooleanProperty(rspfKeywordNames::ENABLED_KW,theEnableFlag);
   }
   return rspfConnectableObject::getProperty(name);
}

void rspfSource::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfConnectableObject::getPropertyNames(propertyNames);
   
   propertyNames.push_back(rspfKeywordNames::ENABLED_KW);
}

const rspfSource& rspfSource::operator=(const rspfSource& /* rhs */)
{
   return *this;
}

std::ostream& rspfSource::print(std::ostream& out) const
{
   out << "rspfSource::print:\n"
      << "theEnableFlag:       " << theEnableFlag
      << "\ntheInitializedFlag:  " << theInitializedFlag
      << endl;

   return rspfErrorStatusInterface::print(out);
}
