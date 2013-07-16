//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfConnectableContainer.cpp 21850 2012-10-21 20:09:55Z dburken $

#include <rspf/base/rspfConnectableContainer.h>
#include <rspf/base/rspfIdManager.h>
#include <rspf/base/rspfObjectDestructingEvent.h>
#include <rspf/base/rspfPropertyEvent.h>
#include <rspf/base/rspfConnectionEvent.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfObjectFactoryRegistry.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfVisitor.h>
#include <algorithm>
#include <stack>

static rspfTrace traceDebug("rspfConnectableContainer:debug");

RTTI_DEF2(rspfConnectableContainer, "rspfConnectableContainer", rspfConnectableObject, rspfConnectableContainerInterface);

class rspfConnectableContainerChildListener : public rspfConnectableObjectListener
{
public:
   rspfConnectableContainerChildListener(rspfConnectableContainer* container)
      :theContainer(container),
       theEnabledFlag(true)
      {}
   void enableListener(){theEnabledFlag = true;}
   void disableListener(){theEnabledFlag = false;}

   virtual void objectDestructingEvent(rspfObjectDestructingEvent& event)
      {
         rspfConnectableObject* object = PTR_CAST(rspfConnectableObject, event.getObject());
         if(object&&theContainer)
         {
            if(theContainer->removeChild(object))
            {
               object->removeListener((rspfConnectableObjectListener*) this);
            }
         }
         propagatePropertyEvent(event);

      }
   virtual void disconnectInputEvent(rspfConnectionEvent& event)
      {
         propagatePropertyEvent(event);
      }
   virtual void connectInputEvent(rspfConnectionEvent& event)
      {
         propagatePropertyEvent(event);
      }

   void propagatePropertyEvent(rspfEvent& event)
      {
         if(theEnabledFlag)
         {
            if(event.getObject())
            {
               rspfConnectableObject* connectable = PTR_CAST(rspfConnectableObject,
                                                             event.getObject());
               if(connectable)
               {
                  rspfPropertyEvent evt(event.getObject());
                  connectable->propagateEventToOutputs(evt);
               }
            }
         }
      }
   rspfConnectableContainer* theContainer;
   bool theEnabledFlag;

};

rspfConnectableContainer::rspfConnectableContainer(rspfConnectableObject* owner)
   :rspfConnectableObject(owner, 0, 0, true, true),
    rspfConnectableContainerInterface(NULL)
{
   rspfConnectableContainerInterface::theBaseObject = this;
   theChildListener = new rspfConnectableContainerChildListener(this);
}

rspfConnectableContainer::~rspfConnectableContainer()
{
   // will need to destory and detach all children
   //
   deleteAllChildren();

   if(theChildListener)
   {
      delete theChildListener;
   }
   theChildListener = 0;
}

rspfConnectableObject* rspfConnectableContainer::findFirstObjectOfType(const RTTItypeid& typeInfo,
                                                                         bool recurse)
{
   rspfConnectableObject* result = NULL;
   connectablObjectMapType::iterator current;

   current = theObjectMap.begin();
   while(current != theObjectMap.end())
   {
      if(((*current).second)->canCastTo(typeInfo))
      {
         return (*current).second.get();
      }
      ++current;
   }

   if(recurse)
   {
      current = theObjectMap.begin();
      while(current != theObjectMap.end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).second.get());

         if(child)
         {
            result = child->findFirstObjectOfType(typeInfo);
            if(result)
            {
               return result;
            }
         }
         ++current;
      }
   }

   return result;
}

rspfConnectableObject* rspfConnectableContainer::findFirstObjectOfType(const rspfString& className,
                                                                         bool recurse)
{
   rspfConnectableObject* result = NULL;
   connectablObjectMapType::iterator current;

   current = theObjectMap.begin();
   while(current != theObjectMap.end())
   {
      if( ((*current).second)->canCastTo(className) )
      {
         return (*current).second.get();
      }
      ++current;
   }

   if(recurse)
   {
      current = theObjectMap.begin();
      while(current != theObjectMap.end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).second.get());

         if(child)
         {
            result = child->findFirstObjectOfType(className);
            if(result)
            {
               return result;
            }
         }
         ++current;
      }
   }

   return result;
}

