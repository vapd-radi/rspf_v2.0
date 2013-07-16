//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
// Description:
// 
// Contains class declaration for rspfConnectableObject the base class for
// all connectable objects.
//
//*************************************************************************
// $Id: rspfConnectableObject.h 21852 2012-10-21 20:16:04Z dburken $

#ifndef rspfConnectableObject_HEADER
#define rspfConnectableObject_HEADER 1

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfId.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfListenerManager.h>
#include <rspf/base/rspfPropertyInterface.h>
#include <rspf/base/rspfRefPtr.h>
#include <vector>

class rspfVisitor;
class rspfConnectableContainer;

class  RSPFDLLEXPORT  rspfConnectableObject : public rspfObject,
                                                public rspfListenerManager,
                                                public rspfPropertyInterface
{
public:
   typedef std::vector<rspfRefPtr<rspfConnectableObject> > ConnectableObjectList;
   enum rspfConnectableObjectDirectionType
   {
      CONNECTABLE_DIRECTION_NONE   = 0,
      CONNECTABLE_DIRECTION_INPUT  = 1,
      CONNECTABLE_DIRECTION_OUTPUT = 2
   };
   /**
    * Base constructor of this object.
    */
   rspfConnectableObject(rspfObject* owner=0);
   rspfConnectableObject(rspfObject* owner,
                          rspf_int32 inputListSize,
                          rspf_int32 outputListSize,
                          bool inputListIsFixedFlag=true,
                          bool outputListIsFixedFlag=true);

   virtual ~rspfConnectableObject();
   /**
    * All connectable objects will have id's.  This allows us to
    * set the id of this object.
    */
   void setId(const rspfId& id);

   /**
    * Will allow us to get this object's id.
    */
   const rspfId& getId()const;
   
   /**
    * Fetches the current owner, most likely a container but not limited to one.
    */
   const rspfObject* getOwner() const;

   /**
    * Permits changing the object's owner.
    */
   virtual void changeOwner(rspfObject* owner);

   virtual void setDescription(const rspfString& description);
   virtual rspfString getDescription()const;
   
   /**
    * will check the direction specified to see if all slots are full.
    * If any slot is null then false is returned.
    * If a filter requires options testing they
    * need to override this method.  The argument can be ored together. If you
    * want both checked then pass
    * CONNECTABLE_DIRECTION_INPUT | CONNECTABLE_DIRECTION_OUTPUT
    */
   virtual bool isConnected(
      rspfConnectableObjectDirectionType direction = CONNECTABLE_DIRECTION_INPUT)const;


   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not achieve exactly
    * what you want then you can derive new rules by overriding the virtual
    * visit method in rspfVisitor
    *
    * <pre>
    *    rspfIdVisitor visitor(id, false, rspfVisitor::VISIT_CHILDREN |
    *    rspfVisitor::VISIT_INPUTS);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> object = visitor.getObject();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findConnectableObject(
                             const rspfId& id));

   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not achieve exactly
    * what you want then you can derive new rules by overriding the virtual
    * visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeIdVisitor visitor(typeId, true, rspfVisitor::VISIT_CHILDREN |
    *    rspfVisitor::VISIT_INPUTS);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> object = visitor.getObject();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findObjectOfType(
                             RTTItypeid typeId,
                             rspfConnectableObjectDirectionType directionType,
                             bool recurse = true));

   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding
    * the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor(typeName, true,
    *    rspfVisitor::VISIT_CHILDREN | rspfVisitor::VISIT_INPUTS);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> object = visitor.getObject();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findObjectOfType(
                             const rspfString& obj,
                             rspfConnectableObjectDirectionType directionType,
                             bool recurse = true));
   
   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding
    * the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor(typeName, true,
    *    rspfVisitor::VISIT_CHILDREN | rspfVisitor::VISIT_INPUTS);
    *    connectableObject->accept(visitor);
    *    rspfRefPtr<rspfConnectableObject> object = visitor.getObject();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual rspfConnectableObject* findInputObjectOfType(
                             const rspfString& className));
   
   /**
    * Return a valid index of the input list if the passed in object
    * is found else return -1.
    */
   virtual rspf_int32 findInputIndex(const rspfConnectableObject* object);

   /**
    * Return a valid index of the input list if the passed id
    * is found else return -1.
    */
   virtual rspf_int32 findInputIndex(const rspfId& id);

   /**
    * Return a valid index of the output list if the passed in object
    * is found else return -1.
    */
   virtual rspf_int32 findOutputIndex(const rspfConnectableObject* object);

   /**
    * Return a valid index of the output list if the passed in object
    * is found else return -1.
    */   
   virtual rspf_int32 findOutputIndex(const rspfId& id);
   
   /**
    * Should return the first available index to connect to.  The
    * connectMyInputTo that just takes another rspfConnectableObject as input
    * and not an index will call this method.  By default this method will find
    * the first open slot (not null) or append o the list if it's dynamic. If
    * the list is dynamic it calls conConnectMyInputTo(index, object) on an
    * index = to listSize.
    */
   virtual rspf_int32 getMyInputIndexToConnectTo(
      rspfConnectableObject* object)const;
   

   /**
    * Should return the first available index to connect to.  The
    * connectMyOutputTo that just takes another rspfConnectableObject as input\
    * and not an index will call this method.  By default this method will find
    * the first open slot (not null) or append to the list if it's dynamic. If
    * the list is dynamic it calls canConnectMyOutputTo(index, object) on an
    * index = to listSize.
    */
   virtual rspf_int32 getMyOutputIndexToConnectTo(
      rspfConnectableObject* object)const;
   
   /**
    * required to be overriden by derived classes
    */
   virtual bool canConnectMyInputTo(
      rspf_int32 myInputIndex, const rspfConnectableObject* object)const=0;
   
   /**
    * default implementation is to allow anyone to connect to us.
    */
   virtual bool canConnectMyOutputTo(rspf_int32 myOutputIndex,
                                     const rspfConnectableObject* object)const;
   
   /**
    * Will disconnect the object passed in.
    */
   virtual void disconnect(rspfConnectableObject* object=0);

   /**
    * Will disconnect the object passed in.
    */
   virtual void disconnect(const rspfId& id);
   
   /**
    * Will disconnect the object at the given input index and generate
    * a connection event.
    */
   virtual rspfRefPtr<rspfConnectableObject> disconnectMyInput(
      rspf_int32 inputIndex,
      bool disconnectOutputFlag=true,
      bool createEventFlag = true);
   
  /**
   * Finds the index of the passed in input and calls
   * disconnectMyInput(inputIndex, disconnectOutputFlag, createEventFlag);
   */
   virtual void disconnectMyInput(rspfConnectableObject* input,
                                  bool disconnectOutputFlag=true,
                                  bool createEventFlag = true);
   /**
    *
    */
   virtual void disconnectMyInputs(
      ConnectableObjectList& inputList,
      bool disconnectOutputFlag=true,
      bool createEventFlag=true);
   
   /**
    * Will disconnect the object at the given output index and generate
    * a connection event.  If there is no object at that index then no
    * event is generated and NULL is returned.  The disconnectOutputFlag
    * says do you want this method to disconnect the output pointer to
    * this object.
    */
   virtual rspfRefPtr<rspfConnectableObject> disconnectMyOutput(
      rspf_int32 outputIndex,
      bool disconnectInputFlag=true,
      bool createEventFlag = true);
   
   /**
    * Will disconnect the output object.  It will get the index of
    * the object and call disconnectMyOutput(index, disconnectOutputFlag).
    */
   virtual void disconnectMyOutput(rspfConnectableObject* output,
                                   bool disconnectInputFlag=true,
                                   bool createEventFlag=true);

   virtual void disconnectMyOutputs(
      ConnectableObjectList& outputList,
      bool disconnectOutputFlag=true,
      bool createEventFlag=true);
   
   /**
    * Will disconnect all of the input objects.
    */
   virtual void disconnectAllInputs();

   /**
    * Will disconnect all of the output objects.
    */
   virtual void disconnectAllOutputs();

   /**
    * Will try to connect this objects input to the passed in object.
    * It will return a valid index >= 0 if successful.  Will use the
    * getMyInputIndexToConnectTo method to implement the connection
    */
   virtual rspf_int32 connectMyInputTo(rspfConnectableObject* inputObject,
                                        bool makeOutputConnection=true,
                                        bool createEventFlag=true);

   /**
    * Will connect the specified input to the passed in object
    */
   virtual rspf_int32 connectMyInputTo(rspf_int32 inputIndex,
                                        rspfConnectableObject* inputObject,
                                        bool makeOutputConnection=true,
                                        bool createEventFlag=true);
   
   virtual bool connectMyInputTo(
      ConnectableObjectList& inputList,
      bool makeOutputConnection=true,
      bool createEventFlag = true);   

   /**
    * Will try to connect this objects output to the passed in object.
    * It will return a valid index >= 0 if successful.  It will in turn call
    * the passed in objects connect input command if makeInputConnection
    * is set to true.
    *
    * Will use the getMyOutputIndexToConnectTo to implement this method
    */
   virtual rspf_int32 connectMyOutputTo(rspfConnectableObject* outputObject,
                                         bool makeInputConnection=true,
                                         bool createEventFlag=true);
   
   virtual bool connectMyOutputTo(
      ConnectableObjectList& outputList,
      bool makeInputConnection=true,
      bool createEventFlag=true);
   
   /**
    * Will disconnect itself from all inputs and reset to the passed in
    * input list.  It will return true if all objects were successfully
    * added.
    */
   virtual bool connectInputList(
      ConnectableObjectList& inputList);
   
   /**
    * Will disconnect itself from all outputs and reset to the passed in
    * output list.  It will return true if all objects were successfully
    * added.
    */
   virtual bool connectOutputList(
      ConnectableObjectList& outputList);
   
   /**
    * Returns the number of input objects.
    */
   virtual rspf_uint32 getNumberOfInputs()const;

   /**
    * Return the number of output objects
    */
   virtual rspf_uint32 getNumberOfOutputs()const;
   
   /**
    * returns the object at the specified index.
    * if the index is not valid then NULL is
    * returned
    */
   rspfConnectableObject* getInput(rspf_uint32 index=0);   

   /**
    * returns the object at the specified index.
    * if the index is not valid then NULL is
    * returned
    */
   const rspfConnectableObject* getInput(rspf_uint32 index=0)const;

   /**
    * returns the object at the specified index.
    * if the index is not valid then NULL is
    * returned
    */
   rspfConnectableObject* getOutput(rspf_uint32 index=0);

   /**
    * returns the object at the specified index.
    * if the index is not valid then NULL is
    * returned
    */
   const rspfConnectableObject* getOutput(rspf_uint32 index=0)const;
   
   /**
    * Will set the number of inputs.  This will expand the list if the number
    * of inputs is larger than the current number of inputs.  Will shrink
    * the list if the current number of inputs is larger than the passed in
    * inputs.
    */
   virtual void setNumberOfInputs(rspf_int32 numberOfInputs);

   virtual bool getInputListIsFixedFlag()const;
   
   virtual bool getOutputListIsFixedFlag()const;

   /**
    * Will set the number of outputs.  This will expand the list if the
    * number of outputs is larger than the current number of outputs.
    * Will shrink the list if the current number of outputs is larger than
    * the passed in outputs.
    */
   virtual void setNumberOfOutputs(rspf_int32 numberOfInputs);
   
   const ConnectableObjectList& getInputList()const;

   const ConnectableObjectList& getOutputList()const;

   ConnectableObjectList& getInputList();

   ConnectableObjectList& getOutputList();

   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not achieve exactly
    * what you want then you can derive new rules by overriding the virtual
    * visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor("<put type name here>",
    *    false, rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
    RSPF_DEPRECATE_METHOD(virtual void findAllObjectsOfType(
                              ConnectableObjectList& result,
                              const RTTItypeid& typeInfo, bool recurse=true) );
    
   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not achieve exactly
    * what you want then you can derive new rules by overriding the virtual
    * visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor("<put type name here>", false,
    *    rspfVisitor::VISIT_CHILDREN);
    *    connectableObject->accept(visitor);
    *    rspfCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual void findAllObjectsOfType(
                             ConnectableObjectList& result,
                             const rspfString& className,
                             bool recurse=true));

