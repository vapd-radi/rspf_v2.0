//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfStringListProperty.h 9968 2006-11-29 14:01:53Z gpotts $
//
#ifndef rspfStringListProperty_HEADER
#define rspfStringListProperty_HEADER
#include <rspf/base/rspfProperty.h>

class RSPFDLLEXPORT rspfStringListProperty : public rspfProperty
{
public:
   rspfStringListProperty(const rspfString& name=rspfString(""),
                           const std::vector<rspfString>& value = std::vector<rspfString>());
   rspfStringListProperty(const rspfStringListProperty& rhs);
   
   rspfObject* dup()const;
   virtual const rspfProperty& assign(const rspfProperty& rhs);
  virtual bool setValue(const rspfString& value);
   virtual void valueToString(rspfString& valueResult)const;

   void clearValueList();
   rspfString getValueAt(int idx)const;
   bool setValueAt(int idx,
                   const rspfString& value);
   bool addValue(const rspfString& value);
   rspf_uint32 getNumberOfValues()const;

   rspf_uint32 getNumberOfContraints()const;
   rspfString getConstraintAt(rspf_uint32 idx)const;
   
   void setConstraints(const std::vector<rspfString>& constraints);
   bool hasConstraints()const;
   void setUniqueFlag(bool flag);
   void setOrderMattersFlag(bool flag);

   /*!
    * You can specify how many values are allowed in the value list
    * If any of the values are less than 0 then now constraints are
    * placed on that value.  So if the min was < 0 then there is
    * no minumum number of values.  If the max is < 0 then there is
    * no maximum number of values.
    */
   void setNumberOfValuesBounds(int minNumber,
                                int maxNumber);
   void getNumberofValuesBounds(int& minNumber,
                                int& maxNumber)const;
protected:
   std::vector<rspfString> theValueList;
   std::vector<rspfString> theConstraintList;
   bool                     theUniqueFlag;
   bool                     theOrderMattersFlag;
   int                      theMinNumberOfValues;
   int                      theMaxNumberOfValues;
   
   bool findValueInConstraintList(const rspfString& value)const;
   bool findValueInValueList(const rspfString& value)const;
   bool canAddValue(const rspfString& value)const;
TYPE_DATA
};

#endif