rspfConnectableObject::ConnectableObjectList rspfConnectableContainer::findAllObjectsOfType(const RTTItypeid& typeInfo,
                                                                                     bool recurse)
{
   rspfConnectableObject::ConnectableObjectList result;

   connectablObjectMapType::iterator current;

   current = theObjectMap.begin();
   while(current != theObjectMap.end())
   {
      if(((*current).second)->canCastTo(typeInfo))
      {
         result.push_back( (*current).second.get());
      }
      ++current;
   }

   if(recurse)
   {
      current = theObjectMap.begin();
      while(current != theObjectMap.end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).second.get());

         if(child)
         {
            ConnectableObjectList temp;
            temp = child->findAllObjectsOfType(typeInfo, recurse);
            for(long index=0; index < (long)temp.size();++index)
            {
               result.push_back(temp[index]);
            }
         }
         ++current;
      }
   }
   return result;
}

rspfConnectableObject::ConnectableObjectList rspfConnectableContainer::findAllObjectsOfType(const rspfString& className,
                                                                                     bool recurse)
{
   rspfConnectableObject::ConnectableObjectList result;

   connectablObjectMapType::iterator current;

   current = theObjectMap.begin();
   while(current != theObjectMap.end())
   {
      if(((*current).second)->canCastTo(className))
      {
         result.push_back( (*current).second.get());
      }
      ++current;
   }

   if(recurse)
   {
      current = theObjectMap.begin();
      while(current != theObjectMap.end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).second.get());

         if(child)
         {
            rspfConnectableObject::ConnectableObjectList temp;
            temp = child->findAllObjectsOfType(className, true);
            for(long index=0; index < (long)temp.size();++index)
            {
               result.push_back(temp[index]);
            }
         }
         ++current;
      }
   }
   return result;
}

rspfConnectableObject* rspfConnectableContainer::findObject(const rspfId& id,
                                                              bool recurse)
{
   connectablObjectMapType::iterator current;

   current = theObjectMap.begin();

   while(current != theObjectMap.end())
   {
      if((*current).second.valid() && ((*current).second->getId()==id))
      {
         return (*current).second.get();
      }
      ++current;
   }

   if(recurse)
   {
      current = theObjectMap.begin();

      while(current != theObjectMap.end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).second.get());

         if(child)
         {
            rspfConnectableObject* object = child->findObject(id, true);

            if(object) return object;
         }
         ++current;
      }
   }
   return NULL;
}

rspfConnectableObject* rspfConnectableContainer::findObject(const rspfConnectableObject* obj,
                                                              bool recurse)
{
   connectablObjectMapType::iterator current;

   current = theObjectMap.begin();

   while(current != theObjectMap.end())
   {
      if((*current).second.valid() && ((*current).second==obj))
      {
         return (*current).second.get();
      }
      ++current;
   }

   if(recurse)
   {
      current = theObjectMap.begin();

      while(current != theObjectMap.end())
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).second.get());

         if(child)
         {
            rspfConnectableObject* object = child->findObject(obj, true);

            if(object) return object;
         }
         ++current;
      }
   }

   return NULL;
}

void rspfConnectableContainer::makeUniqueIds()
{
   connectablObjectMapType::iterator current;
   std::vector<rspfRefPtr<rspfConnectableObject> > objectList;


   current = theObjectMap.begin();

   setId(rspfIdManager::instance()->generateId());
   while(current != theObjectMap.end())
   {
      objectList.push_back((*current).second.get());
      ++current;
   }
   theObjectMap.clear();

   for(long index = 0; index < (long)objectList.size(); ++index)
   {
      rspfConnectableContainerInterface* container = PTR_CAST(rspfConnectableContainerInterface,
                                                               objectList[index].get());
      if(container)
      {
         container->makeUniqueIds();
         theObjectMap.insert(std::make_pair(objectList[index]->getId().getId(),
                                            objectList[index].get()));
      }
      else
      {
         objectList[index]->setId(rspfIdManager::instance()->generateId());
         theObjectMap.insert(std::make_pair(objectList[index]->getId().getId(),
                             objectList[index].get()));
      }
   }
}

