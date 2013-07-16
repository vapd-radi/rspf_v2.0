//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc.
//
// LICENSE: See top level LICENSE.txt file.
//
// Author: Garrett Potts
//
//*************************************************************************
// $Id: rspfAdjustmentInfo.cpp 15833 2009-10-29 01:41:53Z eshirschorn $
#include <rspf/base/rspfAdjustmentInfo.h>
#include <rspf/base/rspfKeywordNames.h>

static const char* PARAM_PREFIX             = "adj_param_";
static const char* NUMBER_OF_PARAMS_KW      = "number_of_params";
static const char* DIRTY_FLAG_KW            = "dirty_flag";


std::ostream& operator <<(std::ostream& out, const rspfAdjustmentInfo& data)
{
   rspf_int32 idx = 0;

   out << "Description:      " << data.getDescription()
       << "\nNumber of Params: " << data.theParameterList.size()
       << "\nDirty flag:       " << data.theDirtyFlag << std::endl;

   for(idx = 0; idx < (int)data.getNumberOfAdjustableParameters(); ++idx)
   {
      out << "Param " << idx << std::endl;
      out << data.theParameterList[idx] << std::endl;
   }

   return out;
}


rspfAdjustmentInfo::rspfAdjustmentInfo(int numberOfAdjustableParameters)
   :theParameterList(numberOfAdjustableParameters),
    theDescription(""),
    theDirtyFlag(false)
{
}

rspfAdjustmentInfo::rspfAdjustmentInfo(const rspfAdjustmentInfo& rhs)
   :theParameterList(rhs.theParameterList),
    theDescription(rhs.theDescription),
    theDirtyFlag(rhs.theDirtyFlag)
{
}

void rspfAdjustmentInfo::setNumberOfAdjustableParameters(rspf_uint32 numberOfAdjustableParameters)
{
   std::vector<rspfAdjustableParameterInfo> temp = theParameterList;

   theParameterList.resize(numberOfAdjustableParameters);
   if(temp.size() < numberOfAdjustableParameters)
   {
      std::copy(temp.begin(),
                temp.end(),
                theParameterList.begin());
   }
   else if(temp.size() > numberOfAdjustableParameters)
   {
      if(numberOfAdjustableParameters > 0)
      {
         std::copy(temp.begin(),
                   temp.begin()+numberOfAdjustableParameters,
                   theParameterList.begin());
      }
   }
}

rspf_uint32 rspfAdjustmentInfo::getNumberOfAdjustableParameters()const
{
   return (rspf_uint32)theParameterList.size();
}

rspfString rspfAdjustmentInfo::getDescription()const
{
   return theDescription;
}

void rspfAdjustmentInfo::setDescription(const rspfString& description)
{
   theDescription = description;
}

bool rspfAdjustmentInfo::isDirty()const
{
   return theDirtyFlag;
}

void rspfAdjustmentInfo::setDirtyFlag(bool flag)
{
   theDirtyFlag = flag;
}

void rspfAdjustmentInfo::setLockFlag(bool flag,
                                      rspf_uint32 idx)
{
   if(idx < theParameterList.size())
   {
      theParameterList[idx].setLockFlag(flag);
   }
}

void rspfAdjustmentInfo::keep()
{
   rspf_uint32 idx = 0;

   for(idx = 0; idx < theParameterList.size();++idx)
   {
      double center = theParameterList[idx].computeOffset();
      theParameterList[idx].setParameter(0.0);
      theParameterList[idx].setCenter(center);
   }
}


std::vector<rspfAdjustableParameterInfo>& rspfAdjustmentInfo::getParameterList()
{
   return theParameterList;
}

const std::vector<rspfAdjustableParameterInfo>& rspfAdjustmentInfo::getParameterList()const
{
   return theParameterList;
}

bool rspfAdjustmentInfo::saveState(rspfKeywordlist& kwl,
                                    const rspfString& prefix)const
{
   kwl.add(prefix,
           rspfKeywordNames::DESCRIPTION_KW,
           getDescription(),
           true);
   kwl.add(prefix.c_str(),
           NUMBER_OF_PARAMS_KW,
           static_cast<rspf_uint32>(theParameterList.size()),
           true);

   kwl.add(prefix,
           DIRTY_FLAG_KW,
           (short)theDirtyFlag,
           true);

   rspfString value;
   for(rspf_uint32 idx = 0; idx < theParameterList.size();++idx)
   {
      rspfString newPrefix = rspfString(prefix) + (rspfString(PARAM_PREFIX) + rspfString::toString(idx)+".");
      theParameterList[idx].saveState(kwl, newPrefix.c_str());
   }

   return true;
}

bool rspfAdjustmentInfo::loadState(const rspfKeywordlist& kwl,
                                    const rspfString& prefix)
{
   setDescription(kwl.find(prefix, rspfKeywordNames::DESCRIPTION_KW));
   setNumberOfAdjustableParameters(rspfString(kwl.find(prefix, NUMBER_OF_PARAMS_KW)).toUInt32());
   const char *dirtyFlag = kwl.find(prefix, DIRTY_FLAG_KW);
   rspfString value;

   if(dirtyFlag)
   {
      theDirtyFlag = rspfString(dirtyFlag).toBool();
   }
   else
   {
      theDirtyFlag = false;
   }

   for(rspf_uint32 idx = 0; idx < theParameterList.size();++idx)
   {
      rspfString newPrefix = rspfString(prefix) + (rspfString(PARAM_PREFIX) + rspfString::toString(idx)+".");
      if(!theParameterList[idx].loadState(kwl, newPrefix.c_str()))
      {
         return false;
      }
   }

   return true;
}
