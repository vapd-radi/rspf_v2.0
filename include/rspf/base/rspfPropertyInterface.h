//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// LICENSE: LGPL see top level license.txt
//
// Author: Garrett Potts (gpotts@imagelinks)
//
//*************************************************************************
// $Id: rspfPropertyInterface.h 12989 2008-06-04 17:05:41Z gpotts $
#ifndef rspfPropertyInterface_HEADER
#define rspfPropertyInterface_HEADER
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfObject.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfProperty.h>


class RSPF_DLL rspfPropertyInterface
{
public:
   rspfPropertyInterface()
      {}
   
   virtual ~rspfPropertyInterface()
      {}

   virtual void setProperty(const rspfString& name,
                            const rspfString& value);
   virtual void setProperty(rspfRefPtr<rspfProperty> property);
   virtual rspfRefPtr<rspfProperty> getProperty(const rspfString& name)const; 
	virtual rspfString getPropertyValueAsString(const rspfString& name)const;
   virtual void getPropertyNames(std::vector<rspfString>& propertyNames)const;
   void getPropertyList(std::vector<rspfRefPtr<rspfProperty> >& propertyList)const;
   void setProperties(std::vector<rspfRefPtr<rspfProperty> >& propertyList);
   
protected:
   
TYPE_DATA
};

#endif
