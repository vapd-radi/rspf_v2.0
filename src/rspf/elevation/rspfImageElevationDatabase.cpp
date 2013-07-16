//----------------------------------------------------------------------------
//
// File: rspfImageElevationDatabase.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description:  See class desciption in header file.
// 
//----------------------------------------------------------------------------
// $Id$

#include <rspf/elevation/rspfImageElevationDatabase.h>
#include <rspf/base/rspfCallback1.h>
#include <rspf/base/rspfDpt.h>
#include <rspf/base/rspfString.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/elevation/rspfImageElevationHandler.h>
#include <rspf/util/rspfFileWalker.h>
#include <cmath>

static rspfTrace traceDebug(rspfString("rspfImageElevationDatabase:debug"));

RTTI_DEF1(rspfImageElevationDatabase, "rspfImageElevationDatabase", rspfElevationDatabase);

//---
// Call back class to register with rspfFileWalker for call to
// rspfImageElevationDatabase::processFile
//
// Placed here as it is unique to this class.
//---
class ProcessFileCB: public rspfCallback1<const rspfFilename&>
{
public:
   ProcessFileCB(
      rspfImageElevationDatabase* obj,
      void (rspfImageElevationDatabase::*func)(const rspfFilename&))
      :
      m_obj(obj),
      m_func(func)
   {}
      
   virtual void operator()(const rspfFilename& file) const
   {
      (m_obj->*m_func)(file);
   }

private:
   rspfImageElevationDatabase* m_obj;
   void (rspfImageElevationDatabase::*m_func)(const rspfFilename& file);
};

rspfImageElevationDatabase::rspfImageElevationDatabase()
   :
   rspfElevationCellDatabase(),
   m_entryMap(),
   m_lastMapKey(0),
   m_lastAccessedId(0)
{
}

// Protected destructor as this is derived from rspfRefenced.
rspfImageElevationDatabase::~rspfImageElevationDatabase()
{
}

bool rspfImageElevationDatabase::open(const rspfString& connectionString)
{
   // return false; // tmp drb...
   
   static const char M[] = "rspfImageElevationDatabase::open";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered...\n"
         << "\nConnection string: " << connectionString << "\n";
   }                   
   
   bool result = false;

   close();

   if ( connectionString.size() )
   {
      m_connectionString = connectionString.c_str();

      loadFileMap();

      if ( m_entryMap.size() )
      {
         result = true;
      }
      else
      {
         m_connectionString.clear();
      }
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " result=" << (result?"true\n":"false\n");
   }

   return result;
}

void rspfImageElevationDatabase::close()
{
   m_meanSpacing = 0.0;
   m_geoid = 0;
   m_connectionString.clear();
}

double rspfImageElevationDatabase::getHeightAboveMSL(const rspfGpt& gpt)
{
   if(isSourceEnabled())
   {
      rspfRefPtr<rspfElevCellHandler> handler = getOrCreateCellHandler(gpt);
      if(handler.valid())
      {
         return handler->getHeightAboveMSL(gpt); // still need to shift
      }
   }
   return rspf::nan();
}

double rspfImageElevationDatabase::getHeightAboveEllipsoid(const rspfGpt& gpt)
{
   double h = getHeightAboveMSL(gpt);
   if(h != rspf::nan())
   {
      h += getOffsetFromEllipsoid(gpt);
   }
   return h;
}

