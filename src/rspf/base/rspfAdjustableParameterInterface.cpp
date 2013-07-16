//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAdjustableParameterInterface.cpp 20610 2012-02-27 12:19:25Z gpotts $
#include <algorithm>
#include <rspf/base/rspfAdjustableParameterInterface.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfUnitTypeLut.h>

RTTI_DEF(rspfAdjustableParameterInterface, "rspfAdjustableParameterInterface");

static const char* NUMBER_OF_ADJUSTMENTS_KW = "number_of_adjustments";
static const char* NUMBER_OF_ADJUSTMENTS_OLD_KW = "number_of_adjustements";
static const char* CURRENT_ADJUSTMENT_OLD_KW    = "current_adjustement";
static const char* CURRENT_ADJUSTMENT_KW    = "current_adjustment";
static const char* ADJUSTMENT_PREFIX        = "adjustment_";

rspfAdjustableParameterInterface::rspfAdjustableParameterInterface()
{
   theCurrentAdjustment = 0;
}

rspfAdjustableParameterInterface::rspfAdjustableParameterInterface(const rspfAdjustableParameterInterface& rhs)
   :theAdjustmentList(rhs.theAdjustmentList),
    theCurrentAdjustment(rhs.theCurrentAdjustment)
{
}

void rspfAdjustableParameterInterface::newAdjustment(rspf_uint32 numberOfParameters)
{
   theAdjustmentList.push_back(rspfAdjustmentInfo());
   if(numberOfParameters > 0)
   {
      theAdjustmentList[theAdjustmentList.size()-1].setNumberOfAdjustableParameters(numberOfParameters);
      theAdjustmentList[theAdjustmentList.size()-1].setDescription("Initial adjustment");
   }

   theCurrentAdjustment = (rspf_uint32)theAdjustmentList.size() - 1;

}

void rspfAdjustableParameterInterface::setAdjustmentDescription(const rspfString& description)
{
   if(!theAdjustmentList.size())
   {
      return;
   }
   theAdjustmentList[theCurrentAdjustment].setDescription(description);
}

rspfString rspfAdjustableParameterInterface::getAdjustmentDescription()const
{
   if(theAdjustmentList.size())
   {
      return theAdjustmentList[theCurrentAdjustment].getDescription();
   }

   return "";
}

void rspfAdjustableParameterInterface::setCurrentAdjustment(rspf_uint32 adjustmentIdx, bool notify)
{
   if(adjustmentIdx < theAdjustmentList.size())
   {
      theCurrentAdjustment = adjustmentIdx;
      if(notify)
      {
         adjustableParametersChanged();
      }
   }
}


void rspfAdjustableParameterInterface::initAdjustableParameters()
{
}

void rspfAdjustableParameterInterface::resetAdjustableParameters(bool notify)
{
    if(!theAdjustmentList.size())
    {
       return;
    }
    
    rspf_uint32 saveCurrent = theCurrentAdjustment;
    copyAdjustment();
    initAdjustableParameters();
    rspf_uint32 numberOfAdjustables = getNumberOfAdjustableParameters();
    rspf_uint32 idx = 0;
    
    for(idx = 0; idx < numberOfAdjustables; ++idx)
    {
       theAdjustmentList[saveCurrent].getParameterList()[idx].setParameter(theAdjustmentList[theAdjustmentList.size()-1].getParameterList()[idx].getParameter());
    }

    setCurrentAdjustment(saveCurrent);

    eraseAdjustment((rspf_uint32)theAdjustmentList.size()-1, false);
    
    if(notify)
    {
       adjustableParametersChanged();
    }
}

void rspfAdjustableParameterInterface::copyAdjustment(rspf_uint32 idx, bool notify)
{
    if(!theAdjustmentList.size())
    {
       return;
    }
    if(idx < theAdjustmentList.size())
    {
       theAdjustmentList.push_back(theAdjustmentList[idx]);

       if(idx == theCurrentAdjustment)
       {
          theCurrentAdjustment = (rspf_uint32)theAdjustmentList.size() - 1;
       }
       if(notify)
       {
          adjustableParametersChanged();
       }
    }
    
}

