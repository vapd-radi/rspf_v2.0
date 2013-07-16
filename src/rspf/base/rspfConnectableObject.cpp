//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfConnectableObject.cpp 21850 2012-10-21 20:09:55Z dburken $

#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfIdManager.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfConnectableContainerInterface.h>
#include <rspf/base/rspfConnectableContainer.h>
#include <rspf/base/rspfTextProperty.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfVisitor.h>
#include <algorithm>

RTTI_DEF3(rspfConnectableObject,
          "rspfConnectableObject",
          rspfObject,
          rspfListenerManager,
          rspfPropertyInterface);

const char* CONNECTABLE_INPUT_LIST_FIXED_KW  = "input_list_fixed";
const char* CONNECTABLE_OUTPUT_LIST_FIXED_KW = "output_list_fixed";

rspfConnectableObject::rspfConnectableObject(rspfObject* owner)
:rspfObject(),
rspfListenerManager(),
theInputListIsFixedFlag(false),
theOutputListIsFixedFlag(false)
{
   theId    = rspfIdManager::instance()->generateId();
   theOwner = owner;
}

rspfConnectableObject::rspfConnectableObject(rspfObject* owner,
                                               rspf_int32 inputListSize,
                                               rspf_int32 outputListSize,
                                               bool inputListIsFixedFlag,
                                               bool outputListIsFixedFlag)
:rspfObject(),
rspfListenerManager(),
theInputListIsFixedFlag(inputListIsFixedFlag),
theOutputListIsFixedFlag(outputListIsFixedFlag)
{
   theId    = rspfIdManager::instance()->generateId();
   theOwner = owner;
   
   setNumberOfInputs(inputListSize);
   setNumberOfOutputs(outputListSize);
}

rspfConnectableObject::~rspfConnectableObject()
{
   
   // tell the immediate listeners that we are destructing.
   rspfObjectDestructingEvent event(this);
   
   //    if(theOwner)
   //    {
   //       rspfListenerManager* manager = PTR_CAST(rspfListenerManager, theOwner);
   
   //       if(manager)
   //       {
   //          // notify the owner that you are destructing
   //          //
   //          manager->fireEvent(event);
   //       }
   //    }
   
   // notify all other listeners that you are destructing
   //
   fireEvent(event);
   
 }

void rspfConnectableObject::changeOwner(rspfObject* owner)
{
   theOwner = owner;
}

void rspfConnectableObject::setDescription(const rspfString& description)
{
   theDescription = description;
}

rspfString rspfConnectableObject::getDescription()const
{
   return theDescription;
}

bool rspfConnectableObject::isConnected(rspfConnectableObjectDirectionType direction)const
{
   if(direction & CONNECTABLE_DIRECTION_INPUT)
   {
      if(getNumberOfInputs())
      {
         ConnectableObjectList::const_iterator current = theInputObjectList.begin();
         
         while(current != theInputObjectList.end())
         {
            if(! (*current))
            {
               return false;
            }
            
            ++current;
         }
      }
      else if(!theInputListIsFixedFlag)
      {
         return false;
      }
   }
   
   if(direction & CONNECTABLE_DIRECTION_OUTPUT)
   {
      if(theOutputObjectList.empty())
      {
         return theOutputListIsFixedFlag;
         
      }
      ConnectableObjectList::const_iterator current = theOutputObjectList.begin();
      current = theOutputObjectList.begin();
      while(current != theOutputObjectList.end())
      {
         if(! (*current))
         {
            return false;
         }
         
         ++current;
      }
   }
   
   return true;
}

rspfConnectableObject* rspfConnectableObject::findConnectableObject(const rspfId& id)
{
   ConnectableObjectList::iterator current;
   
   current = theInputObjectList.begin();
   while(current != theInputObjectList.end())
   {
      rspfConnectableObject* temp = (*current).get();
      
      if(temp->getId() == id)
      {
         return temp;
      }
      
      ++current;
   }
   
   // go through the outputs
   current = theOutputObjectList.begin();
   while(current != theOutputObjectList.end())
   {
      rspfConnectableObject* temp = (*current).get();
      
      if(temp->getId() == id)
      {
         return temp;
      }
      
      ++current;
   }
   
   return 0;
}

rspfConnectableObject* rspfConnectableObject::findObjectOfType(
   RTTItypeid typeId,
   rspfConnectableObjectDirectionType directionType,
   bool recurse)
{
   rspfConnectableObject* result = 0;

   if(directionType != CONNECTABLE_DIRECTION_NONE)
   {
      ConnectableObjectList* connectableList = 0;

      if ( CONNECTABLE_DIRECTION_INPUT )
      {
         connectableList = &theInputObjectList;
      }
      else // (directionType == CONNECTABLE_DIRECTION_OUTPUT)
      {
         connectableList = &theOutputObjectList;
      }

      // see if it is in the immediate list
      for(rspf_uint32 index = 0; index < connectableList->size(); ++index)
      {
         if( (*connectableList)[index].valid() )
         {
            if( (*connectableList)[index]->canCastTo( typeId ) )
            {
               result = (*connectableList)[index].get();
               break;
            }
         }
      }

      if ( !result )
      {
         rspfVisitor::VisitorType vType = rspfVisitor::VISIT_NONE;
         
         if ( CONNECTABLE_DIRECTION_INPUT )
         {
            if ( recurse )
            {
               // Cast needed for compiler...
               vType = (rspfVisitor::VisitorType)
                  (rspfVisitor::VISIT_INPUTS|rspfVisitor::VISIT_CHILDREN);
            }
            else
            {
               vType = rspfVisitor::VISIT_INPUTS;
            }
         }
         else // (directionType == CONNECTABLE_DIRECTION_OUTPUT)
         {
            if ( recurse )
            {
               // Cast needed for compiler...
               vType = (rspfVisitor::VisitorType)
                  (rspfVisitor::VISIT_OUTPUTS|rspfVisitor::VISIT_CHILDREN);
            }
            else
            {
               vType = rspfVisitor::VISIT_OUTPUTS;
            }
         }
         
         rspfTypeIdVisitor visitor( typeId,
                                     true, // firstofTypeFlag
                                     vType );


         this->accept( visitor );
         result = dynamic_cast<rspfConnectableObject*>( visitor.getObject(0) );
      }
      
   } // Matches: if(directionType != CONNECTABLE_DIRECTION_NONE)
   
   return result;
   
} // End: findObjectOfType( RTTItypeid ...

rspfConnectableObject* rspfConnectableObject::findObjectOfType(
   const rspfString& className,
   rspfConnectableObjectDirectionType directionType,
   bool recurse )
{
   rspfConnectableObject* result = 0;
   
   if(directionType != CONNECTABLE_DIRECTION_NONE)
   {
      ConnectableObjectList* connectableList = 0;
      
      if ( CONNECTABLE_DIRECTION_INPUT )
      {
         connectableList = &theInputObjectList;
      }
      else // (directionType == CONNECTABLE_DIRECTION_OUTPUT)
      {
         connectableList = &theOutputObjectList;
      }
      
      // see if it is in the immediate list
      for(rspf_uint32 index = 0; index < connectableList->size(); ++index)
      {
         if( (*connectableList)[index].valid() )
         {
            if( (*connectableList)[index]->canCastTo( className ) )
            {
               result = (*connectableList)[index].get();
               break;
            }
         }
      }
      
      if ( !result )
      {
         rspfVisitor::VisitorType vType = rspfVisitor::VISIT_NONE;
         
         if ( CONNECTABLE_DIRECTION_INPUT )
         {
            if ( recurse )
            {
               // Cast needed for compiler...
               vType = (rspfVisitor::VisitorType)
                  (rspfVisitor::VISIT_INPUTS|rspfVisitor::VISIT_CHILDREN);
            }
            else
            {
               vType = rspfVisitor::VISIT_INPUTS;
            }
         }
         else // (directionType == CONNECTABLE_DIRECTION_OUTPUT)
         {
            if ( recurse )
            {
               // Cast needed for compiler...
               vType = (rspfVisitor::VisitorType)
                  (rspfVisitor::VISIT_OUTPUTS|rspfVisitor::VISIT_CHILDREN);
            }
            else
            {
               vType = rspfVisitor::VISIT_OUTPUTS;
            }
         }
         
         rspfTypeNameVisitor visitor( className,
                                       true, // firstofTypeFlag
                                       vType );
         this->accept( visitor );
         result = dynamic_cast<rspfConnectableObject*>( visitor.getObject(0) );
      }
      
   } // Matches: if(directionType != CONNECTABLE_DIRECTION_NONE)
   
   return result;
   
} // End: findObjectOfType( const rspfString& className ...