rspfRefPtr<rspfElevCellHandler> rspfImageElevationDatabase::createCell(
   const rspfGpt& gpt)
{
   rspfRefPtr<rspfElevCellHandler> result = 0;
   
   std::map<rspf_uint64, rspfImageElevationFileEntry>::iterator i = m_entryMap.begin();
   while ( i != m_entryMap.end() )
   {
      if ( (*i).second.m_loadedFlag == false )
      {
         // not loaded
         rspfRefPtr<rspfImageElevationHandler> h = new rspfImageElevationHandler();

         if ( (*i).second.m_rect.isLonLatNan() )
         {
            if ( h->open( (*i).second.m_file ) )
            {
               // First time opened.  Capture the rectangle. for next time.
               (*i).second.m_rect = h->getBoundingGndRect();
            }
            else
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfImageElevationDatabase::createCell WARN:\nCould not open: "
                  << (*i).second.m_file << "\nRemoving file from map!" << std::endl;

               // Get a copy of the iterator to delet.
               std::map<rspf_uint64, rspfImageElevationFileEntry>::iterator badIter = i;
               
               ++i; // Go to next image.

               // Must put lock around erase.
               m_cacheMapMutex.lock();
               m_entryMap.erase(badIter);
               m_cacheMapMutex.unlock();
               
               continue; // Skip the rest of this loop.
            }
         }

         // Check the North up bounding rectangle for intersect.
         if ( (*i).second.m_rect.pointWithin(gpt) )
         {
            if ( h->isOpen() == false )
            {
               h->open( (*i).second.m_file );
            }

            if ( h->isOpen() )
            {
               //---
               // Check point coverage again as image may not be geographic and pointHasCoverage
               // has a check on worldToLocal point.
               //---
               if (  h->pointHasCoverage(gpt) )
               {
                  m_lastAccessedId = (*i).first;
                  (*i).second.m_loadedFlag = true;
                  result = h.get();
                  break;
               }
               else
               {
                  h = 0;
               }
            }
         }
         else
         {
            h = 0;
         }
      }

      ++i;
   }
   
   return result;
}

rspfRefPtr<rspfElevCellHandler> rspfImageElevationDatabase::getOrCreateCellHandler(
   const rspfGpt& gpt)
{
   rspfRefPtr<rspfElevCellHandler> result = 0;
   
   // Note: Must do mutex lock / unlock around any cach map access.
   m_cacheMapMutex.lock();

   if ( m_cacheMap.size() )
   {
      //---
      // Look in existing map for handler.
      //
      // Note: Cannot key off of id from gpt as cells can be any arbituary dimensions.
      //---

      CellMap::iterator lastAccessedCellIter = m_cacheMap.find(m_lastAccessedId);
      CellMap::iterator iter = lastAccessedCellIter;
        
      // Check from last accessed to end.
      while ( iter != m_cacheMap.end() )
      {
         if ( iter->second->m_handler->pointHasCoverage(gpt) )
         {
            result = iter->second->m_handler.get();
            break;
         }
         ++iter;
      }
     
      if ( result.valid() == false )
      {
         iter = m_cacheMap.begin();
              
         // Beginning to last accessed.
         while ( iter != lastAccessedCellIter)
         {
            if ( iter->second->m_handler->pointHasCoverage(gpt) )
            {
               result = iter->second->m_handler.get();
               break;
            }
            ++iter;
         }
      }

      if ( result.valid() )
      {
         m_lastAccessedId  = iter->second->m_id;
         iter->second->updateTimestamp();
      }
   }
   m_cacheMapMutex.unlock();
  
   if ( result.valid() == false )
   {
      // Not in m_cacheMap.  Create a new cell for point if we have coverage.
      result = createCell(gpt);

      if(result.valid())
      {
         OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_cacheMapMutex);

         //---
         // Add the cell to map.
         // NOTE: rspfImageElevationDatabase::createCell sets m_lastAccessedId to that of
         // the entries map key.
         //---
         m_cacheMap.insert(std::make_pair(m_lastAccessedId,
                                          new CellInfo(m_lastAccessedId, result.get())));

         ++m_lastMapKey;

         // Check the map size and purge cells if needed.
         if(m_cacheMap.size() > m_maxOpenCells)
         {
            flushCacheToMinOpenCells();
         }
      }
   }

   return result;
}