#if 0 /* Deprecated code. Left here until thoroughly debugged. (drb) */
   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding
    * the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeIdVisitor visitor(<put type id here>, false,
    *    rspfVisitor::VISIT_CHILDREN | rspfVisitor::VISIT_INPUTS);
    *    connectableObject->accept(visitor);
    *    rspfCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
   RSPF_DEPRECATE_METHOD(virtual void findAllInputsOfType(
      ConnectableObjectList& result,
      const RTTItypeid& typeInfo,
      bool propagateToInputs=true,
      bool recurseChildren=false));
   
   /*!
    * These methods are now deprecated.  You can achieve the same thing by
    * using the new visitor design pattern.  If this does not 
    * achieve exactly what you want then you can derive new rules by overriding
    * the virtual visit method in rspfVisitor
    *
    * <pre>
    *    rspfTypeNameVisitor visitor("<put type name here>",
    *    false, rspfVisitor::VISIT_CHILDREN | rspfVisitor::VISIT_INPUTS);
    *    connectableObject->accept(visitor);
    *    rspfCollectionVisitor::List& collection = visitor.getObjects();
    * </pre>
    */
   
   RSPF_DEPRECATE_METHOD(virtual void findAllInputsOfType(
                             ConnectableObjectList& result,
                             const rspfString& className,
                             bool propagateToInputs=true,
                             bool recurseChildren=false));
