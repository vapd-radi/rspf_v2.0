//*************************************************************************
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author: Garrett Potts
// 
//**************************************************************************
// $Id: rspfVpfAnnotationLibraryInfo.cpp 15836 2009-10-30 12:29:09Z dburken $

#include <algorithm>

#include <rspf/imaging/rspfVpfAnnotationLibraryInfo.h>
#include <rspf/imaging/rspfVpfAnnotationFeatureInfo.h>
#include <rspf/vec/rspfVpfLibrary.h>
#include <rspf/vec/rspfVpfDatabase.h>
#include <rspf/base/rspfTrace.h>

static rspfTrace traceDebug("rspfVpfAnnotationLibraryInfo:debug");


rspfVpfAnnotationLibraryInfo::rspfVpfAnnotationLibraryInfo(const rspfString& libraryName, bool enabledFlag)
   :
      theName(libraryName),
      theEnabledFlag(enabledFlag),
      theDatabase(NULL)
{
}

rspfVpfAnnotationLibraryInfo::~rspfVpfAnnotationLibraryInfo()
{
   deleteAllCoverage();
}

bool rspfVpfAnnotationLibraryInfo::getEnabledFlag()const
{
   return theEnabledFlag;
}

void rspfVpfAnnotationLibraryInfo::setEnabledFlag(bool flag)
{
   theEnabledFlag = flag;
}

const rspfString& rspfVpfAnnotationLibraryInfo::getName()const
{
   return theName;
}

void rspfVpfAnnotationLibraryInfo::setName(const rspfString& libraryName)
{
   theName = libraryName;
}

void rspfVpfAnnotationLibraryInfo::setDatabase(rspfVpfDatabase* database)
{
   theDatabase = database;
}

rspfVpfDatabase* rspfVpfAnnotationLibraryInfo::getDatabase()
{
   return theDatabase;
}

rspfIrect rspfVpfAnnotationLibraryInfo::getBoundingProjectedRect()const
{
   rspfIrect result;
   result.makeNan();
   
   for(rspf_uint32 idx = 0; idx < theCoverageLayerList.size(); ++idx)
   {
      rspfIrect tempRect = theCoverageLayerList[idx]->getBoundingProjectedRect();
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

void rspfVpfAnnotationLibraryInfo::getAllFeatures(std::vector<rspfVpfAnnotationFeatureInfo*>& features)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationLibraryInfo::getAllFeatures DEBUG: entered..." << std::endl;
   }
   
   for(rspf_uint32 idx = 0; idx < theCoverageLayerList.size(); ++idx)
   {
      theCoverageLayerList[idx]->getAllFeatures(features);
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationLibraryInfo::getAllFeatures DEBUG: leaving..." << std::endl;
   }
}

void rspfVpfAnnotationLibraryInfo::transform(rspfImageGeometry* geom)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationLibraryInfo::transform DEBUG: entered..." << std::endl;
   }
   
   
   for(rspf_uint32 idx = 0; idx < theCoverageLayerList.size(); ++idx)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "transforming coverage = " << theCoverageLayerList[idx]->getName() << std::endl;
      }
      
      theCoverageLayerList[idx]->transform(geom);
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationLibraryInfo::transform DEBUG: leaving..." << std::endl;
   }
}

void rspfVpfAnnotationLibraryInfo::buildLibrary()
{
   buildLibrary( "", "");
}

void rspfVpfAnnotationLibraryInfo::buildLibrary(const rspfString& coverageName, const rspfString& feature)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationLibraryInfo::buildLibrary DEBUG: entered..." << std::endl;
   }
   deleteAllCoverage();
   if(!theDatabase)
   {
      return;
   }
   
   rspfVpfLibrary* library = theDatabase->getLibrary(theName);
   if(library)
   {
      std::vector<rspfString> coverageNames;
      library->getCoverageNames(coverageNames);
      for(rspf_uint32 idx = 0; idx < coverageNames.size(); ++idx)
      {
         rspfString s1 = coverageName;
         rspfString s2 = coverageNames[idx];
         s1.downcase();
         s2.downcase();
         if ( ! coverageName.length() || (s1 == s2) )
         {
            rspfVpfAnnotationCoverageInfo* coverageInfo =
               new rspfVpfAnnotationCoverageInfo;
            coverageInfo->setName(coverageNames[idx]);
            coverageInfo->setLibrary(library);
            theCoverageLayerList.push_back(coverageInfo);
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "coverage name = " << coverageNames[idx] << std::endl;
            }
            coverageInfo->buildCoverage(feature);
         }
      }
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfVpfAnnotationLibraryInfo::buildLibrary DEBUG: leaving..." << std::endl;
   }
}


void rspfVpfAnnotationLibraryInfo::drawAnnotations(rspfRgbImage* tile)
{
  for(rspf_uint32 idx = 0; idx < theCoverageLayerList.size(); ++idx)
    {
      theCoverageLayerList[idx]->drawAnnotations(tile);
    }
}

void rspfVpfAnnotationLibraryInfo::deleteAllCoverage()
{
  for(rspf_uint32 idx = 0; idx < theCoverageLayerList.size(); ++idx)
    {
      delete theCoverageLayerList[idx];
    }

  theCoverageLayerList.clear();
}

bool rspfVpfAnnotationLibraryInfo::saveState(rspfKeywordlist& kwl,
					      const char* prefix)const
{
   rspf_uint32 idx;
   
   kwl.add(prefix,
           "name",
           theName,
           true);
   
   for(idx = 0; idx < theCoverageLayerList.size(); ++idx)
   {
      theCoverageLayerList[idx]->saveState(kwl,
					   (rspfString(prefix) + "coverage" + rspfString::toString(idx) + ".").c_str());
   }
   
   return true;
}

bool rspfVpfAnnotationLibraryInfo::loadState(const rspfKeywordlist& kwl,
					      const char* prefix)
{
  deleteAllCoverage();
  theName = kwl.find(prefix, "name");
  
  rspfVpfLibrary* library = theDatabase->getLibrary(theName);

  if(!library)
    {
      return false;
    }
  rspfString regExpression =  rspfString("^(") + rspfString(prefix) + "coverage[0-9]+.)";
  vector<rspfString> keys =
    kwl.getSubstringKeyList( regExpression );
  std::vector<int> theNumberList(keys.size());
  int offset = (int)(rspfString(prefix)+"coverage").size();
  int idx = 0;
  for(idx = 0; idx < (int)theNumberList.size();++idx)
    {
      rspfString numberStr(keys[idx].begin() + offset,
			    keys[idx].end());
      theNumberList[idx] = numberStr.toInt();
    }
  std::sort(theNumberList.begin(), theNumberList.end());
  
  for(idx = 0; idx < (int)keys.size(); ++idx)
    {
      rspfString newPrefix = rspfString(prefix);
      newPrefix += rspfString("coverage");
      newPrefix += rspfString::toString(theNumberList[idx]);
      newPrefix += rspfString(".");
      rspfVpfAnnotationCoverageInfo* coverageInfo = new rspfVpfAnnotationCoverageInfo;
      coverageInfo->setLibrary(library);
      theCoverageLayerList.push_back(coverageInfo);
      coverageInfo->loadState(kwl,
			      newPrefix);
    }
  
  return true;
}
