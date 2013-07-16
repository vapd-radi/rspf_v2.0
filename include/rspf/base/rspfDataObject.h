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
// $Id: rspfDataObject.h 19931 2011-08-10 11:53:25Z gpotts $

#ifndef rspfDataObject_HEADER
#define rspfDataObject_HEADER

#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfDpt3d.h>

class rspfSource;

class RSPFDLLEXPORT rspfDataObject : public rspfObject
{
public:
   rspfDataObject(rspfSource* source=0,
                   rspfDataObjectStatus status=RSPF_STATUS_UNKNOWN);
   
   rspfDataObject(const rspfDataObject& rhs);
   virtual ~rspfDataObject();



   virtual rspf_uint32 getHashId()const=0;
   
   /**
    * Sets the owner of this Data object.
    */
   virtual void setOwner(rspfSource* aSource);

   virtual rspfSource* getOwner();

   virtual const rspfSource* getOwner() const;

   /**
    * Full list found in rspfConstants.h
    *
    * RSPF_STATUS_UNKNOWN
    * RSPF_NULL            Null data.  The buffers are never allocated
    * RSPF_EMPTY           Allocated data but its empty.  Most of the
    *                       time this will be the default for an initialized
    *                       data object.
    * RSPF_PARTIAL         Says that some of the object is empty and is only
    *                       partially full
    * RSPF_FULL            This data is completey full
    */
   virtual void  setDataObjectStatus(rspfDataObjectStatus status) const;

   /**
    * @return Returns enumerated status:
    * RSPF_STATUS_UNKNOWN = 0
    * RSPF_NULL           = 1
    * RSPF_EMPTY          = 2
    * RSPF_PARTIAL        = 3
    * RSPF_FULL           = 4
    */
   virtual rspfDataObjectStatus getDataObjectStatus() const;

   /**
    * @return Returns the status as a string, like "RSPF_NULL".
    */
   virtual rspfString getDataObjectStatusString() const;

   virtual rspf_uint32 getObjectSizeInBytes()const;

   virtual rspf_uint32 getDataSizeInBytes()const=0;

   /**
    * This is to be overriden in the derived classes so they can check
    * If their data is null.
    */
   virtual bool isInitialize() const;

   /**
    * Will allow derived classes to initialize their buffers.  This class
    * does nothing with this and reset any internal attributes.
    */
   virtual void initialize()=0;
   
   virtual void assign(const rspfDataObject* rhs);

   /**
    * Derived classes should implement this method.  For data objects
    * there is usually 2 types of tests a shallow and a deep test.  If
    * deepTest is false then a shallow test is performed.  I shallow test
    * just doesn't compare the data but instead compares the fields that
    * describes the data. For example if it were an rspfImageData then a
    * shallow compare will just compare the origin, scale, width, height,
    * number of bands, min, max, null, and scalar type.  If
    * it were a deep compare it will also test the actual data to one another.
    */
   virtual bool isEqualTo(const rspfDataObject& rhs,
                          bool deepTest=false)const=0;
   
   virtual const rspfDataObject* operator =(const rspfDataObject* rhs);
   
   virtual const rspfDataObject& operator=(const rspfDataObject& rhs);

   virtual bool operator==(const rspfDataObject& rhs) const;
   virtual bool operator!=(const rspfDataObject& rhs) const;

   virtual std::ostream& print(std::ostream& out) const;
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix=0)const;
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

protected:

   //***
   // Note:  This object is not responsible for deletion of the rspfSource*
   //        "theSource".  It is simply a hook to his owner for things
   //        like unregister and so forth...
   //***
   rspfSource*                      theOwner;
   mutable rspfDataObjectStatus     theDataObjectStatus;

TYPE_DATA
};

#endif