void rspfAdjustableParameterInterface::copyAdjustment(bool notify)
{
   copyAdjustment(theCurrentAdjustment, notify);
}

void rspfAdjustableParameterInterface::keepAdjustment(rspf_uint32 idx,
                                                       bool createCopy)
{
    if(!theAdjustmentList.size())
    {
       return;
    }
    if(idx < theAdjustmentList.size())
    {
       if(createCopy)
       {
          copyAdjustment(idx);
       }
       theAdjustmentList[theCurrentAdjustment].keep();
    }
}

void rspfAdjustableParameterInterface::keepAdjustment(bool createCopy)
{
   keepAdjustment(theCurrentAdjustment, createCopy);
}

const rspfAdjustableParameterInterface& rspfAdjustableParameterInterface::operator = (const rspfAdjustableParameterInterface& rhs)
{
   theAdjustmentList    = rhs.theAdjustmentList;
   theCurrentAdjustment = rhs.theCurrentAdjustment;

   return *this;
}

void rspfAdjustableParameterInterface::removeAllAdjustments()
{
   theAdjustmentList.clear();
   theCurrentAdjustment = 0;
}

rspf_uint32 rspfAdjustableParameterInterface::getNumberOfAdjustableParameters()const
{
   if(theAdjustmentList.size())
   {
      return theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters();
   }

   return 0;
}

void rspfAdjustableParameterInterface::eraseAdjustment(bool notify)
{
   eraseAdjustment(theCurrentAdjustment, notify);
}

void rspfAdjustableParameterInterface::eraseAdjustment(rspf_uint32 idx, bool notify)
{
   if(!theAdjustmentList.size())
   {
      return;
   }
   
   if(theCurrentAdjustment == idx)
   {
      theAdjustmentList.erase(theAdjustmentList.begin() + theCurrentAdjustment);
      if(theCurrentAdjustment >= theAdjustmentList.size())
      {
         if(theAdjustmentList.size() < 1)
         {
            theCurrentAdjustment = 0;
         }
         else
         {
            theCurrentAdjustment = (rspf_uint32)theAdjustmentList.size() - 1;
         }
         
      }
      
      if(notify)
      {
         adjustableParametersChanged();
      }
   }
   else if(idx < theAdjustmentList.size())
   {
      theAdjustmentList.erase(theAdjustmentList.begin() + idx);
      if(theAdjustmentList.size() < 1)
      {
         theCurrentAdjustment = 0;
      }
      else
      {
         if(theCurrentAdjustment > idx)
         {
            --theCurrentAdjustment;
            if(notify)
            {
               adjustableParametersChanged();
            }
         }
      }
      if(notify)
      {
         adjustableParametersChanged();
      }
   }
}

double rspfAdjustableParameterInterface::getAdjustableParameter(rspf_uint32 idx)const
{
   if(theAdjustmentList.size())
   {
      if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
      {
         return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getParameter();
      }
   }
   
   return 0.0;
}

void rspfAdjustableParameterInterface::setAdjustableParameter(rspf_uint32 idx, double value, double sigma, bool notify)
{
   if(!theAdjustmentList.size())
   {
      return;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setParameter(value);
      theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setSigma(sigma);
      if(notify)
      {
         adjustableParametersChanged();
      }
   }
   
}

void rspfAdjustableParameterInterface::setAdjustableParameter(rspf_uint32 idx, double value, bool notify)
{
   if(!theAdjustmentList.size())
   {
      return;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setParameter(value);

      if(notify)
      {
         adjustableParametersChanged();
      }
   }
}

double rspfAdjustableParameterInterface::getParameterSigma(rspf_uint32 idx)const
{
   if(theAdjustmentList.size())
   {
      if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
      {
         return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getSigma();
      }
   }

   return 0.0;
}

