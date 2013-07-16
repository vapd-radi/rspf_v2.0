//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfColorProperty.h 15766 2009-10-20 12:37:09Z gpotts $
#ifndef rspfColorProperty_HEADER
#define rspfColorProperty_HEADER
#include <rspf/base/rspfProperty.h>
#include <rspf/base/rspfRtti.h>
#include <rspf/base/rspfRgbVector.h>

class RSPFDLLEXPORT rspfColorProperty : public rspfProperty
{
public:
   rspfColorProperty(const rspfString& name=rspfString(""),
                      const rspfRgbVector& value = rspfRgbVector(0,0,0));
   rspfColorProperty(const rspfColorProperty& rhs);
   virtual rspfObject* dup()const;
   virtual const rspfProperty& assign(const rspfProperty& rhs);
   virtual bool setValue(const rspfString& value);
   virtual void valueToString(rspfString& valueResult)const;
   const rspfRgbVector& getColor()const;
   void setColor(const rspfRgbVector& value);
   rspf_uint8 getRed()const;
   rspf_uint8 getGreen()const;
   rspf_uint8 getBlue()const;
   void setRed(rspf_uint8 r);
   void setGreen(rspf_uint8 r);
   void setBlue(rspf_uint8 r);
protected:
   virtual ~rspfColorProperty();
   rspfRgbVector theValue;

TYPE_DATA
};

#endif
