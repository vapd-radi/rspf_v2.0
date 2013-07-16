//*****************************************************************************
// Copyright (C) 2005 Garrett Potts, all rights reserved.
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  Garrett Potts
// 
// DESCRIPTION:
//   Contains declaration of class rspfTraceManager.
//
//*****************************************************************************
// $Id: rspfTraceManager.cpp 9094 2006-06-13 19:12:40Z dburken $

#include <algorithm>

#include <rspf/base/rspfTraceManager.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfRegExp.h>

rspfTraceManager* rspfTraceManager::theInstance = 0;

rspfTraceManager::rspfTraceManager()
{
   theInstance = this;
}

rspfTraceManager* rspfTraceManager::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfTraceManager;
   }
   
   return theInstance;
}

void rspfTraceManager::setTracePattern(const rspfString& pattern)
{
   thePattern = pattern;
   setTraceFlags(true);
}

void rspfTraceManager::addTrace(rspfTrace* traceObj)
{
   if(!traceObj)
   {
      return;
   }
   
   std::vector<rspfTrace*>::iterator iter = std::find(theTraceList.begin(),
                                                       theTraceList.end(),
                                                       traceObj);
   if(iter == theTraceList.end())
   {
      theTraceList.push_back(traceObj);

      if(thePattern.size())
      {
         rspfRegExp rxp;
         rxp.compile(thePattern.c_str());
         if(rxp.find(traceObj->getTraceName().c_str()))
         {
            // Found a match so set the flag and increment the counter.
            traceObj->setTraceFlag(true);
         }
         else
         {
            traceObj->setTraceFlag(false);
         }
      }
      else
      {
         traceObj->setTraceFlag(false);
      } 
   }
}

void rspfTraceManager::removeTrace(rspfTrace* traceObj)
{
   std::vector<rspfTrace*>::iterator iter = std::find(theTraceList.begin(),
                                                       theTraceList.end(),
                                                       traceObj);

   if(iter !=  theTraceList.end())
   {
      theTraceList.erase(iter);
   }
}

void rspfTraceManager::setTraceFlags(bool flag)
{
   rspfRegExp rxp;
   rxp.compile(thePattern.c_str());
   
   for(rspf_uint32 idx = 0; idx < theTraceList.size(); ++idx)
   {
      if (rxp.find(theTraceList[idx]->getTraceName().c_str()))
      {
         // Found a match so set the flag and increment the counter.
         theTraceList[idx]->setTraceFlag(flag);
      }
   }
}