bool rspfImageElevationDatabase::pointHasCoverage(const rspfGpt& gpt) const
{
   //---
   // NOTE:
   //
   // The bounding rect is the North up rectangle.  So if the underlying image projection is not
   // a geographic projection and there is a rotation this could return false positives.  Inside
   // the rspfImageElevationDatabase::createCell there is a call to
   // rspfImageElevationHandler::pointHasCoverage which does a real check from the
   // rspfImageGeometry of the image.
   //---
   bool result = false;
   std::map<rspf_uint64, rspfImageElevationFileEntry>::const_iterator i = m_entryMap.begin();
   while ( i != m_entryMap.end() )
   {
      if ( (*i).second.m_rect.pointWithin(gpt) )
      {
         result = true;
         break;
      }
      ++i;
   }
   return result;
}


bool rspfImageElevationDatabase::getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const
{
   if(pointHasCoverage(gpt))
   {
     info.m_surfaceName = "Image Elevation";
   }

   return false;
}

bool rspfImageElevationDatabase::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   static const char M[] = "rspfImageElevationDatabase::loadState";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered..." << "\nkwl:\n" << kwl << "\n";
   }     
   bool result = false;
   const char* lookup = kwl.find(prefix, "type");
   if ( lookup )
   {
      std::string type = lookup;
      if ( ( type == "image_directory" ) || ( type == "rspfImageElevationDatabase" ) )
      {
         result = rspfElevationDatabase::loadState(kwl, prefix);

         if ( result )
         {
            loadFileMap();
         }
      }
   }

   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << M << " result=" << (result?"true\n":"false\n");
   }

   return result;
}

bool rspfImageElevationDatabase::saveState(rspfKeywordlist& kwl, const char* prefix) const
{
   return rspfElevationDatabase::saveState(kwl, prefix);
}

void rspfImageElevationDatabase::processFile(const rspfFilename& file)
{
   static const char M[] = "rspfImageElevationDatabase::processFile";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << M << " entered...\n" << "file: " << file << "\n";
   }

   // Add the file.
   m_entryMap.insert( std::make_pair(m_lastMapKey++, rspfImageElevationFileEntry(file)) );

   if(traceDebug())
   {
      // Since rspfFileWalker is threaded output the file so we know which job exited.
      rspfNotify(rspfNotifyLevel_DEBUG) << M << "\nfile: " << file << "\nexited...\n";
   } 
}

void rspfImageElevationDatabase::loadFileMap()
{
   if ( m_connectionString.size() )
   {
      // Create a file walker which will find files we can load from the connection string.
      rspfFileWalker* fw = new rspfFileWalker();
      fw->initializeDefaultFilterList();
      rspfCallback1<const rspfFilename&>* cb =
         new ProcessFileCB(this, &rspfImageElevationDatabase::processFile);
      fw->registerProcessFileCallback(cb);
      
      rspfFilename f = m_connectionString;

      // rspfFileWalker::walk will in turn call back to processFile method for each file it finds.
      fw->walk(f); 
      
      delete fw;
      fw = 0;
      delete cb;
      cb = 0;
   }
}

// Hidden from use:
rspfImageElevationDatabase::rspfImageElevationDatabase(
   const rspfImageElevationDatabase& /* copy_this */)
{
}

// Hidden from use:
const rspfImageElevationDatabase& rspfImageElevationDatabase::operator=(
   const rspfImageElevationDatabase& /* rhs */)
{
   return *this;
}

// Private container class:
rspfImageElevationDatabase::rspfImageElevationFileEntry::rspfImageElevationFileEntry()
   : m_file(),
     m_rect(),
     m_loadedFlag(false)
{
   m_rect.makeNan();
}

// Private container class:
rspfImageElevationDatabase::rspfImageElevationFileEntry::rspfImageElevationFileEntry(
   const rspfFilename& file)
   : m_file(file),
     m_rect(),
     m_loadedFlag(false)
{
   m_rect.makeNan();
}

