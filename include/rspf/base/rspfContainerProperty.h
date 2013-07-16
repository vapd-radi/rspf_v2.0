//*******************************************************************
//
// LICENSE: See top level LICENSE.txt
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfContainerProperty.h 19917 2011-08-09 11:12:24Z gpotts $
#ifndef rspfContainerProperty_HEADER
#define rspfContainerProperty_HEADER
#include <vector>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfProperty.h>

class RSPFDLLEXPORT rspfContainerProperty : public rspfProperty
{
public:
   friend class rspfProperty;
   rspfContainerProperty(const rspfString& name=rspfString(""));
   rspfContainerProperty(const rspfContainerProperty& rhs);
   virtual ~rspfContainerProperty();
   
   virtual rspfObject* dup()const;

   virtual const rspfProperty& assign(const rspfProperty& rhs);

   virtual void addChildren(std::vector<rspfRefPtr<rspfProperty> >& propertyList);

   virtual void addChild(rspfProperty* propery);

   virtual void addStringProperty(const rspfString& name,
                                  const rspfString& value,
                                  bool readOnlyFlag = false);
   
   virtual const rspfContainerProperty* asContainer()const;
   virtual rspfContainerProperty* asContainer();
   
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name,
                                                  bool recurse=false);
   virtual void deleteChildren();
   virtual bool setValue(const rspfString& value);
   virtual void valueToString(rspfString& valueResult)const;

   virtual rspf_uint32 getNumberOfProperties()const;
   virtual rspfRefPtr<rspfProperty> getProperty(rspf_uint32 idx);

   virtual void getPropertyList(
      std::vector<rspfRefPtr<rspfProperty> >& children) const;
   
   virtual rspfRefPtr<rspfXmlNode> toXml()const;
   virtual void setReadOnlyFlag(bool flag);

   virtual void accept(rspfVisitor& visitor);
protected:
   std::vector<rspfRefPtr<rspfProperty> > theChildPropertyList;

   virtual void copyList(const rspfContainerProperty& rhs);

   
TYPE_DATA
};

#endif
