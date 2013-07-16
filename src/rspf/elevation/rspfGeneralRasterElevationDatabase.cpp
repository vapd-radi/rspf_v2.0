#include <rspf/elevation/rspfGeneralRasterElevationDatabase.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/base/rspfNotifyContext.h>
#include <rspf/base/rspfDirectory.h>
#include <sstream>
#include <iomanip>

RTTI_DEF1(rspfGeneralRasterElevationDatabase, "rspfGeneralRasterElevationDatabase", rspfElevationCellDatabase);

double rspfGeneralRasterElevationDatabase::getHeightAboveMSL(const rspfGpt& gpt)
{
   if(!isSourceEnabled()) return rspf::nan();
   rspfRefPtr<rspfElevCellHandler> handler = getOrCreateCellHandler(gpt);

   if(handler.valid())
   {
      return handler->getHeightAboveMSL(gpt); // still need to shift
   }
   return rspf::nan();
}
double rspfGeneralRasterElevationDatabase::getHeightAboveEllipsoid(const rspfGpt& gpt)
{
   double h = getHeightAboveMSL(gpt);
   if(h != rspf::nan())
   {
      h += getOffsetFromEllipsoid(gpt);
   }
   
   return h;
}
bool rspfGeneralRasterElevationDatabase::open(const rspfString& connectionString)
{
   bool result = false;
   rspfFilename file = rspfFilename(connectionString);
   m_connectionString = connectionString;
   result = openGeneralRasterDirectory(file);
   
   return result;
}

bool rspfGeneralRasterElevationDatabase::openGeneralRasterDirectory(const rspfFilename& dir)
{
   m_cellHandler = new rspfGeneralRasterElevHandler;
   bool foundOne = false;
   
   if(dir.exists())
   {
      if(dir.isDir())
      {
         rspfDirectory d(dir);
         
         rspfFilename file;
         rspf_uint32 maxCount = 10; // search at least the first 10 files to see if there are any here
         rspf_uint32 count = 0;
         if(d.getFirst(file))
         {
            do
            {
               ++count;
               rspfString ext = file.ext();
               ext = ext.downcase();
               if(ext == "ras")
               {
                  if(m_cellHandler->open(file, m_memoryMapCellsFlag))
                  {
                     m_meanSpacing = m_cellHandler->getMeanSpacingMeters();
                     foundOne = true;
                  }
               }
            } while(d.getNext(file) &&
                    (foundOne ||
                     (!foundOne && (count < maxCount))));
         }
      }
   }  
   
   if(!foundOne)
   {
      m_cellHandler = 0;
   }
   
   return m_cellHandler.valid();
}

bool rspfGeneralRasterElevationDatabase::pointHasCoverage(const rspfGpt& gpt) const
{
   bool result = false;
   
   if(m_cellHandler.valid())
   {
      result = m_cellHandler->pointHasCoverage(gpt);
   }
   
   return result;
}

void rspfGeneralRasterElevationDatabase::createRelativePath(
   rspfFilename& /* file */, const rspfGpt& /* gpt */)const
{
}

rspfRefPtr<rspfElevCellHandler> rspfGeneralRasterElevationDatabase::createHandler(const rspfGpt&  gpt )
{
   if(pointHasCoverage(gpt))
   {
      return m_cellHandler.get();
   }
   return 0;
}

rspfRefPtr<rspfElevCellHandler> rspfGeneralRasterElevationDatabase::createCell(const rspfGpt& gpt)
{
   if(pointHasCoverage(gpt))
   {
      return m_cellHandler.get();
   }
   return 0;
}


bool rspfGeneralRasterElevationDatabase::loadState(const rspfKeywordlist& kwl, const char* prefix )
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

bool rspfGeneralRasterElevationDatabase::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   bool result = rspfElevationDatabase::saveState(kwl, prefix);
   
   return result;
}


