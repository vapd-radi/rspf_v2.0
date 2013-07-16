//*************************************************************************
// Copyright (C) 2004 Intelligence Data Systems, Inc.  All rights reserved.
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
//**************************************************************************
// $Id: rspfVpfAnnotationCoverageInfo.cpp 15833 2009-10-29 01:41:53Z eshirschorn $

#include <vector>
#include <algorithm>
#include <rspf/imaging/rspfVpfAnnotationCoverageInfo.h>
#include <rspf/vec/rspfVpfLibrary.h>
#include <rspf/vec/rspfVpfFeatureClassSchema.h>

bool rspfVpfAnnotationCoverageInfo::hasRenderableFeature()const
{
  for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size();++idx)
  {
     if(theFeatureInfoArray[idx]->getEnabledFlag())
     {
        return true;
     }
  }

  return false;
}

void rspfVpfAnnotationCoverageInfo::getEnabledFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& result)
{
  for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size();++idx)
    {
      if(theFeatureInfoArray[idx]->getEnabledFlag())
	{
	  result.push_back(theFeatureInfoArray[idx]);
	}
    }
}

void rspfVpfAnnotationCoverageInfo::getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& result)
{
  for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size();++idx)
  {
     result.push_back(theFeatureInfoArray[idx]);
  }
}

rspfIrect rspfVpfAnnotationCoverageInfo::getBoundingProjectedRect()const
{
  rspfIrect result;
  result.makeNan();

  for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size();++idx)
    {
      rspfIrect tempRect = theFeatureInfoArray[idx]->getBoundingProjectedRect();
      if(!tempRect.hasNans())
	{
	  if(result.hasNans())
	    {
	      result = tempRect;
	    }
	  else
	    {
	      result = result.combine(tempRect);
	    }
	}
    }

  return result;
}


void rspfVpfAnnotationCoverageInfo::transform(rspfImageGeometry* geom)
{
  for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size();++idx)
  {
     theFeatureInfoArray[idx]->transform(geom);
  }
}

void rspfVpfAnnotationCoverageInfo::buildCoverage()
{
   buildCoverage( "" );
}

void rspfVpfAnnotationCoverageInfo::buildCoverage(const rspfString& feature)
{
   deleteAllFeatures();
   if(theLibrary)
   {
      rspfVpfCoverage coverage;
      
      if(theLibrary->getCoverage(theName, coverage))
      {
         rspfVpfFeatureClassSchema schema;
         if(coverage.openFeatureClassSchema(schema))
         {
            std::vector<rspfString> featureClassArray;
            schema.getFeatureClasses(featureClassArray);
            schema.closeTable();
            for(rspf_uint32  idx = 0; idx < featureClassArray.size(); ++idx)
            {
               rspfString s1 = feature;
               rspfString s2 = featureClassArray[idx];
               s1.downcase();
               s2.downcase();
               if (! feature.length() || (s1 == s2) )
               {
                  rspfVpfAnnotationFeatureInfo* featureInfo =
                     new rspfVpfAnnotationFeatureInfo;
                  featureInfo->setName(featureClassArray[idx]);
                  featureInfo->setCoverage(coverage);
                  theFeatureInfoArray.push_back(featureInfo);
                  featureInfo->buildFeature();
               }
            }
         }
      }
   }
}

void rspfVpfAnnotationCoverageInfo::drawAnnotations(rspfRgbImage* tile)
{
   for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size();++idx)
   {
      if(theFeatureInfoArray[idx]->getEnabledFlag())
      {
         theFeatureInfoArray[idx]->drawAnnotations(tile);
      }
   }
}

void rspfVpfAnnotationCoverageInfo::deleteAllFeatures()
{
   for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size();++idx)
   {
      theFeatureInfoArray[idx];
   }
   
   theFeatureInfoArray.clear();
}



bool rspfVpfAnnotationCoverageInfo::saveState(rspfKeywordlist& kwl,
					       const char* prefix)const
{
   kwl.add(prefix,
           "name",
           theName,
           true);
   
   for(rspf_uint32 idx = 0; idx < theFeatureInfoArray.size(); ++idx)
   {
      theFeatureInfoArray[idx]->saveState(kwl,
					  (rspfString(prefix) + "feature" +
					   rspfString::toString(idx) + ".").c_str());
   }
   
   return true;
}

bool rspfVpfAnnotationCoverageInfo::loadState(const rspfKeywordlist& kwl,
					       const char* prefix)
{
  deleteAllFeatures();

  if(!theLibrary)
    {
      return false;
    }

  theName = kwl.find(prefix, "name");
  rspfVpfCoverage coverage;

  if(theLibrary->getCoverage(theName, coverage))
    {
      rspfString regExpression =  rspfString("^(") + rspfString(prefix) + "feature[0-9]+.)";
      vector<rspfString> keys =
	kwl.getSubstringKeyList( regExpression );
      std::vector<int> theNumberList(keys.size());
      int offset = (int)(rspfString(prefix)+"feature").size();
      int idx = 0;
      for(idx = 0; idx < (int)theNumberList.size();++idx)
	{
	  rspfString numberStr(keys[idx].begin() + offset,
				keys[idx].end());
	  theNumberList[idx] = numberStr.toInt();
	}
      std::sort(theNumberList.begin(), theNumberList.end());
      
      for(idx=0;idx < (int)keys.size();++idx)
	{
	  rspfString newPrefix = rspfString(prefix);
	  newPrefix += rspfString("feature");
	  newPrefix += rspfString::toString(theNumberList[idx]);
	  newPrefix += rspfString(".");
	  rspfVpfAnnotationFeatureInfo* featureInfo = new rspfVpfAnnotationFeatureInfo;
	  featureInfo->setCoverage(coverage);
	  theFeatureInfoArray.push_back(featureInfo);
	  if(!featureInfo->loadState(kwl,
				     newPrefix))
	    {
	      return false;
	    }
	}
    }
  else
    {
      return false;
    }

  return true;
}