rspfConnectableObject* rspfConnectableObject::findInputObjectOfType(
   const rspfString& className)
{
   rspfConnectableObject* result = 0;
   
   // See if we are of class type.
   if ( canCastTo( className ) )
   {
      result = this;
   }

   if ( !result )
   {
      ConnectableObjectList* connectableList = &theInputObjectList;

      // see if it is in the immediate list
      for(rspf_uint32 index = 0; index < connectableList->size(); ++index)
      {
         if( (*connectableList)[index].valid() )
         {
            if( (*connectableList)[index]->canCastTo( className ) )
            {
               result = (*connectableList)[index].get();
               break;
            }
         }
      }
      
      if ( !result )
      {
         rspfTypeNameVisitor visitor( className,
                                       true, // firstofTypeFlag
                                       (rspfVisitor::VISIT_INPUTS|
                                        rspfVisitor::VISIT_CHILDREN) );
         this->accept( visitor );
         result = dynamic_cast<rspfConnectableObject*>( visitor.getObject(0) );
      }
   }
   
   return result;
   
} // End: findInputObjectOfType( const rspfString& className )

// Old findObject findInputObject code kept here until debugged to satisfaction.
#if 0 /* drb */

rspfConnectableObject* rspfConnectableObject::findObjectOfType(RTTItypeid typeId,
                                                                 rspfConnectableObjectDirectionType directionType,
                                                                 bool recurse)
{
   ConnectableObjectList* connectableList = &theInputObjectList;
   
   
   if(directionType == CONNECTABLE_DIRECTION_NONE)
   {
      return 0;
   }
   
   if(directionType == CONNECTABLE_DIRECTION_OUTPUT)
   {
      connectableList = &theOutputObjectList;
   }
   // see if it is in the immediate list
   for(rspf_uint32 index = 0; index < connectableList->size(); ++index)
   {
      if((*connectableList)[index].valid())
      {
         if((*connectableList)[index]->canCastTo(typeId))//typeId.can_cast(TYPE_INFO(theInputObjectList[index])))
         {
            return (*connectableList)[index].get();
         }
      }
   }
   
   if(recurse)
   {
      rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                           this);
      if(inter)
      {
         rspfConnectableObject* tempObj = inter->findFirstObjectOfType(typeId);
         if(tempObj)
         {
            return tempObj;
         }
      }
      
      for(rspf_uint32 index = 0; index < connectableList->size(); ++index)
      {
         inter = PTR_CAST(rspfConnectableContainerInterface,
                          (*connectableList)[index].get());
         if(inter)
         {
            rspfConnectableObject* tempObj = inter->findFirstObjectOfType(typeId);
            if(tempObj)
            {
               return tempObj;
            }
         }
         if((*connectableList)[index].valid())
         {
            rspfConnectableObject* result = (*connectableList)[index]->findObjectOfType(typeId,
                                                                                         directionType,
                                                                                         recurse);
            if(result)
            {
               return result;
            }
         }
      }
   }
   
   rspfConnectableObject* result = 0;
   for(rspf_uint32 index = 0; (index < connectableList->size())&&!result; ++index)
   {
      if((*connectableList)[index].valid())
      {
         result = ((*connectableList)[index])->findObjectOfType(typeId,
                                                                directionType,
                                                                recurse);
      }
   }
   
   return result;
}

rspfConnectableObject* rspfConnectableObject::findObjectOfType(
                                                                 const rspfString& className,
                                                                 rspfConnectableObjectDirectionType directionType,
                                                                 bool recurse)
{
   ConnectableObjectList *connectableList = &theInputObjectList;
   
   if(directionType == CONNECTABLE_DIRECTION_NONE)
   {
      return 0;
   }
   
   if(directionType == CONNECTABLE_DIRECTION_OUTPUT)
   {
      connectableList = &theOutputObjectList;
      
   }
   
   // see if it is in the immediate list
   for(rspf_uint32 index = 0; index < connectableList->size(); ++index)
   {
      if((*connectableList)[index].valid())
      {
         if((*connectableList)[index]->canCastTo(className))//typeId.can_cast(TYPE_INFO(theInputObjectList[index])))
         {
            return (*connectableList)[index].get();
         }
      }
   }
   
   if(recurse)
   {
      rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                           this);
      if(inter)
      {
         rspfConnectableObject* tempObj = inter->findFirstObjectOfType(className);
         if(tempObj)
         {
            return tempObj;
         }
      }
      for(rspf_uint32 index = 0; index < connectableList->size(); ++index)
      {
         inter = PTR_CAST(rspfConnectableContainerInterface,
                          (*connectableList)[index].get());
         if(inter)
         {
            rspfConnectableObject* tempObj = inter->findFirstObjectOfType(className);
            if(tempObj)
            {
               return tempObj;
            }
         }
         if((*connectableList)[index].valid())
         {
            rspfConnectableObject* result = (*connectableList)[index]->findObjectOfType(className,
                                                                                         directionType,
                                                                                         recurse);
            if(result)
            {
               return result;
            }
         }
      }
   }
   rspfConnectableObject* result = 0;
   for(rspf_uint32 index = 0; (index < connectableList->size())&&!result; ++index)
   {
      if((*connectableList)[index].valid())
      {
         result = ((*connectableList)[index])->findObjectOfType(className,
                                                                directionType,
                                                                recurse);
      }
   }
   
   return result;
}

rspfConnectableObject* rspfConnectableObject::findInputObjectOfType(
    
                                                                      const rspfString& className)
{
   // See if we are of class type.
   if (canCastTo(className))
   {
      return this;
   }
   rspfConnectableObject* result = 0;
   // If we are a container, look inside for type.
   rspfConnectableContainerInterface* container =
   PTR_CAST(rspfConnectableContainerInterface, this);
   if (container)
   {
      const rspf_uint32 NUMBER_OF_OBJECTS =
      container->getNumberOfObjects(false);
      if (NUMBER_OF_OBJECTS)
      {
         for (rspf_uint32 idx = NUMBER_OF_OBJECTS; idx > 0; --idx) 
         {
            result =
            container->getConnectableObject(idx-1);
            if (result)
            {
               if (result->canCastTo(className))
               {
                  return result;
               }
               else 
               {
                  result = result->findInputObjectOfType(className);
                  if(result)
                  {
                     return result;
                  }
               }
            }
         }
      }
   }
  
   rspf_uint32 inputs = getNumberOfInputs();
   rspf_uint32 inputIdx = 0;
   for(inputIdx = 0; inputIdx < inputs; ++inputIdx)
   {
      rspfConnectableObject* input = getInput(inputIdx);
      if(input)
      {
         result = input->findInputObjectOfType(className);
         if(result)
         {
            return result;
         }
      }
   }
   return result;
}
#endif /* drb */
// End: Old findObject findInputObject code kept here until debugged to satisfaction.

