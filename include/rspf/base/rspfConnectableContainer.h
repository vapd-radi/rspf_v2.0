//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfConnectableContainer.h 20316 2011-12-02 15:56:38Z oscarkramer $

#ifndef rspfConnectableContainer_HEADER
#define rspfConnectableContainer_HEADER
#include <map>

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfConnectableObject.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfConnectableContainerInterface.h>

class RSPFDLLEXPORT rspfConnectableContainerChildListener;

class RSPFDLLEXPORT rspfConnectableContainer : public rspfConnectableObject,
                                  public rspfConnectableContainerInterface,
                                  public rspfConnectableObjectListener
{
public:
   typedef std::map<rspf_int64,
      rspfRefPtr<rspfConnectableObject> > connectablObjectMapType;
   
   rspfConnectableContainer(rspfConnectableObject* owner=0);

   virtual rspfObject* getObject();
   
   virtual const rspfObject* getObject()const;

   /**
    * @param index Index of object to get.
    *
    * @see getNumberOfObjects(false) to get the number of objects inside
    * the container.
    *
    * @return Container's object at that index or NULL if out of range.
    *
    * @note This does not recurse into other containers.
    */
   virtual rspfConnectableObject* getConnectableObject(rspf_uint32 index);

   
   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor("<put type name here>", false, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject::ConnectableObjectList findAllObjectsOfType(const RTTItypeid& typeInfo,
                                                            bool recurse=true));
   
   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfConnectableTypeIdVisitor visitor(typeId, false, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfConnectableCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject::ConnectableObjectList findAllObjectsOfType(const rspfString& className,
                                                            bool recurse=true));

   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeIdVisitor visitor(typeId, true, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> obj = visitor.getObject();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findFirstObjectOfType(const RTTItypeid& typeInfo,
                                                 bool recurse=true));
   
   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor(className, true, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> obj = visitor.getObject();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findFirstObjectOfType(const rspfString& className,
                                                bool recurse=true));

   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfIdVisitor visitor(id, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> obj = visitor.object();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(rspfConnectableObject* findObject(const rspfId& id,
                                     bool recurse=true));

   /**
    * Please use the new rspfVisitor design pattern to visit objects in the container.
    */
   RSPF_DEPRECATE_METHOD(rspfConnectableObject* findObject(const rspfConnectableObject* obj,
                                     bool recurse=true));

   /*!
    * Will cycle through all sources setting their ids. the idLast wlil be
    * updated
    * so we can recurse into other containers.
    */
   void makeUniqueIds();

   /*!
    * Returns the number of objects within this container and all child
    * containers.
    */
   rspf_uint32 getNumberOfObjects(bool recurse=true)const;
   
   /*!
    * Will add an object to the container and then set the added objects owner
    * to this.
    */
   virtual bool addChild(rspfConnectableObject* attachableObject);


   virtual bool removeChild(rspfConnectableObject* object);
   
   virtual bool canConnectMyInputTo(rspf_int32 index,
                                    const rspfConnectableObject* obj) const;

   virtual bool canConnectMyOutputTo(rspf_int32 index,
                                     const rspfConnectableObject* obj) const;
   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0)const;

   //____________________PLACE ALL EVENT HANDLING STUFF HERE_____________
//   virtual void objectDestructingEvent(rspfObjectDestructingEvent& event);
//   virtual void propertyEvent(rspfPropertyEvent& event);

//   virtual void disconnectInputEvent(rspfConnectionEvent& event);
//   virtual void connectInputEvent(rspfConnectionEvent& event);

//   virtual void disconnectOutputEvent(rspfConnectionEvent& event);
//   virtual void connectOutputEvent(rspfConnectionEvent& event);
   
   
   virtual void getChildren(std::vector<rspfConnectableObject*>& children,
                            bool immediateChildrenOnlyFlag);
   void deleteAllChildren();
   
   virtual void accept(rspfVisitor& visitor);
//    void propagateEventToOutputs(rspfEvent& event,
//                                 rspfConnectableObject* start);
//    void propagateEventToInputs(rspfEvent& event,
//                                rspfConnectableObject* start);
     
   /*!
    *  Inserts all of this object's children and inputs into the container provided. Since this is
    *  itself a container, this method will consolidate this with the argument container. 
    *  Consequently, this object will not be represented in the argument container but its children
    *  will be. Ownership of children will pass to the argument container.
    *  @return Returns TRUE if successful.
    */
   virtual bool fillContainer(rspfConnectableContainer& container);

protected:
   virtual ~rspfConnectableContainer();
   rspfConnectableContainer(const rspfConnectableContainer& rhs);

   void removeAllListeners();
   bool addAllObjects(std::map<rspfId, std::vector<rspfId> >& idMapping,
                      const rspfKeywordlist& kwl,
                      const char* prefix);
      
   bool connectAllObjects(const std::map<rspfId, std::vector<rspfId> >& idMapping);
   
   void findInputConnectionIds(std::vector<rspfId>& result,
                               const rspfKeywordlist& kwl,
                               const char* prefix);
   
   /*!
    * Every object added must have a unique id.  We will sort them in a easy to
    * query form.  Since the container can have hundreds of objects we will use a
    * more efficient map that allows us to do binary searches of the objects.
    *
    * map<key, value>  The key will be the objectsUnique id and the value
    * is a pointer to the attachable object.
    */
   connectablObjectMapType theObjectMap;

   rspfConnectableContainerChildListener* theChildListener;
TYPE_DATA
};

#endif