#endif  /* deprecated code. (drb) */

   virtual void propagateEventToOutputs(rspfEvent& event);
   virtual void propagateEventToInputs(rspfEvent& event);

   /* ------------------- PROPERTY INTERFACE -------------------- */
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual void setProperty(const rspfString& name, const rspfString& value);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   /* ------------------ PROPERTY INTERFACE END ------------------- */

   
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix = 0);
   
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix = 0)const;

   /**
    * Save the state of all inputs to a keyword list.  This will do a
    * recursive call through all inputs and save their state to a
    * keyword list.
    *
    * @param kwl Keyword list to save state to.
    *
    * @param saveThisStateFlag If the true the state of this object will be
    * saved as well as all inputs. Default = true.
    *
    * @param  objectIndex Index to be used for prefix.  So if
    * objectIndex = 1, the first object will have a prefix of: "object1."
    * Default = 1.
    *
    * @param prefix Prefix to tack onto keyword.  If prefix = "myChain." and
    * objectIndex = 1, then the first keyword would be
    * "myChain.object1.some_keyword".
    *
    * @return Returns the next object index.  This is the last index used
    * plus one.  Used internally to keep track of index to use for prefix.
    */
   virtual rspf_uint32 saveStateOfAllInputs(rspfKeywordlist& kwl,
                                             bool saveThisStateFlag=true,
                                             rspf_uint32 objectIndex=1,
                                             const char* prefix=0) const;

   /**
    * Inserts this object and all of its children and inputs into the container
    * provided.
    * @return Returns TRUE if successful.
    */
   virtual bool fillContainer(rspfConnectableContainer& container);

   /**
    * Moves the input connection matching id up one in the connection list.
    * @param id The id to move.
    * @return true if action was performed, false if not.
    */
   bool moveInputUp(const rspfId& id);

   /**
    * Moves the input connection matching id down one in the connection list.
    * @param id The id to move.
    * @return true if action was performed, false if not.
    */
   bool moveInputDown(const rspfId& id);

   /**
    * Moves the input connection matching id to the top of the connection list.
    * @param id The id to move.
    * @return true if action was performed, false if not.
    */   
   bool moveInputToTop(const rspfId& id);

   /**
    * Moves the input connection matching id to the bottom of the connection
    * list.
    * @param id The id to move.
    * @return true if action was performed, false if not.
    */     
   bool moveInputToBottom(const rspfId& id);
   
   /**
    * We will add a visitor interface for all connectable objects.
    */
   virtual void accept(rspfVisitor& visitor);
   
protected:

   rspfConnectableObject* findObjectOfType(
      ConnectableObjectList* connectableList,
      rspfVisitor& visitor );
   
   rspfId      theId;
   rspfString  theDescription;
   rspfObject* theOwner;

   /**
    * Indicates whether the theInputObjectList is fixed
    */
   bool theInputListIsFixedFlag;

   /**
    * Indicates whether the theOutputObjectList is fixed
    */
   bool theOutputListIsFixedFlag;
   
   /**
    * Holds a list of input objects.
    */
   ConnectableObjectList theInputObjectList;
   
   /**
    * Holds a list of output objects.
    */
   ConnectableObjectList theOutputObjectList;

private:
TYPE_DATA
};

#endif /* #ifndef rspfConnectableObject_HEADER */
