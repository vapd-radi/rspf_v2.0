//----------------------------------------------------------------------------
//
// File: rspfTiledElevationDatabase.h
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

#ifndef rspfTiledElevationDatabase_HEADER
#define rspfTiledElevationDatabase_HEADER 1

#include <rspf/elevation/rspfElevationDatabase.h>

#include <rspf/base/rspfGrect.h>
#include <rspf/base/rspfRefPtr.h>
#include <rspf/base/rspfRtti.h>

#include <vector>

class rspfDblGrid;
class rspfFilename;
class rspfFileWalker;
class rspfImageData;
class rspfImageGeometry;
class rspfImageHandler;
class rspfProjection;
class rspfSingleImageChain;
class rspfString;

/**
 * @class rspfTiledElevationDatabase
 *
 * Elevation source used for working with generic images opened by an
 * rspfImageHandler.  This class supplies a mapRegion method used to map a
 * region of elevation to a grid.  The grid in turn is used for the
 * getHeightAboveMSL.  This class is for applications that know their region
 * of interest up front and want to bypass the rspfElevManager and grid the
 * elevation prior to processing for speed.  Can work on a file or a
 * directory of files.
 */
class RSPF_DLL rspfTiledElevationDatabase : public rspfElevationDatabase
{
public:

   /** default constructor */
   rspfTiledElevationDatabase();

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


   /** @brief Initialize from keyword list. */
   virtual bool loadState(const rspfKeywordlist& kwl, const char* prefix=0);

   /** @brief Save the state to a keyword list. */
   virtual bool saveState(rspfKeywordlist& kwl, const char* prefix)const;
   
    bool getAccuracyInfo(rspfElevationAccuracyInfo& info, const rspfGpt& /*gpt*/) const
    {
       info.makeNan();

       return false;
    }
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
   /** Protected destructor as this is derived from rspfRefenced. */
   ~rspfTiledElevationDatabase();

private:

   // Private container to hold bounding rect and image handler.
   struct rspfTiledElevationEntry
   {
      /** @brief default constructor */
      rspfTiledElevationEntry();

      /** @brif Constructor that takes rectangle and chain. */
      rspfTiledElevationEntry(const rspfGrect& rect,
                               rspfRefPtr<rspfSingleImageChain> sic );

      /** Bounding rectangle in decimal degrees. */
      rspfGrect m_rect;

      /** Hold pointer to single image chain. */
      rspfRefPtr<rspfSingleImageChain> m_sic;
   };

   /**
    * @brief adds entry to the list checking for duplicates.
    */
   void addEntry(const rspfTiledElevationEntry& entry);

   /**
    * @brief Initializes m_referenceProj from the first entry.
    */
   void initializeReferenceProjection();

   /**
    * @return true if file is a directory based image and the stager should go
    * on to next directory; false if stager should continue with directory.
    */
   bool isDirectoryBasedImage(rspfRefPtr<rspfImageHandler> ih);

   /**
    * @brief Check for match of the following against the first entry of:
    *  bands, projection, scalar type and scale.
    * @returns true if good, false if not the same.
    */
   bool isCompatible(rspfImageHandler* ih,
                     rspfImageGeometry* geom,
                     rspfProjection* proj) const;

   /**
    * @brief Initialize bounding rectangle from image handler.
    *
    * Sets boundingRect to nan if rspfImageGeometry::getCornerGpts returns false.
    * 
    * @param ih Image handler.
    * @param boundingRect Initialized by method.
    */
   void getBoundingRect(rspfRefPtr<rspfImageGeometry> geom, rspfGrect& boundingRect) const;

   /** @brief Loads m_requestedRect into m_grid from m_entries. */
   void mapRegion();

   /** @brief Templated fill grid method. */
   template <class T> void fillGrid(T dummyTemplate, rspfRefPtr<rspfImageData> data);

   /** Hidden from use copy constructor */
   rspfTiledElevationDatabase(const rspfTiledElevationDatabase& copy_this);

   /** Hidden from use assignment operator */
   const rspfTiledElevationDatabase& operator=(const rspfTiledElevationDatabase& rhs);

   std::vector<rspfTiledElevationEntry> m_entries;

   /** Hold region of elevation. */
   rspfDblGrid* m_grid;

   /** Projection of the first entry.  Stored for convenience. */
   rspfRefPtr<rspfProjection> m_referenceProj;

   rspfGrect m_requestedRect;
   rspfGrect m_entryListRect;
   rspfGrect m_mappedRect; // Requested expanded to even post boundary.

   rspfFileWalker* m_fileWalker;

   TYPE_DATA 
};

#endif /* rspfTiledElevationDatabase_HEADER */
