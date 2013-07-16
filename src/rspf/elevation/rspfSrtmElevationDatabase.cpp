//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id$

#include <rspf/elevation/rspfSrtmElevationDatabase.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfTrace.h>
#include <sstream>
#include <iomanip>
#include <cstdlib> /* for abs(int) */

static rspfTrace traceDebug("rspfSrtmElevationDatabase:debug");

RTTI_DEF1(rspfSrtmElevationDatabase, "rspfSrtmElevationDatabase", rspfElevationCellDatabase);

double rspfSrtmElevationDatabase::getHeightAboveMSL(const rspfGpt& gpt)
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

double rspfSrtmElevationDatabase::getHeightAboveEllipsoid(const rspfGpt& gpt)
{
   double h = getHeightAboveMSL(gpt);
   if(h != rspf::nan())
   {
      h += getOffsetFromEllipsoid(gpt);
   }
   
   return h;
}
bool rspfSrtmElevationDatabase::open(const rspfString& connectionString)
{
   bool result = false;
   rspfFilename file = rspfFilename(connectionString);
   m_connectionString = connectionString;
   result = openSrtmDirectory(file);
   
   return result;
}

bool rspfSrtmElevationDatabase::getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const
{
   bool result = false;
   info.makeNan();

   if(pointHasCoverage(gpt))
   {
      result = true;
      if(m_meanSpacing > 100.0)
      {
         // 30 arc second
         info.m_absoluteCE  = 20.0;
         info.m_absoluteLE = 16.0;
         info.m_surfaceName = "SRTM30";

      }
      else if (m_meanSpacing > 40.0)
      {
        // SRTM 3 arc
         info.m_absoluteCE  = 20.0;
         info.m_absoluteLE = 16.0;
         info.m_surfaceName = "SRTM3";
      }
      else
      {
          // SRTM 1 arc
         info.m_absoluteCE = 20.0;
         info.m_absoluteLE = 10.0;
         info.m_surfaceName = "SRTM1";
       }
   }

   return result;
}

bool rspfSrtmElevationDatabase::openSrtmDirectory(const rspfFilename& dir)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfSrtmElevationDatabase::open: Entered....\n";
   }
   if(!m_geoid.valid())
   {
      m_geoid = rspfGeoidManager::instance()->findGeoidByShortName("geoid1996", false);
   }
   
   //---
   // Sample format:
   //            dir
   //         |          |
   //    N35W121.hgt N35W121.hgt
   //---
   rspf_uint32 count = 0;
   rspf_uint32 maxCount = 25;
   if (!dir.isDir())
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfSrtmElevationDatabase::open: Not a directory, leaving ... " << dir << "\n";
      }
      return false;
   }
   
   rspfDirectory od(dir);
   if (od.isOpened() == false)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfSrtmElevationDatabase::open: Unable to open directory " << dir << ", Leaving ... \n";
      }
      return false;
   }
   
   rspfFilename f;
   //rspfSrtmSupportData sd;
   rspfRefPtr<rspfSrtmHandler> handler = new rspfSrtmHandler;
   if(od.getFirst(f, rspfDirectory::RSPF_DIR_FILES))
   {
      do
      {
         ++count;
         if(handler->open(f))
         {
            m_meanSpacing = handler->getMeanSpacingMeters();
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "rspfSrtmElevationDatabase::open: Found file " << f << "\n";
               if(!m_geoid.valid())
               {
                  rspfNotify(rspfNotifyLevel_DEBUG) << "rspfSrtmElevationDatabase::open: Unable to load goeid grid 1996 for SRTM database\n";
               }
            }
            return true;
         }
      }while((od.getNext(f)) &&(count < maxCount));
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfSrtmElevationDatabase::open: Leaving ... \n";
   }
   return false;
}

void rspfSrtmElevationDatabase::createRelativePath(rspfFilename& file, const rspfGpt& gpt)const
{
   int ilat =  static_cast<int>(floor(gpt.latd()));
   if (ilat < 0)
   {
      file = "S";
   }
   else
   {
      file = "N";
   }
   
   ilat = abs(ilat);
   std::ostringstream  os1;
   
   os1 << std::setfill('0') << std::setw(2) <<ilat;
   
   file += os1.str().c_str();
   
   int ilon = static_cast<int>(floor(gpt.lond()));
   
   if (ilon < 0)
   {
      file += "W";
   }
   else
   {
      file += "E";
   }
   
   ilon = abs(ilon);
   std::ostringstream  os2;
   os2 << std::setfill('0') << std::setw(3) << ilon;
   
   file += os2.str().c_str();
   file.setExtension("hgt");
}

bool rspfSrtmElevationDatabase::loadState(const rspfKeywordlist& kwl, const char* prefix )
{
   bool result = rspfElevationDatabase::loadState(kwl, prefix);
   if(result)
   {
      if(!m_connectionString.empty()&&rspfFilename(m_connectionString).exists())
      {
         result = open(m_connectionString);
      }
      else
      {
         // can't open the connection because it does not exists or empty
         result = false;
      }
   }
   
   return result;
}

bool rspfSrtmElevationDatabase::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   bool result = rspfElevationDatabase::saveState(kwl, prefix);
   
   return result;
}

rspfRefPtr<rspfElevCellHandler>
rspfSrtmElevationDatabase::createCell(const rspfGpt& gpt)
{

  rspfRefPtr<rspfElevCellHandler> result = 0;
  rspfFilename f;
  createFullPath(f, gpt);

  if(f.exists())
  {
     rspfRefPtr<rspfSrtmHandler> h = new rspfSrtmHandler();
     if (h->open(f, m_memoryMapCellsFlag))
     {
        result = h.get();
     }
  }

  return result;
}
