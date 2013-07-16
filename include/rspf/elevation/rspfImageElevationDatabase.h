//----------------------------------------------------------------------------
//
// File: rspfImageElevationDatabase.h
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: See description for class below.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef rspfImageElevationDatabase_HEADER
#define rspfImageElevationDatabase_HEADER 1

#include <rspf/elevation/rspfElevationDatabase.h>
#include <rspf/base/rspfConstants.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfGrect.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfRtti.h>
#include <map>

class rspfString;

/**
 * @class rspfTiledElevationDatabase
 *
 * Elevation source used for working with generic images opened by an
 * rspfImageHandler. This class is typically utilized through the
 * rspfElevManager.
 */
class RSPF_DLL rspfImageElevationDatabase : public rspfElevationCellDatabase
{
public:

   /** default constructor */
   rspfImageElevationDatabase();

   /**
    * @brief Open a connection to a database.
    *
    * @param connectionString File or directory to open.  In most cases this
    * will point to a directory containing DEMs. Satisfies pure virtual
    * rspfElevationDatabase::open().
    *
    * @return true on success, false on error.
    */   
   virtual bool open(const rspfString& connectionString);

   /** @brief close method. Unreferences all data. */
   virtual void close();

   /**
    * @brief Maps elevation data for region to a grid.
    *
    * This uses connectionString passed to open method as starting point.
    */
   void mapRegion(const rspfGrect& region);

   /**
    * @brief Get height above MSL for point.
    *
    * Satisfies pure virtual rspfElevSource::getHeightAboveMSL().
    * 
    * @return Height above MSL.
    */
   virtual double getHeightAboveMSL(const rspfGpt& gpt);

   /**
    * @brief Get height above ellipsoid for point.
    *
    * Satisfies pure virtual rspfElevSource::getHeightAboveMSL().
    * 
    * @return Height above MSL.
    */
   virtual double getHeightAboveEllipsoid(const rspfGpt&);
   
   /**
    * Satisfies pure virtual rspfElevSource::pointHasCoverage
    * 
    * @return true if database has coverage for point.
    */
   virtual bool pointHasCoverage(const rspfGpt& gpt) const;


   virtual bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& gpt) const;


   /**
    * Statisfies pure virtual rspfElevSource::getAccuracyLE90.
    * @return The vertical accuracy (90% confidence) in the
    * region of gpt:
    */
   //virtual double getAccuracyLE90(const rspfGpt& gpt) const;
   
   /**
    * Statisfies pure virtual rspfElevSource::getAccuracyCE90.
    * @return The horizontal accuracy (90% confidence) in the
    * region of gpt.
    */
   //virtual double getAccuracyCE90(const rspfGpt& gpt) const;

   /** @brief Initialize from keyword list. */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   /** @brief Save the state to a keyword list. */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix)const;

   /**
    * @brief Gets the bounding rectangle/coverage of elevation.
    *
    * @param rect Rectangle to initialize.
    */
   void getBoundingRect(rspfGrect& rect) const;

   /**
    * @brief ProcessFile method.
    *
    * This method is linked to the rspfFileWalker::walk method via a callback
    * mechanism.  So it is called by the rspfFileWalk (caller).  This class
    * (callee) sets recurse and return flags accordingly to control
    * the rspfFileWalker, e.g. don't recurse, stop altogether.
    * 
    * @param file to process.
    */
   void processFile(const rspfFilename& file);
   
protected:
   /**
    * @Brief Protected destructor.
    *
    * This class is derived from rspfReferenced so users should always use
    * rspfRefPtr<rspfImageElevationDatabase> to hold instance.
    */
   ~rspfImageElevationDatabase();

   virtual rspfRefPtr<rspfElevCellHandler> createCell(const rspfGpt& gpt);

   // virtual rspf_uint64 createId(const rspfGpt& pt) const;

   /**
    * @brief Gets cell for point.
    *
    * This override rspfElevationCellDatabase::getOrCreateCellHandler as we cannot use
    * the createId as our cells could be of any size.
    */
   virtual rspfRefPtr<rspfElevCellHandler> getOrCreateCellHandler(const rspfGpt& gpt);

   /**
    * @brief Removes an entry from the m_cacheMap and m_entryMap maps.
    */
   virtual void remove(rspf_uint64 id);

private:

   // Private container to hold bounding rect and image handler.
   struct rspfImageElevationFileEntry
   {
      /** @brief default constructor */
      rspfImageElevationFileEntry();

      /** @brief Constructor that takes a file name. */
      rspfImageElevationFileEntry(const rspfFilename& file);

      /** file name */
      rspfFilename m_file;

      /** Bounding rectangle in decimal degrees. */
      rspfGrect m_rect;

      /** True if in rspfElevationCellDatabase::m_cacheMap. */
      bool m_loadedFlag;
   };  

   /**
    * @brief Initializes m_entryMap with all loadable files from
    * m_connectionString.
    */
   void loadFileMap();

   /** Hidden from use copy constructor */
   rspfImageElevationDatabase(const rspfImageElevationDatabase& copy_this);
   
   /** Hidden from use assignment operator */
   const rspfImageElevationDatabase& operator=(const rspfImageElevationDatabase& rhs);

   std::map<rspf_uint64, rspfImageElevationFileEntry> m_entryMap;
   rspf_uint64       m_lastMapKey;
   rspf_uint64       m_lastAccessedId;

   TYPE_DATA 
};

inline void rspfImageElevationDatabase::remove(rspf_uint64 id)
{
   std::map<rspf_uint64, rspfImageElevationFileEntry>::iterator entryIter = m_entryMap.find(id);
   if ( entryIter != m_entryMap.end() )
   {
      (*entryIter).second.m_loadedFlag = false;
   }
   rspfElevationCellDatabase::remove(id);
}

#endif /* rspfImageElevationDatabase_HEADER */
