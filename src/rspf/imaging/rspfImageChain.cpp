//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfImageChain.cpp 21850 2012-10-21 20:09:55Z dburken $

#include <rspf/imaging/rspfImageChain.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/base/rspfConnectableContainer.h>
#include <rspf/base/rspfDrect.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfEventIds.h>
#include <rspf/base/rspfObjectEvents.h>
#include <rspf/base/rspfIdManager.h>
#include <rspf/base/rspfVisitor.h>
#include <rspf/imaging/rspfImageData.h>
#include <rspf/imaging/rspfImageGeometry.h>
#include <algorithm>
#include <iostream>
#include <iterator>

static rspfTrace traceDebug("rspfImageChain");


RTTI_DEF3(rspfImageChain, "rspfImageChain", rspfImageSource,
          rspfConnectableObjectListener, rspfConnectableContainerInterface);

void rspfImageChain::processEvent(rspfEvent& event)
{
   rspfConnectableObjectListener::processEvent(event);
   rspfConnectableObject* obj = PTR_CAST(rspfConnectableObject, event.getCurrentObject());
   
   if((rspfConnectableObject*)getFirstSource() == obj)
   {
      if(event.isPropagatingToOutputs())
      {
         rspfConnectableObject::ConnectableObjectList& outputList = getOutputList();
         rspf_uint32 idx = 0;
         for(idx = 0; idx < outputList.size();++idx)
         {
            if(outputList[idx].valid())
            {
               outputList[idx]->fireEvent(event);
               outputList[idx]->propagateEventToOutputs(event);
            }
         }
      }
   }
}

rspfImageChain::rspfImageChain()
:rspfImageSource(0,
                  0, // number of inputs
                  0, // number of outputs
                  false, // input's fixed
                  false), // outputs are not fixed
    rspfConnectableContainerInterface((rspfObject*)NULL),
    theBlankTile(NULL),
    theLoadStateFlag(false)
{
   rspfConnectableContainerInterface::theBaseObject = this;
   //thePropagateEventFlag = false;
   addListener((rspfConnectableObjectListener*)this);
}

rspfImageChain::~rspfImageChain()
{
   removeListener((rspfConnectableObjectListener*)this);
   deleteList();
}

bool rspfImageChain::addFirst(rspfConnectableObject* obj)
{
   rspfConnectableObject* rightOfThisObj =
      (rspfConnectableObject*)getFirstObject();

   return insertRight(obj, rightOfThisObj);
}

bool rspfImageChain::addLast(rspfConnectableObject* obj)
{
   if(imageChainList().size() > 0)
   {
      rspfConnectableObject* lastSource = imageChainList()[ imageChainList().size() -1].get();
//      if(dynamic_cast<rspfImageSource*>(obj)&&lastSource)
      if(lastSource)
      {
//         obj->disconnect();
         rspfConnectableObject::ConnectableObjectList tempIn = getInputList();
         lastSource->disconnectAllInputs();
         lastSource->connectMyInputTo(obj);
         obj->changeOwner(this);
         obj->connectInputList(tempIn);

         tempIn = obj->getInputList();
         theInputListIsFixedFlag = obj->getInputListIsFixedFlag();
         setNumberOfInputs(obj->getNumberOfInputs());
         imageChainList().push_back(obj);
         
         obj->addListener((rspfConnectableObjectListener*)this);
         // Send an event to any listeners.
         rspfContainerEvent event((rspfObject*)this,
                                   RSPF_EVENT_ADD_OBJECT_ID);
         event.setObjectList(obj);
         fireEvent(event);
         return true;
      }
   }
   else
   {
      return add(obj);
   }

   return false;;
}

rspfImageSource* rspfImageChain::getFirstSource()
{
   if(imageChainList().size()>0)
   {
      return dynamic_cast<rspfImageSource*>(imageChainList()[0].get());
   }

   return 0;
}

const rspfImageSource* rspfImageChain::getFirstSource() const
{
   if(imageChainList().size()>0)
      return dynamic_cast<const rspfImageSource*>(imageChainList()[0].get());

   return 0;
}

rspfObject* rspfImageChain::getFirstObject()
{
   if(imageChainList().size()>0)
   {
      return dynamic_cast<rspfImageSource*>(imageChainList()[0].get());
   }

   return 0;
}

rspfImageSource* rspfImageChain::getLastSource()
{
   if(imageChainList().size()>0)
   {
      return dynamic_cast<rspfImageSource*>((*(imageChainList().end()-1)).get());
   }

   return NULL;
}

const rspfImageSource* rspfImageChain::getLastSource() const
{
   if(imageChainList().size()>0)
      return dynamic_cast<const rspfImageSource*>((*(imageChainList().end()-1)).get());

   return NULL;
}

rspfObject* rspfImageChain::getLastObject()
{
   if(imageChainList().size()>0)
   {
      return dynamic_cast<rspfImageSource*>((*(imageChainList().end()-1)).get());
   }

   return 0;
}

rspfConnectableObject* rspfImageChain::findObject(const rspfId& id,
                                                    bool /* recurse */)
{
   std::vector<rspfRefPtr<rspfConnectableObject> >::iterator current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      if((*current).get())
      {
         if(id == (*current)->getId())
         {
            return (*current).get();
         }
      }
      
      ++current;
   }
   
   current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface,
                                                         (*current).get());
      
      if(child)
      {
         rspfConnectableObject* object = child->findObject(id, true);

         if(object) return object;
      }
      ++current;
   }
   
   return NULL;
}

