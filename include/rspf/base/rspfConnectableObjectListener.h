//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfConnectableObjectListener.h 19962 2011-08-16 18:10:52Z gpotts $
#ifndef rspfConnectableObjectListener_HEADER
#define rspfConnectableObjectListener_HEADER

#include <rspf/base/rspfListener.h>
#include <rspf/base/rspfConnectionEvent.h>
#include <rspf/base/rspfObjectDestructingEvent.h>
#include <rspf/base/rspfPropertyEvent.h>
#include <rspf/base/rspfContainerEvent.h>
#include <rspf/base/rspfRefreshEvent.h>

class RSPFDLLEXPORT rspfConnectableObjectListener : public rspfListener
{
public:
   rspfConnectableObjectListener():rspfListener(){}
   virtual ~rspfConnectableObjectListener(){}
   
   virtual void processEvent(rspfEvent& event);
   
   virtual void objectDestructingEvent(rspfObjectDestructingEvent& /*event*/)
   {}
   virtual void connectionEvent(rspfConnectionEvent& /* event */)
   {}
   virtual void disconnectInputEvent(rspfConnectionEvent& /* event */)
   {}
   virtual void disconnectOutputEvent(rspfConnectionEvent& /* event */)
   {}
   virtual void connectInputEvent(rspfConnectionEvent& /* event */)
   {}
   virtual void connectOutputEvent(rspfConnectionEvent& /* event */)
   {}
   
   virtual void propertyEvent(rspfPropertyEvent& /* event */)
   {}
   
   /*!
    * Typically isued by objects that contain children.  If anyone is
    * interested, can latch on to this event.  Other objects within the
    * system might be interest in this event even 
    */
   virtual void addObjectEvent(rspfContainerEvent& /* event */)
   {}
   
   virtual void removeObjectEvent(rspfContainerEvent& /* event */)
   {}
   
   virtual void containerEvent(rspfContainerEvent& /* event */)
   {}
   
   virtual void refreshEvent(rspfRefreshEvent& /* event */)
   {}
   TYPE_DATA
};

#endif
