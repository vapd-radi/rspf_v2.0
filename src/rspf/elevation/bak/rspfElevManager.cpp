//**************************************************************************
// FILE: rspfElevManager.cpp
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// DESCRIPTION:
//   Contains implementation of class rspfElevManager. This object 
//   provides a single interface for accessing multiple elevation
//   sources. This object owns one or more elevation sources in an ordered
//   list. When queried for an elevation at a particular point, it searches
//   the available sources for the best result, instantiating new sources if
//   necessary.
//
// SOFTWARE HISTORY:
//>
//   23Apr2001  Oscar Kramer
//              Initial coding.
//<
//**************************************************************************
// $Id: rspfElevManager.cpp 21970 2012-12-05 18:19:32Z okramer $

#include <algorithm>
#include <rspf/elevation/rspfElevManager.h>
#include <rspf/base/rspfEnvironmentUtility.h>
#include <rspf/elevation/rspfElevationDatabase.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/elevation/rspfElevationDatabaseRegistry.h>
#include <rspf/base/rspfKeywordNames.h>

rspfElevManager* rspfElevManager::m_instance = 0;
static rspfTrace traceDebug("rspfElevManager:debug");

void rspfElevManager::ConnectionStringVisitor::visit(rspfObject* obj)
{
   if(!hasVisited(obj))
   {
      rspfElevationDatabase* databsase = dynamic_cast<rspfElevationDatabase*>(obj);
      if(databsase)
      {
         if(m_connectionString == databsase->getConnectionString())
         {
            m_database = databsase;
            m_stopTraversalFlag = true;
         }
      }
   }
}


rspfElevManager* rspfElevManager::instance()
{
   if (!m_instance)
   {
      m_instance = new rspfElevManager();
      m_instance->loadStandardElevationPaths();
   }
   
   return m_instance;
}
rspfElevManager::rspfElevManager()
   :rspfElevSource(),
    m_elevationDatabaseList(0),
    m_defaultHeightAboveEllipsoid(rspf::nan()),
    m_elevationOffset(rspf::nan()),
    m_useGeoidIfNullFlag(false),
    m_mutex()
{
   m_instance = this;
}

rspfElevManager::~rspfElevManager()
{
}

double rspfElevManager::getHeightAboveEllipsoid(const rspfGpt& gpt)
{
   if (!isSourceEnabled())
      return rspf::nan();

   double result = rspf::nan();
   for (rspf_uint32 idx = 0; (idx < m_elevationDatabaseList.size()) && rspf::isnan(result); ++idx)
   {
      result = m_elevationDatabaseList[idx]->getHeightAboveEllipsoid(gpt);
   }

   if (rspf::isnan(result))
   {
      // No elevation value was returned from the database, so try next best alternatives depending
      // on rspf_preferences settings. Priority goes to default ellipsoid height if available:
      if (!rspf::isnan(m_defaultHeightAboveEllipsoid))
      {
         result = m_defaultHeightAboveEllipsoid;
      }
      else if (m_useGeoidIfNullFlag)
      {
         result = rspfGeoidManager::instance()->offsetFromEllipsoid(gpt);
      }
   }

   // Next, rspf_preferences may have indicated an elevation offset to use (top of trees, error
   // bias, etc):
   if (!rspf::isnan(m_elevationOffset) && !rspf::isnan(result))
      result += m_elevationOffset;

   return result;
}

double rspfElevManager::getHeightAboveMSL(const rspfGpt& gpt)
{
   if (!isSourceEnabled())
      return rspf::nan();

   double result = rspf::nan();
   for (rspf_uint32 idx = 0; (idx < m_elevationDatabaseList.size()) && rspf::isnan(result); ++idx)
   {
      result = m_elevationDatabaseList[idx]->getHeightAboveMSL(gpt);
   }

   if (rspf::isnan(result) && m_useGeoidIfNullFlag)
   {
      // No elevation value was returned from the database, so try next best alternatives depending
      // on rspf_preferences settings. First default to height at MSL itself:
      result = 0.0; // MSL
      if (!rspf::isnan(m_defaultHeightAboveEllipsoid))
      {
         // Use the default height above ellipsoid corrected for best guess of MSL above ellipsoid
         // (i.e., the geoid):
         double dh = rspfGeoidManager::instance()->offsetFromEllipsoid(gpt);
         if (!rspf::isnan(dh))
            result = m_defaultHeightAboveEllipsoid - dh;
      }
   }

   // rspf_preferences may have indicated an elevation offset to use (top of trees, error bias, etc)
   if (!rspf::isnan(result) && (!rspf::isnan(m_elevationOffset)))
      result += m_elevationOffset;

   return result;
}

void rspfElevManager::loadStandardElevationPaths()
{
   rspfFilename userDir    = rspfEnvironmentUtility::instance()->getUserOssimSupportDir();
   rspfFilename installDir = rspfEnvironmentUtility::instance()->getInstalledOssimSupportDir();
   
   userDir = userDir.dirCat("elevation");
   installDir = installDir.dirCat("elevation");
   
   loadElevationPath(userDir);
   loadElevationPath(installDir);
   
   rspfString paths = rspfEnvironmentUtility::instance()->getEnvironmentVariable("RSPF_ELEVATION_PATH");
   std::vector<rspfString> pathArray;
   rspfString pathSeparator = ":";
#if defined(WIN32) && !defined(CYGWIN)
   pathSeparator = ";";
#endif
   
   if(!paths.empty())
   {
      paths.split(pathArray, pathSeparator);
      if(!pathArray.empty())
      {
         rspf_uint32 idx = 0;
         for(idx = 0; idx < pathArray.size(); ++idx)
         {
            rspfFilename file(pathArray[idx]);
            
            if(file.exists())
            {
               loadElevationPath(file);
            }
         }
      }
   }
}