rspf_uint32 rspfConnectableContainer::getNumberOfObjects(bool recurse)const
{
   rspf_uint32 result = 0;

   connectablObjectMapType::const_iterator current;

   current = theObjectMap.begin();

   while(current != theObjectMap.end())
   {
      ++result;

      if(recurse)
      {
         rspfConnectableContainerInterface* child=PTR_CAST(rspfConnectableContainerInterface, (*current).second.get());
         if(child)
         {
            result += child->getNumberOfObjects(true);
         }
      }
      ++current;
   }

   return result;
}

bool rspfConnectableContainer::addChild(rspfConnectableObject* object)
{
   if(object)
   {
      // if it's not already a child
      
      rspfIdVisitor visitor(
         object->getId(),
         (rspfVisitor::VISIT_INPUTS|rspfVisitor::VISIT_CHILDREN) );
      accept( visitor );
      
      // if(!findObject(object->getId()))
      if ( !visitor.getObject() )      
      {
         object->changeOwner(this);
         theObjectMap.insert(std::make_pair(object->getId().getId(), object));
         object->addListener(theChildListener);
      }
      
      return true;
   }
   
   return false;
}

bool rspfConnectableContainer::removeChild(rspfConnectableObject* object)
{
   rspfConnectableObject* result = NULL;

   if(object)
   {
      connectablObjectMapType::iterator childIter = theObjectMap.find(object->getId().getId());
      if(childIter != theObjectMap.end())
      {
         result = (*childIter).second.get();
         if(result->getOwner() == this)
         {
            result->changeOwner(NULL);
            result->removeListener(theChildListener);
//            result->removeListener(this);
         }
         theObjectMap.erase(childIter);
      }
   }

   return (result!=NULL);
}

bool rspfConnectableContainer::loadState(const rspfKeywordlist& kwl,
                                               const char* prefix)
{
   static const char* MODULE = "rspfConnectableContainer::loadState(kwl, prefix)";

   rspfString copyPrefix = prefix;
   bool result = rspfConnectableObject::loadState(kwl, copyPrefix.c_str());

   std::map<rspfId, std::vector<rspfId> > idMapping;
   result = addAllObjects(idMapping, kwl, prefix);
   if(!result)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " error: " << "problems adding sources" << std::endl;
   }
   result = connectAllObjects(idMapping);
   if(!result)
   {
      rspfNotify(rspfNotifyLevel_FATAL)
         << MODULE << " error: " << "problems connecting sources\n";
   }

   return result;
}

bool rspfConnectableContainer::saveState(rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   bool result = true;
   connectablObjectMapType::const_iterator current;
   
   rspf_int32 childIndex = 1;
   current = theObjectMap.begin();
   while(current != theObjectMap.end())
   {
      rspfString newPrefix = ( (rspfString(prefix) +
                                 rspfString("object") +
                                 rspfString::toString(childIndex) + "."));
      if((*current).second.valid())
      {
         bool test = ((*current).second)->saveState(kwl, newPrefix);
         if(!test)
         {
            result = false;
         }
      }
      ++childIndex;
      ++current;
   }
   if(result)
   {
      result = rspfConnectableObject::saveState(kwl, prefix);
   }
   
   return result;
}

void rspfConnectableContainer::getChildren(std::vector<rspfConnectableObject*>& children,
                                               bool immediateChildrenOnlyFlag)
{
   connectablObjectMapType::iterator current;
   current = theObjectMap.begin();
   std::vector<rspfConnectableObject*> temp;

   while(current != theObjectMap.end())
   {
      temp.push_back((*current).second.get());
   }
   rspf_uint32 i;
   for(i = 0; i < temp.size();++i)
   {

      if(!immediateChildrenOnlyFlag)
      {
         rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                             (*current).second.get());
         if(!inter)
         {
            children.push_back(temp[i]);
         }
      }
      else
      {
         children.push_back(temp[i]);
      }
   }

   if(!immediateChildrenOnlyFlag)
   {
      for(i = 0; i < temp.size(); ++i)
      {
         rspfConnectableContainerInterface* inter = PTR_CAST(rspfConnectableContainerInterface,
                                                             (*current).second.get());

         if(inter)
         {
            inter->getChildren(children,
                               immediateChildrenOnlyFlag);
         }
      }
   }
}