rspfConnectableObject* rspfImageChain::findObject(const rspfConnectableObject* obj,
                                                    bool /* recurse */)
{
   std::vector<rspfRefPtr<rspfConnectableObject> >::iterator current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      if((*current).valid())
      {
         if(obj == (*current).get())
         {
            return (*current).get();
         }
      }
      
      ++current;
   }
   
   current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).get());
      
      if(child)
      {
         rspfConnectableObject* object = child->findObject(obj, true);

         if(object) return object;
      }
      ++current;
   }
   
   return 0;
}

rspfConnectableObject* rspfImageChain::findFirstObjectOfType(const RTTItypeid& typeInfo,
                                                               bool recurse)
{
   vector<rspfRefPtr<rspfConnectableObject> >::iterator current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      if((*current).valid()&&
         (*current)->canCastTo(typeInfo))
      {
         return (*current).get();
      }
      ++current;
   }

   if(recurse)
   {
      current =  imageChainList().begin();
      while(current != imageChainList().end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, 
                                                            (*current).get());
         
         if(child)
         {
            rspfConnectableObject* temp = child->findFirstObjectOfType(typeInfo, recurse);
            if(temp)
            {
               return temp;
            }
         }
         ++current;
      }
   }
   
   return (rspfConnectableObject*)NULL;   
}

rspfConnectableObject* rspfImageChain::findFirstObjectOfType(const rspfString& className,
                                                               bool recurse)
{
   vector<rspfRefPtr<rspfConnectableObject> >::iterator current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      if((*current).valid()&&
         (*current)->canCastTo(className) )
      {
         return (*current).get();
      }
      ++current;
   }

   if(recurse)
   {
      current =  imageChainList().begin();
      while(current != imageChainList().end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).get());
         
         if(child)
         {
            rspfConnectableObject* temp = child->findFirstObjectOfType(className, recurse);
            if(temp)
            {
               return temp;
            }
         }
         ++current;
      }
   }
   
   return (rspfConnectableObject*)0;   
}

rspfConnectableObject::ConnectableObjectList rspfImageChain::findAllObjectsOfType(const RTTItypeid& typeInfo,
                                                                           bool recurse)
{
   rspfConnectableObject::ConnectableObjectList result;
   rspfConnectableObject::ConnectableObjectList::iterator current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      if((*current).valid()&&
         (*current)->canCastTo(typeInfo))
      {
         rspfConnectableObject::ConnectableObjectList::iterator iter = std::find(result.begin(), 
                                                                                      result.end(), 
                                                                                      (*current).get());
         if(iter == result.end())
         {
            result.push_back((*current).get());
         }
      }
      ++current;
   }

   if(recurse)
   {
      current =  imageChainList().begin();
      while(current != imageChainList().end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).get());
         
         if(child)
         {
            rspfConnectableObject::ConnectableObjectList temp;
            temp = child->findAllObjectsOfType(typeInfo, recurse);
            for(long index=0; index < (long)temp.size();++index)
            {
               rspfConnectableObject::ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), temp[index]);
               if(iter == result.end())
               {
                  result.push_back(temp[index]);
               }
            }
         }
         ++current;
      }
   }
   
   return result;
   
}

rspfConnectableObject::ConnectableObjectList rspfImageChain::findAllObjectsOfType(const rspfString& className,
                                                                           bool recurse)
{
   rspfConnectableObject::ConnectableObjectList result;
   rspfConnectableObject::ConnectableObjectList::iterator current =  imageChainList().begin();
   
   while(current != imageChainList().end())
   {
      if((*current).valid()&&
         (*current)->canCastTo(className))
      {
         rspfConnectableObject::ConnectableObjectList::iterator iter = std::find(result.begin(), 
                                                                         result.end(), 
                                                                         (*current).get());
         if(iter == result.end())
         {
            result.push_back((*current).get());
         }
      }
      ++current;
   }

   if(recurse)
   {
      current =  imageChainList().begin();
      while(current != imageChainList().end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).get());
         
         if(child)
         {
            rspfConnectableObject::ConnectableObjectList temp;
            temp = child->findAllObjectsOfType(className, recurse);
            for(long index=0; index < (long)temp.size();++index)
            {
               rspfConnectableObject::ConnectableObjectList::iterator iter = std::find(result.begin(), result.end(), temp[index]);
               if(iter == result.end())
               {
                  result.push_back(temp[index]);
               }
            }
         }
         ++current;
      }
   }
   
   return result;
   
}

void rspfImageChain::makeUniqueIds()
{
   setId(rspfIdManager::instance()->generateId());
   for(long index = 0; index < (long)imageChainList().size(); ++index)
   {
      rspfConnectableContainerInterface* container = PTR_CAST(rspfConnectableContainerInterface,
                                                               imageChainList()[index].get());
      if(container)
      {
         container->makeUniqueIds();
      }
      else
      {
         if(imageChainList()[index].valid())
         {
            imageChainList()[index]->setId(rspfIdManager::instance()->generateId());
         }
      }
   }
}

rspf_uint32 rspfImageChain::getNumberOfObjects(bool recurse)const
{
   rspf_uint32 result = (rspf_uint32)imageChainList().size();
   
   if(recurse)
   {
      for(rspf_uint32 i = 0; i < imageChainList().size(); ++i)
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, imageChainList()[i].get());
         if(child)
         {
            result += child->getNumberOfObjects(true);
         }
      }
   }
   
   return result;   
}

rspf_uint32 rspfImageChain::getNumberOfSources() const
{
   rspfNotify(rspfNotifyLevel_NOTICE)
      << "rspfImageChain::getNumberOfSources is deprecated!"
      << "\nUse: rspfImageChain::getNumberOfObjects(false)"
      << std::endl;
   return getNumberOfObjects(false);
}

