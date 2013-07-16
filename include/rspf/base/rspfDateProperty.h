//*******************************************************************
//
// LICENSE: LGPL see top level license.txt
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfDateProperty.h 9094 2006-06-13 19:12:40Z dburken $
#ifndef rspfDateProperty_HEADER
#define rspfDateProperty_HEADER
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfDate.h>

class RSPF_DLL rspfDateProperty : public rspfProperty
{
public:
   rspfDateProperty();
   rspfDateProperty(const rspfString& name,
                     const rspfString& value);
   rspfDateProperty(const rspfString& name,
                     const rspfLocalTm& value);
   rspfDateProperty(const rspfDateProperty& src);
   virtual rspfObject* dup()const;
   
   void setDate(const rspfLocalTm& localTm);
   const rspfLocalTm& getDate()const;
   virtual bool setValue(const rspfString& value);
   virtual void valueToString(rspfString& valueResult)const;
   virtual const rspfProperty& assign(const rspfProperty& rhs);
   
protected:
   rspfLocalTm theValue;

TYPE_DATA
};

#endif
