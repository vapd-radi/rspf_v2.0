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
// $Id: rspfUnitConversionTool.cpp 17503 2010-06-02 11:18:49Z dburken $

#include <iostream>
#include <iomanip>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfGpt.h>

std::ostream& operator<<(std::ostream& out,
                         const rspfUnitConversionTool& data)
{
   out << std::setprecision(15) << std::setiosflags(std::ios::fixed)
       << "Meters:           " << data.getMeters()
       << "\nFeet:             " << data.getFeet()
       << "\nU.S. Survey Feet: " << data.getUsSurveyFeet()
       << "\nNautical miles:   " << data.getNauticalMiles()
       << "\nDegrees:          " << data.getDegrees()
       << "\nMinutes:          " << data.getMinutes()
       << "\nSeconds:          " << data.getSeconds() << std::endl;
   
   return out;
}

rspfUnitConversionTool::rspfUnitConversionTool(double value,
                                                 rspfUnitType unitType)
   : theValue(value),
     theUnitType(unitType),
     theOrigin()
{
}

rspfUnitConversionTool::rspfUnitConversionTool(const rspfGpt& origin,
                                                 double value,
                                                 rspfUnitType unitType)
   : theValue(value),
     theUnitType(unitType),
     theOrigin(origin)
{
}

void rspfUnitConversionTool::setOrigin(const rspfGpt& gpt)
{
   theOrigin = gpt;
}

rspfGpt rspfUnitConversionTool::getOrigin()const
{
   return theOrigin;
}

void rspfUnitConversionTool::getOrigin(rspfGpt& result)const
{
   result = theOrigin;
}

void rspfUnitConversionTool::setValue(double value,
                                       rspfUnitType unitType)
{
   theValue    = value;
   theUnitType = unitType;
}

double rspfUnitConversionTool::getValue(rspfUnitType unitType) const
{
   switch(unitType)
   {
      case RSPF_METERS:
      {
         return getMeters();
      }
      case RSPF_RADIANS:
      {
         return getRadians();
      }
      case RSPF_DEGREES:
      {
         return getDegrees();
      }
      case RSPF_US_SURVEY_FEET:
      {
         return getUsSurveyFeet();
      }
      case RSPF_FEET:
      {
         return getFeet();
      }
      case RSPF_SECONDS:
      {
         return getSeconds();
      }
      case RSPF_MINUTES:
      {
         return getMinutes();
      }
      case RSPF_NAUTICAL_MILES:
      {
         return getNauticalMiles();
      }
      case RSPF_MILES:
      {
         return getMiles();
      }
      case RSPF_MICRONS:
      {
         return getMicrons();
      }
      case RSPF_CENTIMETERS:
      {
         return getCentimeters();
      }
      case RSPF_MILLIMETERS:
      {
         return getMillimeters();
      }
      case RSPF_YARDS:
      {
         return getYards();
      }
      case RSPF_INCHES:
      {
         return getInches();
      }
      case RSPF_KILOMETERS:
      {
         return getKilometers();
      }
      default:
         break;
   }

   return rspf::nan();
}

void rspfUnitConversionTool::setMeters(double value)
{
   setValue(value, RSPF_METERS);
}

void rspfUnitConversionTool::setRadians(double value)
{
   setValue(value, RSPF_RADIANS);
}

void rspfUnitConversionTool::setDegrees(double value)
{
   setValue(value, RSPF_DEGREES);
}

void rspfUnitConversionTool::setMinutes(double value)
{
   setValue(value, RSPF_MINUTES);
}

void rspfUnitConversionTool::setSeconds(double value)
{
   setValue(value, RSPF_SECONDS);
}

void rspfUnitConversionTool::setUsSurveyFeet(double value)
{
   setValue(value, RSPF_US_SURVEY_FEET);
}

void rspfUnitConversionTool::setFeet(double value)
{
   setValue(value, RSPF_FEET);
}

void rspfUnitConversionTool::setNauticalMiles(double value)
{
   setValue(value, RSPF_NAUTICAL_MILES);
}

void rspfUnitConversionTool::setMiles(double value)
{
   setValue(value, RSPF_MILES);
}

void rspfUnitConversionTool::setMillimeters(double value)
{
   setValue(value, RSPF_MILLIMETERS);
}

void rspfUnitConversionTool::setMicrons(double value)
{
   setValue(value, RSPF_MICRONS);
}

void rspfUnitConversionTool::setCentimeters(double value)
{
   setValue(value, RSPF_CENTIMETERS);
}

void rspfUnitConversionTool::setYards(double value)
{
   setValue(value, RSPF_YARDS);
}

void rspfUnitConversionTool::setInches(double value)
{
   setValue(value, RSPF_INCHES);
}

void rspfUnitConversionTool::setKilometers(double value)
{
   setValue(value, RSPF_KILOMETERS);
}

