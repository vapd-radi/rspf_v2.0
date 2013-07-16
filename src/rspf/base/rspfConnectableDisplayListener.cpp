
//----------------------------------------------------------------------------
// Copyright (c) 2004, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: 
//
//----------------------------------------------------------------------------
// $Id: rspfConnectableDisplayListener.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <rspf/base/rspfConnectableDisplayListener.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfDisplayListEvent.h>
#include <rspf/base/rspfDisplayRefreshEvent.h>

#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF1(rspfConnectableDisplayListener,
          "rspfConnectableDisplayListener",
          rspfListener);

rspfConnectableDisplayListener::rspfConnectableDisplayListener()
   : rspfListener()
{}

void rspfConnectableDisplayListener::processEvent(rspfEvent& event)
{
   switch(event.getId())
   {
      case RSPF_EVENT_OBJECT_DESTRUCTING_ID:
      {
         rspfObjectDestructingEvent* eventCast =
            static_cast<rspfObjectDestructingEvent*>(&event);
         objectDestructingEvent(*eventCast);
         
         break;
      }
      case RSPF_EVENT_CONNECTION_CONNECT_ID:
      case RSPF_EVENT_CONNECTION_DISCONNECT_ID:
      {
         rspfConnectionEvent* eventCast =
            static_cast<rspfConnectionEvent*>(&event);
         
         connectionEvent(*eventCast);
         if(event.getId() == RSPF_EVENT_CONNECTION_DISCONNECT_ID)
         {
            if(eventCast->isInputDirection())
            {
               disconnectInputEvent(*eventCast);
            }
            else if(eventCast->isOutputDirection())
            {
               disconnectOutputEvent(*eventCast);
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfConnectableDisplayListener::processEvent, Direction not set\n";
            }
         }
         else
         {
            rspfConnectionEvent* eventCast = static_cast<rspfConnectionEvent*>(&event);
            
            if(eventCast->isInputDirection())
            {
               connectInputEvent(*eventCast);
            }
            else if(eventCast->isOutputDirection())
            {
               connectOutputEvent(*eventCast);
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN) << "rspfConnectableDisplayListener::processEvent, Direction not set\n";
            }
         }
         break;
      }
      case RSPF_EVENT_PROPERTY_ID:
      {
         rspfPropertyEvent* eventCast = static_cast<rspfPropertyEvent*>(&event);
         propertyEvent(*eventCast);
         break;
      }
      case RSPF_EVENT_ADD_OBJECT_ID:
      {
         rspfContainerEvent* eventCast = static_cast<rspfContainerEvent*>(&event);
         addObjectEvent(*eventCast);
         break;
      }
      case RSPF_EVENT_REMOVE_OBJECT_ID:
      {
         rspfContainerEvent* eventCast = static_cast<rspfContainerEvent*>(&event);
         removeObjectEvent(*eventCast);
         break;
      }
      case RSPF_EVENT_REFRESH_ID:
      {
         rspfRefreshEvent* eventCast = static_cast<rspfRefreshEvent*>(&event);
         refreshEvent(*eventCast);
         break;
      }
      case RSPF_EVENT_DISPLAY_LIST_ID:
      {
         rspfDisplayListEvent* eventCast =
            static_cast<rspfDisplayListEvent*>(&event);
         displayListEvent(*eventCast);
         break;
      }
      case RSPF_EVENT_DISPLAY_REFRESH_ID:
      {
         rspfDisplayRefreshEvent* eventCast =
            static_cast<rspfDisplayRefreshEvent*>(&event);
         displayRefreshEvent(*eventCast);
         break;
      }
      default:
      {
         rspfListener::processEvent(event);
         break;
      }
   }
}

void rspfConnectableDisplayListener::objectDestructingEvent(
   rspfObjectDestructingEvent& /*event*/)
{
}

void rspfConnectableDisplayListener::connectionEvent(
   rspfConnectionEvent& /* event */)
{}

void rspfConnectableDisplayListener::disconnectInputEvent(
   rspfConnectionEvent& /* event */)
{}

void rspfConnectableDisplayListener::disconnectOutputEvent(
   rspfConnectionEvent& /* event */)
{}

void rspfConnectableDisplayListener::connectInputEvent(
   rspfConnectionEvent& /* event */)
{}

void rspfConnectableDisplayListener::connectOutputEvent(
   rspfConnectionEvent& /* event */)
{}

void rspfConnectableDisplayListener::propertyEvent(
   rspfPropertyEvent& /* event */)
{}

void rspfConnectableDisplayListener::displayListEvent(rspfDisplayListEvent&)
{}

void rspfConnectableDisplayListener::displayRefreshEvent(
   rspfDisplayRefreshEvent&)
{}

void rspfConnectableDisplayListener::addObjectEvent(
   rspfContainerEvent& /* event */)
{}

void rspfConnectableDisplayListener::removeObjectEvent(
   rspfContainerEvent& /* event */)
{}

void rspfConnectableDisplayListener::refreshEvent(
   rspfRefreshEvent& /* event */)
{}

