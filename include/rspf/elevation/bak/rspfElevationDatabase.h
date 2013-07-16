#ifndef rspfElevationDatabase_HEADER
#define rspfElevationDatabase_HEADER
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfReferenced.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfGpt.h>
#include <rspf/base/rspfGeoid.h>
#include <rspf/base/rspfGeoidManager.h>
#include <rspf/elevation/rspfElevSource.h>
#include <rspf/elevation/rspfElevCellHandler.h>
#include <rspf/base/rspfTimer.h>
#include <vector>
#include <map>
class RSPF_DLL rspfElevationDatabase : public rspfElevSource
{
public:
   
   rspfElevationDatabase()
   :rspfElevSource(),
   m_meanSpacing(0.0)
   {
   }
      
   rspfElevationDatabase(const rspfElevationDatabase& src)
   :rspfElevSource(src),
   m_connectionString(src.m_connectionString),
   m_geoid(src.m_geoid),
   m_meanSpacing(src.m_meanSpacing)
   {
   }

   const rspfElevationDatabase& operator = (const rspfElevationDatabase& rhs);
      
   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& /*info*/, const rspfGpt& /*gpt*/) const
   {
      return false;
   }
  virtual double getMeanSpacingMeters() const
   {
      return m_meanSpacing;
   }
   virtual void setGeoid(rspfGeoid* geoid)
   {
      m_geoid = geoid;
   }
   rspfGeoid* getGeoid()
   {
      return m_geoid.get();
   }
   const rspfGeoid* getGeoid()const
   {
      return m_geoid.get();
   }
   
   /**
    * Open a connection to a database.  In most cases this will be a pointer
    * to a directory like in a Dted directory reader.  
    *
    */
   virtual bool open(const rspfString& connectionString)=0;
   const rspfString& getConnectionString()const
   {
      return m_connectionString;
   }
   virtual void getOpenCellList(std::vector<rspfFilename>& /* list */) const
   {
   }

   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix)const;
   
protected:
   virtual ~rspfElevationDatabase()
   {
      m_geoid = 0;
   }
   virtual double getOffsetFromEllipsoid(const rspfGpt& gpt)const;

   rspfString m_connectionString;
   rspfRefPtr<rspfGeoid>    m_geoid;
   rspf_float64              m_meanSpacing;
   
TYPE_DATA
};

class RSPF_DLL rspfElevationCellDatabase : public rspfElevationDatabase
{
public:
   struct CellInfo : rspfReferenced
   {
      CellInfo(rspf_uint64 id,
               rspfElevCellHandler* handler = 0)
         :rspfReferenced(),
          m_id(id),
          m_handler(handler),
          m_timestamp(0)
      {
            m_timestamp = rspfTimer::instance()->tick();
      }
      CellInfo(const CellInfo& src)
         :rspfReferenced(src),
          m_id(src.m_id),
          m_handler(src.m_handler),
          m_timestamp(src.m_timestamp)
      {
      }
      CellInfo()
         :rspfReferenced(),
          m_id(0),
          m_handler(0),
          m_timestamp(0)
      {
      }
      const CellInfo& operator =(const CellInfo& src)
      {
         if (this != &src)
         {
            m_id = src.m_id;
            m_handler = src.m_handler;
            m_timestamp = src.m_timestamp;
         }
         return *this;
      }
      void updateTimestamp()
      {
         m_timestamp = rspfTimer::instance()->tick();
      }
      rspf_uint64 id()const
      {
         return m_id;
      }
      rspf_uint64                      m_id;
      rspfRefPtr<rspfElevCellHandler> m_handler;
      rspfTimer::Timer_t               m_timestamp;
   };
   typedef std::map<rspf_uint64, rspfRefPtr<CellInfo> > CellMap;
   
   rspfElevationCellDatabase()
      :rspfElevationDatabase(),
      m_minOpenCells(5),
      m_maxOpenCells(10),
      m_memoryMapCellsFlag(false)
   {
   }
   rspfElevationCellDatabase(const rspfElevationCellDatabase& src)
      :rspfElevationDatabase(src),
      m_minOpenCells(src.m_minOpenCells),
      m_maxOpenCells(src.m_maxOpenCells),
      m_cacheMap(src.m_cacheMap),
      m_memoryMapCellsFlag(src.m_memoryMapCellsFlag)
   {
   }
   const rspfElevationCellDatabase& operator=(const rspfElevationCellDatabase& rhs);

   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix)const;
   virtual rspf_uint32 getMinOpenCells()const
   {
      return m_minOpenCells;
   }
   virtual rspf_uint32 getMaxOpenCells()const
   {
      return m_maxOpenCells;
   }
   virtual void setMinMaxOpenCells(rspf_uint64 minCellCount,
                                   rspf_uint64 maxCellCount)
   {
      m_minOpenCells = minCellCount;
      m_maxOpenCells = maxCellCount;
   }
   virtual bool getMemoryMapCellsFlag()const
   {
      return m_memoryMapCellsFlag;
   }
   virtual void setMemoryMapCellsFlag(bool flag)
   {
      m_memoryMapCellsFlag = flag;
   }
   virtual void getOpenCellList(std::vector<rspfFilename>& list) const;
   virtual rspf_uint64 createId(const rspfGpt& /* pt */)const
   {
      return 0;
   }
   virtual rspfRefPtr<rspfElevCellHandler> getOrCreateCellHandler(const rspfGpt& gpt);
protected:
   virtual rspfRefPtr<rspfElevCellHandler> createCell(const rspfGpt& /* gpt */)
   {
      return 0;
   }
   virtual void remove(rspf_uint64 id)
   {
      CellMap::iterator iter = m_cacheMap.find(id);
      if(iter != m_cacheMap.end())
      {
         m_cacheMap.erase(iter);
      }
   }
   void flushCacheToMinOpenCells()
   {
      // lets flush the cache from least recently used to recent.
      //
      CellMap sortedMap;
      CellMap::iterator iter = m_cacheMap.begin();
      
      while(iter != m_cacheMap.end())
      {
         sortedMap.insert(std::make_pair(iter->second->m_timestamp, iter->second));
         ++iter;
      }
      
      iter = sortedMap.begin();
      while((iter!=sortedMap.end())&&(m_cacheMap.size() > m_minOpenCells))
      {
         remove(iter->second->id());
         ++iter;
      }
   }
   
   rspf_uint32               m_minOpenCells;
   rspf_uint32               m_maxOpenCells;
   mutable OpenThreads::Mutex m_cacheMapMutex;
   CellMap                    m_cacheMap;
   rspf_uint32               m_memoryMapCellsFlag;
   
   TYPE_DATA;
};

#endif
