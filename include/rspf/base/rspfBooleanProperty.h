//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfBooleanProperty.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfBoolenaProperty_HEADER
#define rspfBoolenaProperty_HEADER
#include <rspf/base/rspfProperty.h>

class RSPFDLLEXPORT rspfBooleanProperty : public rspfProperty
{
public:
   rspfBooleanProperty(const rspfString& name=rspfString(""),
                        bool value = false);
   rspfBooleanProperty(const rspfBooleanProperty& rhs);
   virtual rspfObject* dup()const;
   virtual const rspfProperty& assign(const rspfProperty& rhs);
   virtual bool setValue(const rspfString& value);
   bool setBooleanValue(bool value,
			rspfString& msg);
   void valueToString(rspfString& valueResult)const;
   bool getBoolean()const;
   
protected:
   bool theValue;

TYPE_DATA
};

#endif
