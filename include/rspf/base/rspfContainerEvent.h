//*******************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfContainerEvent.h 19966 2011-08-16 18:12:56Z gpotts $
#ifndef rspfContainerEvent_HEADER
#define rspfContainerEvent_HEADER
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfRefPtr.h>
#include <vector>

class RSPFDLLEXPORT rspfContainerEvent : public rspfEvent
{
public:

   typedef std::vector<rspfRefPtr<rspfObject> > ObjectList;
   
   /** constuctor */
   rspfContainerEvent(rspfObject* obj1,// the object firing the event.
                       long id);

   void setObjectList(rspfObject* obj);
   void setObjectList(ObjectList& objects);
   ObjectList& getObjectList(){return m_objectList;}
   const ObjectList& getObjectList()const{return m_objectList;}
   
protected:
   
   /*!
    * This is the object associated with the container event.
    * if this was an add then it s the object added to the
    * container.
    */
      ObjectList m_objectList;

TYPE_DATA
};

#endif /* end of: "#ifndef rspfContainerEvent_HEADER" */