void rspfAdjustableParameterInterface::setParameterSigma(rspf_uint32 idx, double value, bool notify)
{
   if(!theAdjustmentList.size())
   {
      return;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setSigma(value);
      if(notify)
      {
         adjustableParametersChanged();
      }
   }
}

rspfUnitType rspfAdjustableParameterInterface::getParameterUnit(rspf_uint32 idx)const
{
   if(theAdjustmentList.size())
   {
      if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
      {
         return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getUnit();
      }
   }

   return RSPF_UNIT_UNKNOWN;
}

void rspfAdjustableParameterInterface::setParameterUnit(rspf_uint32 idx, rspfUnitType unit)
{
   if(theAdjustmentList.size())
   {
      if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
      {
         theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setUnit(unit);
      }
   }
}

void rspfAdjustableParameterInterface::setParameterUnit(rspf_uint32 idx, const rspfString& unit)
{
   if(theAdjustmentList.size())
   {
      if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
      {
         theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setUnit((rspfUnitType)rspfUnitTypeLut::instance()->getEntryNumber(unit));
      }
   }
}


rspfString rspfAdjustableParameterInterface::getParameterDescription(rspf_uint32 idx)const
{
   if(theAdjustmentList.size())
   {
      if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
      {
         return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getDescription();
      }
   }

   return rspfString("");
}

void rspfAdjustableParameterInterface::setParameterDescription(rspf_uint32 idx,
                                                                const rspfString& description)
{
   if(!theAdjustmentList.size())
   {
      return;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setDescription(description);
   }
}

void  rspfAdjustableParameterInterface::setParameterCenter(rspf_uint32 idx, double center, bool notify)
{
   if(!theAdjustmentList.size())
   {
      return;
   }

   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
     theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setCenter(center);

	 if(notify)
	 {
	   adjustableParametersChanged();
	 }
   }
}

double rspfAdjustableParameterInterface::getParameterCenter(rspf_uint32 idx)const
{
   if(!theAdjustmentList.size())
   {
      return 0.0;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getCenter();
   }

   return 0.0;
}

double   rspfAdjustableParameterInterface::computeParameterOffset(rspf_uint32 idx)const
{
   if(!theAdjustmentList.size())
   {
      return 0.0;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].computeOffset();
   }

   return 0.0;
}

rspf_int32 rspfAdjustableParameterInterface::findParameterIdxGivenDescription(rspf_uint32 adjustmentIdx,
                                                                                const rspfString& name)const
{
   rspf_int32 result = -1;
   if(adjustmentIdx < getNumberOfAdjustments())
   {
      rspf_uint32 idx = 0;
      rspf_uint32 n = theAdjustmentList[adjustmentIdx].getNumberOfAdjustableParameters();
      bool found = false;
      for(idx = 0; ((idx < n)&&(!found)); ++idx)
      {
         if(theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getDescription() == name)
         {
            result = (rspf_int32) idx;
         }
      }
   }
   
   return result;
}

rspf_int32 rspfAdjustableParameterInterface::findParameterIdxContainingDescription(rspf_uint32 adjustmentIdx,
                                                                                     const rspfString& name)const
{
   rspf_int32 result = -1;
   if(adjustmentIdx < getNumberOfAdjustments())
   {
      rspf_uint32 idx = 0;
      rspf_uint32 n = theAdjustmentList[adjustmentIdx].getNumberOfAdjustableParameters();
      bool found = false;
      for(idx = 0; ((idx < n)&&(!found)); ++idx)
      {
         if(theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getDescription().contains(name))
         {
            result = (rspf_int32) idx;
         }
      }
   }

   return result;
}


bool rspfAdjustableParameterInterface::isParameterLocked(rspf_uint32 idx)const
{
   if(!theAdjustmentList.size())
   {
      return false;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getLockFlag();
   }

   return false;
   
}

