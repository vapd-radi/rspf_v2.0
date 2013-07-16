//*************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// License:  See top level LICENSE.txt file.
//
// Author: Oscar Kramer
//
//*************************************************************************************************
//  $Id: rspfEpsgProjectionDatabase.h 19879 2011-07-30 16:21:50Z dburken $

#ifndef rspfEpsgProjectionDatabase_HEADER
#define rspfEpsgProjectionDatabase_HEADER

#include <rspf/projection/rspfProjectionFactoryBase.h>
#include <rspf/base/rspfFilename.h>
#include <fstream>
#include <rspf/projection/rspfMapProjection.h>

class rspfProjection;
class rspfString;
class rspfEpsgProjectionFactory;
class rspfUtmProjection;

//*************************************************************************************************
//! Projection Database for coded projections defined in database files and specified via some 
//! coordinate reference system group:code, such as EPSG:26715.
//*************************************************************************************************
class RSPFDLLEXPORT rspfEpsgProjectionDatabase : public rspfReferenced
{
   friend class rspfEpsgProjectionFactory;

public:
   //! Instantiates singleton instance of this class:
   static rspfEpsgProjectionDatabase* instance();

   //! Destructor
   virtual ~rspfEpsgProjectionDatabase();

   //! Returns a projection corresponding to the projection specified, or NULL if no entry found.
   //! Normally <proj_spec> takes the form of <group>:<code> (e.g., "EPSG:26715"). Other forms 
   //! accepted are <code> (assumed EPSG), or <proj_name> to match a name in the database.
   rspfProjection* findProjection(const rspfString& proj_spec) const;

   //! Returns a projection corresponding to the epsg code specified, or NULL if no entry found.
   rspfProjection* findProjection(rspf_uint32 epsg_code) const;

   //! Given a projection name, assigns the group (e.g., "EPSG") and code of the projection. 
   //! Returns integer EPSG code if match was found or 0 if not found.
   rspf_uint32 findProjectionCode(const rspfString& projection_name) const;

   //! Given a projection instance, this method determines the corresponding EPSG code. Obviously
   //! this is only needed if the projection does not have its PCS code assigned (it is NULL). This
   //! happens when the projection was constructed with full parameters instead of the EPSG code.
   //! Returns integer EPSG code if match was found or 0 if not found.
   rspf_uint32 findProjectionCode(const rspfMapProjection& lost_proj) const;

   //! ### NECESSARY HACK ### (originally implemented by ESH)
   //! ArcMap (9.2 and less) doesn't understand the non-meters HARN codes.  
   //! This method provides acess to the projection name given the EPSG code. The name provides 
   //! enough info for recognizing a non-meters HARN-based projection. 
   rspfString findProjectionName(rspf_uint32 epsg_code) const;

   //! Populates caller's list with all projections currently represented in the database.
   //! The format is: 
   //!    EPSG:<code>  "<Name as appears in Db>"
   //! There may be duplicate entries if the projection is duplicated in the various Db files.
   void getProjectionsList(std::vector<rspfString>& typeList) const;

   //! ENGINEERING CODE. Used for testing
   size_t numRecords() const { return m_projDatabase.size(); }

protected:
   enum RecordFormat
   {  
      NOT_ASSIGNED=0,
      FORMAT_A,
      FORMAT_B,
      FORMAT_C,
      CUSTOM
   };

   //! Type for database record consists of EPSG code and serialized form of corresponding RSPF 
   //! projection (as a keywordlist)
   class ProjDbRecord : public rspfReferenced
   {
   public:
      ProjDbRecord() : 
            code(0), 
            name(""), 
            datumValid(false), 
            csvFormat(NOT_ASSIGNED), 
            proj(0) {}

      rspf_uint32     code;
      rspfString      name;
      bool             datumValid; //!< FALSE if the datum code was not parsed and WGS84 defaulted
      RecordFormat     csvFormat;
      std::vector<rspfString>        csvRecord;
      rspfRefPtr<rspfMapProjection> proj;
   };

   //! Constructor loads all Db files specified in the rspf prefs. Protected as part of
   //! singleton implementation.
   rspfEpsgProjectionDatabase();

   //! Parses the "Spadac EPSG" Db record format and produces a projection (or NULL if invalid)   
   rspfMapProjection* createProjFromFormatARecord(ProjDbRecord* record) const;

   //! Parses the State Plane Db record format and produce a projection (or NULL if invalid)
   rspfMapProjection* createProjFromFormatBRecord(ProjDbRecord* record) const;

   //!  ### HACK ###
   //! UTM projections as specified in the EPSG are indistinguishable from regular TM. Unfortunately
   //! RSPF does make a distinction. For the time being, parse the code and programmatically arrive 
   //! at the UTM projection.
   rspfMapProjection* createProjFromUtmCode(rspf_uint32 code) const;

   //! Given UTM projection, derives the associated EPSG code. This is faster than a Db lookup.
   rspf_uint32 getCodeFromUtmProj(const rspfUtmProjection* proj) const;

   //! Populates the database with contents of DB files as specified in rspf_preferences.
   void initialize();

   mutable std::multimap<rspf_uint32, rspfRefPtr<ProjDbRecord> > m_projDatabase;
   static rspfEpsgProjectionDatabase*  m_instance; //!< Singleton implementation

};

#endif