rspf_int32 rspfConnectableObject::findInputIndex(const rspfConnectableObject* object)
{
   rspf_int32 i = 0;
   for(i = 0; i < (rspf_int32)theInputObjectList.size(); ++i)
   {
      if(theInputObjectList[i].get() == object)
      {
         return i;
      }
   }
   
   return -1;
}

rspf_int32 rspfConnectableObject::findInputIndex(const rspfId& id)
{
   rspf_int32 i = 0;
   for(i = 0; i < (rspf_int32)theInputObjectList.size(); ++i)
   {
      if(theInputObjectList[i].valid() &&
         (theInputObjectList[i]->getId() == id))
      {
         return i;
      }
   }
   
   return -1;
}

rspf_int32 rspfConnectableObject::findOutputIndex(const rspfConnectableObject* object)
{
   rspf_int32 i = 0;
   for(i = 0; i < (rspf_int32)theOutputObjectList.size(); ++i)
   {
      if(theOutputObjectList[i] == object)
      {
         return i;
      }
   }
   
   return -1;
}

rspf_int32 rspfConnectableObject::findOutputIndex(const rspfId& id)
{
   rspf_int32 i = 0;
   for(i = 0; i < (rspf_int32)theOutputObjectList.size(); ++i)
   {
      if(theOutputObjectList[i].valid() &&
         (theOutputObjectList[i]->getId() == id))
      {
         return i;
      }
   }
   
   return -1;
}

rspf_int32 rspfConnectableObject::getMyInputIndexToConnectTo(rspfConnectableObject* object)const
{
   ConnectableObjectList::const_iterator current;
   
   rspf_int32 index = 0;
   current = theInputObjectList.begin();
   while(current != theInputObjectList.end())
   {
      if(!(*current))
      {
         if(canConnectMyInputTo(index, object))
         {
            return index;
         }
      }
      ++current;
      ++index;
   }
   if(!theInputListIsFixedFlag)
   {
      if(canConnectMyInputTo((rspf_int32)theInputObjectList.size(),
                             object))
      {
         return (rspf_int32)theInputObjectList.size();
      }
   }
   return -1;
}

rspf_int32 rspfConnectableObject::getMyOutputIndexToConnectTo(rspfConnectableObject* object)const
{
   ConnectableObjectList::const_iterator current;
   
   rspf_int32 index = 0;
   current = theOutputObjectList.begin();
   while(current != theOutputObjectList.end())
   {
      if(!(*current))
      {
         if(canConnectMyOutputTo(index, object))
         {
            return index;
         }
      }
      ++current;
      ++index;
   }
   if(!theOutputListIsFixedFlag)
   {
      if(canConnectMyOutputTo((rspf_int32)theOutputObjectList.size(),
                              object))
      {
         return (rspf_int32)theOutputObjectList.size();
      }
   }
   // default to return the size.  This will force an append.
   //
   return -1;
}

void rspfConnectableObject::disconnect(rspfConnectableObject* object)
{
   if( (object==this) || (object==0) )
   {
      disconnectAllInputs();
      disconnectAllOutputs();
   }
   else
   {
      rspf_int32 index = findInputIndex(object);
      if(index > -1)
      {
         disconnectMyInput(index);
      }
      index = findOutputIndex(object);
      if(index > -1)
      {
         disconnectMyOutput(index, object);
      }
   }
}

void rspfConnectableObject::disconnect(const rspfId& id)
{
   if(id == theId)
   {
      disconnect(this);
   }
   else
   {
      rspfIdVisitor visitor( id,
                              (rspfVisitor::VISIT_CHILDREN |
                               rspfVisitor::VISIT_INPUTS   |
                               rspfVisitor::VISIT_OUTPUTS) );
      accept( visitor );
      disconnect( visitor.getObject() );
   }
}

rspfRefPtr<rspfConnectableObject> rspfConnectableObject::disconnectMyInput(rspf_int32 inputIndex,
                                                                  bool disconnectOutputFlag,
                                                                  bool createEventFlag)
{
   if(theInputObjectList.size() == 0)
   {
      return 0;
   }
   
   ConnectableObjectList::iterator current;
   rspfRefPtr<rspfConnectableObject> result = 0;
   
   if( (inputIndex > -1)&&
      (inputIndex < (rspf_int32)theInputObjectList.size()))
   {
      current = (theInputObjectList.begin()+inputIndex);
      result = (*current).get();
      
      if(!theInputListIsFixedFlag)
      {
         current = theInputObjectList.erase(current);
      }
      else
      {
         *current = 0;
      }
      if(createEventFlag&&result.valid())
      {
         rspfConnectionEvent event(this,  // owner of message
                                    RSPF_EVENT_CONNECTION_DISCONNECT_ID,
                                    0,  // new object
                                    result.get(),// old object
                                    rspfConnectionEvent::RSPF_INPUT_DIRECTION);
         
         // send event to all listeners.
         //
         fireEvent(event);
      }
      
      if(disconnectOutputFlag&&result.valid())
      {
         result->disconnectMyOutput(this, false, createEventFlag);
      }
   }
   return result;
}


void rspfConnectableObject::disconnectMyInput(rspfConnectableObject* input,
                                               bool disconnectOutputFlag,
                                               bool createEventFlag)
{
   disconnectMyInput(findInputIndex(input),
                     disconnectOutputFlag,
                     createEventFlag);
}

void rspfConnectableObject::disconnectMyInputs(ConnectableObjectList& inputList,
                                                bool disconnectOutputFlag,
                                                bool createEventFlag)
{
   if(theInputObjectList.size() == 0) return;
   ConnectableObjectList oldInputs;
   ConnectableObjectList newInputs;
   rspf_int32 i = 0;
   
   for(i = 0; i < (rspf_int32)inputList.size(); ++i)
   {
      if(findInputIndex(inputList[i].get()) >= 0)
      {
         disconnectMyInput(inputList[i].get(), disconnectOutputFlag, false);
         oldInputs.push_back(inputList[i]);
      }
   }
   if(createEventFlag && oldInputs.size())
   {
      rspfConnectionEvent event(this,
                                 RSPF_EVENT_CONNECTION_DISCONNECT_ID,
                                 newInputs,
                                 oldInputs,
                                 rspfConnectionEvent::RSPF_INPUT_DIRECTION);
      fireEvent(event);
   }
}

rspfRefPtr<rspfConnectableObject> rspfConnectableObject::disconnectMyOutput(rspf_int32 outputIndex,
                                                                   bool disconnectInputFlag,
                                                                   bool createEvent)
{
   if(theOutputObjectList.size() == 0)
   {
      return 0;
   }
   
   ConnectableObjectList::iterator current;
   rspfRefPtr<rspfConnectableObject> result = 0;
   if( (outputIndex > -1)&&
      (outputIndex < (rspf_int32)theOutputObjectList.size()))
   {
      current = (theOutputObjectList.begin()+outputIndex);
      result = (*current).get();
      if(!theOutputListIsFixedFlag)
      {
         current = theOutputObjectList.erase(current);
      }
      else
      {
         *current = 0;
      }
      if(createEvent)
      {
         rspfConnectionEvent event(this,  // owner of message
                                    RSPF_EVENT_CONNECTION_DISCONNECT_ID,
                                    0,  // new object
                                    result.get(),// old object
                                    rspfConnectionEvent::RSPF_OUTPUT_DIRECTION);
         
         // send event to all listeners.
         //
         fireEvent(event);
      }
      
      if(disconnectInputFlag&&result.valid())
      {
         result->disconnectMyInput(this, false, createEvent);
      }
   }
   return result;
}

void rspfConnectableObject::disconnectMyOutput(rspfConnectableObject* output,
                                                bool disconnectInputFlag,
                                                bool createEventFlag)
{
   disconnectMyOutput(findOutputIndex(output), disconnectInputFlag, createEventFlag);
}

