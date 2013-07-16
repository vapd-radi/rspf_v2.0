//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfUnitConversionTool.h 10378 2007-01-26 14:27:21Z gpotts $
#ifndef rspfUnitConversionTool_HEADER
#define rspfUnitConversionTool_HEADER
#include <iostream>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfGpt.h>

class RSPF_DLL rspfUnitConversionTool
{
public:
   friend std::ostream& operator<<(std::ostream& out,
                                   const rspfUnitConversionTool& data);

   rspfUnitConversionTool(double value=1.0,
                           rspfUnitType unitType=RSPF_METERS);

   rspfUnitConversionTool(const rspfGpt& origin,
                           double value,
                           rspfUnitType unitType=RSPF_METERS);

   void setOrigin(const rspfGpt& gpt);

   rspfGpt getOrigin()const;

   void getOrigin(rspfGpt& result)const;

   void setValue(double value, rspfUnitType unitType=RSPF_METERS);

   double getValue(rspfUnitType unitType=RSPF_METERS) const;
   
   void setMeters(double value);
   void setRadians(double value);
   void setDegrees(double value);
   void setMinutes(double value);
   void setSeconds(double value);
   void setUsSurveyFeet(double value);
   void setFeet(double value);
   void setNauticalMiles(double value);
   void setMiles(double value);
   void setMillimeters(double value);
   void setMicrons(double value);
   void setCentimeters(double value);
   void setYards(double value);
   void setInches(double value);
   void setKilometers(double value);
   double getMeters()const;
   double getRadians()const;
   double getDegrees()const;
   double getMinutes()const;
   double getSeconds()const;
   double getUsSurveyFeet()const;
   double getFeet()const;
   double getNauticalMiles()const;
   double getMiles()const;
   double getMillimeters()const;
   double getMicrons()const;
   double getCentimeters()const;
   double getYards()const;
   double getInches()const;
   double getKilometers()const;
   
protected:
   double         theValue;
   rspfUnitType  theUnitType;
   rspfGpt       theOrigin;

   double computeMeters()const;
};

#endif