bool rspfImageChain::addChild(rspfConnectableObject* object)
{
   return add(object);
}

bool rspfImageChain::removeChild(rspfConnectableObject* object)
{
   bool result = false;
   vector<rspfRefPtr<rspfConnectableObject> >::iterator current =  std::find(imageChainList().begin(), imageChainList().end(), object);
   
   
   if(current!=imageChainList().end())
   {
      result = true;
      object->removeListener((rspfConnectableObjectListener*)this);
      if(current == imageChainList().begin())
      {
         object->removeListener((rspfConnectableObjectListener*)this);
      }
      if(imageChainList().size() == 1)
      {
         object->changeOwner(0);
         current = imageChainList().erase(current);
      }
      else 
      {
         rspfConnectableObject::ConnectableObjectList input  = object->getInputList();
         rspfConnectableObject::ConnectableObjectList output = object->getOutputList();
         object->changeOwner(0);// set the owner to 0
         bool erasingBeginning = (current == imageChainList().begin());
         // bool erasingEnd = (current+1) == imageChainList().end();
         current = imageChainList().erase(current);
         object->disconnect();
         
         if(!imageChainList().empty())
         {
            if(erasingBeginning) // the one that receives the first getTile
            {
               (*imageChainList().begin())->addListener(this);
            }
            
            else if(current==imageChainList().end()) // one that receives the last getTIle
            {
               current = imageChainList().begin()+(imageChainList().size()-1);
               (*current)->connectInputList(input);
               theInputObjectList = (*current)->getInputList();
               theInputListIsFixedFlag = (*current)->getInputListIsFixedFlag();
            }
            else
            {
               // prepare interior setup and removal and connect surrounding nodes
               // take the  outputs of the node we are removing and connect them to the old inputs 
               rspf_uint32 outIndex = 0;
               for(outIndex = 0; outIndex < output.size();++outIndex)
               {
                  output[outIndex]->connectInputList(input);
               }
            }
         }
      }

      // Send an event to any listeners.
      rspfContainerEvent event((rspfObject*)this,
                                RSPF_EVENT_REMOVE_OBJECT_ID);
      event.setObjectList(object);
      fireEvent(event);
   }
   
   return result;
}

rspfConnectableObject* rspfImageChain::removeChild(const rspfId& id)
{
   rspfIdVisitor visitor( id,
                           (rspfVisitor::VISIT_CHILDREN|rspfVisitor::VISIT_INPUTS ) );
   accept( visitor );
   rspfConnectableObject* obj = visitor.getObject();
   if ( obj )
   {
      removeChild(obj);
   }
   return obj;
}

void rspfImageChain::getChildren(vector<rspfConnectableObject*>& children,
                                  bool immediateChildrenOnlyFlag)
{
   rspf_uint32 i = 0;
   
   vector<rspfConnectableObject*> temp;
   for(i = 0; i < imageChainList().size();++i)
   {
      temp.push_back(imageChainList()[i].get());
   }

   for(i = 0; i < temp.size();++i)
   {
      rspfConnectableContainerInterface* interface = PTR_CAST(rspfConnectableContainerInterface,
                                                               temp[i]);
      if(immediateChildrenOnlyFlag)
      {
         children.push_back(temp[i]);
      }
      else if(!interface)
      {
         children.push_back(temp[i]);         
      }
   }
   
   if(!immediateChildrenOnlyFlag)
   {
      for(i = 0; i < temp.size();++i)
      {
         rspfConnectableContainerInterface* interface = PTR_CAST(rspfConnectableContainerInterface,
                                                                  temp[i]);
         if(interface)
         {
            interface->getChildren(children, false);
         }
      }
   }
}

bool rspfImageChain::add(rspfConnectableObject* source)
{
   bool result = false;
//   if(PTR_CAST(rspfImageSource, source))
   {
     source->changeOwner(this);
     if(imageChainList().size() > 0)
     {
        source->disconnectAllOutputs();
        theOutputListIsFixedFlag = source->getOutputListIsFixedFlag();
        imageChainList()[0]->removeListener(this);
        imageChainList().insert(imageChainList().begin(), source);
        imageChainList()[0]->addListener(this);
        source->addListener((rspfConnectableObjectListener*)this);
        imageChainList()[0]->connectMyInputTo(imageChainList()[1].get());
        result = true;
     }
     else
     {
        theInputListIsFixedFlag = false;
        theOutputListIsFixedFlag = false;
        
        if(!theInputObjectList.empty())
        {
           source->connectInputList(getInputList());
        }
        theInputObjectList = source->getInputList();
        theInputListIsFixedFlag = source->getInputListIsFixedFlag();
     //   theOutputObjectList     = source->getOutputList();
     //   theOutputListIsFixedFlag= source->getOutputListIsFixedFlag();
        imageChainList().push_back(source);
        source->addListener((rspfConnectableObjectListener*)this);
        source->addListener(this);
        result = true;
     }
   }

   if (result && source)
   {
      rspfContainerEvent event(this, RSPF_EVENT_ADD_OBJECT_ID);
      event.setObjectList(source);
      fireEvent(event);
   }
   
   return result;
}