void rspfConnectableObject::disconnectMyOutputs(ConnectableObjectList& outputList,
                                                 bool disconnectInputFlag,
                                                 bool createEventFlag)
{
   if(theOutputObjectList.size() == 0) return;
   
   if(theOutputObjectList.size() == 1)
   {
      disconnectMyOutput((rspf_int32)0);
      return;
   }
   ConnectableObjectList oldOutputs;
   ConnectableObjectList newOutputs;
   rspf_int32 i = 0;
   for(i = 0; i < (rspf_int32)outputList.size(); ++i)
   {
      if(findOutputIndex(outputList[i].get()) >= 0)
      {
         disconnectMyOutput(outputList[i].get(), disconnectInputFlag, false);
         oldOutputs.push_back(outputList[i]);
      }
   }
   if(createEventFlag && oldOutputs.size())
   {
      rspfConnectionEvent event(this,
                                 RSPF_EVENT_CONNECTION_DISCONNECT_ID,
                                 newOutputs,
                                 oldOutputs,
                                 rspfConnectionEvent::RSPF_OUTPUT_DIRECTION);
      fireEvent(event);
   }
}

void rspfConnectableObject::disconnectAllInputs()
{
   if(theInputObjectList.size() == 0) return;
   if(theInputObjectList.size() == 1)
   {
      //     rspfConnectableObject* obj = disconnectMyInput((rspf_int32)0, false);
      disconnectMyInput((rspf_int32)0);
      //     if(obj)
      //     {
      //        obj->disconnectMyOutput(this, false);
      //     }
      return;
   }
   ConnectableObjectList::iterator current;
   ConnectableObjectList oldInputs = theInputObjectList;
   ConnectableObjectList newInputs;
   
   
   current = theInputObjectList.begin();
   while(current != theInputObjectList.end())
   {
      if(!theInputListIsFixedFlag)
      {
         current = theInputObjectList.erase(current);
      }
      else
      {
         *current = 0;
         ++current;
      }
   }
   rspfConnectionEvent event(this,
                              RSPF_EVENT_CONNECTION_DISCONNECT_ID,
                              newInputs,
                              oldInputs,
                              rspfConnectionEvent::RSPF_INPUT_DIRECTION);
   fireEvent(event);
   
   for(rspf_uint32 index = 0; index < oldInputs.size(); ++index)
   {
      if(oldInputs[index].valid())
      {
         oldInputs[index]->disconnectMyOutput(this, false);
      }
   }
}

void rspfConnectableObject::disconnectAllOutputs()
{
   if(theOutputObjectList.size() == 0) return;
   if(theOutputObjectList.size() == 1)
   {
      //     rspfConnectableObject* obj = disconnectMyOutput((rspf_int32)0, false);
      disconnectMyOutput((rspf_int32)0);
      //     if(obj)
      //     {
      //        obj->disconnectMyInput(this,
      //                               false);
      //     }
      return;
   }
   ConnectableObjectList::iterator current;
   
   ConnectableObjectList oldOutputs = theOutputObjectList;
   ConnectableObjectList newOutputs;
   
   current = theOutputObjectList.begin();
   while(current != theOutputObjectList.end())
   {
      if(!theOutputListIsFixedFlag)
      {
         current = theOutputObjectList.erase(current);
      }
      else
      {
         *current = 0;
         ++current;
      }
   }
   rspfConnectionEvent event(this,
                              RSPF_EVENT_CONNECTION_DISCONNECT_ID,
                              newOutputs,
                              oldOutputs,
                              rspfConnectionEvent::RSPF_OUTPUT_DIRECTION);
   fireEvent(event);
   
   for(rspf_uint32 index = 0; index < oldOutputs.size(); ++index)
   {
      if(oldOutputs[index].valid())
      {
         oldOutputs[index]->disconnectMyInput(this, false);
      }
   }
}

rspf_int32 rspfConnectableObject::connectMyInputTo(rspfConnectableObject* object,
                                                     bool makeOutputConnection,
                                                     bool createEventFlag)
{
   if(!object) return false;
   
   rspf_int32 index = findInputIndex(object);
   if(index >= 0) return index;
   
   index = getMyInputIndexToConnectTo(object);
   
   if(index>-1)
   {
      if(index >= (rspf_int32)theInputObjectList.size())
      {
         if(theInputListIsFixedFlag) return -1;
         
         index = (rspf_int32)theInputObjectList.size();
         theInputObjectList.push_back(object);
      }
      else
      {
         if(!theInputObjectList[index])
         {
            theInputObjectList[index] = object;
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN) << "Must issue a detach first!! trying to attach object " << object->getClassName()
            << "\n to input index " << index << " in " << getClassName() << "\n";
            
            return -1;
         }
      }
      if(createEventFlag)
      {
         rspfConnectionEvent event(this,  // owner of message
                                    RSPF_EVENT_CONNECTION_CONNECT_ID,
                                    theInputObjectList[index].get(),  // new object
                                    0,// old object
                                    rspfConnectionEvent::RSPF_INPUT_DIRECTION);
         // send event to any listener.
         //
         fireEvent(event);
      }
      
      if(makeOutputConnection&&object)
      {
         // make sure we tell the input not to connect
         // back to us or infinite loop
         //
         object->connectMyOutputTo(this, false, createEventFlag);
      }
   }
   else
   {
      return index;
   }
   
   return index;
}

rspf_int32 rspfConnectableObject::connectMyInputTo(rspf_int32 inputIndex,
                                                     rspfConnectableObject* inputObject,
                                                     bool makeOutputConnection,
                                                     bool createEventFlag)
{
   if(!inputObject)
   {
      if(inputIndex < (rspf_int32)theInputObjectList.size())
      {
         rspfConnectableObject* oldObject = theInputObjectList[inputIndex].get();
         if(theInputListIsFixedFlag)
         {
            theInputObjectList[inputIndex] = 0;
         }
         else
         {
            theInputObjectList.erase(theInputObjectList.begin() + inputIndex);
         }
         if(createEventFlag)
         {
            rspfConnectionEvent event(this,  // owner of message
                                       RSPF_EVENT_CONNECTION_CONNECT_ID,
                                       0,  // new object
                                       oldObject,// old object
                                       rspfConnectionEvent::RSPF_INPUT_DIRECTION);
            // send event to any listener.
            //
            fireEvent(event);
         }
         return -1;
      }
      return -1;
   }
   
   rspf_int32 index = findInputIndex(inputObject);
   
   if((index >= 0)&&!inputObject) return index;
   if(!inputObject) return -1;
   
   if(canConnectMyInputTo(inputIndex, inputObject))
   {
      if(inputIndex>-1)
      {
         rspfConnectableObject* oldObject=0;
         if(inputIndex >= (rspf_int32)theInputObjectList.size())
         {
            if(theInputListIsFixedFlag) return -1;
            
            inputIndex = (rspf_int32)theInputObjectList.size();
            theInputObjectList.push_back(inputObject);
         }
         else
         {
            oldObject = theInputObjectList[inputIndex].get();
            theInputObjectList[inputIndex] = inputObject;
            if(oldObject)
            {
               oldObject->disconnectMyOutput(this, false);
            }
         }
         
         if(createEventFlag)
         {
            rspfConnectionEvent event(this,  // owner of message
                                       RSPF_EVENT_CONNECTION_CONNECT_ID,
                                       theInputObjectList[inputIndex].get(),  // new object
                                       oldObject,// old object
                                       rspfConnectionEvent::RSPF_INPUT_DIRECTION);
            // send event to any listener.
            //
            fireEvent(event);
         }
         
         if(makeOutputConnection&&inputObject)
         {
            // make sure we tell the input not to connect
            // back to us or infinite loop
            //
            inputObject->connectMyOutputTo(this, false, createEventFlag);
         }
      }
      else
      {
         return inputIndex;
      }
      
      return inputIndex;
   }
   
   return -1;
}

