//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: Base class for rspf objects.
//
//*************************************************************************
// $Id: rspfObject.h 19852 2011-07-21 15:26:12Z gpotts $

#ifndef rspfObject_HEADER
#define rspfObject_HEADER

#include <iosfwd>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfRtti.h>

class rspfKeywordlist;
class rspfString;
class rspfVisitor;

class RSPFDLLEXPORT rspfObject : public rspfReferenced
{
public:
   rspfObject();
   virtual ~rspfObject();
   
   virtual rspfObject* dup() const;
   
   virtual rspfString getShortName()   const;
   virtual rspfString getLongName()    const;
   virtual rspfString getDescription() const;
   virtual rspfString getClassName()   const;
   
   /*!
    * Will return the type of this class in an RTTI format.
    */
   virtual RTTItypeid getType() const;

   virtual bool canCastTo(rspfObject* obj) const;
   
   virtual bool canCastTo(const RTTItypeid& id) const;
   
   /*!
    * Will use RTTI to search the derived classes to see if it derives from
    * the passed in type.  If so then this object can be casted to the
    * passed in class name.
    */
   virtual bool canCastTo(const rspfString& parentClassName) const;

   /*!
    * Method to save the state of the object to a keyword list.
    * Return true if ok or false on error.
    */
   virtual bool saveState(rspfKeywordlist& kwl,
                          const char* prefix=0) const;

   /*!
    * Method to the load (recreate) the state of the object from a keyword
    * list.  Return true if ok or false on error.
    */
   virtual bool loadState(const rspfKeywordlist& kwl,
                          const char* prefix=0);
   
   /**
    * Generic print method.  Derived classes should re-implement as they see
    * fit.
    * 
    * @return std::ostream&
    */
   virtual std::ostream& print(std::ostream& out) const;

   /**
    * @note  Since the print method is virtual, derived classes only need
    *        to implement that, not an addition operator<<.
    */
   friend RSPFDLLEXPORT std::ostream& operator<<(std::ostream& out,
                                                  const rspfObject& obj);
   
   virtual bool isEqualTo(const rspfObject& obj, rspfCompareType compareType = RSPF_COMPARE_FULL)const;
   virtual void accept(rspfVisitor& visitor);

 protected:
TYPE_DATA
};

#endif /* #ifndef rspfObject_HEADER */