void rspfAdjustableParameterInterface::setParameterLockFlag(rspf_uint32 idxParam, bool flag)
{
   if(!theAdjustmentList.size())
   {
      return;
   }
   if(idxParam < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      theAdjustmentList[theCurrentAdjustment].getParameterList()[idxParam].setLockFlag(flag);
   }
}

bool rspfAdjustableParameterInterface::getParameterLockFlag(rspf_uint32 idx)const
{
   if(!theAdjustmentList.size())
   {
      return false;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      return theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].getLockFlag();
   }

   return false;
}

void rspfAdjustableParameterInterface::lockAllParametersCurrentAdjustment()
{
   lockAllParameters(theCurrentAdjustment);
}

void rspfAdjustableParameterInterface::unlockAllParametersCurrentAdjustment()
{
   unlockAllParameters(theCurrentAdjustment);
}

void rspfAdjustableParameterInterface::lockAllParameters(rspf_uint32 idxAdjustment)
{
   if(idxAdjustment < getNumberOfAdjustments())
   {
      rspf_uint32 idx = 0;
      rspf_uint32 n   = theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters();
      
      for(idx = 0; idx < n; ++idx)
      {
         theAdjustmentList[idxAdjustment].getParameterList()[idx].setLockFlag(true);
      }
   }
}

void rspfAdjustableParameterInterface::unlockAllParameters(rspf_uint32 idxAdjustment)
{
   if(idxAdjustment < getNumberOfAdjustments())
   {
      rspf_uint32 idx = 0;
      rspf_uint32 n   = theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters();
      
      for(idx = 0; idx < n; ++idx)
      {
         theAdjustmentList[idxAdjustment].getParameterList()[idx].setLockFlag(false);
      }
   }
}


void rspfAdjustableParameterInterface::setParameterOffset(rspf_uint32 idx,
                                                           rspf_float64 value,
                                                           bool notify)
{
//    double center   = getParameterCenter(idx);
//    double sigma    = getParameterSigma(idx);
//    double minValue = center - sigma;
//    double maxValue = center + sigma;
//    double x = 0.0;
   
//    if(sigma != 0.0)
//    {
//       x = (value - center)/sigma;
      
//       value = center + x*sigma;
      
//       if(value < minValue)
//       {
//          x = -1;
//       }
//       else if(value >maxValue)
//       {
//          x = 1.0;
//       }
//       setAdjustableParameter(idx, x, false);
//    }
   
   if(!theAdjustmentList.size())
   {
      return;
   }
   if(idx < theAdjustmentList[theCurrentAdjustment].getNumberOfAdjustableParameters())
   {
      theAdjustmentList[theCurrentAdjustment].getParameterList()[idx].setOffset(value);
      if(notify)
      {
         adjustableParametersChanged();
      }
   }
}

void rspfAdjustableParameterInterface::resizeAdjustableParameterArray(rspf_uint32 numberOfParameters)
{
   if(!theAdjustmentList.size())
   {
      newAdjustment(numberOfParameters);
      return;
   }

   theAdjustmentList[theCurrentAdjustment].setNumberOfAdjustableParameters(numberOfParameters);
}

void rspfAdjustableParameterInterface::setAdjustment(const rspfAdjustmentInfo& adj, bool notify)
{
   setAdjustment(theCurrentAdjustment, adj, notify);
}

void rspfAdjustableParameterInterface::setAdjustment(rspf_uint32 idx, const rspfAdjustmentInfo& adj, bool notify)
{
   if(idx < getNumberOfAdjustments())
   {
      theAdjustmentList[(int)idx] = adj;
      if(notify)
      {
         adjustableParametersChanged();
      }
   }
}


void rspfAdjustableParameterInterface::addAdjustment(const rspfAdjustmentInfo& adj, bool notify)
{
   theAdjustmentList.push_back(adj);
   if(notify)
   {
      adjustableParametersChanged();
   }
}

void rspfAdjustableParameterInterface::getAdjustment(rspfAdjustmentInfo& adj)
{
   getAdjustment(theCurrentAdjustment,  adj);
}