bool rspfConnectableObject::connectMyInputTo(ConnectableObjectList& inputList,
                                              bool makeOutputConnection,
                                              bool createEventFlag)
{
   bool result = true;
   ConnectableObjectList oldInputs;
   ConnectableObjectList newInputs;
   
   if(inputList.size() == 1)
   {
      if(inputList[0].valid())
      {
         return (connectMyInputTo(inputList[0].get(), makeOutputConnection, createEventFlag) >= 0);
      }
      else
      {
         if(theInputListIsFixedFlag)
         {
            theInputObjectList[0] = 0;
         }
         else
         {
            theInputObjectList.clear();
         }
         if(createEventFlag)
         {
            rspfConnectionEvent event(this,
                                       RSPF_EVENT_CONNECTION_CONNECT_ID,
                                       0,
                                       theInputObjectList[0].get(),
                                       rspfConnectionEvent::RSPF_INPUT_DIRECTION);
            fireEvent(event);
         }
         
         return true;
      }
   }
   rspf_int32 i = 0;
   for(i = 0; i < (rspf_int32)inputList.size(); ++i)
   {
      if(inputList[i].valid())
      {
         if(connectMyInputTo(inputList[i].get(), makeOutputConnection, false)<0)
         {
            result = false;
         }
         else
         {
            newInputs.push_back(inputList[i]);
         }
      }
      else
      {
         newInputs.push_back(0);
      }
   }
   if(createEventFlag)
   {
      rspfConnectionEvent event(this,
                                 RSPF_EVENT_CONNECTION_CONNECT_ID,
                                 newInputs,
                                 oldInputs,
                                 rspfConnectionEvent::RSPF_INPUT_DIRECTION);
      fireEvent(event);
   }
   return result;
}

rspf_int32 rspfConnectableObject::connectMyOutputTo(rspfConnectableObject* output,
                                                      bool makeInputConnection,
                                                      bool createEventFlag)
{
   rspf_int32 index = findOutputIndex(output);
   if(index >= 0) return index;
   
   index = getMyOutputIndexToConnectTo(output);
   
   if(index > -1)
   {
      if((index >= (rspf_int32)theOutputObjectList.size())&&
         !theOutputListIsFixedFlag)
      {
         index = (rspf_int32)theOutputObjectList.size();
         theOutputObjectList.push_back(output);
      }
      else
      {
         if(!theOutputObjectList[index])
         {
            theOutputObjectList[index] = output;
         }
         else
         {
            rspfNotify(rspfNotifyLevel_WARN) << "Must issue a disconnect first!! trying to connect object " << output->getClassName()
            << "\n to output index " << index << " in " << getClassName() << "\n";
            
            return -1;
         }
      }
      if(createEventFlag)
      {
         rspfConnectionEvent event(this,  // owner of message
                                    RSPF_EVENT_CONNECTION_CONNECT_ID,
                                    theOutputObjectList[index].get(),  // new object
                                    0,// old object
                                    rspfConnectionEvent::RSPF_OUTPUT_DIRECTION);
         // send event to any listener.
         //
         fireEvent(event);
      }
      if(makeInputConnection&&output)
      {
         // tell the output object not to connect back
         // to us since this is already done.
         //
         output->connectMyInputTo(this, false, createEventFlag);
      }
   }
   else
   {
      return index;
   }
   
   return index;
}

bool rspfConnectableObject::connectMyOutputTo(ConnectableObjectList& outputList,
                                               bool makeInputConnection,
                                               bool createEventFlag)
{
   bool result = true;
   ConnectableObjectList oldOutputs;
   ConnectableObjectList newOutputs;
   
   if(outputList.size() == 0)
   {
      disconnectAllOutputs();
      return true;
   }
   if((outputList.size() == 1)&&outputList[0].valid())
   {
      if(outputList[0].valid())
      {
         return (connectMyOutputTo(outputList[0].get(), makeInputConnection, createEventFlag) >= 0);
      }
      else
      {
         disconnectAllOutputs();
      }
   }
   
   rspf_int32 i = 0;
   for(i = 0; i < (rspf_int32)outputList.size(); ++i)
   {
      if((connectMyOutputTo(outputList[i].get(), makeInputConnection, false)<0)&&outputList[i].valid())
      {
         newOutputs.push_back(outputList[i].get());
         result = false;
      }
   }
   if(createEventFlag&&newOutputs.size())
   {
      rspfConnectionEvent event(this,
                                 RSPF_EVENT_CONNECTION_CONNECT_ID,
                                 newOutputs,
                                 oldOutputs,
                                 rspfConnectionEvent::RSPF_OUTPUT_DIRECTION);
      fireEvent(event);
   }
   
   return result;
}

rspfConnectableObject* rspfConnectableObject::getInput(rspf_uint32 idx)
{
   if(idx < theInputObjectList.size())
   {
      return theInputObjectList[idx].get();
   }
   
   return 0;
}

const rspfConnectableObject* rspfConnectableObject::getInput(rspf_uint32 idx)const
{
   if( idx < theInputObjectList.size())
   {
      return theInputObjectList[idx].get();
   }
   
   return 0;
}

rspfConnectableObject* rspfConnectableObject::getOutput(rspf_uint32 idx)
{
   if(idx < theOutputObjectList.size())
   {
      return theOutputObjectList[idx].get();
   }
   
   return 0;
}

bool rspfConnectableObject::connectInputList(ConnectableObjectList& inputList)
{
   bool result = true;
   rspf_uint32 i = 0;
   ConnectableObjectList oldInputs = theInputObjectList;
   ConnectableObjectList newInputs;
   ConnectableObjectList::iterator currentInput = inputList.begin();
   ConnectableObjectList tempOld;
   if(theInputObjectList.size())
   {
      for(i = 0; i < theInputObjectList.size(); ++i)
      {
         if(oldInputs[i].valid())
         {
            tempOld.push_back(oldInputs[i].get());
         }
         theInputObjectList[i] = 0;
      }
   }
   if(theInputListIsFixedFlag && (theInputObjectList.size()==0))
   {
      return false;
   }
   
   if(!theInputListIsFixedFlag)
   {
      theInputObjectList.clear();
   }
   if(tempOld.size())
   {
      for(i = 0; i < tempOld.size(); ++ i)
      {
         tempOld[i]->disconnectMyOutput(this, false);
      }
   }
   //   disconnectAllInputs();
   
   if(inputList.size() == 1)
   {
      return (connectMyInputTo(inputList[0].get())>=0);
   }
   i = 0;
   // now connect the new outputs
   //
   currentInput = inputList.begin();
   result = false;
   
   while(currentInput != inputList.end())
   {
      if((*currentInput).valid())
      {
         if(connectMyInputTo((*currentInput).get(), false) >= 0)
         {
            newInputs.push_back((*currentInput).get());
            result = true;
         }
      }
      ++currentInput;
   }
   rspfConnectionEvent event(this,
                              RSPF_EVENT_CONNECTION_DISCONNECT_ID,
                              newInputs,
                              oldInputs,
                              rspfConnectionEvent::RSPF_INPUT_DIRECTION);
   
   if(theInputObjectList.size())
   {
      fireEvent(event);
   }
   
   newInputs          = theInputObjectList;
   
   
   
   event = rspfConnectionEvent(this,
                                RSPF_EVENT_CONNECTION_CONNECT_ID,
                                newInputs,
                                oldInputs,
                                rspfConnectionEvent::RSPF_INPUT_DIRECTION);
   fireEvent(event);
   
   return result;
}