void rspfConnectableContainer::deleteAllChildren()
{
   connectablObjectMapType::iterator current;
   rspfConnectableObject* temp;
   current = theObjectMap.begin();
   removeAllListeners();

   while(theObjectMap.size())
   {
      current = theObjectMap.begin();
      temp = (*current).second.get();
      if(temp)
      {
         temp->removeListener(theChildListener);
         temp->disconnect();
         (*current).second = NULL;
      }
      theObjectMap.erase(current);
   }
}

void rspfConnectableContainer::removeAllListeners()
{
   connectablObjectMapType::iterator current;
   rspfConnectableObject* temp;
   current = theObjectMap.begin();

   while(current != theObjectMap.end())
   {
      temp = (*current).second.get();
      if(temp)
      {
         temp->removeListener(theChildListener);
//         temp->removeListener((rspfConnectableObjectListener*) this);
      }
      ++current;
   }
}

bool rspfConnectableContainer::addAllObjects(std::map<rspfId,
                                              std::vector<rspfId> >& idMapping,
                                              const rspfKeywordlist& kwl,
                                              const char* prefix)
{
   rspfString copyPrefix = prefix;

   std::vector<rspfId> inputConnectionIds;

   rspfString regExpression =  rspfString("^(") + copyPrefix + "object[0-9]+.)";
   std::vector<rspfString> keys =
      kwl.getSubstringKeyList( regExpression );
   long numberOfSources = (long)keys.size();

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
         rspfNotify(rspfNotifyLevel_DEBUG) << "trying to create source with prefix: " << newPrefix << "\n";
      }
      rspfRefPtr<rspfObject> object = rspfObjectFactoryRegistry::instance()->createObject(kwl,
                                                                                 newPrefix.c_str());
      if(object.valid())
      {
         rspfConnectableObject* connectable = PTR_CAST(rspfConnectableObject, object.get());
         if(connectable)
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "Created source with prefix: " << newPrefix << "\n";
            }
            // we did find a source so include it in the count
            rspfId id = connectable->getId();
            inputConnectionIds.clear();

            findInputConnectionIds(inputConnectionIds,
                                   kwl,
                                   newPrefix);

            if(inputConnectionIds.size() != 0)
            {
               idMapping.insert(std::make_pair(id, inputConnectionIds));
            }
            addChild(connectable);
         }
      }
   }

   return true;
}

void rspfConnectableContainer::findInputConnectionIds(std::vector<rspfId>& result,
                                                       const rspfKeywordlist& kwl,
                                                       const char* prefix)
{
   rspfString newPrefix = prefix;

   rspf_uint32 counter = 0;
   rspf_uint32 numberOfMatches = 0;

   rspfString regExpression =  rspfString("^") + newPrefix + "input_connection[0-9]+";
   rspf_uint32 numberOfKeys = kwl.getNumberOfSubstringKeys( regExpression);
   std::vector<rspfString> l = kwl.getSubstringKeyList(regExpression);
   if(numberOfKeys > 0)
   {
      newPrefix += "input_connection";

      while(numberOfMatches < numberOfKeys)
      {
         const char* lookup = kwl.find(newPrefix,
                                       rspfString::toString(counter));
         if(lookup)
         {
            ++numberOfMatches;
            long id = rspfString(lookup).toLong();
            if(id != rspfId::INVALID_ID)
            {
               result.push_back(rspfId(id));
            }
         }
         ++counter;
      }
   }
}

