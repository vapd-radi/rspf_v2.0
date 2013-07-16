//*******************************************************************
//
// License:  See top level LICENSE.txt file.
// 
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfTextProperty.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfTextProperty_HEADER
#define rspfTextProperty_HEADER
#include <rspf/base/rspfProperty.h>

class RSPFDLLEXPORT rspfTextProperty : public rspfProperty
{
public:
   rspfTextProperty(const rspfString& name  = rspfString(""),
                     const rspfString& value = rspfString(""),
                     bool isMultiLine = false);
   rspfTextProperty(const rspfTextProperty& rhs);
   virtual rspfObject* dup()const;

   bool isMulitLine()const;
   void setMultiLineFlag(bool flag);

   virtual void valueToString(rspfString& valueResult)const;
  virtual bool setValue(const rspfString& value);
   virtual const rspfProperty& assign(const rspfProperty& rhs);
   
protected:
   rspfString theValue;
   bool theMultiLineFlag;

TYPE_DATA
};

#endif
