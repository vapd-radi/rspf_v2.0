//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfDataObject.cpp 19931 2011-08-10 11:53:25Z gpotts $
#include <rspf/base/rspfDataObject.h>
#include <rspf/base/rspfSource.h>
#include <rspf/base/rspfScalarTypeLut.h>

RTTI_DEF1(rspfDataObject, "rspfDataObject", rspfObject)
   
rspfDataObject::rspfDataObject(rspfSource* source,
                                 rspfDataObjectStatus status)
   :
      theOwner(source),
      theDataObjectStatus(status)
{
}

rspfDataObject::rspfDataObject(const rspfDataObject& rhs)
   : rspfObject(),
     theOwner(0),
     theDataObjectStatus(rhs.theDataObjectStatus)
{
}

rspfDataObject::~rspfDataObject()
{
}

void rspfDataObject::setOwner(rspfSource* aSource)
{
   theOwner = aSource;
}

rspfSource* rspfDataObject::getOwner()
{
   return theOwner;
}

const rspfSource* rspfDataObject::getOwner() const
{
   return theOwner;
}

void rspfDataObject::setDataObjectStatus(rspfDataObjectStatus status) const
{
   theDataObjectStatus = status;
}

rspfDataObjectStatus rspfDataObject::getDataObjectStatus()const
{
   return theDataObjectStatus;
}

rspfString rspfDataObject::getDataObjectStatusString() const
{
   rspfString s;
   
   switch (theDataObjectStatus)
   {
      case RSPF_NULL:
         s = "RSPF_NULL";
         break;

      case RSPF_EMPTY:
         s = "RSPF_EMPTY";
         break;

      case RSPF_PARTIAL:
         s = "RSPF_PARTIAL";
         break;

      case RSPF_FULL:
         s = "RSPF_FULL";
         break;
         
      case RSPF_STATUS_UNKNOWN:
      default:
         s = "RSPF_STATUS_UNKNOWN";
         break;
   }
   
   return s;
}

rspf_uint32 rspfDataObject::getObjectSizeInBytes()const
{
   return sizeof(theOwner);
}

bool rspfDataObject::isInitialize()const
{
   return (getDataObjectStatus()!=RSPF_NULL);
}

bool rspfDataObject::operator!=(const rspfDataObject& rhs) const
{
   return ( theOwner               != rhs.theOwner ||
            theDataObjectStatus    != rhs.theDataObjectStatus );
}

bool rspfDataObject::operator==(const rspfDataObject& rhs) const
{
   return (theOwner            == rhs.theOwner &&
           theDataObjectStatus == rhs.theDataObjectStatus);
}

void rspfDataObject::assign(const rspfDataObject* rhs)
{
   if(rhs != this)
   {
      theOwner               = rhs->theOwner;
      theDataObjectStatus     = rhs->theDataObjectStatus;
   }
}

const rspfDataObject* rspfDataObject::operator=(const rspfDataObject* rhs)
{
   assign(rhs);
   return this;
}

const rspfDataObject& rspfDataObject::operator=(const rspfDataObject& rhs)
{
   if (this != &rhs)
   {
      theOwner            = rhs.theOwner;
      theDataObjectStatus = rhs.theDataObjectStatus; 
   }
   return *this;
}

std::ostream& rspfDataObject::print(ostream& out) const
{
   out << "rspfDataObject::print:";
   
   if (theOwner)
   {
      out << "\ntheOwner->getClassName():  "
          << theOwner->getClassName()<<endl;
   }
   else
   {
      out << "\ntheOwner is null"
          <<endl;
   }
   out << "theDataObjectStatus: " << getDataObjectStatusString()
       << endl;

   return out;
}

bool rspfDataObject::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix, "data_object_status", getDataObjectStatusString(), true);
   return rspfObject::saveState(kwl, prefix);
}

bool rspfDataObject::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   rspfString data_object_status = kwl.find(prefix, "data_object_status");
   if(!data_object_status.empty())
   {
      data_object_status = data_object_status.upcase();
      if(data_object_status == "RSPF_FULL")
      {
         theDataObjectStatus = RSPF_FULL;
      }
      else if(data_object_status == "RSPF_PARTIAL")
      {
         theDataObjectStatus = RSPF_PARTIAL;
      }
      else if(data_object_status == "RSPF_EMPTY")
      {
         theDataObjectStatus = RSPF_EMPTY;
      }
      else if(data_object_status == "RSPF_STATUS_UNKNOWN")
      {
         theDataObjectStatus = RSPF_STATUS_UNKNOWN;
      }
   }
   
   return rspfObject::loadState(kwl, prefix);
}                     
