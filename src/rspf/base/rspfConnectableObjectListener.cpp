//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfConnectableObjectListener.cpp 19961 2011-08-16 18:10:36Z gpotts $

#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfNotifyContext.h>

RTTI_DEF1(rspfConnectableObjectListener,
          "rspfConnectableObjectListener",
          rspfListener);

void rspfConnectableObjectListener::processEvent(rspfEvent& event)
{
   switch(event.getId())
   {
      case RSPF_EVENT_OBJECT_DESTRUCTING_ID:
      {
         rspfObjectDestructingEvent* eventCast = dynamic_cast<rspfObjectDestructingEvent*>(&event);
         if(eventCast) objectDestructingEvent(*eventCast);
         
         break;
      }
      case RSPF_EVENT_CONNECTION_CONNECT_ID:
      case RSPF_EVENT_CONNECTION_DISCONNECT_ID:
      {
         rspfConnectionEvent* eventCast = dynamic_cast<rspfConnectionEvent*>(&event);
         
         if(eventCast)
         {
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
                  rspfNotify(rspfNotifyLevel_WARN) << "rspfConnectableObjectListener::processEvent, Direction not set\n";
               }
            }
            else
            {
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
                  rspfNotify(rspfNotifyLevel_WARN) << "rspfConnectableObjectListener::processEvent, Direction not set\n";
               }
            }
         }
         break;
      }
      case RSPF_EVENT_PROPERTY_ID:
      {
         rspfPropertyEvent* eventCast = dynamic_cast<rspfPropertyEvent*>(&event);
         if(eventCast) propertyEvent(*eventCast);
         break;
      }
      case RSPF_EVENT_ADD_OBJECT_ID:
      {
         rspfContainerEvent* eventCast = dynamic_cast<rspfContainerEvent*>(&event);
         if(eventCast)
         {
            containerEvent(*eventCast);
            addObjectEvent(*eventCast);
         }
         break;
      }
      case RSPF_EVENT_REMOVE_OBJECT_ID:
      {
         rspfContainerEvent* eventCast = dynamic_cast<rspfContainerEvent*>(&event);
         if(eventCast)
         {
            containerEvent(*eventCast);
            removeObjectEvent(*eventCast);
         }
         break;
      }
      case RSPF_EVENT_REFRESH_ID:
      {
         rspfRefreshEvent* eventCast = dynamic_cast<rspfRefreshEvent*>(&event);
         if(eventCast) refreshEvent(*eventCast);
         break;
      }
      default:
      {
         rspfListener::processEvent(event);
         break;
      }
   }
}