bool rspfConnectableContainer::connectAllObjects(const std::map<rspfId, std::vector<rspfId> >& idMapping)
{
   if(idMapping.size())
   {
      
      rspfIdVisitor visitor( (rspfVisitor::VISIT_INPUTS|rspfVisitor::VISIT_CHILDREN) );

      std::map<rspfId, std::vector<rspfId> >::const_iterator iter = idMapping.begin();

      while(iter != idMapping.end())
      {
         visitor.setId( (*iter).first );
         accept( visitor );
         rspfConnectableObject* currentObject = visitor.getObject();
         // rspfConnectableObject* currentObject = findObject((*iter).first);

         if(currentObject)
         {
            long upperBound = (long)(*iter).second.size();
            for(long index = 0; index < upperBound; ++index)
            {
               visitor.reset();
               visitor.setId( (*iter).second[index] );
               accept( visitor );
               rspfConnectableObject* inputObject = visitor.getObject();
               if ( inputObject )
               {
                  currentObject->connectMyInputTo(index, inputObject);
               }
               // rspfConnectableObject* inputObject = findObject((*iter).second[index]);
               // currentObject->connectMyInputTo(index, inputObject);
            }
         }
         else
         {
            rspfNotify(rspfNotifyLevel_FATAL)
               << "rspfConnectableContainer::connectAllObjects, Could not find "
               << (*iter).first << " for source: \n";
            return false;
         }
         visitor.reset();
         ++iter;
      }
   }
   return true;
}

rspfObject* rspfConnectableContainer::getObject()
{
   return this;
}

const rspfObject* rspfConnectableContainer::getObject()const
{
   return this;
}

rspfConnectableObject* rspfConnectableContainer::getConnectableObject(
   rspf_uint32 index)
{
   rspfConnectableObject* result = NULL;

   if (index > theObjectMap.size())
   {
      return result;
   }

   connectablObjectMapType::iterator current;
   current = theObjectMap.begin();
   rspf_uint32 i = 0;
   while(current != theObjectMap.end())
   {
      if (i == index)
      {
         return (*current).second.get();
      }
      ++current;
      ++i;
   }
   
   return result; 
}

bool rspfConnectableContainer::canConnectMyInputTo(rspf_int32,
                                                    const rspfConnectableObject*) const
{
   return false;
}

bool rspfConnectableContainer::canConnectMyOutputTo(rspf_int32,
                                                     const rspfConnectableObject*) const
{
   return false;
}

void rspfConnectableContainer::accept(rspfVisitor& visitor)
{
   if(!visitor.hasVisited(this))
   {
      visitor.visit(this);
      rspfVisitor::VisitorType currentType = visitor.getVisitorType();
      // lets make sure inputs and outputs are turned off for we are traversing all children and we should not have
      // to have that enabled
      //
      visitor.turnOffVisitorType(rspfVisitor::VISIT_INPUTS|rspfVisitor::VISIT_OUTPUTS);
      if(visitor.getVisitorType() & rspfVisitor::VISIT_CHILDREN)
      {
         connectablObjectMapType::iterator current = theObjectMap.begin();
         while((current != theObjectMap.end())&&!visitor.stopTraversal())
         {
            rspfRefPtr<rspfConnectableObject> currentObject = (current->second);
            if(currentObject.valid()&&!visitor.hasVisited(currentObject.get())) currentObject->accept(visitor);
            ++current;
         }
      }
      visitor.setVisitorType(currentType);
   }
   
   rspfConnectableObject::accept(visitor);
   
}

//**************************************************************************************************
// Inserts all of this object's children and inputs into the container provided. Since this is
// itself a container, this method will consolidate this with the argument container. Therefore 
// this object will not be represented in the argument container (but its children will be).
// Returns TRUE if successful.
//**************************************************************************************************
bool rspfConnectableContainer::fillContainer(rspfConnectableContainer& container)
{
   connectablObjectMapType::iterator current;
   current = theObjectMap.begin();
   // rspf_uint32 i = 0;
   // bool fill_ok;
   while(current != theObjectMap.end())
   {
      rspfRefPtr<rspfConnectableObject> currentObject = current->second;
      if (currentObject.valid())
         container.addChild(currentObject.get());
      current++;
   }
   return true;
}