bool rspfElevManager::loadElevationPath(const rspfFilename& path)
{
  bool result = false;
   rspfElevationDatabase* database = rspfElevationDatabaseRegistry::instance()->open(path);
   
   if(!database&&path.isDir())
   {
      rspfDirectory dir;
      
      if(dir.open(path))
      {
         rspfFilename file;
         dir.getFirst(file, rspfDirectory::RSPF_DIR_DIRS);
         do
         {
            database = rspfElevationDatabaseRegistry::instance()->open(file);
            if(database)
            {
               result = true;
               addDatabase(database);
            }
         }while(dir.getNext(file));
      }
   }
   else if(database)
   {
      result = true;
      addDatabase(database);
   }
   
   return result;
}

void rspfElevManager::getOpenCellList(std::vector<rspfFilename>& list) const
{
   for(rspf_uint32 idx = 0; idx < m_elevationDatabaseList.size(); ++idx)
   {
      m_elevationDatabaseList[idx]->getOpenCellList(list);
   }
}

void rspfElevManager::clear()
{
   //OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);

  m_elevationDatabaseList.clear();

}

void rspfElevManager::accept(rspfVisitor& visitor)
{
   for(rspf_uint32 idx = 0; ((idx < m_elevationDatabaseList.size())&&!visitor.stopTraversal()); ++idx)
   {
      m_elevationDatabaseList[idx]->accept(visitor);
   }
}

bool rspfElevManager::getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const
{
   for(rspf_uint32 idx = 0;(idx < m_elevationDatabaseList.size()); ++idx)
   {
      if(m_elevationDatabaseList[idx]->getAccuracyInfo(info, gpt))
      {
         return true;
      }
   }

   return false;
}

bool rspfElevManager::saveState(rspfKeywordlist& kwl,
                                 const char* prefix) const
{
   return rspfElevSource::saveState(kwl, prefix);
}

/**
 * Method to the load (recreate) the state of an object from a keyword
 * list.  Return true if ok or false on error.
 */
bool rspfElevManager::loadState(const rspfKeywordlist& kwl,
                                 const char* prefix)
{
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
      << "DEBUG rspfElevManager::loadState: Entered..."
      << std::endl;
   }
   if(!rspfElevSource::loadState(kwl, prefix))
   {
      return false;
   }
   rspfString copyPrefix(prefix);
   rspfString elevationOffset = kwl.find(copyPrefix, "elevation_offset");
   rspfString defaultHeightAboveEllipsoid = kwl.find(copyPrefix, "default_height_above_ellipsoid");
   rspfString useGeoidIfNull = kwl.find(copyPrefix, "use_geoid_if_null");

   if(!elevationOffset.empty())
   {
      m_elevationOffset = elevationOffset.toDouble();
   }
   if(!defaultHeightAboveEllipsoid.empty())
   {
      m_defaultHeightAboveEllipsoid = defaultHeightAboveEllipsoid.toDouble();
   }
   if(!useGeoidIfNull.empty())
   {
      m_useGeoidIfNullFlag = useGeoidIfNull.toBool();
   }
   rspfString regExpression =  rspfString("^(") + copyPrefix + "elevation_source[0-9]+.)";
   vector<rspfString> keys =
   kwl.getSubstringKeyList( regExpression );
   long numberOfSources = (long)keys.size();
   rspf_uint32 offset = (rspf_uint32)(copyPrefix+"elevation_source").size();
   rspf_uint32 idx = 0;
   std::vector<int> theNumberList(numberOfSources);
   for(idx = 0; idx < theNumberList.size();++idx)
   {
      rspfString numberStr(keys[idx].begin() + offset,
                            keys[idx].end());
      theNumberList[idx] = numberStr.toInt();
   }
   std::sort(theNumberList.begin(), theNumberList.end());
   
   for(idx=0;idx < theNumberList.size();++idx)
   {
      rspfString newPrefix = copyPrefix;
      newPrefix += rspfString("elevation_source");
      newPrefix += rspfString::toString(theNumberList[idx]);
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
         << "DEBUG rspfElevManager::loadState:"
         << "\nLooking for key:  " << newPrefix
         << std::endl;
      }
      // first check if new way is supported
      //
      rspfRefPtr<rspfElevationDatabase> database = rspfElevationDatabaseRegistry::instance()->createDatabase(kwl, newPrefix+".");
      if(database.valid())
      {
        if (traceDebug())
        {
           rspfNotify(rspfNotifyLevel_DEBUG)
           << "DEBUG rspfElevManager::loadState:"
           << "\nadding elevation database:  "
           << database->getClassName()
           << ": " << database->getConnectionString()
           << std::endl;
        }  
        addDatabase(database.get());
      }
      else
      {
         // if not new elevation load verify the old way by
         // looking at the filename
         //
         rspfString fileKey = newPrefix;
         fileKey += ".";
         fileKey += rspfKeywordNames::FILENAME_KW;
         rspfString lookup = kwl.find(prefix, fileKey.c_str());
         if (!lookup.empty())
         {
            loadElevationPath(rspfFilename(lookup));
         } // end if lookup
      }
   } // end for loop
   return true;
}

void rspfElevManager::addDatabase(rspfElevationDatabase* database)
{
   if(!database) return;
   rspfRefPtr<rspfElevationDatabase> tempDb = database;
  // OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_mutex);
   if(std::find(m_elevationDatabaseList.begin(), 
                m_elevationDatabaseList.end(),
                database) == m_elevationDatabaseList.end())
   {
      m_elevationDatabaseList.push_back(database);
   }
}

