//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfEvent.h 13362 2008-08-01 14:02:32Z gpotts $
#ifndef rspfEvent_HEADER
#define rspfEvent_HEADER
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfObject.h>

class RSPFDLLEXPORT rspfEvent : public rspfObject
{
public:
   enum PropagationType
   {
      PROPAGATION_NONE = 0,
      PROPAGATION_INPUT = 1,
      PROPAGATION_OUTPUT = 2
   };
   /**
    * @param object The object associated with the event if any.
    *
    * @param id The event id.
    */
   rspfEvent(rspfObject* object=NULL,
              long id=RSPF_EVENT_NULL_ID);

   rspfEvent(const rspfEvent& rhs);

   long getId()const;
   void setId(long id);

   bool isConsumed()const;
   void setConsumedFlag(bool flag=true);
   void consume();

   /**
    * This is the originating object that originally 
    * produced the event.
    */
   const rspfObject* getObject()const;
   rspfObject*       getObject();
   const rspfObject* getCurrentObject()const;
   rspfObject*       getCurrentObject();

   void setObject(rspfObject* object);
   void setCurrentObject(rspfObject* object);

   void setPropagationType(PropagationType type);
   bool isPropagatingToOutputs()const;
   bool isPropagatingToInputs()const;
protected:
   rspfObject* theObject;
   rspfObject* theCurrentObject;
   long         theId;
   bool         theIsConsumedFlag;
   PropagationType thePropagationType;

TYPE_DATA
};

#endif /* #ifndef rspfEvent_HEADER */
