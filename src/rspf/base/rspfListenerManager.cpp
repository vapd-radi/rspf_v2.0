//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See LICENSE.txt file in the top level directory.
//
// Author: Garrett Potts
// 
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfListenerManager.cpp 17193 2010-04-23 14:59:44Z dburken $

#include <algorithm>
#include <rspf/base/rspfListenerManager.h>
#include <rspf/base/rspfEvent.h>
#include <rspf/base/rspfListener.h>

RTTI_DEF(rspfListenerManager, "rspfListenerManager");

rspfListenerManager::rspfListenerManager()
{
   theFireEventFlag = false;
}

rspfListenerManager::rspfListenerManager(const rspfListenerManager& /* rhs */)
{
//    replaceListeners(rhs.getListeners());
}

rspfListenerManager::~rspfListenerManager()
{
   theListenerList.clear();
}

void rspfListenerManager::fireEvent(rspfEvent& event)
{
   // only process the event if it has not been consumed.
   event.setCurrentObject(PTR_CAST(rspfObject, this));
   if(event.isConsumed())
   {
      return;
   }
   theFireEventFlag = true;

   std::list<rspfListener*>::iterator currentIterator = theListenerList.begin();
   
   while(currentIterator != theListenerList.end())
   {
      // only fire if the event is not consumed
      if(!event.isConsumed())
      {
         if(*currentIterator)
         {
            if(theDelayedRemove.end()==std::find(theDelayedRemove.begin(),
                                                 theDelayedRemove.end(),
                                                 (*currentIterator)))
            {
               if((*currentIterator)->isListenerEnabled())
               {
                  (*currentIterator)->processEvent(event);
               }
            }
         }
      }
      else
      {
         // the event is now consumed so stop propagating.
         //
         theFireEventFlag = false;
         break;
      }
      ++currentIterator;
   }
   
   theFireEventFlag = false;
   
   if(theDelayedAdd.size())
   {
      for(std::list<rspfListener*>::iterator current = theDelayedAdd.begin();
          current != theDelayedAdd.end();++current)
      {
         addListener(*current);
      }
      theDelayedAdd.clear();
   }
   
   if(theDelayedRemove.size())
   {
      for(std::list<rspfListener*>::iterator current = theDelayedRemove.begin();
          current != theDelayedRemove.end();++current)
      {
         removeListener(*current);
      }
      theDelayedRemove.clear();
   }
}

bool rspfListenerManager::addListener(rspfListener* listener)
{
   if(theFireEventFlag)
   {
      theDelayedAdd.push_back(listener);
   }
   else
   {
      
      if(!findListener(listener))
      {
         theListenerList.push_back(listener);
      }
   }
   
   return true;
}

bool rspfListenerManager::removeListener(rspfListener* listener)
{
   
   if(theFireEventFlag)
   {
      theDelayedRemove.push_back(listener);
      return true;
   }
   
   std::list<rspfListener*>::iterator current=theListenerList.begin();
   while(current!=theListenerList.end())
   {
      if( (*current) == listener)
      {
         current = theListenerList.erase(current);
         break; // Should only be in list once... (drb)
      }
      else
      {
         ++current;
      }
   }
   
   return true;
}

bool rspfListenerManager::findListener(rspfListener* listener)
{
   bool result = false;

   if(listener)
   {
      std::list<rspfListener*>::iterator currentIter =
         std::find(theListenerList.begin(),
                   theListenerList.end(),
                   listener);
      result = (currentIter != theListenerList.end());
   }
   
   return result;
}
