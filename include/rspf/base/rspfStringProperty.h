//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL see top level license.txt
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfStringProperty.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfStringProperty_HEADER
#define rspfStringProperty_HEADER
#include <rspf/base/rspfProperty.h>
#include <vector>

class RSPFDLLEXPORT rspfStringProperty : public rspfProperty
{
public:
   rspfStringProperty(const rspfString& name = rspfString(""),
                       const rspfString& value = rspfString(""),
                       bool editableFlag = true,
                       const std::vector<rspfString>& constraintList = std::vector<rspfString>());
   
   rspfStringProperty(const rspfStringProperty& rhs);

   /*!
    * Duplicates the object and returns a new one
    */
   virtual rspfObject* dup()const;

   virtual const rspfProperty& assign(const rspfProperty& rhs);
   
   void setEditableFlag(bool flag);
   bool getEditableFlag()const;
   bool isEditable()const;
   
   /*!
    * Clears the constraints for this string.  You can constrain
    * it to a string list by calling setConstraints.
    */
   void clearConstraints();

   /*!
    * Sets the constraints for this property.  If the editable
    * flag is present then this says the editing is not readonly
    * and one can hand type the value in without only selecting
    * from a choice list.
    */
   void setConstraints(const std::vector<rspfString>& constraintList);

   void addConstraint(const rspfString& value);
   const std::vector<rspfString>& getConstraints()const;
   
   bool hasConstraints()const;
   
   /*!
    * will try to set the value.  If the constraints are set
    * it will make sure that the value is in the list and
    * if its not then it will return false 
    */
   virtual bool setValue(const rspfString& value);
   
   virtual void valueToString(rspfString& valueResult)const;
  
   
protected:
   rspfString theValue;
   bool        theEditableFlag;
   std::vector<rspfString> theConstraints;

TYPE_DATA
};

#endif
