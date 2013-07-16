//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: A brief description of the contents of the file.
//
//*************************************************************************
// $Id: rspfConnectableContainerInterface.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfConnectableContainerInterface_HEADER
#define rspfConnectableContainerInterface_HEADER

#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfId.h>
#include <rspf/base/rspfConnectableObject.h>
#include <vector>

class rspfString;

class RSPFDLLEXPORT rspfConnectableContainerInterface
{
public:
   rspfConnectableContainerInterface(rspfObject* obj):theBaseObject(obj){}
   virtual ~rspfConnectableContainerInterface(){theBaseObject=NULL;}
   
   /*!
    * Will find all objects of the past in type.  Use the RTTI type info.  An optional
    * recurse flag will say if there is another container then recurse it to
    * find the type you are looking for else it just looks within its immediate
    * children
    * 
    * Example: passing STATIC_TYPE_INFO(rspfImageRenderer) as an argument will
    *          look for all rspfImageRenderer's and return the list.
    */
   virtual rspfConnectableObject::ConnectableObjectList findAllObjectsOfType(const RTTItypeid& typeInfo,
                                                                              bool recurse=true)=0;
   virtual rspfConnectableObject::ConnectableObjectList findAllObjectsOfType(const rspfString& className,
                                                                              bool recurse=true)=0;
   
   /*!
    * Will find the firt object of the past in type.  Use the RTTI type info.An optional
    * recurse flag will say if there is another container then recurse it to
    * find the type you are looking for else it just looks within its immediate
    * children
    * 
    * Example: passing STATIC_TYPE_INFO(rspfImageRenderer) as an argument will
    *          look for the first rspfImageRenderer and return that object.
    */
   virtual rspfConnectableObject* findFirstObjectOfType(const RTTItypeid& typeInfo,
                                                         bool recurse=true)=0;
   
   virtual rspfConnectableObject* findFirstObjectOfType(const rspfString& className,
                                                         bool recurse=true)=0;

   /*!
    * will search for the object given an id.  If recurse is true it will recurse
    * to other containers.
    */ 
   virtual rspfConnectableObject* findObject(const rspfId& id,
                                              bool recurse=true)=0;

   virtual rspfConnectableObject* findObject(const rspfConnectableObject* obj,
                                              bool recurse=true)=0;
   /*!
    * Will cycle through all sources setting their ids. the idLast wlil be updated
    * so we can recurse into other containers.
    */
   virtual void makeUniqueIds()=0;

   /*!
    * Returns the number of objects within this container and all child containers.
    */
   virtual rspf_uint32 getNumberOfObjects(bool recurse=true)const=0;
   
   
   /*!
    * Will add an object to the container and then set the added objects owner
    * to this.
    */
   virtual bool addChild(rspfConnectableObject* attachableObject)=0;

   /*!
    * Will remove the child from the container.  Changes the owner of the
    * child to be NULL;
    */
   virtual bool removeChild(rspfConnectableObject* object)=0;

   /*!
    * Gives access to the this point of the base object that everyone derives
    * from.
    */
   rspfObject*       getObject(){return theBaseObject;}
   const rspfObject* getObject()const{return theBaseObject;}

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
   virtual rspfConnectableObject* getConnectableObject(rspf_uint32 index)=0;

   void deleteAllChildren();
   virtual void getChildren(std::vector<rspfConnectableObject*>& children,
                            bool immediateChildrenOnlyFlag)=0;
protected:
   rspfObject* theBaseObject;
TYPE_DATA
};

#endif
