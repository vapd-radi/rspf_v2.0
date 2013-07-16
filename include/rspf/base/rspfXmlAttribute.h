//*******************************************************************
// Copyright (C) 2001 ImageLinks Inc.  All rights reserved.
//
// License:  See top level LICENSE.txt file.
// 
// Author:  Oscar Kramer (rspf port by D. Burken)
//
// Description:  
//
// Contains declaration of class rspfAttribute.
//
//*****************************************************************************
// $Id: rspfXmlAttribute.h 12521 2008-02-28 20:09:25Z gpotts $
#ifndef rspfXmlAttribute_HEADER
#define rspfXmlAttribute_HEADER

#include <rspf/base/rspfErrorStatusInterface.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfObject.h>

class RSPFDLLEXPORT rspfXmlAttribute : public rspfObject,
                                         public rspfErrorStatusInterface
{
public:
   rspfXmlAttribute(rspfString& spec);  // attribute removed from spec string
   rspfXmlAttribute();
   rspfXmlAttribute(const rspfXmlAttribute& src);
   rspfXmlAttribute(const rspfString& name,
                     const rspfString& value);
   virtual rspfObject* dup() const
   {
      return new rspfXmlAttribute(*this);
   }
   ~rspfXmlAttribute();

   bool read(std::istream& in);
   const rspfString& getName()  const;
   const rspfString& getValue() const;
   void setNameValue(const rspfString& name,
                     const rspfString& value);
   void setName(const rspfString& name);
   void setValue(const rspfString& value);
   
   RSPFDLLEXPORT friend std::ostream& operator<<(
      std::ostream& os, const rspfXmlAttribute* xml_attr);

protected:
   rspfString theName;
   rspfString theValue;

   bool readName(std::istream& in);
   bool readValue(std::istream& in);
TYPE_DATA
};

#endif /* #ifndef rspfXmlAttribute_HEADER */
