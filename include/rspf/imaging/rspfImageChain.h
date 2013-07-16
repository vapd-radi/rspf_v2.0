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
// $Id: rspfImageChain.h 20316 2011-12-02 15:56:38Z oscarkramer $
#ifndef rspfImageChain_HEADER
#define rspfImageChain_HEADER
#include <vector>
#include <map>
using namespace std;

#include <rspf/imaging/rspfImageSource.h>
#include <rspf/base/rspfConnectableObjectListener.h>
#include <rspf/base/rspfId.h>
#include <rspf/base/rspfConnectableContainerInterface.h>

class RSPFDLLEXPORT rspfImageChain : public rspfImageSource,
                                       public rspfConnectableObjectListener,
                                       public rspfConnectableContainerInterface
{
public:
   rspfImageChain();
   virtual ~rspfImageChain();

   rspfConnectableObject::ConnectableObjectList& getChainList(){return theImageChainList;}
   const rspfConnectableObject::ConnectableObjectList& getChainList()const{return theImageChainList;}
   
   /**
    * @param index Index of object to get.
    *
    * @see getNumberOfObjects(false) to get the number of objects inside
    * the container.
    *
    * @return Container's object at that index or NULL if out of range.
    */
   rspfConnectableObject* operator[](rspf_uint32 index);

   /**
    * Same as operator[](rspf_uint32 i)
    * 
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
   
   virtual rspf_int32 indexOf(rspfConnectableObject* obj)const;
   /**
    * Return the first source which is the one that first receives the
    * getTile request
    */
   virtual rspfImageSource* getFirstSource();
   virtual const rspfImageSource* getFirstSource() const;
   
   virtual rspfObject* getFirstObject();

   /**
    * Return the last source which is the one that last receives the
    * getTile request.
    */
   virtual rspfImageSource* getLastSource();
   virtual const rspfImageSource* getLastSource() const;
   
   virtual rspfObject* getLastObject();


   /**
    * Adds it to the start of the chain.  This is the first one to receive
    * the getTile request.  This basically inserts it to the right of the
    * right most filter.
    * Returns true on success, false on error.
    */
   bool addFirst(rspfConnectableObject* obj);

   /**
    * Adds it to the end.  this is the last one to receive the getTile
    * request.  This basically inserts it to the left of the left most filter.
    * Returns true on success, false on error.
    */
   bool addLast(rspfConnectableObject* obj);
   
   /**
    * Deletes the first object.
    * Note this is really most right of the chain.
    * Returns true if deletion occurred, false if the chain is empty.
    */
   bool deleteFirst();

   /**
    * Deletes the last object.
    * Note this is really most left of the chain.
    * Returns true if deletion occurred, false if the chain is empty.
    */
   bool deleteLast();

   /**
    * 
    */
   bool insertRight(rspfConnectableObject* newObj,
                    rspfConnectableObject* rightOfThisObj);

   /**
    *
    */
   bool insertRight(rspfConnectableObject* newObj,
                    const rspfId& id);

   /**
    *
    */
   bool insertLeft(rspfConnectableObject* newObj,
                   const rspfId& id);
   /**
    *
    */
   bool insertLeft(rspfConnectableObject* newObj,
                   rspfConnectableObject* leftOfThisObj);
   
   bool replace(rspfConnectableObject* newObj,
                rspfConnectableObject* oldObj);
   /**
    * Will return true or false if an image source was
    * added to the chain.  It will add and do a connection
    * to the previous source automatically.  This is a
    * chain and the order of insertion matters. It adds it to the beginning
    * of the list.  The last one added is the first one to receive
    * the getTile.  Basically inserts to the right of the rightmost filter
    *
    */
   virtual bool add(rspfConnectableObject* source);

   /**
    * Within the image chain will pass the head of the
    * list.
    */
   virtual rspfRefPtr<rspfImageData> getTile(const rspfIrect& tileRect,
                                               rspf_uint32 resLevel=0);
   
   /**
    * this call is passed to the head of the list.
    */
   virtual rspf_uint32 getNumberOfInputBands() const;
   
   /**
    * This call is passed to the head of the list.
    */
   virtual rspfScalarType getOutputScalarType() const;

   virtual double getNullPixelValue(rspf_uint32 band=0)const;
   virtual double getMinPixelValue(rspf_uint32 band=0)const;
   virtual double getMaxPixelValue(rspf_uint32 band=0)const;
   
   virtual void getOutputBandList(std::vector<rspf_uint32>& bandList) const;

   /**
    * This call is passed to the head of the list.
    */
   virtual rspf_uint32 getTileWidth()const;

   /**
    * This call is passed to the head of the list.
    */
   virtual rspf_uint32 getTileHeight()const;
   
   /**
    * Will pass this call to the head of the list.
    */
   virtual rspfIrect getBoundingRect(rspf_uint32 resLevel=0)const;
   virtual void getValidImageVertices(vector<rspfIpt>& validVertices,
                                      rspfVertexOrdering ordering=RSPF_CLOCKWISE_ORDER,
                                      rspf_uint32 resLevel=0)const;
   
   virtual rspfRefPtr<rspfImageGeometry> getImageGeometry();

   virtual void getDecimationFactor(rspf_uint32 resLevel,
                                    rspfDpt& result) const;
   virtual void getDecimationFactors(vector<rspfDpt>& decimations) const;
   virtual rspf_uint32 getNumberOfDecimationLevels()const;
   
   /**
    * Will save the state of the chain.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=NULL)const;

   /**
    * Will load the state.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=NULL);
   
   virtual void initialize();
   virtual void enableSource();
   virtual void disableSource();
   
   bool canConnectMyInputTo(rspf_int32 myInputIndex,
                            const rspfConnectableObject* object)const
      {
         if(theImageChainList.size()&&
            theImageChainList[theImageChainList.size()-1].valid())
         {
            rspfConnectableObject* obj = PTR_CAST(rspfConnectableObject, theImageChainList[theImageChainList.size()-1].get());
            if(obj)
            {
               return obj->canConnectMyInputTo(myInputIndex,
                                               object);
            }
         }
         else if(!theImageChainList.size())
         {
            return true;
         }
         return false;
      }
      
//    bool canConnectMyOutputTo(int32 myOutputIndex,
//                              const rspfConnectableObject* object)const
//       {
//          if(theImageChainList.size()&&theImageChainList[0])
//          {
// 	   rspfConnectableObject* obj = PTR_CAST(rspfConnectableObject, theImageChainList[0]);
// 	   if(obj)
// 	     {
// 	       return obj->canConnectMyOutputTo(myOutputIndex,
// 						object);
// 	     }
//          }
         
//          return false;
//       }
   
   // ________________________CONNECTABLE CONTAINER DEFINTIONS_________________
   //
   
   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeIdVisitor visitor(tyeId, false, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD( virtual rspfConnectableObject::ConnectableObjectList findAllObjectsOfType(const RTTItypeid& typeInfo,
                                                                     bool recurse=true));
   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor(typeName, false, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject::ConnectableObjectList findAllObjectsOfType(const rspfString& className,
                                                                     bool recurse=true));

   /*!
    * These methods are now deprecated.  You can achieve the same thing by using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeIdVisitor visitor(tyeId, true, rspfVisitor::VISIT_CHILDREN);
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
    *    rspfIdVisitor visitor(tyeId, true, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> obj = visitor.getObject();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findObject(const rspfId& id,
                                              bool recurse=true));

   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findObject(const rspfConnectableObject* obj,
                                              bool recurse=true));

   /**
    * Will cycle through all sources setting their ids. the idLast wlil
    * be updated
    * so we can recurse into other containers.
    */
   virtual void makeUniqueIds();

   /**
    * Returns the number of objects within this container and all child
    * containers.
    *
    * @param recurse If true and object is a contains containers; all objects
    * will be conter.  If false only the objects of this container will be
    * counted (theImageChainList.size()).
    */
   virtual rspf_uint32 getNumberOfObjects(bool recurse=true)const;

   /**
    * Deprecated! Please use getNumberOfObjects(false).
    *
    * @return The number of objects in this container or
    * (theImageChainList.size()).
    */
   virtual rspf_uint32 getNumberOfSources() const;
   
   /**
    * Will add an object to the container and then set the added objects owner
    * to this.
    */
   virtual bool addChild(rspfConnectableObject* attachableObject);

   /**
    * Will remove the child from the container.  Changes the owner of the
    * child to be NULL;
    */
   virtual bool removeChild(rspfConnectableObject* object);
   virtual rspfConnectableObject* removeChild(const rspfId& id);
   
   virtual void getChildren(vector<rspfConnectableObject*>& children,
                            bool immediateChildrenOnlyFlag);
   //______________END CONNECTABLE CONTAINER INTERFACE____________
   
   virtual void disconnectInputEvent(rspfConnectionEvent& event);
   virtual void disconnectOutputEvent(rspfConnectionEvent& event);
   virtual void connectInputEvent(rspfConnectionEvent& event);
   virtual void connectOutputEvent(rspfConnectionEvent& event);
   virtual void objectDestructingEvent(rspfObjectDestructingEvent& event);

   virtual void propagateEventToOutputs(rspfEvent& event);
   virtual void propagateEventToInputs(rspfEvent& event);
 
   virtual void processEvent(rspfEvent& event);

   virtual void accept(rspfVisitor& visitor);
   void deleteList();

   /**
    * These access methods greatly facilitate the implementation of an image chain adaptor class.
    * They shouldn't be accessed directly by any other classes.  These methods really should be 
    * "protected" but the compiler complains if this base class' declarations are "protected".
    */
   virtual rspfConnectableObject::ConnectableObjectList& imageChainList() { return theImageChainList; }
   virtual const rspfConnectableObject::ConnectableObjectList& imageChainList() const { return theImageChainList; }
   
   /**
    * Inserts all of this object's children and inputs into the container provided. Since this is
    * itself a form of container, this method will consolidate this chain with the argument
    * container. Consequently, this chain object will not be represented in the container, but its
    * children will be, with correct input and output connections to external objects. Ownership
    * of children will pass to the argument container.
    * @return Returns TRUE if successful.
    */
   virtual bool fillContainer(rspfConnectableContainer& container);

protected:
   void prepareForRemoval(rspfConnectableObject* connectableObject);
   
  /**
    * This will hold a sequence of image sources.
    * theFirst one in the list will be the head of the
    * list and the last one is the tail.
    */
   rspfConnectableObject::ConnectableObjectList theImageChainList;
   
   rspfRefPtr<rspfImageData>     theBlankTile;
  // mutable bool                    thePropagateEventFlag;
   mutable bool                    theLoadStateFlag;
   /**
    * For dynamic loading to take place we must allocate all objects first and
    * then assign id's later.  We must remember the id's so we can do this.
    * we will create a map that takes the id of the source as a key and a
    * vector of input id's to connect it's inputs to.
    */
   bool addAllSources(map<rspfId, vector<rspfId> >& idMapping,
                      const rspfKeywordlist& kwl,
                      const char* prefix = NULL);
   void findInputConnectionIds(vector<rspfId>& result,
                               const rspfKeywordlist& kwl,
                               const char* prefix=NULL);
   bool connectAllSources(const map<rspfId, vector<rspfId> >& idMapping);
   
   
TYPE_DATA
};
#endif /* #ifndef rspfImageChain_HEADER */