bool rspfImageChain::insertRight(rspfConnectableObject* newObj,
                                  rspfConnectableObject* rightOfThisObj)
{
   if(!newObj&&!rightOfThisObj) return false;
   if(!imageChainList().size())
   {
      return add(newObj);
   }
   std::vector<rspfRefPtr<rspfConnectableObject> >::iterator iter = std::find(imageChainList().begin(), imageChainList().end(), rightOfThisObj);
   
   if(iter!=imageChainList().end())
   {
      if(iter == imageChainList().begin())
      {
         return add(newObj);
      }
      else //if(PTR_CAST(rspfImageSource, newObj))
      {
         rspfConnectableObject::ConnectableObjectList outputList = rightOfThisObj->getOutputList();
         rightOfThisObj->disconnectAllOutputs();

         // Core dump fix.  Connect input prior to outputs. (drb)
         newObj->connectMyInputTo(rightOfThisObj); 
         newObj->connectOutputList(outputList);
         newObj->changeOwner(this);
         newObj->addListener((rspfConnectableObjectListener*)this);
         imageChainList().insert(iter, newObj);
         // Send event to any listeners.
         rspfContainerEvent event(this, RSPF_EVENT_ADD_OBJECT_ID);
         event.setObjectList(newObj);
         fireEvent(event);
         return true;
      }
   }

   return false;
}

bool rspfImageChain::insertRight(rspfConnectableObject* newObj,
                                  const rspfId& id)
{

#if 1
   rspfIdVisitor visitor( id, rspfVisitor::VISIT_CHILDREN );
   accept( visitor );
   rspfConnectableObject* obj = visitor.getObject();
   if ( obj )
   {
      return insertRight(newObj, obj);
   }
   return false;
#else
   rspfConnectableObject* obj = findObject(id, false);
   if(obj)
   {
      return insertRight(newObj, obj);
   }

   return false;
#endif
}

bool rspfImageChain::insertLeft(rspfConnectableObject* newObj,
                                 rspfConnectableObject* leftOfThisObj)
{
   if(!newObj&&!leftOfThisObj) return false;
   if(!imageChainList().size())
   {
      return add(newObj);
   }
   std::vector<rspfRefPtr<rspfConnectableObject> >::iterator iter = std::find(imageChainList().begin(), imageChainList().end(), leftOfThisObj);
   if(iter!=imageChainList().end())
   {
      if((iter+1)==imageChainList().end())
      {
         return addLast(newObj);
      }
      else
      {
         rspfConnectableObject::ConnectableObjectList inputList = leftOfThisObj->getInputList();
         
         newObj->connectInputList(inputList);
         
         leftOfThisObj->disconnectAllInputs();
         leftOfThisObj->connectMyInputTo(newObj);
         newObj->changeOwner(this);
         newObj->addListener((rspfConnectableObjectListener*)this);
         imageChainList().insert(iter+1, newObj);
         // Send an event to any listeners.
         rspfContainerEvent event(this, RSPF_EVENT_ADD_OBJECT_ID);
         event.setObjectList(newObj);
         fireEvent(event);
         return true;
      }
   }

   return false;
}

bool rspfImageChain::insertLeft(rspfConnectableObject* newObj,
                                 const rspfId& id)
{
#if 1
   rspfIdVisitor visitor( id,
                           rspfVisitor::VISIT_CHILDREN|rspfVisitor::VISIT_INPUTS);
   accept( visitor );
   rspfConnectableObject* obj = visitor.getObject();
   if ( obj )
   {
      return insertLeft(newObj, obj);
   }
   return false;
#else
   rspfConnectableObject* obj = findObject(id, false);
   if(obj)
   {
      return insertLeft(newObj, obj);
   }
   return false;
#endif   
}

bool rspfImageChain::replace(rspfConnectableObject* newObj,
                              rspfConnectableObject* oldObj)
{
   rspf_int32 idx = indexOf(oldObj);
   if(idx >= 0)
   {
      rspfConnectableObject::ConnectableObjectList& inputList  = oldObj->getInputList();
      rspfConnectableObject::ConnectableObjectList& outputList = oldObj->getOutputList();
      oldObj->removeListener((rspfConnectableObjectListener*)this);
      oldObj->removeListener(this);
      oldObj->changeOwner(0);
      imageChainList()[idx] = newObj;
      newObj->connectInputList(inputList);
      newObj->connectOutputList(outputList);
      newObj->changeOwner(this);
      newObj->addListener((rspfConnectableObjectListener*)this);
      if(idx == 0)
      {
         newObj->addListener(this);
      }
   }
   
   return (idx >= 0);
}

rspfRefPtr<rspfImageData> rspfImageChain::getTile(
   const rspfIrect& tileRect,
   rspf_uint32 resLevel)
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* inputSource = PTR_CAST(rspfImageSource,
                                             imageChainList()[0].get());
      
      if(inputSource)
      {
         // make sure we initialize in reverse order.
         // some source may depend on the initialization of
         // its inputs
         return inputSource->getTile(tileRect, resLevel);
      }  
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* inputSource = PTR_CAST(rspfImageSource, getInput(0));
         if(inputSource)
         {
            rspfRefPtr<rspfImageData> inputTile = inputSource->getTile(tileRect, resLevel);
//            if(inputTile.valid())
//            {
//               std::cout << *(inputTile.get()) << std::endl;
//            }
            return inputTile.get();
         }
      }
   }
//   std::cout << "RETURNING A BLANK TILE!!!!" << std::endl;
/*
   if(theBlankTile.get())
   {
      theBlankTile->setImageRectangle(tileRect);
   }
   return theBlankTile;
 */
   return 0;
}

rspf_uint32 rspfImageChain::getNumberOfInputBands() const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());
      if(interface)
      {
         return interface->getNumberOfOutputBands();
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getNumberOfOutputBands();
         }
      }
   }

   return 0;
}

double rspfImageChain::getNullPixelValue(rspf_uint32 band)const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());
      if(interface)
      {
         return interface->getNullPixelValue(band);
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getNullPixelValue(band);
         }
      }
   }
   
   return rspf::defaultNull(getOutputScalarType());
}

