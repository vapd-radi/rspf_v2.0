#ifndef rspfDtedElevationDatabase_HEADER
#define rspfDtedElevationDatabase_HEADER
#include "rspfElevationDatabase.h"
#include <rspf/base/rspfFilename.h>
#include <rspf/elevation/rspfDtedHandler.h>
#include <OpenThreads/Mutex>

class RSPF_DLL rspfDtedElevationDatabase : public rspfElevationCellDatabase
{
public:
   typedef std::vector<rspfRefPtr<CellInfo> > DirectMap; // 360x180 cell grid
   rspfDtedElevationDatabase()
   :rspfElevationCellDatabase(),
   m_extension("")
   {
   }
   rspfDtedElevationDatabase(const rspfDtedElevationDatabase& rhs)
   :rspfElevationCellDatabase(rhs),
   m_extension(rhs.m_extension)
   {
   }
   virtual ~rspfDtedElevationDatabase()
   {
   }
   rspfObject* dup()const
   {
      return new rspfDtedElevationDatabase(*this);
   }
   virtual bool open(const rspfString& connectionString);
   virtual bool pointHasCoverage(const rspfGpt& gpt) const
   {
      rspfFilename filename;
      createFullPath(filename, gpt);
      
      return filename.exists();
   }
   
   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const;
   virtual double getHeightAboveMSL(const rspfGpt&);
   virtual double getHeightAboveEllipsoid(const rspfGpt& gpt);
   virtual rspf_uint64 createId(const rspfGpt& pt)const
   {
      rspf_uint64 y = static_cast<rspf_uint64>(rspf::wrap(pt.latd(), -90.0, 90.0)+90.0);
      rspf_uint64 x = static_cast<rspf_uint64>(rspf::wrap(pt.lond(),-180.0,180.0)+180.0);
      // map the extreme edge to the same ID ax the 179 west cell and the same for the 89
      // degree north cell.
      //
      x = x==360?359:x;
      y = y==180?179:y;
      // dted databases are 1x1 degree cells and we will use a world 
      // grid for id generation.
      //
      return (y*360+x);
   }
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix = 0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix = 0)const;
   
protected:
   rspfString                m_extension;
   rspfRefPtr<rspfElevCellHandler> m_lastHandler;
   mutable OpenThreads::Mutex m_mutex;
   
   bool openDtedDirectory(const rspfFilename& dir);
   void createRelativePath(rspfFilename& file, const rspfGpt& gpt)const;
   void createFullPath(rspfFilename& file, const rspfGpt& gpt)const
   {
      rspfFilename relativeFile;
      createRelativePath(relativeFile, gpt);
      file = rspfFilename(m_connectionString).dirCat(relativeFile);
   }
   virtual rspfRefPtr<rspfElevCellHandler> createCell(const rspfGpt& gpt);
TYPE_DATA
};
#endif
