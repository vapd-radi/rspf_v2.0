#ifndef rspfGeneralRasterElevationDatabase_HEADER
#define rspfGeneralRasterElevationDatabase_HEADER
#include "rspfElevationDatabase.h"
#include <rspf/base/rspfFilename.h>
#include <rspf/elevation/rspfGeneralRasterElevHandler.h>
#include <OpenThreads/Mutex>

class RSPF_DLL rspfGeneralRasterElevationDatabase : public rspfElevationCellDatabase
{
public:
   rspfGeneralRasterElevationDatabase()
   :rspfElevationCellDatabase()
   {
   }
   rspfGeneralRasterElevationDatabase(const rspfGeneralRasterElevationDatabase& rhs)
   :rspfElevationCellDatabase(rhs)
   {
   }
   virtual ~rspfGeneralRasterElevationDatabase()
   {
      if(m_cellHandler.valid())
      {
         m_cellHandler->close();
      }
      m_cellHandler = 0;
   }
   rspfObject* dup()const
   {
      return new rspfGeneralRasterElevationDatabase(*this);
   }
   virtual bool open(const rspfString& connectionString);
   virtual bool pointHasCoverage(const rspfGpt& gpt) const;

   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& /*info*/, const rspfGpt& /*gpt*/) const
   {
      return false;
   }

   /**
    * METHODS: accuracyLE90(), accuracyCE90()
    * Returns the vertical and horizontal accuracy (90% confidence) in the
    * region of gpt:
    */
//   virtual double getAccuracyLE90(const rspfGpt& /* gpt */) const
//   {
//      std::cout << "rspfGeneralElevationDatabase::getAccuracyLE90 \n";
//      return 0.0;
//   }
//   virtual double getAccuracyCE90(const rspfGpt& /* gpt */) const
 //  {
 //     std::cout << "rspfGeneralElevationDatabase::getAccuracyCE90 \n";
 //     return 0.0;
 //  }
   virtual double getHeightAboveMSL(const rspfGpt&);
   virtual double getHeightAboveEllipsoid(const rspfGpt& gpt);
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix = 0);
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix = 0)const;
   virtual rspf_uint64 createId(const rspfGpt& /* pt */)const
   {
     return 0;
   }
   
protected:
   rspfRefPtr<rspfGeneralRasterElevHandler> m_cellHandler;
   bool openGeneralRasterDirectory(const rspfFilename& dir);
   void createRelativePath(rspfFilename& file, const rspfGpt& gpt)const;
   void createFullPath(rspfFilename& file, const rspfGpt& gpt)const
   {
      rspfFilename relativeFile;
      createRelativePath(relativeFile, gpt);
      file = rspfFilename(m_connectionString).dirCat(relativeFile);
   }
   rspfRefPtr<rspfElevCellHandler> createHandler(const rspfGpt& /* gpt */);
   virtual rspfRefPtr<rspfElevCellHandler> createCell(const rspfGpt& /* gpt */);
   
   
   TYPE_DATA
};
#endif