double rspfImageChain::getMinPixelValue(rspf_uint32 band)const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());
      if(interface)
      {
         return interface->getMinPixelValue(band);
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getMinPixelValue(band);
         }
      }
   }

   return rspf::defaultMin(getOutputScalarType());
}

double rspfImageChain::getMaxPixelValue(rspf_uint32 band)const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* inter = PTR_CAST(rspfImageSource,
                                                  imageChainList()[0].get());
      if(inter)
      {
         return inter->getMaxPixelValue(band);
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getMaxPixelValue(band);
         }
      }
   }

   return rspf::defaultMax(getOutputScalarType());
}

void rspfImageChain::getOutputBandList(std::vector<rspf_uint32>& bandList) const
{
   if( (imageChainList().size() > 0) && isSourceEnabled() )
   {
      rspfRefPtr<const rspfImageSource> inter =
         dynamic_cast<const rspfImageSource*>( imageChainList()[0].get() );
      if( inter.valid() )
      {
         // cout << "cn: " << inter->getClassName() << endl;
         inter->getOutputBandList(bandList);
      }
   }
}
   
rspfScalarType rspfImageChain::getOutputScalarType() const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());
      if(interface)
      {
         return interface->getOutputScalarType();
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getOutputScalarType();
         }
      }
   }
   
   return RSPF_SCALAR_UNKNOWN;
}

rspf_uint32 rspfImageChain::getTileWidth()const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         imageChainList()[0].get());
         if(interface)
         {
            return interface->getTileWidth();;
         }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getTileWidth();
         }
      }
   }
   
   return 0;   
}

rspf_uint32 rspfImageChain::getTileHeight()const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         imageChainList()[0].get());
         if(interface)
         {
            return interface->getTileHeight();
         }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getTileHeight();
         }
      }
   }
   
   return 0;   
}
   
rspfIrect rspfImageChain::getBoundingRect(rspf_uint32 resLevel)const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {

      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());

      if(interface)
      {
         return interface->getBoundingRect(resLevel);
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getBoundingRect();
         }
      }
   }
   rspfDrect rect;
   rect.makeNan();
   
   return rect;
   
}

void rspfImageChain::getValidImageVertices(vector<rspfIpt>& validVertices,
                                            rspfVertexOrdering ordering,
                                            rspf_uint32 resLevel)const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface =PTR_CAST(rspfImageSource,
                                                     imageChainList()[0].get());

      if(interface)
      {
         interface->getValidImageVertices(validVertices,
                                          ordering,
                                          resLevel);
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            interface->getValidImageVertices(validVertices,
                                             ordering,
                                             resLevel);
         }
      }
   }
}

rspfRefPtr<rspfImageGeometry> rspfImageChain::getImageGeometry()
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource, imageChainList()[0].get());
      if( interface )
      {
         return interface->getImageGeometry();
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource, getInput(0));
         if(interface)
         {
            return interface->getImageGeometry();
         }
      }
   }
   return rspfRefPtr<rspfImageGeometry>();
}

void rspfImageChain::getDecimationFactor(rspf_uint32 resLevel,
                                          rspfDpt& result) const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());
      if(interface)
      {
         interface->getDecimationFactor(resLevel,
                                        result);
         return;
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            interface->getDecimationFactor(resLevel, result);
            return;
         }
      }
   }

   result.makeNan();
}

void rspfImageChain::getDecimationFactors(vector<rspfDpt>& decimations) const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());
      if(interface)
      {
         interface->getDecimationFactors(decimations);
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            interface->getDecimationFactors(decimations);
            return;
         }
      }
   }
}

rspf_uint32 rspfImageChain::getNumberOfDecimationLevels()const
{
   if((imageChainList().size() > 0)&&(isSourceEnabled()))
   {
      rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                      imageChainList()[0].get());

      if(interface)
      {
         return interface->getNumberOfDecimationLevels();
      }
   }
   else
   {
      if(getInput(0))
      {
         rspfImageSource* interface = PTR_CAST(rspfImageSource,
                                                         getInput(0));
         if(interface)
         {
            return interface->getNumberOfDecimationLevels();
         }
      }
   }

   return 1;
}

