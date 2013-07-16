//----------------------------------------------------------------------------
// Copyright (c) 2004, David Burken, all rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: rspfConnectableDisplayListener
// 
// This class is intended for connectable displays that wants to be a
// rspfConnectableObjectListener and a rspfDisplayEventListener to avoid
// a triangle inheritance.
//
//----------------------------------------------------------------------------
// $Id: rspfConnectableDisplayListener.h 9094 2006-06-13 19:12:40Z dburken $

#ifndef rspfConnectableDisplayListener_HEADER
#define rspfConnectableDisplayListener_HEADER

#include <rspf/base/rspfListener.h>

class rspfConnectionEvent;
class rspfObjectDestructingEvent;
class rspfPropertyEvent;
class rspfContainerEvent;
class orspfDisplayListEvent;
class rspfRefreshEvent;
class rspfDisplayListEvent;
class rspfDisplayRefreshEvent;

/**
 * This class is intended for connectable displays that wants to be a
 * rspfConnectableObjectListener and a rspfDisplayEventListener to avoid
 * a triangle inheritance.
 */
class RSPFDLLEXPORT rspfConnectableDisplayListener : public rspfListener
{
public:

   /** default constructor */
   rspfConnectableDisplayListener();

   /**
    * Method called by listener managers.  This will in turn call the correct
    * xxxEvent method.
    *
    * @param event Event to handle.
    */
   virtual void processEvent(rspfEvent& event);

   /** @param event Event to handle signifying an object destroying. */
   virtual void objectDestructingEvent(rspfObjectDestructingEvent& event);

   /** @param event Event to handle signifying an object connecting. */
   virtual void connectionEvent(rspfConnectionEvent& event);

   /** @param event Event to handle signifying a disconnect input event. */
   virtual void disconnectInputEvent(rspfConnectionEvent& event);

   /** @param event Event to handle signifying an output disconnect event. */
   virtual void disconnectOutputEvent(rspfConnectionEvent& event);

   /** @param event Event to handle signifying a connect input event. */
   virtual void connectInputEvent(rspfConnectionEvent& event);

   /** @param event Event to handle signifying a connect  output event. */
   virtual void connectOutputEvent(rspfConnectionEvent& event);

   /**
    * @param event Event to handle signifying a property has changed  event.
    */
   virtual void propertyEvent(rspfPropertyEvent& event);

   /**
    * @param event Event to handle signifying a dislay list  has changed
    * event.
    */
   virtual void displayListEvent(rspfDisplayListEvent&);

   /**
    * @param event Event to handle signifying a dislay refresh
    * is needed.
    */
   virtual void displayRefreshEvent(rspfDisplayRefreshEvent&);

   /**
    * @param event Event to handle signifying a refresh is needed.
    */
   virtual void refreshEvent(rspfRefreshEvent& event);

   /**
    * Container Events:
    * Typically isued by objects that contain children.  If anyone is
    * interested, can latch on to this event.  Other objects within the
    * system might be interest in this event even 
    */

   /** @param event Event signifying an object was added to a container. */
   virtual void addObjectEvent(rspfContainerEvent& event);

   /** @param event Event signifying an object was removed from a container. */
   virtual void removeObjectEvent(rspfContainerEvent& event);

TYPE_DATA
};

#endif
