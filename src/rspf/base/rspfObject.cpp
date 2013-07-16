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
// $Id: rspfObject.cpp 20070 2011-09-07 18:48:35Z dburken $
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfVisitor.h>

RTTI_DEF(rspfObject, "rspfObject")

rspfObject::rspfObject()
{}

rspfObject::~rspfObject()
{
}
   
rspfObject* rspfObject::dup()const
{
   rspfObject* result = NULL;
   rspfKeywordlist kwl;

   saveState(kwl);
   
   result = rspfObjectFactoryRegistry::instance()->createObject(kwl);

   if(!result)
   {
      rspfNotify(rspfNotifyLevel_WARN)
         << "dup() not implemented for object = "
         << getClassName() << ". Please implement!\n";
   }
   
   return result;
}

rspfString rspfObject::getShortName()const
{
   return getClassName();
}

rspfString rspfObject::getLongName()const
{
   return getClassName();
}

rspfString rspfObject::getDescription()const
{
   return getLongName();
}


rspfString rspfObject::getClassName()const
{
   return TYPE_NAME(this);
}

RTTItypeid rspfObject::getType()const
{
   return TYPE_INFO(this);
}

bool rspfObject::canCastTo(rspfObject* obj)const
{
   if(obj)
   {
      return obj->getType().can_cast(this->getType());
   }
   
   return false;
}

bool rspfObject::canCastTo(const rspfString& parentClassName)const
{
   return (getType().find_baseclass(parentClassName.c_str()) !=
           getType().null_type());
}

bool rspfObject::canCastTo(const RTTItypeid& id)const
{
   return id.can_cast(this->getType());
}

bool rspfObject::saveState(rspfKeywordlist& kwl,
                            const char* prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::TYPE_KW,
           getClassName(),
           true);

   return true;
}

bool rspfObject::isEqualTo(const rspfObject& obj, rspfCompareType /* compareType */)const
{
   return (getClassName() == obj.getClassName());
}

void rspfObject::accept(rspfVisitor& visitor)
{
   if(!visitor.stopTraversal()&&!visitor.hasVisited(this))
   {
      visitor.visit(this);
   }
}

bool rspfObject::loadState(const rspfKeywordlist&, const char*)
{
   return true;
}

std::ostream& rspfObject::print(std::ostream& out) const
{
   return out;
}

std::ostream& operator<<(std::ostream& out, const rspfObject& obj)
{
   return obj.print(out);
}
