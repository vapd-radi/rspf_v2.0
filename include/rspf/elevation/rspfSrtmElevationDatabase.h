//----------------------------------------------------------------------------
//
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfSrtmElevationDatabase_HEADER
#define rspfSrtmElevationDatabase_HEADER 1
#include "rspfElevationDatabase.h"
#include <rspf/base/rspfFilename.h>
#include <rspf/elevation/rspfSrtmHandler.h>
#include <OpenThreads/Mutex>

class RSPF_DLL rspfSrtmElevationDatabase : public rspfElevationCellDatabase
{
public:
   typedef std::vector<rspfRefPtr<CellInfo> > DirectMap; // 360x180 cell grid
   rspfSrtmElevationDatabase()
   :rspfElevationCellDatabase()
   {
   }
   rspfSrtmElevationDatabase(const rspfSrtmElevationDatabase& rhs)
   :rspfElevationCellDatabase(rhs)
   {
   }
   virtual ~rspfSrtmElevationDatabase()
   {
   }
   rspfObject* dup()const
   {
      return new rspfSrtmElevationDatabase(*this);
   }
   virtual bool open(const rspfString& connectionString);
   bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& /*gpt*/) const;
   virtual bool pointHasCoverage(const rspfGpt& gpt) const
   {
      rspfFilename filename;
      createFullPath(filename, gpt);
      
      return filename.exists();
   }
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
      // Srtm databases are 1x1 degree cells and we will use a world 
      // grid for id generation.
      //
      return (y*360+x);
   }
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix = 0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix = 0)const;
   
protected:
   bool openSrtmDirectory(const rspfFilename& dir);
   void createRelativePath(rspfFilename& file, const rspfGpt& gpt)const;
   void createFullPath(rspfFilename& file, const rspfGpt& gpt)const
   {
      rspfFilename relativeFile;
      createRelativePath(relativeFile, gpt);
      file = rspfFilename(m_connectionString).dirCat(relativeFile);
   }

   rspfRefPtr<rspfElevCellHandler> createCell(const rspfGpt& gpt);

   TYPE_DATA
};
#endif
