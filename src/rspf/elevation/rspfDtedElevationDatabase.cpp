#include <rspf/elevation/rspfDtedElevationDatabase.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfDirectory.h>
#include <rspf/base/rspfTrace.h>
#include <sstream>
#include <iomanip>
#include <cstdlib> /* for abs(int) */

static rspfTrace traceDebug("rspfDtedElevationDatabase:debug");
RTTI_DEF1(rspfDtedElevationDatabase, "rspfDtedElevationDatabase", rspfElevationCellDatabase);

double rspfDtedElevationDatabase::getHeightAboveMSL(const rspfGpt& gpt)
{
   if(!isSourceEnabled()) return rspf::nan();
   m_mutex.lock();
   if(m_lastHandler.valid())
   {
      if(m_lastHandler->pointHasCoverage(gpt))
      {
         double result = m_lastHandler->getHeightAboveMSL(gpt);
         m_mutex.unlock();
         
         return result;
      }
   }
   m_lastHandler = getOrCreateCellHandler(gpt);
   
   if(m_lastHandler.valid())
   {
      double result = m_lastHandler->getHeightAboveMSL(gpt);
      m_mutex.unlock();
     return result; // still need to shift
   }
   m_mutex.unlock();
   return rspf::nan();
}

double rspfDtedElevationDatabase::getHeightAboveEllipsoid(const rspfGpt& gpt)
{
   double h = getHeightAboveMSL(gpt);
   if(h != rspf::nan())
   {
      double offset = getOffsetFromEllipsoid(gpt);
      
      h += offset;
   }
   
   return h;
}
bool rspfDtedElevationDatabase::open(const rspfString& connectionString)
{
   bool result = false;
   rspfFilename file = rspfFilename(connectionString);
   
   result = openDtedDirectory(file);

   return result;
}

bool rspfDtedElevationDatabase::openDtedDirectory(const rspfFilename& dir)
{
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfDtedElevationDatabase::open entered ...\n";
   }
   bool result = dir.isDir();
   if(result)
   {
      rspf_uint32 count = 0;
      rspf_uint32 maxCount = 10;
      rspfDirectory od;
      result = od.open(dir);
      if(result)
      {
         result = false;
         rspfFilename f;
         // Get the first directory.
         od.getFirst(f, rspfDirectory::RSPF_DIR_DIRS);
         
         do
         {
            ++count;
            // Must be a directory.
            if (f.isDir())
            {
               // Discard any full path.
               rspfFilename fileOnly = f.file();
               
               // Downcase it.
              fileOnly.downcase();
               // Must start with 'e' or 'w'.
               bool foundCell = ( ((fileOnly.c_str()[0] == 'e') || ( fileOnly.c_str()[0] == 'w')) &&
                         (fileOnly.size() == 4));
               if(foundCell)
               {
                  rspf_uint32 maxCount2 = 10;
                  rspf_uint32 count2 = 0;
                  rspfDirectory d2;
                  if(d2.open(f))
                  {
                     d2.getFirst(f, rspfDirectory::RSPF_DIR_FILES);
                     do
                     {
                        rspfRefPtr<rspfDtedHandler> dtedHandler = new rspfDtedHandler();
                        if(dtedHandler->open(f, false))
                        {
                           if(traceDebug())
                           {
                              rspfNotify(rspfNotifyLevel_DEBUG) << "rspfDtedElevationDatabase::open: Found dted file " << f << "\n";
                           }
                           result = true;
                           m_extension = "."+f.ext();
                           m_connectionString = dir;
                           m_meanSpacing = dtedHandler->getMeanSpacingMeters();
                       }
                        dtedHandler->close();
                        dtedHandler = 0;
                        ++count2;
                     }while(!result&&d2.getNext(f)&&(count2 < maxCount2));
                  }
               }
            }
         }while(!result&&(od.getNext(f))&&(count < maxCount));
      }
   }
   
   if(result)
   {
      if(!m_geoid.valid())
      {
         m_geoid = rspfGeoidManager::instance()->findGeoidByShortName("geoid1996", false);
      }
      
      if(!m_geoid.valid()&&traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "rspfDtedElevationDatabase::open: Unable to load goeid grid 1996 for DTED database\n";
      }
   }
   
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "rspfDtedElevationDatabase::open leaving ...\n";
   }
   return result;
}

bool rspfDtedElevationDatabase::getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const
{
   bool result = false;
   
   m_mutex.lock();
   rspfDtedElevationDatabase* thisPtr = const_cast<rspfDtedElevationDatabase*>(this);
   rspfRefPtr<rspfElevCellHandler> tempHandler = thisPtr->getOrCreateCellHandler(gpt);
   m_mutex.unlock();

   if(tempHandler.valid())
   {
      result = tempHandler->getAccuracyInfo(info, gpt);
   }
  return result;
}

void rspfDtedElevationDatabase::createRelativePath(rspfFilename& file, const rspfGpt& gpt)const
{
   rspfFilename lon, lat;
   int ilon = static_cast<int>(floor(gpt.lond()));
   
   if (ilon < 0)
   {
      lon = "w";
   }
   else
   {
      lon = "e";
   }
   
   ilon = abs(ilon);
   std::ostringstream  s1;
   s1 << std::setfill('0') << std::setw(3)<< ilon;
   
   lon += s1.str().c_str();//rspfString::toString(ilon);
   
   int ilat =  static_cast<int>(floor(gpt.latd()));
   if (ilat < 0)
   {
      lat += "s";
   }
   else
   {
      lat += "n";
   }
   
   ilat = abs(ilat);
   std::ostringstream  s2;
   
   s2<< std::setfill('0') << std::setw(2)<< ilat;
   
   lat += s2.str().c_str();
   
   file = lon.dirCat(lat+m_extension);
}
rspfRefPtr<rspfElevCellHandler> rspfDtedElevationDatabase::createCell(const rspfGpt& gpt)
{
  rspfRefPtr<rspfElevCellHandler> result = 0;
  rspfFilename f;
  createFullPath(f, gpt);
  if(f.exists())
  {
     rspfRefPtr<rspfDtedHandler> h = new rspfDtedHandler(f, m_memoryMapCellsFlag);
     if (!(h->getErrorStatus()))
     {
        result = h.get();
     }
  }

  return result;
}

bool rspfDtedElevationDatabase::loadState(const rspfKeywordlist& kwl, const char* prefix )
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

bool rspfDtedElevationDatabase::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   bool result = rspfElevationDatabase::saveState(kwl, prefix);
   
   return result;
}
