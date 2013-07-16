//*******************************************************************
// Copyright (C) 2003 Stroage Area Networks, Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Kenneth Melero (kmelero@sanz.com)
//
//*******************************************************************
//  $Id: rspfMetadataFileWriter.cpp 17195 2010-04-23 17:32:18Z dburken $

#include <rspf/imaging/rspfMetadataFileWriter.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfViewController.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfErrorContext.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/imaging/rspfImageHandlerRegistry.h>
#include <rspf/imaging/rspfImageHandler.h>
#include <rspf/base/rspfFilenameProperty.h>

RTTI_DEF3(rspfMetadataFileWriter,
          "rspfMetadataFileWriter",
          rspfConnectableObject,
          rspfProcessInterface,
          rspfConnectableObjectListener);

static rspfTrace traceDebug("rspfMetadataFileWriter:degug");

rspfMetadataFileWriter::rspfMetadataFileWriter()
   :rspfConnectableObject(0),
      rspfProcessInterface(),
      rspfConnectableObjectListener(),
      theInputConnection(NULL),
      theFilename(),
      thePixelType(RSPF_PIXEL_IS_POINT),
      theAreaOfInterest()
{
   addListener((rspfConnectableObjectListener*)this);
   theAreaOfInterest.makeNan();
}

rspfMetadataFileWriter::~rspfMetadataFileWriter()
{
   removeListener((rspfConnectableObjectListener*)this);
}

void rspfMetadataFileWriter::initialize()
{
   theInputConnection = dynamic_cast<rspfImageSource*> (getInput(0));
}

bool rspfMetadataFileWriter::loadState(const rspfKeywordlist& kwl,
                                     const char* prefix)
{
   bool result = rspfConnectableObject::loadState(kwl, prefix);

   // Get the filename...
   const char* lookup = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   if(lookup)
   {
      rspfString filename = lookup;

      setFilename(filename);
   }

   // Note: Defaulted to point in constructor.
   lookup = kwl.find(prefix, rspfKeywordNames::PIXEL_TYPE_KW);
   if (lookup)
   {
      rspfString s = lookup;
      s.downcase();
      if (s.contains("area"))
      {
         thePixelType = RSPF_PIXEL_IS_AREA;
      }
      else if (s.contains("point"))
      {
         thePixelType = RSPF_PIXEL_IS_POINT;
      }
   }
   
   if(theInputObjectList.size() != 1)
   {
      theInputObjectList.resize(1);
      theInputListIsFixedFlag = true;
   }
   
   theOutputListIsFixedFlag = false;
      
   return result;
}

void rspfMetadataFileWriter::disconnectInputEvent(rspfConnectionEvent& /* event */)
{
   initialize();
}

void rspfMetadataFileWriter::connectInputEvent(rspfConnectionEvent& /* event */)
{
   initialize();
}

void rspfMetadataFileWriter::propertyEvent(rspfPropertyEvent& /* event */)
{
   initialize();
}

void rspfMetadataFileWriter::setPixelType(rspfPixelType pixelType)
{
   thePixelType = pixelType;
}

rspfPixelType rspfMetadataFileWriter::getPixelType()const
{
   return thePixelType;
}

rspfObject* rspfMetadataFileWriter::getObject()
{
   return this;
}

const rspfObject* rspfMetadataFileWriter::getObject() const
{
   return this;
}

bool rspfMetadataFileWriter::execute()
{
   bool result = false;
   if (!theInputConnection)
   {
      initialize();
      if (!theInputConnection)
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfMetadataFileWriter::execute DEBUG:\n"
               << "theInputConnection is null!"
               << endl;
         }
         return result;
      }
   }
   
   if(theFilename == rspfFilename::NIL)
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfMetadataFileWriter::execute DEBUG:\n"
            << "theFilename is not set!"
            << endl;
      }
      return result;
   }
   
   //---
   // If theAreaOfInterest hasNans then it was probably not set so use
   // the bounding rectangle of the input source.
   //---
   if (theAreaOfInterest.hasNans())
   {
      theAreaOfInterest = theInputConnection->getBoundingRect();
      if (theAreaOfInterest.hasNans())
      {
         if (traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "rspfMetadataFileWriter::execute DEBUG:\n"
               << "theAreaOfInterest is not set!"
               << endl;
         }
         return result;
      }
   }

   setProcessStatus(rspfProcessInterface::PROCESS_STATUS_EXECUTING);

   setPercentComplete(0.0);

   result = writeFile();

   setProcessStatus(rspfProcessInterface::PROCESS_STATUS_NOT_EXECUTING);

   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspfMetadataFileWriter::execute DEBUG:\n"
         << (result?"successful":"unsuccessful") << " write of file:  "
         << theFilename
         << endl;
   }
   
   return result;
}

void rspfMetadataFileWriter::setPercentComplete(double percentComplete)
{
   rspfProcessInterface::setPercentComplete(percentComplete);
   rspfProcessProgressEvent event(this,
                                   percentComplete);
   fireEvent(event);      
}

void rspfMetadataFileWriter::setFilename(const rspfFilename& file)
{
   theFilename = file;
}

const rspfFilename& rspfMetadataFileWriter::getFilename() const
{
   return theFilename;
}

bool rspfMetadataFileWriter::canConnectMyInputTo(rspf_int32 inputIndex,
                                                  const rspfConnectableObject* object) const
{
   return (object &&
           ((PTR_CAST(rspfImageSource, object) &&
             inputIndex == 0)||
            (PTR_CAST(rspfViewController, object) &&
             inputIndex == 1)));
}

void rspfMetadataFileWriter::setAreaOfInterest(
   const rspfIrect& areaOfInterest)
{
   theAreaOfInterest = areaOfInterest;
}

rspfIrect rspfMetadataFileWriter::getAreaOfInterest() const
{
   return theAreaOfInterest;
}

void rspfMetadataFileWriter::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid())return;
   
   rspfString name = property->getName();

   if(name == rspfKeywordNames::FILENAME_KW)
   {
      setFilename(property->valueToString());
   }
   else
   {
      rspfConnectableObject::setProperty(property);
   }
}

rspfRefPtr<rspfProperty> rspfMetadataFileWriter::getProperty(const rspfString& name)const
{
   if(name == rspfKeywordNames::FILENAME_KW)
   {
      rspfFilenameProperty* filenameProp =
         new rspfFilenameProperty(name, theFilename);
      
      filenameProp->setIoType(rspfFilenameProperty::rspfFilenamePropertyIoType_OUTPUT);
      
      return filenameProp;
      
   }

   return rspfConnectableObject::getProperty(name);
}

void rspfMetadataFileWriter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfConnectableObject::getPropertyNames(propertyNames);
   propertyNames.push_back(rspfKeywordNames::FILENAME_KW);
}