bool rspfImageChain::addAllSources(map<rspfId, vector<rspfId> >& idMapping,
                                    const rspfKeywordlist& kwl,
                                    const char* prefix)
{
   static const char* MODULE = "rspfImageChain::addAllSources";
   rspfString copyPrefix = prefix;
   bool result =  rspfImageSource::loadState(kwl, copyPrefix.c_str());

   if(!result)
   {
      return result;
   }
   long index = 0;

//   rspfSource* source = NULL;

   vector<rspfId> inputConnectionIds;
   rspfString regExpression =  rspfString("^(") + copyPrefix + "object[0-9]+.)";
   vector<rspfString> keys =
      kwl.getSubstringKeyList( regExpression );
   long numberOfSources = (long)keys.size();//kwl.getNumberOfSubstringKeys(regExpression);

   int offset = (int)(copyPrefix+"object").size();
   int idx = 0;
   std::vector<int> theNumberList(numberOfSources);
   for(idx = 0; idx < (int)theNumberList.size();++idx)
     {
       rspfString numberStr(keys[idx].begin() + offset,
			     keys[idx].end());
       theNumberList[idx] = numberStr.toInt();
     }
   std::sort(theNumberList.begin(), theNumberList.end());
   for(idx=0;idx < (int)theNumberList.size();++idx)
   {
      rspfString newPrefix = copyPrefix;
      newPrefix += rspfString("object");
      newPrefix += rspfString::toString(theNumberList[idx]);
      newPrefix += rspfString(".");

      if(traceDebug())
      {
         CLOG << "trying to create source with prefix: " << newPrefix
              << std::endl;
      }
      rspfRefPtr<rspfObject> object = rspfObjectFactoryRegistry::instance()->createObject(kwl,
                                                                                 newPrefix.c_str());
      rspfConnectableObject* source = PTR_CAST(rspfConnectableObject, object.get());
      
      if(source)
      {
         // we did find a source so include it in the count
         if(traceDebug())
         {
            CLOG << "Created source with prefix: " << newPrefix << std::endl;
         }
         //if(PTR_CAST(rspfImageSource, source))
         {
            rspfId id = source->getId();
            inputConnectionIds.clear();
            
            findInputConnectionIds(inputConnectionIds,
                                   kwl,
                                   newPrefix);
            if(inputConnectionIds.size() == 0)
            {
               // we will try to do a default connection
               //
               
               if(imageChainList().size())
               {
                  if(traceDebug())
                  {
                     CLOG << "connecting " << source->getClassName() << " to "
                          << imageChainList()[0]->getClassName() << std::endl;
                  }
                  source->connectMyInputTo(0, imageChainList()[0].get());
               }
            }
            else
            {
               // we remember the connection id's so we can connect this later.
               // this way we make sure all sources were actually
               // allocated.
               //
               idMapping.insert(std::make_pair(id, inputConnectionIds));
            }
            add(source);
         }
      //   else
      //   {
            source = 0;
      //   }
      }
      else
      {
         object = 0;
         source = 0;
      }
      
      ++index;
   }
   if(imageChainList().size())
   {
     rspfConnectableObject* obj = imageChainList()[(rspf_int32)imageChainList().size()-1].get();
     if(obj)
     {
        setNumberOfInputs(obj->getNumberOfInputs());
     }
   }
   
   return result;
}

void rspfImageChain::findInputConnectionIds(vector<rspfId>& result,
                                             const rspfKeywordlist& kwl,
                                             const char* prefix)
{
   rspfString copyPrefix = prefix;
   rspf_uint32 idx = 0;
   
   rspfString regExpression =  rspfString("^") + rspfString(prefix) + "input_connection[0-9]+";
   vector<rspfString> keys =
      kwl.getSubstringKeyList( regExpression );
   
   rspf_int32 offset = (rspf_int32)(copyPrefix+"input_connection").size();
   rspf_uint32 numberOfKeys = (rspf_uint32)keys.size();
   std::vector<int> theNumberList(numberOfKeys);
   for(idx = 0; idx < theNumberList.size();++idx)
   {
      rspfString numberStr(keys[idx].begin() + offset,
                            keys[idx].end());
      theNumberList[idx] = numberStr.toInt();
   }
   std::sort(theNumberList.begin(), theNumberList.end());
   copyPrefix += rspfString("input_connection");
   for(idx=0;idx < theNumberList.size();++idx)
   {
      const char* lookup = kwl.find(copyPrefix,rspfString::toString(theNumberList[idx]));
      if(lookup)
      {
         long id = rspfString(lookup).toLong();
         result.push_back(rspfId(id));
      }
   }
}
                    

bool rspfImageChain::connectAllSources(const map<rspfId, vector<rspfId> >& idMapping)
{
   // cout << "this->getId(): " << this->getId() << endl;
   
   if(idMapping.size())
   {
      map<rspfId, vector<rspfId> >::const_iterator iter = idMapping.begin();

      
      while(iter != idMapping.end())
      {
         // cout << "(*iter).first): " << (*iter).first << endl;
#if 0
         rspfConnectableObject* currentSource = findObject((*iter).first);
#else
         rspfIdVisitor visitor( (*iter).first,
                                 (rspfVisitor::VISIT_CHILDREN ) );
                                  // rspfVisitor::VISIT_INPUTS ) );
         accept( visitor );
         rspfConnectableObject* currentSource = visitor.getObject();
#endif

         if(currentSource)
         {
            // cout << "currentSource->getClassName: " << currentSource->getClassName() << endl;
            long upperBound = (long)(*iter).second.size();
            for(long index = 0; index < upperBound; ++index)
            {
               //cout << "(*iter).second[index]: " << (*iter).second[index] << endl;
               
               if((*iter).second[index].getId() > -1)
               {
#if 0
                  rspfConnectableObject* inputSource =
                      PTR_CAST(rspfConnectableObject, findObject((*iter).second[index]));
#else
                  visitor.reset();
                  visitor.setId( (*iter).second[index] );
                  accept( visitor );
                  rspfConnectableObject* inputSource = visitor.getObject();
#endif
                  // cout << "inputSource is " << (inputSource?"good...":"null...") << endl;
                  if ( inputSource )
                  {
                     // cout << "inputSource->getClassName(): " << inputSource->getClassName() << endl;
 
                     // Check for connection to self.
                     if ( this != inputSource )
                     {
                        currentSource->connectMyInputTo(index, inputSource);
                     }
                     // else warning???
                  }
               }
               else // -1 id
               {
                  currentSource->disconnectMyInput((rspf_int32)index);
               }
            }
         }
         else
         {
            cerr << "Could not find " << (*iter).first << " for source: ";
            return false;
         }
         ++iter;
      }
   }

   // abort();
   return true;
}

