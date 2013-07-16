//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks)
// Description: A brief description of the contents of the file.
//
//
//*************************************************************************
// $Id: rspfListenerManager.h 14789 2009-06-29 16:48:14Z dburken $
#ifndef rspfListenerManager_HEADER
#define rspfListenerManager_HEADER
#include <rspf/base/rspfConstants.h>
#include <list>
#include <rspf/base/rspfRtti.h>

class rspfListener;
class rspfEvent;

class RSPFDLLEXPORT rspfListenerManager
{
public:
   /*!
    * Default constructor and needs no initialization.
    */
   rspfListenerManager();

   /*!
    * Default destructor.  Note the Manager doesn't own the listeners
    * and will not delete them
    */
   virtual ~rspfListenerManager();

   /*!
    * Traverses through all listeners and fires an event to them.
    * if any listener sets the consumed flag on the event it
    * will stop traversing the list.
    */
   virtual void fireEvent(rspfEvent& event);
   
   /*!
    * Appends the listener onto the list.
    */
   virtual bool addListener(rspfListener* listener);

   /*!
    * Will push the lister to the front of the list.
    */
  //   virtual bool insertFrontListener(rspfListener* listener);

   /*!
    * Finds and removes the listener.
    */
   virtual bool removeListener(rspfListener* listener);

   
   /*!
    * Will find the listener passed in as the first argument
    * and insert the passed in listener before it.  If not
    * found then it will default to a push to front.
    */
//    virtual bool insertBeforeListener(rspfListener* listenerSearchKey,
//                                      rspfListener* listener);

   /*!
    * Will find the listener passed in as the first argument
    * and insert the passed in listener after it.  If not
    * found then it will default to an append.
    */
//    virtual bool insertAfterListener(rspfListener* listenerSearchKey,
//                                     rspfListener* listener);
   
   /*!
    * Will insert the listener before the specified
    * listener.  If the index is invalid it defaults
    * to a push to front.  An invalid index is any
    * index < 0 or larger than the number of items
    * currently in the list.
    */
//    virtual bool insertBeforeListener(rspfListener* listener,
//                                      long index);

   /*!
    * Will insert the listener after the specified
    * listener.  If the index is invalid it defaults
    * to an addListener.  An invalid index is any
    * index < 0 or larger than the number of items
    * currently in the list.
    */
//    virtual bool insertAfterListener(rspfListener* listener,
//                                     long index);

   /*!
    * Searches the list and sees if a listener is found
    */
  virtual bool findListener(rspfListener* listener);
  
   /*!
    * Will search the list and return the index if found else
    * returns a negative value.
    */
  //   virtual long getIndexOfListener(rspfListener* listener);

  //   const std::vector<rspfListener*>& getListeners()const{return theListenerList;}

   
protected:
  rspfListenerManager(const rspfListenerManager& rhs);
  
  /*!
   *  Holds the list of listeners.
   */
  std::list<rspfListener*> theListenerList;
  bool theEnabledFlag;
  mutable std::list<rspfListener*> theDelayedAdd;
  mutable std::list<rspfListener*> theDelayedRemove;
  mutable bool theFireEventFlag;
//  mutable std::list<rspfListener*>::iterator theCurrentIterator;
  
  
TYPE_DATA
};

#endif