double rspfUnitConversionTool::getMeters()const
{
   if(theUnitType == RSPF_METERS)
   {
      return theValue;
   }
   
   return computeMeters();
}

double rspfUnitConversionTool::getRadians()const
{
   if(theUnitType == RSPF_RADIANS)
   {
      return theValue;
   }
   return getDegrees()*RAD_PER_DEG;
}

double rspfUnitConversionTool::getDegrees()const
{
   switch (theUnitType)
   {
      case RSPF_DEGREES:
      {
         return theValue;
      }
      case RSPF_MINUTES:
      {
         return (theValue / 60.0);
      }
      case RSPF_SECONDS:
      {
         return (theValue / 3600.0);
      }
      case RSPF_RADIANS:
      {
         return (theValue * DEG_PER_RAD);
      }
      default:
         break;
   }
   
   rspfDpt pt = theOrigin.metersPerDegree();
   return (computeMeters() /((pt.x+pt.y)*.5));
}

double rspfUnitConversionTool::getMinutes()const
{
   if(theUnitType == RSPF_MINUTES)
   {
      return theValue;
   }
   return (getDegrees()*60.0);
}

double rspfUnitConversionTool::getSeconds()const
{
   if(theUnitType == RSPF_SECONDS)
   {
      return theValue;
   }
   return (getDegrees()*3600.0);
}

double rspfUnitConversionTool::getUsSurveyFeet()const
{
   if(theUnitType == RSPF_US_SURVEY_FEET)
   {
      return theValue;
   }
   return (computeMeters()/US_METERS_PER_FT);
}

double rspfUnitConversionTool::getFeet()const
{
   if(theUnitType == RSPF_FEET)
   {
      return theValue;
   }
   return (computeMeters()*FT_PER_MTRS);
}

double rspfUnitConversionTool::getNauticalMiles()const
{
   if(theUnitType == RSPF_NAUTICAL_MILES)
   {
      return theValue;
   }
   
   return (computeMeters()/(theOrigin.metersPerDegree().y/60.0));
}

double rspfUnitConversionTool::getMiles()const
{
   if(theUnitType == RSPF_MILES)
   {
      return theValue;
   }
   return ((computeMeters()*FT_PER_MTRS)/FT_PER_MILE);
}

double rspfUnitConversionTool::getMillimeters()const
{
   if(theUnitType == RSPF_MILLIMETERS)
   {
      return theValue;
   }
   return ((computeMeters()*1e3));
}

double rspfUnitConversionTool::getMicrons()const
{
   if(theUnitType == RSPF_MICRONS)
   {
      return theValue;
   }
   return ((computeMeters()*1e6));
}

double rspfUnitConversionTool::getCentimeters()const
{
   if(theUnitType == RSPF_KILOMETERS)
   {
      return theValue;
   }

   return ((computeMeters()*1e2));   
}

double rspfUnitConversionTool::getYards()const
{
   if(theUnitType == RSPF_INCHES)
   {
      return theValue;
   }

   return ((computeMeters()/0.914));   
}

double rspfUnitConversionTool::getInches()const
{
   if(theUnitType == RSPF_INCHES)
   {
      return theValue;
   }

   return ((getFeet()*12.0));   
}

double rspfUnitConversionTool::getKilometers()const
{
   if(theUnitType == RSPF_KILOMETERS)
   {
      return theValue;
   }
   return ((computeMeters()*1e-3));
}

double rspfUnitConversionTool::computeMeters()const
{
   switch(theUnitType)
   {
      case RSPF_METERS:
      {
         return theValue;
         break;
      }
      case RSPF_RADIANS:
      {
         rspfDpt pt = theOrigin.metersPerDegree();
         return (theValue*DEG_PER_RAD)*((pt.x+pt.y)*.5);
      }
      case RSPF_DEGREES:
      {
         rspfDpt pt = theOrigin.metersPerDegree();
         return theValue*((pt.x+pt.y)*.5);
      }
      case RSPF_US_SURVEY_FEET:
      {
         return  US_METERS_PER_FT*theValue;
      }
      case RSPF_FEET:
      {
         return MTRS_PER_FT*theValue;
      }
      case RSPF_SECONDS:
      {
         rspfDpt pt = theOrigin.metersPerDegree();
         return (theValue/3600.0)*((pt.x+pt.y)*.5);
      }
      case RSPF_MINUTES:
      {
         rspfDpt pt = theOrigin.metersPerDegree();
         return (theValue/60.0)*((pt.x+pt.y)*.5);
      }
      case RSPF_NAUTICAL_MILES:
      {
         return (theOrigin.metersPerDegree().y/60.0)*theValue;
      }
      case RSPF_MILES:
      {
         return MTRS_PER_FT*FT_PER_MILE*theValue;
      }
      case RSPF_MILLIMETERS:
      {
         return (theValue/(1e3));
      }
      case RSPF_MICRONS:
      {
         return (theValue/(1e6));
      }
      default:
         break;
   }

   return rspf::nan();
}
