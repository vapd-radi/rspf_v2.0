
//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
//
// Description:
//
//*******************************************************************
//  $Id: rspfImageSourceFilter.cpp 18920 2011-02-18 20:06:11Z gpotts $

#include <rspf/imaging/rspfImageSourceFilter.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfProperty.h>
 
static rspfTrace traceDebug("rspfImageSourceFilter:degug");

RTTI_DEF2(rspfImageSourceFilter, "rspfImageSourceFilter", rspfImageSource, rspfConnectableObjectListener)

rspfImageSourceFilter::rspfImageSourceFilter(rspfObject* owner)
   : rspfImageSource(owner,
                      1, // number of inputs
                      0, // number of outputs
                      true, // input's fixed
                      false), // outputs ar not fixed
     theInputConnection(NULL)
{
   addListener((rspfConnectableObjectListener*)this);
}

rspfImageSourceFilter::rspfImageSourceFilter(rspfImageSource* inputSource)
   : rspfImageSource(NULL,
                      1,
                      0,
                      true,
                      false),
     theInputConnection(inputSource)
{
   if(inputSource)
   {
     connectMyInputTo(0, inputSource);
   }
   addListener((rspfConnectableObjectListener*)this);
}

rspfImageSourceFilter::rspfImageSourceFilter(rspfObject* owner,
                                               rspfImageSource* inputSource)
   : rspfImageSource(owner,
                      1,
                      0,
                      true,
                      false),
     theInputConnection(inputSource)
{
   if(inputSource)
   {
      theInputObjectList[0] = inputSource;
      inputSource->connectMyOutputTo(this, false);
   }
   addListener((rspfConnectableObjectListener*)this);
}


rspfImageSourceFilter::~rspfImageSourceFilter()
{
   removeListener((rspfConnectableObjectListener*)this);
   theInputConnection = NULL;
}

rspf_uint32 rspfImageSourceFilter::getNumberOfInputBands()const
{
   if(theInputConnection)
   {
      return theInputConnection->getNumberOfOutputBands();
   }

   return 0;
}

void rspfImageSourceFilter::initialize()
{
   theInputConnection = PTR_CAST(rspfImageSource, getInput(0));
}

bool rspfImageSourceFilter::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   bool result = rspfImageSource::loadState(kwl, prefix);

   // make sure we have 1 input.
   //setNumberOfInputs(1);

   // we will fix the input
   //if(!theInputListIsFixedFlag)
  // {
  //    theInputListIsFixedFlag = true;
  // }
   
   return result;
}

bool rspfImageSourceFilter::saveState(rspfKeywordlist& kwl,
                                       const char* prefix)const
{
   return rspfImageSource::saveState(kwl, prefix);
}


bool rspfImageSourceFilter::canConnectMyInputTo(rspf_int32 inputIndex,
						 const rspfConnectableObject* object)const
{
  
  return ((PTR_CAST(rspfImageSource, object)||!object) && ( inputIndex == 0 ) );
}

void rspfImageSourceFilter::connectInputEvent(rspfConnectionEvent& /* event */)
{
  if(traceDebug())
    {
       rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageSourceFilter::connectInputEvent: ";

       if(getInput())
       {
          rspfNotify(rspfNotifyLevel_DEBUG) << "to" <<  getInput()->getClassName() << std::endl;
       }
       else
       {
          rspfNotify(rspfNotifyLevel_DEBUG) << "to NULL" << std::endl;
       }
    }
  theInputConnection = PTR_CAST(rspfImageSource, getInput(0));
  initialize();
  if(traceDebug())
  {
     if(theInputConnection)
     {
        rspfNotify(rspfNotifyLevel_DEBUG) << "theInputConnection was set to " << getInput()->getClassName() << std::endl;
     }
     else
     {
        rspfNotify(rspfNotifyLevel_DEBUG) << "theInputConnection was set to NULL" <<  std::endl;
     }
     rspfNotify(rspfNotifyLevel_DEBUG) << "Leaving rspfImageSourceFilter::connectInput" << std::endl;
  }
}
void rspfImageSourceFilter::disconnectInputEvent(rspfConnectionEvent& /* event */)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageSourceFilter::disconnectInputEvent" << std::endl;
   }
   theInputConnection = PTR_CAST(rspfImageSource, getInput(0));
   initialize();
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "Leaving rspfImageSourceFilter::disconnectInput" << std::endl;
   }
}
void rspfImageSourceFilter::propertyEvent(rspfPropertyEvent& /* event */)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageSourceFilter::propertyEvent DEBUG: Entering..." << std::endl;
   }
   initialize();
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageSourceFilter::propertyEvent DEBUG: Leaving..." << std::endl;
   }
}

void rspfImageSourceFilter::refreshEvent(rspfRefreshEvent& /* event */)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageSourceFilter::refreshEvent " << std::endl;
   }
   initialize();
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfImageSourceFilter::refreshEvent " << std::endl;
   }
}

void rspfImageSourceFilter::getOutputBandList(std::vector<rspf_uint32>& bandList) const
{
   if(theInputConnection)
   {
      return theInputConnection->getOutputBandList(bandList);
   }

   return rspfImageSource::getOutputBandList(bandList);
}

void rspfImageSourceFilter::setProperty(rspfRefPtr<rspfProperty> property)
{
   rspfImageSource::setProperty(property);
}

rspfRefPtr<rspfProperty> rspfImageSourceFilter::getProperty(const rspfString& name)const
{
   return rspfImageSource::getProperty(name);
   
}

void rspfImageSourceFilter::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   rspfImageSource::getPropertyNames(propertyNames);
}