void rspfAdjustableParameterInterface::getAdjustment(rspf_uint32 idx, rspfAdjustmentInfo& adj)
{
   adj.setNumberOfAdjustableParameters(0);

   if(idx < getNumberOfAdjustments())
   {
      adj = theAdjustmentList[(int)idx];
   }
}

rspf_uint32 rspfAdjustableParameterInterface::getNumberOfAdjustments()const
{
   return (rspf_uint32)theAdjustmentList.size();
}

rspf_uint32 rspfAdjustableParameterInterface::getCurrentAdjustmentIdx()const
{
   return theCurrentAdjustment;
}

void rspfAdjustableParameterInterface::setDirtyFlag(bool flag)
{
   if(theAdjustmentList.size() > 0)
   {
      theAdjustmentList[theCurrentAdjustment].setDirtyFlag(flag);
   }
}

void rspfAdjustableParameterInterface::setAllDirtyFlag(bool flag)
{
   rspf_uint32 idx = 0;
   
   for(idx = 0; idx < theAdjustmentList.size(); ++idx)
   {
      theAdjustmentList[idx].setDirtyFlag(flag);
   }
}

bool rspfAdjustableParameterInterface::hasDirtyAdjustments()const
{
   rspf_uint32 idx = 0;
      
   for(idx = 0; idx < theAdjustmentList.size(); ++idx)
   {
      if(theAdjustmentList[idx].isDirty())
      {
         return true;
      }
   }

   return false;
}

bool rspfAdjustableParameterInterface::saveAdjustments(rspfKeywordlist& kwl,
                                                        const rspfString& prefix)const
{
   kwl.add(prefix,
           NUMBER_OF_ADJUSTMENTS_KW,
           static_cast<rspf_uint32>(theAdjustmentList.size()),
           true);
   kwl.add(prefix,
           CURRENT_ADJUSTMENT_KW,
           (int)theCurrentAdjustment,
           true);
   
   for(rspf_uint32 adjIdx = 0; adjIdx < theAdjustmentList.size(); ++adjIdx)
   {
      rspfString adjPrefix = prefix + (rspfString(ADJUSTMENT_PREFIX) + rspfString::toString(adjIdx)+".");

      theAdjustmentList[adjIdx].saveState(kwl,
                                          adjPrefix);
   }

   return true;
}

bool rspfAdjustableParameterInterface::loadAdjustments(const rspfKeywordlist& kwl,
                                                        const rspfString& prefix)
{
   theAdjustmentList.clear();
   theCurrentAdjustment = 0;
   
   const char* numberOfAdjustments = kwl.find(prefix, NUMBER_OF_ADJUSTMENTS_KW);
   const char* currentAdjustment   = kwl.find(prefix, CURRENT_ADJUSTMENT_KW);

   if(!numberOfAdjustments)
   {
      numberOfAdjustments = kwl.find(prefix, NUMBER_OF_ADJUSTMENTS_OLD_KW);
   }
   if(!currentAdjustment)
   {
      currentAdjustment = kwl.find(prefix, CURRENT_ADJUSTMENT_OLD_KW);
   }
   if(!numberOfAdjustments) return false;
   
   rspf_int32 maxdjustments = rspfString(numberOfAdjustments).toInt32();
   
   for(rspf_int32 adjIdx = 0; adjIdx <maxdjustments; ++adjIdx)
   {
      rspfAdjustmentInfo info;
      rspfString adjPrefix = prefix + (rspfString(ADJUSTMENT_PREFIX) + rspfString::toString(adjIdx)+".");

      if(!info.loadState(kwl, adjPrefix))
      {
         return false;
      }
      theAdjustmentList.push_back(info);
   }

   theCurrentAdjustment = rspfString(currentAdjustment).toUInt32();
   
   return true;
}

void rspfAdjustableParameterInterface::adjustableParametersChanged()
{
}
