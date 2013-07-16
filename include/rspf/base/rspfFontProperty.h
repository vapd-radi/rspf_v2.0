//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: rspfFontProperty.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfFontProperty_HEADER
#define rspfFontProperty_HEADER
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfFontInformation.h>

class RSPFDLLEXPORT rspfFontProperty : public rspfProperty
{
public:
   rspfFontProperty(const rspfString& name,
                     const rspfFontInformation& value);
   rspfFontProperty(const rspfFontProperty& rhs);
   virtual ~rspfFontProperty();
   virtual rspfObject* dup()const;
   virtual const rspfProperty& assign(const rspfProperty& rhs);
  virtual bool setValue(const rspfString& value);
   virtual void valueToString(rspfString& valueResult)const;

   void setFontInformation(rspfFontInformation& fontInfo);
   const rspfFontInformation& getFontInformation()const;
protected:
   rspfFontInformation theValue;

TYPE_DATA
};

#endif