bool rspfImageChain::saveState(rspfKeywordlist& kwl,
                                const char* prefix)const
{
   bool result = true;
   
   result = rspfImageSource::saveState(kwl, prefix);

   if(!result)
   {
      return result;
   }
   rspf_uint32 upper = (rspf_uint32)imageChainList().size();
   rspf_uint32 counter = 1;

   if (upper)
   {
      rspf_int32 idx = upper-1;
      // start with the tail and go to the head fo the list.
      for(;((idx >= 0)&&result);--idx, ++counter)
      {
         rspfString newPrefix = prefix;
         
         newPrefix += (rspfString("object") +
                       rspfString::toString(counter) +
                       rspfString("."));
         result = imageChainList()[idx]->saveState(kwl, newPrefix.c_str());
      }
   }

   return result;
}

bool rspfImageChain::loadState(const rspfKeywordlist& kwl,
                                const char* prefix)
{
   static const char* MODULE = "rspfImageChain::loadState(kwl, prefix)";
   deleteList();

   rspfImageSource::loadState(kwl, prefix);
   
   
   theLoadStateFlag = true;
   bool result = true;
   
   map<rspfId, vector<rspfId> > idMapping;
   result = addAllSources(idMapping, kwl, prefix);
   if(!result)
   {
      CLOG << "problems adding sources" << std::endl;
   }
   result = connectAllSources(idMapping);
   if(!result)
   {
      CLOG << "problems connecting sources" << std::endl;
   }
   
   theLoadStateFlag = false;
   return result;
}


void rspfImageChain::initialize()
{
   static const char* MODULE = "rspfImageChain::initialize()";
   if (traceDebug()) CLOG << " Entered..." << std::endl;
   
   long upper = (rspf_uint32)imageChainList().size();
   
   for(long index = upper - 1; index >= 0; --index)
   {
      if(traceDebug())
      {
         CLOG << "initilizing source: "
              << imageChainList()[index]->getClassName()
              << std::endl;
      }
      if(imageChainList()[index].valid())
      {
         rspfSource* interface =
            PTR_CAST(rspfSource, imageChainList()[index].get());

         if(interface)
         {
            // make sure we initialize in reverse order.
            // some source may depend on the initialization of
            // its inputs
            interface->initialize();
         }
      }
   }
   if (traceDebug()) CLOG << " Exited..." << std::endl;
}

void rspfImageChain::enableSource()
{
   rspf_int32 upper = static_cast<rspf_int32>(imageChainList().size());
   rspf_int32 index = 0;
   for(index = upper - 1; index >= 0; --index)
   {
      // make sure we initialize in reverse order.
      // some source may depend on the initialization of
      // its inputs
     rspfSource* source = PTR_CAST(rspfSource, imageChainList()[index].get());
     if(source)
     {
        source->enableSource();
     }
   }
   
   theEnableFlag = true;
}

void rspfImageChain::disableSource()
{
   long upper = (rspf_uint32)imageChainList().size();
   
   for(long index = upper - 1; index >= 0; --index)
   {
      // make sure we initialize in reverse order.
      // some source may depend on the initialization of
      // its inputs
     rspfSource* source = PTR_CAST(rspfSource, imageChainList()[index].get());
     if(source)
     {
        source->disableSource();
     }
   }
   
   theEnableFlag = false;
}

void rspfImageChain::prepareForRemoval(rspfConnectableObject* connectableObject)
{
   if(connectableObject)
   {
      connectableObject->removeListener((rspfConnectableObjectListener*)this);
      connectableObject->changeOwner(0);
      connectableObject->disconnect();   
   }
}

bool rspfImageChain::deleteFirst()
{
   if (imageChainList().size() == 0) return false;

   rspfContainerEvent event(this, RSPF_EVENT_REMOVE_OBJECT_ID);
   prepareForRemoval(imageChainList()[0].get());
   // Clear any listeners, memory.
   event.setObjectList(imageChainList()[0].get());
   imageChainList()[0] = 0;
   // Remove from the vector.
   std::vector<rspfRefPtr<rspfConnectableObject> >::iterator i = imageChainList().begin();
   imageChainList().erase(i);
   fireEvent(event);
   return true;
}

bool rspfImageChain::deleteLast()
{
   if (imageChainList().size() == 0) return false;

   rspfContainerEvent event(this, RSPF_EVENT_REMOVE_OBJECT_ID);
  // Clear any listeners, memory.
   rspf_uint32 idx = (rspf_uint32)imageChainList().size() - 1;
   prepareForRemoval(imageChainList()[idx].get());
   event.setObjectList(imageChainList()[idx].get());
   imageChainList()[idx] = 0;
   // Remove from the vector.
   imageChainList().pop_back();
   fireEvent(event);
   return true; 
}

void rspfImageChain::deleteList()
{
   rspf_uint32 upper = (rspf_uint32) imageChainList().size();
   rspf_uint32 idx = 0;
   rspfContainerEvent event(this, RSPF_EVENT_REMOVE_OBJECT_ID);
   for(; idx < upper; ++idx)
   {
      if(imageChainList()[idx].valid())
      {
         event.getObjectList().push_back(imageChainList()[idx].get());
         prepareForRemoval(imageChainList()[idx].get());
         imageChainList()[idx] = 0;
      }
   }
  
   imageChainList().clear();
   fireEvent(event);
}


void rspfImageChain::disconnectInputEvent(rspfConnectionEvent& event)
{
   if(imageChainList().size())
   {
      if(event.getObject()==this)
      {
         if(imageChainList()[imageChainList().size()-1].valid())
         {
            for(rspf_uint32 i = 0; i < event.getNumberOfOldObjects(); ++i)
            {
               imageChainList()[imageChainList().size()-1]->disconnectMyInput(event.getOldObject(i));
            }
         }
      }
   }
}

void rspfImageChain::disconnectOutputEvent(rspfConnectionEvent& /* event */)
{
}