bool rspfConnectableObject::connectOutputList(ConnectableObjectList& outputList)
{
   bool result = true;
   rspf_int32 index  = 0;
   ConnectableObjectList oldOutputs = theOutputObjectList;
   ConnectableObjectList newOutputs;
   ConnectableObjectList::const_iterator currentOutput = outputList.begin();
   
   disconnectAllOutputs();
   
   if(outputList.size() == 1)
   {
      return (connectMyOutputTo(outputList[0].get())>=0);
   }
   while(currentOutput != outputList.end())
   {
      if(!canConnectMyOutputTo(index, (*currentOutput).get()))
      {
         result = false;
      }
      ++currentOutput;
   }
   
   if(!result)
   {
      return false;
   }
   
   theOutputObjectList = outputList;
   newOutputs = theOutputObjectList;
   
   rspfConnectionEvent event(this,
                              RSPF_EVENT_CONNECTION_CONNECT_ID,
                              newOutputs,
                              oldOutputs,
                              rspfConnectionEvent::RSPF_OUTPUT_DIRECTION);
   fireEvent(event);
   
   return result;
}

void rspfConnectableObject::setNumberOfInputs(rspf_int32 numberOfInputs)
{
   if((rspf_int32)theInputObjectList.size() == numberOfInputs)
   {
      return;
   }
   rspf_int32 i = 0;
   if(numberOfInputs < (rspf_int32)theInputObjectList.size())
   {
      ConnectableObjectList v(theInputObjectList.begin(),
                              theInputObjectList.begin()+numberOfInputs);
      ConnectableObjectList disconnectList;
      
      for(i = numberOfInputs;
          i < (rspf_int32)theInputObjectList.size();
          ++i)
      {
         if(theInputObjectList[i].valid())
         {
            disconnectList.push_back(theInputObjectList[i]);
         }
      }
      disconnectMyInputs(disconnectList);
      
      theInputObjectList.clear();
      theInputObjectList = v;
   }
   else
   {
      for(i = (rspf_int32)theInputObjectList.size();
          i < numberOfInputs;
          ++i)
      {
         theInputObjectList.push_back(0);
      }
   }
}

void rspfConnectableObject::setNumberOfOutputs(rspf_int32 numberOfOutputs)
{
   if((rspf_int32)theOutputObjectList.size() == numberOfOutputs)
   {
      return;
   }
   rspf_int32 i = 0;
   if(numberOfOutputs < (rspf_int32)theOutputObjectList.size())
   {
      ConnectableObjectList v(theOutputObjectList.begin(),
                              theOutputObjectList.begin()+numberOfOutputs);
      ConnectableObjectList disconnectList;
      for(i = numberOfOutputs;
          i < (rspf_int32)theOutputObjectList.size();
          ++i)
      {
         if(theOutputObjectList[i].valid())
         {
            disconnectList.push_back(theOutputObjectList[i]);
         }
      }
      disconnectMyOutputs(disconnectList);
      
      theOutputObjectList.clear();
      theOutputObjectList = v;
   }
   else
   {
      for(i = (rspf_int32)theOutputObjectList.size();
          i < numberOfOutputs;
          ++i)
      {
         theOutputObjectList.push_back(0);
      }
   }
}


const rspfConnectableObject* rspfConnectableObject::getOutput(rspf_uint32 idx)const
{
   if(idx < theOutputObjectList.size())
   {
      return theOutputObjectList[idx].get();
   }
   
   return 0;
}

void  rspfConnectableObject::findAllObjectsOfType(ConnectableObjectList& result,
                                                   const RTTItypeid& typeInfo,
                                                   bool recurse)
{
   int j;
   // go through children first
   //
   rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                        this);
   if(inter)
   {
      ConnectableObjectList tempList = inter->findAllObjectsOfType(typeInfo,
                                                                   recurse);
      
      for(j = 0; j < (int)tempList.size(); ++j)
      {
         ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), tempList[j]);
         if(iter == result.end())
         {
            result.push_back(tempList[j].get());
         }
      }
   }
   
}

void rspfConnectableObject::findAllObjectsOfType(ConnectableObjectList& result, 
                                                  const rspfString& className,
                                                  bool recurse)
{
   int j;
   // go through children first
   //
   rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                        this);
   if(inter)
   {
      ConnectableObjectList tempList = inter->findAllObjectsOfType(className,
                                                                   recurse);
      
      for(j = 0; j < (int)tempList.size(); ++j)
      {
         ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), tempList[j]);
         if(iter == result.end())
         {
            result.push_back(tempList[j].get());
         }
      }
   }
}

#if 0
void rspfConnectableObject::findAllInputsOfType(ConnectableObjectList& result,
                                                 const RTTItypeid& typeInfo,
                                                 bool propagateToInputs,
                                                 bool recurseChildren)
{
   int i;
   int j;
   // go through children first
   //
   rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                        this);
   if(inter&&recurseChildren)
   {
      ConnectableObjectList tempList = inter->findAllObjectsOfType(typeInfo,
                                                                   true);
      
      for(j = 0; j < (int)tempList.size(); ++j)
      {
         ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), tempList[j]);
         if(iter == result.end())
         {
            result.push_back(tempList[j].get());
         }
      }
   }
   
   for(i = 0; i < (int)getNumberOfInputs(); ++i)
   {
      rspfConnectableObject* current = getInput(i);
      if(current&&(current->canCastTo(typeInfo)))
      {
         ConnectableObjectList::iterator position = std::find(result.begin(), result.end(), current);
         
         if(position == result.end())
         {
            result.push_back(current);
         }
      }
      inter = PTR_CAST(rspfConnectableContainerInterface, current);
      if(inter)
      {
         ConnectableObjectList tempList = inter->findAllObjectsOfType(typeInfo, true);
         for(j = 0; j < (int)tempList.size(); ++j)
         {
            ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), tempList[j]);
            if(iter == result.end())
            {
               result.push_back(tempList[j]);
            }
         }
      }
      
      if(propagateToInputs&&current)
      {
         current->findAllInputsOfType(result,
                                      typeInfo,
                                      true,
                                      recurseChildren);
      }
   }
}

void rspfConnectableObject::findAllInputsOfType(ConnectableObjectList& result,
                                                 const rspfString& className,
                                                 bool propagateToInputs,
                                                 bool recurseChildren)
{
   int j;
   // go through children first
   //
   rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                        this);
   if(inter&&recurseChildren)
   {
      ConnectableObjectList tempList = inter->findAllObjectsOfType(className,
                                                                   true);
      
      for(j = 0; j < (int)tempList.size(); ++j)
      {
         ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), tempList[j]);
         if(iter == result.end())
         {
            result.push_back(tempList[j]);
         }
      }
   }
   for(rspf_uint32 i = 0; i < getNumberOfInputs(); ++i)
   {
      rspfConnectableObject* current = getInput(i);
      if(current&&(current->canCastTo(className)))
      {
         ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), current);
         if(iter == result.end())
         {
            result.push_back(current);
         }
      }
      rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                           current);
      if(inter)
      {
         ConnectableObjectList tempList = inter->findAllObjectsOfType(className, true);
         for(j = 0; j < (int)tempList.size(); ++j)
         {
            ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), tempList[j]);
            if(iter == result.end())
            {
               result.push_back(tempList[j]);
            }
         }
      }
      
      if(propagateToInputs&&current)
      {
         current->findAllInputsOfType(result,
                                      className,
                                      true,
                                      recurseChildren);
      }
   }
}
#endif

void rspfConnectableObject::propagateEventToOutputs(rspfEvent& event)
{
   rspf_uint32 i;
   
   for(i = 0; i <getNumberOfOutputs(); ++i)
   {
      rspfConnectableObject* obj = getOutput(i);
      if(obj)
      {
         event.setPropagationType(rspfEvent::PROPAGATION_OUTPUT);
         obj->fireEvent(event);
         obj->propagateEventToOutputs(event);
      }
   }
}

