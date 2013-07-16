//*******************************************************************
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfNumericProperty.h 9968 2006-11-29 14:01:53Z gpotts $
#ifndef rspfNumericProperty_HEADER
#define rspfNumericProperty_HEADER
#include <rspf/base/rspfProperty.h>

class RSPFDLLEXPORT rspfNumericProperty : public rspfProperty
{
public:
   enum rspfNumericPropertyType
   {
      rspfNumericPropertyType_INT   = 0,
      rspfNumericPropertyType_UINT,
      rspfNumericPropertyType_FLOAT32,
      rspfNumericPropertyType_FLOAT64
   };
   
   rspfNumericProperty(const rspfString& name=rspfString(""),
                        const rspfString& value=rspfString("0"));
   rspfNumericProperty(const rspfString& name,
                        const rspfString& value,
                        double minValue,
                        double maxValue);
   rspfNumericProperty(const rspfNumericProperty& rhs);
   virtual rspfObject* dup()const;
   virtual const rspfProperty& assign(const rspfProperty& rhs);
   
   virtual bool hasConstraints()const;
   double getMinValue()const;
   double getMaxValue()const;
   
   virtual void clearConstraints();
   virtual void setConstraints(double minValue,
                               double maxValue);
  virtual bool setValue(const rspfString& value);
   virtual void valueToString(rspfString& valueResult)const;

   virtual rspfNumericPropertyType getNumericType()const;
   virtual void setNumericType(rspfNumericPropertyType type);
   
   virtual rspf_float64 asFloat64()const;
   virtual rspf_float32 asFloat32()const;
   virtual rspf_uint32  asUInt32()const;
   virtual rspf_uint16  asUInt16()const;
   virtual rspf_uint8   asUInt8()const;
   virtual rspf_sint32  asInt32()const;
   virtual rspf_sint16  asInt16()const;
   virtual rspf_sint8   asInt8()const;
   
protected:
   rspfString              theValue;
   rspfNumericPropertyType theType;
   std::vector<double>      theRangeConstraint;
TYPE_DATA
};


#endif