void rspfImageChain::connectInputEvent(rspfConnectionEvent& event)
{
   if(imageChainList().size())
   {
      if(event.getObject()==this)
      {
         if(imageChainList()[imageChainList().size()-1].valid())
         {
            for(rspf_uint32 i = 0; i < event.getNumberOfNewObjects(); ++i)
            {
               rspfConnectableObject* obj = event.getNewObject(i);
               imageChainList()[imageChainList().size()-1]->connectMyInputTo(findInputIndex(obj),
                                                                               obj,
                                                                               false);
            }
         }
      }
      else if(event.getObject() == imageChainList()[0].get())
      {
         if(!theLoadStateFlag)
         {
//            theInputObjectList = imageChainList()[0]->getInputList();
         }
      }
      initialize();
   }
}

void rspfImageChain::connectOutputEvent(rspfConnectionEvent& /* event */)
{
} 

// void rspfImageChain::propertyEvent(rspfPropertyEvent& event)
// {
//    if(imageChainList().size())
//    {
//       rspfConnectableObject* obj = PTR_CAST(rspfConnectableObject,
//                                              event.getObject());
      
//       if(obj)
//       {
//          rspfImageSource* interface = findSource(obj->getId());
         
//          if(interface)
//          {
//             rspfConnectableObject* obj = PTR_CAST(rspfConnectableObject,
//                                                    interface.getObject());
//             if(obj)
//             {
               
//             }
//          }
//       }
//    }
// }

void rspfImageChain::objectDestructingEvent(rspfObjectDestructingEvent& event)
{
   if(!event.getObject()) return;

   if(imageChainList().size()&&(event.getObject()!=this))
   {
      removeChild(PTR_CAST(rspfConnectableObject,
                           event.getObject()));
   }
}
void rspfImageChain::propagateEventToOutputs(rspfEvent& event)
{
   //if(thePropagateEventFlag) return;

   //thePropagateEventFlag = true;
   if(imageChainList().size())
   {
      if(imageChainList()[imageChainList().size()-1].valid())
      {
         imageChainList()[imageChainList().size()-1]->fireEvent(event);
         imageChainList()[imageChainList().size()-1]->propagateEventToOutputs(event);
      }
   }
   //rspfConnectableObject::propagateEventToOutputs(event);
  // thePropagateEventFlag = false;
}
void rspfImageChain::propagateEventToInputs(rspfEvent& event)
{
//   if(thePropagateEventFlag) return;

//   thePropagateEventFlag = true;
   if(imageChainList().size())
   {
      if(imageChainList()[0].valid())
      {
         imageChainList()[0]->fireEvent(event);
         imageChainList()[0]->propagateEventToInputs(event);
      }
   }
//   thePropagateEventFlag = false;
}

rspfConnectableObject* rspfImageChain::operator[](rspf_uint32 index)
{
   return getConnectableObject(index);
}

rspfConnectableObject* rspfImageChain::getConnectableObject(
   rspf_uint32 index)
{
   if(imageChainList().size() && (index < imageChainList().size()))
   {
      return imageChainList()[index].get();
   }
   
   return 0; 
}

rspf_int32 rspfImageChain::indexOf(rspfConnectableObject* obj)const
{
   rspf_uint32 idx = 0;
   
   for(idx = 0; idx < imageChainList().size();++idx)
   {
      if(imageChainList()[idx] == obj)
      {
         return (rspf_int32)idx;
      }
   }
   
   return -1;
}

void rspfImageChain::accept(rspfVisitor& visitor)
{
   if(!visitor.hasVisited(this))
   {
      visitor.visit(this);
      rspfVisitor::VisitorType currentType = visitor.getVisitorType();
      //---
      // Lets make sure inputs and outputs are turned off for we are traversing all children
      // and we should not have to have that enabled.
      //---
      visitor.turnOffVisitorType(rspfVisitor::VISIT_INPUTS|rspfVisitor::VISIT_OUTPUTS);
      if(visitor.getVisitorType() & rspfVisitor::VISIT_CHILDREN)
      {
         ConnectableObjectList::reverse_iterator current = imageChainList().rbegin();
         while((current != imageChainList().rend())&&!visitor.stopTraversal())
         {
            rspfRefPtr<rspfConnectableObject> currentObject = (*current);
            if(currentObject.get() && !visitor.hasVisited(currentObject.get())) currentObject->accept(visitor);
            ++current;
         }
      }
      visitor.setVisitorType(currentType);
      rspfConnectableObject::accept(visitor);
   }
}

//**************************************************************************************************
// Inserts all of its children and inputs into the container provided. Since rspfImageChain is
// itself a form of container, this method will consolidate this chain with the argument
// container. Therefore this chain object will not be represented in the container (but its
// children will be, with correct input and output connections to external objects).
// Returns TRUE if successful.
//**************************************************************************************************
bool rspfImageChain::fillContainer(rspfConnectableContainer& container)
{
   // Grab the first source in the chain and let it fill the container with itself and inputs. This
   // will traverse down the chain and will even pick up external sources that feed this chain:
   rspfRefPtr<rspfConnectableObject> first_source = getFirstSource();
   if (!first_source.valid())
      return false;
   first_source->fillContainer(container);

   // Instead of adding ourselves, make sure my first source is properly connected to my output,
   // thus obviating the need for this image chain (my chain items become part of 'container':
   ConnectableObjectList& obj_list = getOutputList();
   rspfRefPtr<rspfConnectableObject> output_connection = 0;
   while (!obj_list.empty())
   {
      // Always pick off the beginning of the list since it is shrinking with each disconnect:
      output_connection = obj_list[0];
      disconnectMyOutput(output_connection.get(), true, false);
      first_source->connectMyOutputTo(output_connection.get());
   }
   return true;
}