void rspfConnectableObject::propagateEventToInputs(rspfEvent& event)
{
   rspf_uint32 i;
   
   for(i = 0; i <getNumberOfInputs(); ++i)
   {
      rspfConnectableObject* obj = getInput(i);
      if(obj)
      {
         obj->fireEvent(event);
         obj->propagateEventToInputs(event);
      }
   }
}

void rspfConnectableObject::setProperty(rspfRefPtr<rspfProperty> property)
{
   if(!property.valid()) return;
   if(property->getName() == "Description")
   {
      property->valueToString(theDescription);
   }
}

void rspfConnectableObject::setProperty(const rspfString& name, const rspfString& value)
{
   rspfPropertyInterface::setProperty(name, value);
}

rspfRefPtr<rspfProperty> rspfConnectableObject::getProperty(const rspfString& name)const
{
   if(name == "Description")
   {
      return new rspfTextProperty(name, theDescription);
   }
   // "Class name" check for backwards compatibility only.
   else if( (name == "class_name") ||
           (name == "Class name") ) 
   {
      rspfProperty* prop = new rspfTextProperty(name,
                                                  getClassName());
      prop->setReadOnlyFlag(true);
      
      return prop;
   }
   return rspfRefPtr<rspfProperty>();
}

void rspfConnectableObject::getPropertyNames(std::vector<rspfString>& propertyNames)const
{
   propertyNames.push_back("class_name");
   propertyNames.push_back("Description");
}

bool rspfConnectableObject::loadState(const rspfKeywordlist& kwl,
                                       const char* prefix)
{
   const char* lookup = kwl.find(prefix,
                                 rspfKeywordNames::ID_KW);
   
   // disconnect(this);
   
   if(lookup)
   {
      theId = rspfId(rspfString(lookup).toLong());
   }
   
   lookup = kwl.find(prefix, CONNECTABLE_INPUT_LIST_FIXED_KW);
   if(lookup)
   {
      theInputListIsFixedFlag = rspfString(lookup).toBool();
   }
   
   lookup = kwl.find(prefix, CONNECTABLE_OUTPUT_LIST_FIXED_KW);
   if(lookup)
   {
      theOutputListIsFixedFlag = rspfString(lookup).toBool();
   }
   
   rspf_int32 numberInputs  = 0;
   rspf_int32 numberOutputs = 0;
   
   rspfString regExpression;
   lookup = kwl.find(prefix, rspfKeywordNames::NUMBER_INPUTS_KW);
   if(lookup)
   {
      numberInputs = rspfString(lookup).toLong();
   }
   else if(!theInputListIsFixedFlag)
   {
      regExpression = rspfString("^(") + rspfString(prefix) + "input_connection[0-9]+)";
      numberInputs = kwl.getNumberOfSubstringKeys(regExpression);
   }
   else
   {
      // if we are fixed then the list should already be set
      numberInputs = (rspf_int32) theInputObjectList.size();
   }

   
   lookup = kwl.find(prefix, rspfKeywordNames::NUMBER_OUTPUTS_KW);
   if(lookup)
   {
      numberOutputs = rspfString(lookup).toLong();
   }
   else if(!theOutputListIsFixedFlag)
   {
      regExpression = rspfString("^(") + rspfString(prefix) + "output_connection[0-9]+)";
      numberOutputs = kwl.getNumberOfSubstringKeys(regExpression);
   }
   else 
   {
      // if we are fixed then the list should already be set
      numberOutputs = (rspf_int32) theOutputObjectList.size();
   }

   lookup = kwl.find(prefix, rspfKeywordNames::DESCRIPTION_KW);
   if (lookup)
   {
      theDescription = lookup;
   }
   
   setNumberOfInputs(numberInputs);
   setNumberOfOutputs(numberOutputs);
   
   return rspfObject::loadState(kwl, prefix);
}

bool rspfConnectableObject::saveState(rspfKeywordlist& kwl,
                                       const char* prefix)const
{
   rspfObject::saveState(kwl, prefix);
   
   kwl.add(prefix,
           rspfKeywordNames::ID_KW,
           theId.getId(),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::DESCRIPTION_KW,
           theDescription,
           true);
   
   kwl.add(prefix,
           CONNECTABLE_INPUT_LIST_FIXED_KW,
           theInputListIsFixedFlag,
           true);
   
   kwl.add(prefix,
           CONNECTABLE_OUTPUT_LIST_FIXED_KW,
           theOutputListIsFixedFlag,
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_INPUTS_KW,
           static_cast<rspf_uint32>(theInputObjectList.size()),
           true);
   
   kwl.add(prefix,
           rspfKeywordNames::NUMBER_OUTPUTS_KW,
           static_cast<rspf_uint32>(theOutputObjectList.size()),
           true);
   
   rspf_int32 i = 0;
   for(i = 1; i <= (rspf_int32)theInputObjectList.size(); ++i)
   {
      rspfString value = "input_connection" + rspfString::toString(i);
      
      rspf_int32 id;
      
      if(theInputObjectList[i-1].valid())
      {
         id = theInputObjectList[i-1]->getId().getId();
      }
      else
      {
         id = -1;
      }
      kwl.add(prefix,
              value.c_str(),
              id,
              true);
   }
   
   for(i = 1; i <= (rspf_int32)theOutputObjectList.size(); ++i)
   {
      rspfString value = "output_connection" + rspfString::toString(i);
      
      rspf_int32 id;
      
      if(theOutputObjectList[i-1].valid())
      {
         id = theOutputObjectList[i-1]->getId().getId();
      }
      else
      {
         id = -1;
      }
      kwl.add(prefix,
              value.c_str(),
              id,
              true);
   }
   
   return true;
}

rspf_uint32 rspfConnectableObject::saveStateOfAllInputs(rspfKeywordlist& kwl,
                                                          bool              saveThisStateFlag,
                                                          rspf_uint32      objectIndex,
                                                          const char*       prefix) const
{
   rspf_uint32 index = objectIndex;

   const rspf_uint32 NUMBER_OF_INPUTS = getNumberOfInputs();
   if (NUMBER_OF_INPUTS)
   {
      // Save all the inputs.
      for(rspf_uint32 i = 0; i < NUMBER_OF_INPUTS; ++i)
      {
         const rspfConnectableObject* input = getInput(i);
         if(input)
            index = input->saveStateOfAllInputs(kwl, true, index, prefix);
      }
   }

   if (saveThisStateFlag)
   {
      rspfString myPrefix;
      if (prefix)
         myPrefix = prefix;

      myPrefix += "object" + rspfString::toString(index) + ".";

      // Save the state of this object.
      saveState(kwl, myPrefix.c_str());
      ++index;
   }

   return index;
}

bool rspfConnectableObject::fillContainer(rspfConnectableContainer& container)
{
   // Insert inputs into the container:
   bool good_fill = true;
   rspf_uint32 num_inputs = getNumberOfInputs();
   for(rspf_uint32 i=0; (i<num_inputs) && good_fill; ++i)
   {
      rspfConnectableObject* input = getInput(i);
      if (input)
         good_fill = input->fillContainer(container);
   }
   
   // Insert this object and all of its children and inputs into the container provided:
   if (good_fill)
      good_fill = container.addChild(this);

   return good_fill;
}

bool rspfConnectableObject::canConnectMyOutputTo(rspf_int32 myOutputIndex,
                                                  const rspfConnectableObject* /* object */ ) const
{
   if(theOutputListIsFixedFlag)
   {
      return ((myOutputIndex >= 0) &&
              (myOutputIndex < (rspf_int32)theOutputObjectList.size()));
   }
   
   return ((myOutputIndex >= 0) &&
           (myOutputIndex  <= (rspf_int32)theOutputObjectList.size()));
}

