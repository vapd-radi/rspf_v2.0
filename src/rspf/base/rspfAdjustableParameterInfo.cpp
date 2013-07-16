//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAdjustableParameterInfo.cpp 11347 2007-07-23 13:01:59Z gpotts $
#include <sstream>
#include <algorithm>
#include <rspf/base/rspfAdjustableParameterInfo.h>
#include <rspf/base/rspfUnitTypeLut.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfCommon.h>
// static const char* PARAM_NAME_KW       = "name";
// static const char* PARAM_UNITS_KW      = "units";
static const char* PARAM_KW            = "parameter";
static const char* PARAM_SIGMA_KW      = "sigma";
static const char* PARAM_CENTER_KW     = "center";
static const char* PARAM_LOCK_FLAG_KW  = "lock_flag";

std::ostream& operator <<(std::ostream& out, const rspfAdjustableParameterInfo& data)
{
   out << "description: " << data.theDescription << std::endl
       << "center:      " << data.theCenter <<  std::endl
       << "parameter:   " << data.theParameter << std::endl
       << "sigma:       " << data.theSigma << std::endl
       << "units:       " << (rspfUnitTypeLut::instance()->getEntryString(data.theUnit))
       << std::endl
       << "locked:       " << (data.theLockFlag?"true":"false") << std::endl;
   
   return out;
}


rspfString rspfAdjustableParameterInfo::getUnitAsString()const
{
   return rspfUnitTypeLut::instance()->getEntryString((int)theUnit);
}

void rspfAdjustableParameterInfo::setCenter(double center)
{
   if(!theLockFlag)
   {
      theCenter = center;
   }
}

double rspfAdjustableParameterInfo::getCenter()const
{
  return theCenter;
}

double rspfAdjustableParameterInfo::computeOffset()const
{
  return theCenter + theSigma*theParameter;
}

void rspfAdjustableParameterInfo::setOffset(rspf_float64 value)
{
   if(!theLockFlag)
   {
      double minValue = theCenter - theSigma;
      double maxValue = theCenter + theSigma;
      double x = 0.0;
      
      if(std::abs(theSigma) > DBL_EPSILON)
      {
         x = (value - theCenter)/theSigma;
         
         value = theCenter + x*theSigma;
         
         if(value < minValue)
         {
            x = -1;
         }
         else if(value > maxValue)
         {
            x = 1.0;
         }
         theParameter = x;
      }
   }
}

bool rspfAdjustableParameterInfo::loadState(const rspfKeywordlist& kwl,
                                             const rspfString& prefix)
{
   const char* param       = kwl.find(prefix, PARAM_KW);
   const char* sigma       = kwl.find(prefix, PARAM_SIGMA_KW);
   const char* center      = kwl.find(prefix, PARAM_CENTER_KW);
   const char* unit        = kwl.find(prefix, rspfKeywordNames::UNITS_KW);
   const char* locked      = kwl.find(prefix, PARAM_LOCK_FLAG_KW);
   
   theDescription          = kwl.find(prefix, rspfKeywordNames::DESCRIPTION_KW);
   
   if(param)
   {
      theParameter = rspfString(param).toDouble();
   }
   else
   {
      theParameter = 0.0;
   }
   if(unit)
   {
      theUnit = (rspfUnitType)(rspfUnitTypeLut::instance()->getEntryNumber(unit));
   }
   else
   {
      theUnit      = RSPF_UNIT_UNKNOWN;
   }
   if(sigma)
   {
      theSigma  = rspfString(sigma).toDouble();
   }
   else
   {
      theSigma     = 0.0;
   }
   if(center)
   {
     theCenter = rspfString(center).toDouble();
   }
   else
   {
     theCenter = 0.0;
   }
   if(locked)
   {
      theLockFlag = rspfString(locked).toBool();
   }
   
   return true;
}

bool rspfAdjustableParameterInfo::saveState(rspfKeywordlist& kwl,
                                             const rspfString& prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::DESCRIPTION_KW,
           theDescription,
           true);
   kwl.add(prefix, rspfKeywordNames::UNITS_KW,
           rspfUnitTypeLut::instance()->getEntryString(theUnit), true);
   kwl.add(prefix, PARAM_KW, theParameter, true);
   kwl.add(prefix, PARAM_SIGMA_KW, theSigma, true);
   kwl.add(prefix, PARAM_CENTER_KW, theCenter, true);
   kwl.add(prefix, PARAM_LOCK_FLAG_KW, theLockFlag, true);
   
   return true;
}
