#include <rspf/elevation/rspfElevationDatabase.h>

RTTI_DEF1(rspfElevationDatabase, "rspfElevationDatabase", rspfObject);
RTTI_DEF1(rspfElevationCellDatabase, "rspfElevationCellDatabase", rspfElevationDatabase);

const rspfElevationDatabase& rspfElevationDatabase::operator=(const rspfElevationDatabase& rhs)
{
   if ( this != &rhs )
   {
      rspfElevSource::operator=(rhs);
      m_connectionString = rhs.m_connectionString;
      m_geoid            = rhs.m_geoid;
      m_meanSpacing      = rhs.m_meanSpacing;
   }
   return *this;
}

double rspfElevationDatabase::getOffsetFromEllipsoid(const rspfGpt& gpt)const
{
   double result = 0.0;
   if(m_geoid.valid())
   {
      result = m_geoid->offsetFromEllipsoid(gpt);
   }
   else 
   {
      result = rspfGeoidManager::instance()->offsetFromEllipsoid(gpt);
   }
   
   if(rspf::isnan(result))
   {
      result = 0.0;
   }
   
   return result;
}

bool rspfElevationDatabase::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   m_connectionString = kwl.find(prefix, "connection_string");
   rspfString minOpenCells = kwl.find(prefix, "min_open_cells");
   rspfString maxOpenCells = kwl.find(prefix, "max_open_cells");
   rspfString geoidType    = kwl.find(prefix, "geoid.type");
   if(m_connectionString.empty())
   {
      // try backward compatability to a filename
      //
      m_connectionString = kwl.find(prefix, rspfKeywordNames::FILENAME_KW);
   }
   if(!geoidType.empty())
   {
      m_geoid = rspfGeoidManager::instance()->findGeoidByShortName(geoidType);
   }
   return rspfObject::loadState(kwl, prefix);
}

bool rspfElevationDatabase::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix, "connection_string", m_connectionString, true);
   
   if(m_geoid.valid())
   {
      kwl.add(prefix, "geoid.type", m_geoid->getShortName(), true);
   }
   
   return rspfObject::saveState(kwl, prefix);
}

const rspfElevationCellDatabase& rspfElevationCellDatabase::operator=(
   const rspfElevationCellDatabase& rhs)
{
   if ( this != &rhs )
   {
      rspfElevationDatabase::operator=(rhs);
      m_minOpenCells       = rhs.m_minOpenCells;
      m_maxOpenCells       = rhs.m_maxOpenCells;
      m_cacheMap           = rhs.m_cacheMap;
      m_memoryMapCellsFlag = rhs.m_memoryMapCellsFlag;
   }
   return *this;
}

void rspfElevationCellDatabase::getOpenCellList(std::vector<rspfFilename>& list) const
{
   CellMap::const_iterator iter = m_cacheMap.begin();

   while(iter!=m_cacheMap.end())
   {
      list.push_back(iter->second->m_handler->getFilename());
      ++iter;
   }

}

rspfRefPtr<rspfElevCellHandler> rspfElevationCellDatabase::getOrCreateCellHandler(const rspfGpt& gpt)
{
  rspfRefPtr<rspfElevCellHandler> result = 0;
  rspf_uint64 id = createId(gpt);
  
  {
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_cacheMapMutex);
    CellMap::iterator iter = m_cacheMap.find(id);
    if(iter != m_cacheMap.end())
    {
      iter->second->updateTimestamp();
      result = iter->second->m_handler.get();
      
      return result.get();
    }
  }
  
  result = createCell(gpt);
  
  {
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_cacheMapMutex);
    if(result.valid())
    {
      m_cacheMap.insert(std::make_pair(id, new CellInfo(id, result.get())));

      // Check the map size and purge cells if needed.
      if(m_cacheMap.size() > m_maxOpenCells)
      {
         flushCacheToMinOpenCells();
      }
    }
  }

  return result;
}

bool rspfElevationCellDatabase::loadState(const rspfKeywordlist& kwl, const char* prefix)
{
   rspfString minOpenCells = kwl.find(prefix, "min_open_cells");
   rspfString maxOpenCells = kwl.find(prefix, "max_open_cells");
   rspfString geoidType    = kwl.find(prefix, "geoid.type");
   if(!minOpenCells.empty()&&
      !maxOpenCells.empty())
   {
      m_minOpenCells = minOpenCells.toUInt32();
      m_maxOpenCells = maxOpenCells.toUInt32();
      if(m_maxOpenCells < m_minOpenCells)
      {
         std::swap(m_minOpenCells, m_maxOpenCells);
      }
   }
   rspfString memoryMapCellsFlag = kwl.find(prefix, "memory_map_cells");
   if(!memoryMapCellsFlag.empty())
   {
      m_memoryMapCellsFlag  = memoryMapCellsFlag.toBool();
   }
   return rspfElevationDatabase::loadState(kwl, prefix);
}

bool rspfElevationCellDatabase::saveState(rspfKeywordlist& kwl, const char* prefix)const
{
   kwl.add(prefix, "memory_map_cells", m_memoryMapCellsFlag, true);
   kwl.add(prefix, "min_open_cells", m_minOpenCells, true);
   kwl.add(prefix, "max_open_cells", m_maxOpenCells, true);

   if(m_geoid.valid())
   {
      kwl.add(prefix, "geoid.type", m_geoid->getShortName(), true);
   }

   return rspfElevationDatabase::saveState(kwl, prefix);
}