bool rspfConnectableObject::moveInputUp(const rspfId& id)
{
   bool result = false;
   
   if (theInputListIsFixedFlag == false)
   {
      if ( theInputObjectList.size() )
      {
         rspf_int32 indexOfId = findInputIndex(id);
         
         if (indexOfId > 0)
         {
            ConnectableObjectList oldInputs =
            theInputObjectList;
            
            // Swap with index above.
            rspfRefPtr<rspfConnectableObject> tmpObj  = theInputObjectList[indexOfId].get();
            theInputObjectList[indexOfId]   = theInputObjectList[indexOfId-1].get();
            theInputObjectList[indexOfId-1] = tmpObj;
            result = true;
            
            ConnectableObjectList newInputs =
            theInputObjectList;
            
            rspfConnectionEvent event(this,
                                       RSPF_EVENT_CONNECTION_CONNECT_ID,
                                       newInputs,
                                       oldInputs,
                                       rspfConnectionEvent::RSPF_INPUT_DIRECTION);
            fireEvent(event);
         }
      }
   }
   
   return result;
}

bool rspfConnectableObject::moveInputDown(const rspfId& id)
{
   bool result = false;
   
   if (theInputListIsFixedFlag == false)
   {
      if ( theInputObjectList.size() )
      {
         rspf_int32 indexOfId = findInputIndex(id);
         
         if ( indexOfId <
             static_cast<rspf_int32>(theInputObjectList.size()-1) )
         {
            ConnectableObjectList oldInputs =
            theInputObjectList;
            
            // Swap with index below.
            rspfRefPtr<rspfConnectableObject> tmpObj  = theInputObjectList[indexOfId].get();
            theInputObjectList[indexOfId]   = theInputObjectList[indexOfId+1].get();
            theInputObjectList[indexOfId+1] = tmpObj;
            result = true;
            
            ConnectableObjectList newInputs =
            theInputObjectList;
            
            rspfConnectionEvent event(this,
                                       RSPF_EVENT_CONNECTION_CONNECT_ID,
                                       newInputs,
                                       oldInputs,
                                       rspfConnectionEvent::RSPF_INPUT_DIRECTION);
            fireEvent(event);
         }
      }
   }
   
   return result;
}

bool rspfConnectableObject::moveInputToTop(const rspfId& id)
{
   bool result = false;
   
   if (theInputListIsFixedFlag == false)
   {
      if ( theInputObjectList.size() )
      {
         ConnectableObjectList::iterator i =
         theInputObjectList.begin();
         
         while (i != theInputObjectList.end())
         {
            if ( (*i)->getId() == id )
            {
               break;
            }
            ++i;
         }
         
         if ( (i != theInputObjectList.begin()) &&
             (i != theInputObjectList.end()) )
         {
            ConnectableObjectList oldInputs =
            theInputObjectList;
            
            rspfRefPtr<rspfConnectableObject> obj = (*i).get();               
            theInputObjectList.erase(i);
            theInputObjectList.insert(theInputObjectList.begin(), obj.get());
            result = true;
            
            ConnectableObjectList newInputs =
            theInputObjectList;
            
            rspfConnectionEvent event(
                                       this,
                                       RSPF_EVENT_CONNECTION_CONNECT_ID,
                                       newInputs,
                                       oldInputs,
                                       rspfConnectionEvent::RSPF_INPUT_DIRECTION);
            fireEvent(event);
         }
      }
   }
   
   return result;
}

bool rspfConnectableObject::moveInputToBottom(const rspfId& id)
{
   bool result = false;
   
   if (theInputListIsFixedFlag == false)
   {
      if ( theInputObjectList.size() )
      {
         ConnectableObjectList::iterator bottom =
         theInputObjectList.end()-1;
         
         // if not bottom already
         if ( (*bottom)->getId() != id ) 
         {
            ConnectableObjectList::iterator i =
            theInputObjectList.begin();
            
            while (i != bottom)
            {
               if ( (*i)->getId() == id )
               {
                  break;
               }
               ++i;
            }
            
            if (i != bottom)
            {
               ConnectableObjectList oldInputs =
               theInputObjectList;
               
               rspfRefPtr<rspfConnectableObject> obj = (*i).get();
               theInputObjectList.erase(i);
               theInputObjectList.push_back(obj);
               result = true;
               
               ConnectableObjectList newInputs =
               theInputObjectList;
               
               rspfConnectionEvent event(
                                          this,
                                          RSPF_EVENT_CONNECTION_CONNECT_ID,
                                          newInputs,
                                          oldInputs,
                                          rspfConnectionEvent::RSPF_INPUT_DIRECTION);
               fireEvent(event);
            }
         }
      }
   }
   
   return result;
}

void rspfConnectableObject::accept(rspfVisitor& visitor)
{
   if(!visitor.stopTraversal())
   {
      if(!visitor.hasVisited(this))
      {
         visitor.visit(this);
      }
      
      if(!visitor.stopTraversal())
      {
         
         if(visitor.getVisitorType() & rspfVisitor::VISIT_INPUTS)
         {
            ConnectableObjectList::iterator current = theInputObjectList.begin();
            while(current != theInputObjectList.end())
            {
               if((*current).get()&&!visitor.hasVisited((*current).get())) (*current)->accept(visitor);
               ++current;
            }
         }
         
         if(visitor.getVisitorType() & rspfVisitor::VISIT_OUTPUTS)
         {
            // go through the outputs
            ConnectableObjectList::iterator current = theOutputObjectList.begin();
            while(current != theOutputObjectList.end())
            {
               if((*current).get()&&!visitor.hasVisited((*current).get())) (*current)->accept(visitor);
               ++current;
            }
            rspfConnectableObject* obj = dynamic_cast<rspfConnectableObject*>(theOwner);
            
            if((!getNumberOfOutputs()||!isConnected(CONNECTABLE_DIRECTION_OUTPUT))&&obj)
            {
               rspfVisitor::VisitorType currentType = visitor.getVisitorType();
               // lets make sure inputs and outputs are turned off for we are traversing all children and we should not have
               // to have that enabled
               //
               visitor.turnOffVisitorType(rspfVisitor::VISIT_INPUTS);// |rspfVisitor::VISIT_CHILDREN);
               
               //obj->accept(visitor);
               visitor.setVisitorType(currentType);
              // visitor.turnOffVisitorType(rspfVisitor::VISIT_INPUTS);
               // now go through outputs
               //
               ConnectableObjectList::iterator current = obj->theOutputObjectList.begin();
               while(current != obj->theOutputObjectList.end())
               {
                  if((*current).get()&&!visitor.hasVisited((*current).get())) (*current)->accept(visitor);
                  ++current;
               }
               
               visitor.setVisitorType(currentType);
               
            }
         } 
      }  
   }
}

void rspfConnectableObject::setId(const rspfId& id)
{
   theId = id;
}

const rspfId& rspfConnectableObject::getId()const
{
   return theId;
}

const rspfObject* rspfConnectableObject::getOwner() const
{
   return theOwner;
}

rspf_uint32 rspfConnectableObject::getNumberOfInputs()const
{
   return (rspf_uint32)theInputObjectList.size();
}

rspf_uint32 rspfConnectableObject::getNumberOfOutputs()const
{
   return (rspf_uint32)theOutputObjectList.size();
}

bool rspfConnectableObject::getInputListIsFixedFlag()const
{
   return theInputListIsFixedFlag;
}

bool rspfConnectableObject::getOutputListIsFixedFlag()const
{
   return theOutputListIsFixedFlag;
}

const rspfConnectableObject::ConnectableObjectList& rspfConnectableObject::getInputList()const
{
   return theInputObjectList;
}

const rspfConnectableObject::ConnectableObjectList& rspfConnectableObject::getOutputList()const
{
   return theOutputObjectList;
}

rspfConnectableObject::ConnectableObjectList& rspfConnectableObject::getInputList()
{
   return theInputObjectList;
}

rspfConnectableObject::ConnectableObjectList& rspfConnectableObject::getOutputList()
{
   return theOutputObjectList;
}
