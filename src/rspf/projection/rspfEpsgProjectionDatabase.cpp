//*************************************************************************************************
//                          RSPF -- Open Source Software Image Map
//
// LICENSE:  See top level LICENSE.txt file.
//
// AUTHOR: Oscar Kramer
//
// DESCRIPTION: Projection database for EPSG coded projections provided in database files
//
//*************************************************************************************************
//  $Id: rspfEpsgProjectionDatabase.cpp 20447 2012-01-12 17:33:47Z gpotts $
#include <rspf/projection/rspfEpsgProjectionDatabase.h>
#include <rspf/projection/rspfStatePlaneProjectionInfo.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfCommon.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/projection/rspfMercatorProjection.h>
#include <rspf/projection/rspfOrthoGraphicProjection.h>
#include <rspf/projection/rspfNewZealandMapGridProjection.h>
#include <rspf/projection/rspfLambertConformalConicProjection.h>
#include <rspf/projection/rspfCassiniProjection.h>
#include <rspf/projection/rspfAlbersProjection.h>
#include <rspf/base/rspfEpsgDatumFactory.h>
#include <rspf/base/rspfDatumFactory.h>
#include <rspf/base/rspfDatumFactoryRegistry.h>
#include <rspf/base/rspfPreferences.h>
#include <rspf/projection/rspfMapProjectionFactory.h>
#include <rspf/base/rspfException.h>
#include <math.h>

rspfEpsgProjectionDatabase* rspfEpsgProjectionDatabase::m_instance = 0;

// Indexes into the DB file fields using "SPADAC EPSG" format (format "A"). 
// IF THE LINE FIELDS CHANGE ORDER, THIS LIST WILL NEED TO BE CHANGED TO REFLECT THE NEW ORDER!
enum
{
   A_CODE = 0,  // This is the EPSG code
   A_NAME,
   A_PROJ_TYPE,
   A_UNITS,
   A_DATUM_CODE,
   A_DATUM_NAME,
   A_ELLIPSOID_NAME,
   A_FALSE_EASTING,
   A_NAT_ORG_LAT,
   A_FALSE_NORTHING,
   A_NAT_ORG_LON,
   A_NAT_ORG_SCALE,
   A_INIT_LINE_AZ,
   A_PROJ_CTR_LAT,
   A_PROJ_CTR_NORTHING,
   A_RECT_TO_SKEW_ANGLE,
   A_INIT_LINE_SCALE,
   A_PROJ_CTR_EASTING,
   A_PROJ_CTR_LON,
   A_PSEUDO_STD_PARL_1,
   A_PSEUDO_STD_PARL_1_SCALE,
   A_ORG_LON,
   A_STD_PARL_2_LAT,
   A_FALSE_ORG_LAT,
   A_FALSE_ORG_LON,
   A_STD_PARL_1_LAT,
   A_FALSE_ORG_NORTHING,
   A_FALSE_ORG_EASTING,
   A_STD_PARL_LAT,
   A_TOPOCTR_HGT,
   A_TOPOCTR_LAT,
   A_TOPOCTR_Z,
   A_TOPOCTR_Y,
   A_TOPOCTR_X,
   A_VIEWPOINT_H,
   A_INIT_LON,
   A_ZONE_WIDTH,
   A_NUM_FIELDS   // Not an index, but a count
};
static const rspfString EPSG_DB_FORMAT_A ("EPSG_DB_FORMAT_A");

// State Plane database CSV file field format (format "B")
// NOTE: Eventually, all EPSG-coded projections, including State Plane/HARN should be specified
// in the EPSG database format. Presently there is a disconnect between Geotrans params and EPSG-Db
// params, particularly regarding datums, that makes a purely EPSG-Db solution too awkward to
// implement, so provision is made here to continue reading the state plane CSV files.
enum 
{
   B_NAME = 0,
   B_CODE,   // This is the EPSG code
   B_PROJ_TYPE,
   B_PARAM1,
   B_PARAM2,
   B_PARAM3,
   B_PARAM4,
   B_FALSE_EASTING,
   B_FALSE_NORTHING,
   B_UNITS,
   B_DATUM_CODE,
   B_NUM_FIELDS   // Not an index, but a count
};
static const rspfString STATE_PLANE_FORMAT_B ("STATE_PLANE");

// "Ming Special" database CSV file format (format "C")
// State Plane Coordinate System (SPCS) coding is an alternate coding scheme that maps to EPSG.
// http://www.pcigeomatics.com/cgi-bin/pcihlp/PROJ%7CSPCS+ZONES
enum 
{
   C_NAME = 0,
   C_CODE,
   C_NUM_FIELDS   // Not an index, but a count
};
static const rspfString SPCS_EPSG_MAP_FORMAT_C ("SPCS_EPSG_MAP");

//*************************************************************************************************
//! Converts sexagesimal DMS to decimal degrees
//*************************************************************************************************
double decodeSexagesimalDms(const rspfString& sex_str)
{
   if (!sex_str.contains("."))
      return sex_str.toDouble();

   double sign = 1.0;
   if (sex_str.chars()[0] == '-')
      sign = -1.0;

   double dsex, msex, ssex;
   std::vector<rspfString> splitstr = sex_str.split(".");
   dsex = splitstr[0].toDouble();
   
   if (splitstr.size() < 2)
      return dsex;

   const char* MMSSssstr = splitstr[1].chars();
   unsigned int str_size = (unsigned int) splitstr[1].size();
   char minstr[] = "00";
   rspfString secstr ("00.");
   minstr[0] = MMSSssstr[0];
   if (str_size > 1)
   {
      minstr[1] = MMSSssstr[1];
      if (str_size > 2)
      {
         secstr = &(MMSSssstr[2]);
         if (str_size == 3)
            secstr += rspfString("0.");
         else if (str_size > 4)
            secstr.insert(2, ".");
      }
   }
   msex = rspfString(minstr).toDouble();
   ssex = secstr.toDouble();
   dsex += sign*(msex + ssex/60.0)/60.0;
   return dsex;
}

//*************************************************************************************************
//! Implements singleton pattern
//*************************************************************************************************
rspfEpsgProjectionDatabase* rspfEpsgProjectionDatabase::instance()
{
   if(!m_instance)
      m_instance = new rspfEpsgProjectionDatabase;
   return m_instance;
}

//*************************************************************************************************
//! Destructor
//*************************************************************************************************
rspfEpsgProjectionDatabase::~rspfEpsgProjectionDatabase()
{
}

//*************************************************************************************************
//! Constructor loads all DB CSV files specified in the rspf prefs
//*************************************************************************************************
rspfEpsgProjectionDatabase::rspfEpsgProjectionDatabase()
{
   // Read the rspf preferences for Db CSV files.
   initialize();
}

//*************************************************************************************************
//! Populates the database with contents of DB files as specified in rspf_preferences.
//*************************************************************************************************
void rspfEpsgProjectionDatabase::initialize()
{
   // Fetch filenames of all projection DB files specified in rspf_preferences:
   rspfString regEx =  rspfString("^epsg_database_file[0-9]+");
   vector<rspfString> keys = 
      rspfPreferences::instance()->preferencesKWL().getSubstringKeyList(regEx);
   vector<rspfString>::const_iterator i = keys.begin();

   // Create only once outside the loop:
   rspfFilename db_name;
   rspfString group_id;
   rspfString format_id;
   rspfString line;

   // Loop over each file and read contents into memory:
   while ( i != keys.end() )
   {
      db_name = rspfPreferences::instance()->preferencesKWL().find( (*i).c_str() );
      ++i;
      if (!db_name.isReadable())
         continue;

      // Open the DB file:
      std::ifstream db_stream (db_name.chars());
      bool good_file = false;
      if (db_stream.good())
      {
         // Format specification implied in file's magic number:
         std::getline(db_stream, format_id.string());
         format_id.trim();
         if ((format_id == EPSG_DB_FORMAT_A) || 
             (format_id == STATE_PLANE_FORMAT_B) ||
             (format_id == SPCS_EPSG_MAP_FORMAT_C))
            good_file = true;
      }
      if (!good_file)
      {
         rspfNotify(rspfNotifyLevel_WARN)<<"rspfEpsgProjectionDatabase::initialize() -- "
            "Encountered bad database file <"<<db_name<<">. Skipping this file."<<endl;
         db_stream.close();
         continue;
      }

      // The file is good. Skip over the column descriptor line:
      std::getline(db_stream, line.string());

      // Loop to read all data records:
      while (!db_stream.eof())
      {
         rspfRefPtr<ProjDbRecord> db_record = new ProjDbRecord;
         std::getline(db_stream, line.string());
         db_record->csvRecord = line.explode(","); // ONLY CSV FILES CONSIDERED HERE
         if (db_record->csvRecord.size())
         {
            rspfRefPtr<rspfMapProjection> proj;

            // Check if primary EPSG database format A:
            if (format_id == EPSG_DB_FORMAT_A)
            {
               db_record->code = db_record->csvRecord[A_CODE].toUInt32();
               db_record->name = db_record->csvRecord[A_NAME];
               db_record->csvFormat = FORMAT_A;
            }

            // Check if State Plane (subset of EPSG but handled differently until projection 
            // geotrans-EPSG disconnect is resolved. 
            else if (format_id == STATE_PLANE_FORMAT_B)
            {
               db_record->code = db_record->csvRecord[B_CODE].toUInt32();
               db_record->name = db_record->csvRecord[B_NAME];
               db_record->csvFormat = FORMAT_B;
            }

            // This format is for Ming-special State Plane Coordinate System coded format.
            // This format is simply a mapping from SPCS spec name (RSPF-specific) to EPSG code.
            // Note that no proj is instantiated and no KWL is populated. Only name and EPSG mapped
            // code is saved.
            else if (format_id == SPCS_EPSG_MAP_FORMAT_C)
            {
               db_record->code = db_record->csvRecord[C_CODE].toUInt32();
               db_record->name = db_record->csvRecord[C_NAME];
               db_record->csvFormat = FORMAT_C;
            }

            m_projDatabase.insert(make_pair(db_record->code, db_record));
         }
      }

      db_stream.close();
   } // end of while loop over all DB files
}

//*************************************************************************************************
//! Returns a projection corresponding to the group (e.g., "EPSG") and PCS code provided, 
//! or NULL if no entry found.
//*************************************************************************************************
rspfProjection* rspfEpsgProjectionDatabase::findProjection(rspf_uint32 epsg_code) const
{
   rspfMapProjection* proj = 0;

   // Quick check for bogus EPSG:
   if ((epsg_code == 0) || (epsg_code == 32767))
      return 0;

   // Check for Google projection:
   else if (epsg_code == 900913)
   {
      const rspfDatum* datum = rspfDatumFactory::instance()->create(rspfString("6055"));
      rspfMercatorProjection* merc_proj = new rspfMercatorProjection();
      rspfGpt origin(0.0,0.0,0.0, datum);
      merc_proj->setFalseEasting(0.0);
      merc_proj->setFalseNorthing(0.0);
      merc_proj->setOrigin(origin); // Also sets the projections datum to the origin's datum
      merc_proj->update();
      merc_proj->setPcsCode(900913);
      proj = merc_proj;
   }

   else
   {
      // Search database for entry:
      std::multimap<rspf_uint32, rspfRefPtr<ProjDbRecord> >::iterator db_iter = 
         m_projDatabase.find(epsg_code);
      if (db_iter != m_projDatabase.end())
      {
         // See if a projection has already been created for this entry:
         ProjDbRecord* db_record = db_iter->second.get();
         if (db_record->proj.valid())
            proj = (rspfMapProjection*) db_record->proj->dup();
         else
         {
            // Try decoding the EPSG code before accessing DB:
            proj = createProjFromUtmCode(epsg_code);
            if (proj)
            {
               db_record->proj = proj;
               db_record->datumValid = true;
            }
            else if (db_iter->second->csvFormat == FORMAT_A)
               proj = createProjFromFormatARecord(db_record);
            else if (db_iter->second->csvFormat == FORMAT_B)
               proj = createProjFromFormatBRecord(db_record);

            if (proj)
            {
               // To save allocated memory, get rid of the original CSV entry since a real 
               // projection is now represented in the database:
               db_record->csvRecord.clear();
               db_record->csvFormat = NOT_ASSIGNED;
            }
         }
      }
   }

   return proj;
}

//*************************************************************************************************
//! Returns a projection corresponding to the group (e.g., "EPSG") and PCS code provided, 
//! or NULL if no entry found.
//*************************************************************************************************
rspfProjection* rspfEpsgProjectionDatabase::findProjection(const rspfString& spec) const
{
   //std::cout << "rspfEpsgProjectionDatabase::findProjection: entered with spec " << spec << "\n";
   rspfProjection* proj = 0;

   // Use the CRS code to access the database. The spec should be <group>:<code> where <group> is 
   // "EPSG" (the only group handled here):
   rspf_uint32 spec_code;
   rspfString spec_group ("epsg"); // default if only integer code provided
   if (spec.contains(":"))
   {
      spec_group = spec.before(":");
      spec_code = spec.after(":").toUInt32();
      spec_group = spec_group.downcase();
   }
   else
   {
      spec_code = spec.toUInt32();
   }

   // Presently only EPSG database is handled:
   if ((spec_code != 0) && (spec_group == "epsg"))
      return findProjection(spec_code);

   // The spec is probably a projection name. Need to search Db by the projection name. 
   // Search database for entry. The spec may use different delimiters than
   // the DB so need to split the strings and compare the words:
   rspfString separators ("_ /()");
   vector<rspfString> split_spec = spec.split(separators, true);
   vector<rspfString> split_db_name;
   rspfRefPtr<rspfMapProjection> map_proj = 0;
   std::multimap<rspf_uint32, rspfRefPtr<ProjDbRecord> >::iterator db_iter = m_projDatabase.begin();
   while ((db_iter != m_projDatabase.end()) && !proj)
   {
      ProjDbRecord* db_record = db_iter->second.get();
      split_db_name.clear();
      db_record->name.split(split_db_name, separators, true);
      if (split_spec == split_db_name)
      {
         // We may already have instantiated this projection, in which case just return its copy.
         // Otherwise, create the projection from the EPSG code that corresponds to the name:
         if (db_record->proj.valid())
            proj = (rspfMapProjection*) db_record->proj->dup();
         else
            proj = findProjection(db_record->code);
         return proj;
      }
      db_iter++;
   }
    
   // No hit? Could be that just a datum was identified, in which case we need a simple 
   // Platte Carree:
   const rspfDatum* datum = rspfDatumFactoryRegistry::instance()->create(spec);
   if (datum)
   {
      rspfEquDistCylProjection* proj = new rspfEquDistCylProjection(*(datum->ellipsoid()));
      proj->setDatum(datum);
      proj->setPcsCode(spec_code);
      return proj;
   }

   return 0;
}


//*************************************************************************************************
//! Given a projection name, assigns the group (e.g., "EPSG") and code of the projection. 
//! Returns "<group>:<code>" if match was found or empty string if not found.
//*************************************************************************************************
rspf_uint32 rspfEpsgProjectionDatabase::findProjectionCode(const rspfString& proj_name) const
{
   std::multimap<rspf_uint32, rspfRefPtr<ProjDbRecord> >::iterator db_iter = m_projDatabase.begin();
   while (db_iter != m_projDatabase.end())
   {
      ProjDbRecord* db_record = db_iter->second.get();
      if (db_record->name == proj_name)
         return (db_record->code);
      db_iter++;
   }
      
   return 0;
}

//*************************************************************************************************
//! Given a projection instance, this method determines the corresponding EPSG code. Obviously
//! this is only needed if the projection does not have its PCS code assigned (it is NULL). This
//! happens when the projection was constructed with full parameters instead of the EPSG code.
//! Returns integer EPSG code if match was found or 0 if not found.
//*************************************************************************************************
rspf_uint32 
rspfEpsgProjectionDatabase::findProjectionCode(const rspfMapProjection& lost_proj) const
{
   rspfString lost_type (lost_proj.getClassName());

   // Shortcut for EPSG:4326 (WGS-85 geographic rectangular -- very common):
   if ((lost_type == "rspfEquDistCylProjection") && (lost_proj.getDatum()->epsgCode() == 6326))
      return 4326;

   rspf_uint32 found_code = 0;
   if (lost_type == "rspfUtmProjection")
   {
      found_code = getCodeFromUtmProj(dynamic_cast<const rspfUtmProjection*>(&lost_proj));
      if (found_code)
         return found_code;
   }

   rspfString lookup;
   std::multimap<rspf_uint32, rspfRefPtr<ProjDbRecord> >::iterator db_iter = m_projDatabase.begin();
   while ((db_iter != m_projDatabase.end()) && (found_code == 0))
   {
      ProjDbRecord* db_record = db_iter->second.get();
      
      // Has a projection already been created for this db iter?
      if (!db_record->proj.valid())
      {
         // No projection has been created yet for this DB entry. 
         // NOTE: THIS IS VERY SLOW BECAUSE WE ARE INSTANTIATING EVERY PROJECTION IN THE DB!!!
         db_record->proj = dynamic_cast<rspfMapProjection*>(findProjection(db_record->code));
      }
      if (db_record->proj.valid() && (*(db_record->proj.get()) == lost_proj))
      {
         found_code = db_record->code;

         // Hack to remap projection code 4087 to 4326 (which is not really a projection 
         // code but other packages like to see 4326 for geographic projections.
         // Hacked under protest (OLK, 08/2010)
         if (found_code == 4087)
            found_code = 4326;
      }
      db_iter++;
   }
   return found_code;
}

//*************************************************************************************************
//! This method provides acess to the projection name given the EPSG code. The name provides 
//! enough info for recognizing a non-meters HARN-based projection. 
//*************************************************************************************************
rspfString rspfEpsgProjectionDatabase::findProjectionName(rspf_uint32 epsg_code) const
{
   rspfString name ("");
   std::multimap<rspf_uint32, rspfRefPtr<ProjDbRecord> >::iterator db_iter = 
       m_projDatabase.find(epsg_code);
   
   if (db_iter != m_projDatabase.end())
      name = db_iter->second->name;
   
   return name;
}

//*************************************************************************************************
//! Populates caller's list with all projections currently represented in the database.
//! The format is: 
//!    EPSG:<code>  "<Name as appears in Db>"
//! There may be duplicate entries if the projection is duplicated in the various Db files.
//*************************************************************************************************
void rspfEpsgProjectionDatabase::getProjectionsList(std::vector<rspfString>& list) const
{
   std::multimap<rspf_uint32, rspfRefPtr<ProjDbRecord> >::iterator db_iter = m_projDatabase.begin();
   while (db_iter != m_projDatabase.end())
   {
      ProjDbRecord* db_record = db_iter->second.get();
      rspfString record ("EPSG:");
      record += rspfString::toString(db_record->code);
      record += "  \"";
      record += db_record->name;
      record += "\"";
      list.push_back(record);
      db_iter++;
   }
   return;
}

//*************************************************************************************************
//! This method is invoked to parse the Db record and produce a projection (or NULL if spec invalid)
//
// LIMITATION: Currently not parsing the datum info from the EPSG database file due to disconnect
// with current rspfDatumFactory. Setting to default WGS84 with warning message.
//*************************************************************************************************
rspfMapProjection* 
rspfEpsgProjectionDatabase::createProjFromFormatARecord(ProjDbRecord* record) const
{
   // Establish EPSG code and test for UTM (full projection is implied in the code itself -- no 
   // accessing the database). Until the database is solidified, it is probably better to do 
   // it this way:
   record->datumValid = true;
   record->proj = 0;

   // Establish the units in which the easting/northing is provided:
   double mtrs_per_unit = 1.0;
   if (record->csvRecord[A_UNITS] == "US survey foot")
      mtrs_per_unit = US_METERS_PER_FT;
   else if (record->csvRecord[A_UNITS].contains("foot")) // THIS IS INTERNATIONAL FOOT, NOT EXACT FOR MANY INTERNATIONAL VARIETIES
      mtrs_per_unit = MTRS_PER_FT;
   else if (record->csvRecord[A_UNITS].contains("kilometre"))
      mtrs_per_unit = 1000.0;
   else if (!record->csvRecord[A_UNITS].contains("metre"))
   {
      // ### SKIP THIS MESSAGE BUT BE AWARE THAT THIS PROJECTION WON'T BE REPRESENTED IN DB ###
      //rspfNotify(rspfNotifyLevel_WARN)<<MODULE<<"EPSG:"<<pcs_code<<" units of <"
      //   <<record->csvRecord[A_UNITS]<<"> not presently supported."<<endl;
      return 0;
   }

   // First create a datum given the datum code in the record:
   rspf_uint32 gcs_code = record->csvRecord[A_DATUM_CODE].toUInt32();
   const rspfDatum* datum = rspfEpsgDatumFactory::instance()->create(gcs_code);
   if (!datum)
   {
      // Default to WGS 84 -- this may throw an exception:
      datum = rspfDatumFactory::instance()->create(rspfString("WGE"));
      record->datumValid = false;
   }
   const rspfEllipsoid* ellipsoid = datum->ellipsoid();

   rspfGpt origin(0,0,0,datum);
   rspfString proj_type = record->csvRecord[A_PROJ_TYPE];
   if (proj_type.contains("Transverse Mercator"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LON]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_NORTHING].toDouble();
      double sf = record->csvRecord[A_NAT_ORG_SCALE].toDouble();
      record->proj = new rspfTransMercatorProjection(*ellipsoid, origin, fe, fn, sf);
   }
   else if (proj_type.contains("Lambert Conic Conformal (1SP)"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LON]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_NORTHING].toDouble();
      record->proj = new rspfLambertConformalConicProjection(*ellipsoid, origin, origin.lat, 
         origin.lat, fe, fn);
   }
   else if (proj_type.contains("Lambert Conic Conformal (2SP)"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_FALSE_ORG_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_FALSE_ORG_LON]);
      double p1 = decodeSexagesimalDms(record->csvRecord[A_STD_PARL_1_LAT]);
      double p2 = decodeSexagesimalDms(record->csvRecord[A_STD_PARL_2_LAT]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_ORG_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_ORG_NORTHING].toDouble();
      record->proj = new rspfLambertConformalConicProjection(*ellipsoid, origin, p1, p2, fe, fn);
   }
   else if (proj_type.contains("Cassini"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LON]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_NORTHING].toDouble();
      record->proj =  new rspfCassiniProjection(*ellipsoid, origin, fe, fn);
   }
   else if (proj_type.contains("Mercator (1SP)") || proj_type.contains("Pseudo-Mercator"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LON]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_NORTHING].toDouble();
      double sf = record->csvRecord[A_NAT_ORG_SCALE].toDouble();
      record->proj = new rspfMercatorProjection(*ellipsoid, origin, fe, fn, sf);
   }
   else if (proj_type.contains("Albers"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_FALSE_ORG_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_FALSE_ORG_LON]);
      double p1 = decodeSexagesimalDms(record->csvRecord[A_STD_PARL_1_LAT]);
      double p2 = decodeSexagesimalDms(record->csvRecord[A_STD_PARL_2_LAT]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_ORG_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_ORG_NORTHING].toDouble();
      record->proj = new rspfAlbersProjection(*ellipsoid, origin, p1, p2, fe, fn);
   }
   else if (proj_type.contains("Equidistant Cylindrical"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_STD_PARL_1_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LON]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_NORTHING].toDouble();
      record->proj = new rspfEquDistCylProjection(*ellipsoid, origin, fe, fn);
   }
   else if (proj_type.contains("New Zealand Map Grid"))
   {
      origin.lat = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LAT]);
      origin.lon = decodeSexagesimalDms(record->csvRecord[A_NAT_ORG_LON]);
      double fe = mtrs_per_unit*record->csvRecord[A_FALSE_EASTING].toDouble();
      double fn = mtrs_per_unit*record->csvRecord[A_FALSE_NORTHING].toDouble();
      rspfNewZealandMapGridProjection* nz_proj = new rspfNewZealandMapGridProjection;
      nz_proj->setOrigin(origin);
      nz_proj->setFalseEastingNorthing(fe, fn);
      record->proj = nz_proj;
   }
   else 
   {
      // Can't handle it now. 
      //rspfNotify(rspfNotifyLevel_FATAL)<<MODULE<<"EPSG:"<<record->csvRecord[A_CODE]<<" \""<<proj_type<<"\" "
      //   "not presently supported. Returning NULL projection..."<<endl;
      return 0;
   }
  
   record->proj->setDatum(datum);
   record->proj->setPcsCode(record->code);
   
   return record->proj.get();
}

//*************************************************************************************************
// ### HACK ###
//! State Plane projections as specified in the EPSG are indistinguishable from regular TM. 
//! Unfortunately RSPF does make a distinction. For the time being, parse the code and 
//! programmatically arrive at the UTM projection.
//*************************************************************************************************
rspfMapProjection* 
rspfEpsgProjectionDatabase::createProjFromFormatBRecord(ProjDbRecord* db_record) const
{
   // Unfortunately, as of this writing, there is a disconnect between the Geotrans projection
   // parameters and those provided by EPSG database. In the meantime, to maintain functionality,
   // we intercept the EPSG code and programmatically arrive at the projection versus pulling
   // all the parameters out of the Db record->
   rspfStatePlaneProjectionInfo  info(db_record->name,
                                       db_record->code,
                                       db_record->csvRecord[B_PROJ_TYPE],
                                       db_record->csvRecord[B_PARAM1],
                                       db_record->csvRecord[B_PARAM2],
                                       db_record->csvRecord[B_PARAM3],
                                       db_record->csvRecord[B_PARAM4],
                                       db_record->csvRecord[B_FALSE_EASTING].toDouble(),
                                       db_record->csvRecord[B_FALSE_NORTHING].toDouble(),
                                       db_record->csvRecord[B_UNITS],
                                       db_record->csvRecord[B_DATUM_CODE]);
   
   // NOTE: In order to avoid infinite recursion with this object, we initialized the PCS code in 
   // info to NULL to insure that the projection is instantiated directly (not via this class):
   rspfKeywordlist kwl;
   info.populateProjectionKeywords(kwl);
   kwl.remove(rspfKeywordNames::PCS_CODE_KW);
   db_record->proj = 
      PTR_CAST(rspfMapProjection, rspfMapProjectionFactory::instance()->createProjection(kwl));
   if (db_record->proj.valid())
   {
      db_record->proj->setPcsCode(db_record->csvRecord[B_CODE].toUInt32());
      db_record->datumValid = true;
   }

   return db_record->proj.get();
}

//*************************************************************************************************
// ### HACK ###
//! UTM projections as specified in the EPSG are indistinguishable from regular TM. Unfortunately
//! RSPF does make a distinction. For the time being, parse the code and programmatically arrive 
//! at the UTM projection.
//*************************************************************************************************
rspfMapProjection* rspfEpsgProjectionDatabase::createProjFromUtmCode(rspf_uint32 code) const
{
   // This code originally found in deprecated rspfPcsCodeProjectionFactory authored by
   // David Burken.
   int type = code/100;
   int zone = code%100;
   rspfUtmProjection* proj = NULL;
   switch (type)
   {
   case 322:
      // utm, WGS72 (WGD), northern hemisphere
      if ( (zone > 0 ) && (zone < 61) )
      {
         proj = new rspfUtmProjection(*(rspfDatumFactory::instance()->wgs72()->ellipsoid()));
         proj->setDatum(rspfDatumFactory::instance()->wgs72());
         proj->setZone(zone);
         proj->setHemisphere('N');
      }
      break;

   case 323:
      // utm, WGS72 (WGD), southern hemisphere
      if ( (zone > 0) && (zone < 61) )
      {
         proj = new rspfUtmProjection(*(rspfDatumFactory::instance()->wgs72()->ellipsoid()));
         proj->setDatum(rspfDatumFactory::instance()->wgs72());
         proj->setZone(zone);
         proj->setHemisphere('S');
      }
      break;

   case 326:
      // utm, WGS84 (WGE), northern hemisphere
      if ( (zone > 0) && (zone < 61) )
      {
         proj = new rspfUtmProjection(*(rspfDatumFactory::instance()->wgs84()->ellipsoid()));
         proj->setDatum(rspfDatumFactory::instance()->wgs84());
         proj->setZone(zone);
         proj->setHemisphere('N');
      }
      break;

   case 327:
      // utm, WGS84 (WGE), southern hemisphere
      if ( (zone > 0) && (zone < 61) )
      {
         proj = new rspfUtmProjection(*(rspfDatumFactory::instance()->wgs84()->ellipsoid()));
         proj->setDatum(rspfDatumFactory::instance()->wgs84());
         proj->setZone(zone);
         proj->setHemisphere('S');
      }
      break;

   case 267:
      // utm, "NAS-C", northern hemisphere
      // Only UTM NAD27 North zones 3 to 22 are in the 267xx range...
      // 26729 through 26803 handled by state plane factory.
      //---
      if ( (code > 26702) && (code < 26723) )
      {
         proj = new rspfUtmProjection(*(rspfDatumFactory::instance()->
            create(rspfString("NAS-C"))->ellipsoid()));
         proj->setDatum(rspfDatumFactory::instance()->create(rspfString("NAS-C")));
         proj->setZone(zone);
         proj->setHemisphere('N');
      }
      break;

   case 269:
      // utm, "NAR-C", northern hemisphere
      // Only UTM NAD83 North zones 3 to 23 are in the 269xx range...
      // 26929 through 26998 handled by state plane factory.
      if ( (code > 26902) && (code < 26924) )
      {
         proj = new rspfUtmProjection(*(rspfDatumFactory::instance()->
            create(rspfString("NAR-C"))->ellipsoid()));
         proj->setDatum(rspfDatumFactory::instance()->create(rspfString("NAR-C")));
         proj->setZone(zone);
         proj->setHemisphere('N');
      }
      break;

   case 248:
      // Provisional S. American 1956, 24818 through 24880
      if ( (code > 24817) && (code < 24881) )
      {
         proj = new rspfUtmProjection(*(rspfDatumFactory::instance()->
            create(rspfString("PRP-M"))->ellipsoid()));
         proj->setDatum(rspfDatumFactory::instance()->create(rspfString("PRP-M")));
         if (zone > 60)
         {
            proj->setZone(zone - 60);
            proj->setHemisphere('S');
         }
         else
         {
            proj->setZone(zone);
            proj->setHemisphere('N');
         }
      }
      break;

   } // End of switch on code.

   if (proj)
      proj->setPcsCode(code);

   return proj;
}

//*************************************************************************************************
//! Given UTM projection, derives the associated EPSG code. This is faster than a Db lookup.
//*************************************************************************************************
rspf_uint32 rspfEpsgProjectionDatabase::getCodeFromUtmProj(const rspfUtmProjection* proj) const
{
   if (proj == NULL)
      return 0;

   char hemisphere = proj->getHemisphere();
   rspf_uint32 zone = proj->getZone();
   rspfString datum_code = proj->getDatum()->code();
   
   if ((zone < 1 ) || (zone > 60))
      return 0;

   rspf_uint32 epsg_code = zone;
   if ((hemisphere == 'N') && (datum_code == "WGD"))
      epsg_code += 32200;
   
   else if ((hemisphere == 'S') && (datum_code == "WGD"))
      epsg_code += 32300;

   else if ((hemisphere == 'N') && (datum_code == "WGE"))
      epsg_code += 32600;

   else if ((hemisphere == 'S') && (datum_code == "WGE"))
      epsg_code += 32700;

   else if ((hemisphere == 'N') && (datum_code == "NAS-C") && (zone > 2) && (zone < 23))
      epsg_code += 26700;

   else if ((hemisphere == 'N') && (datum_code == "NAR-C") && (zone > 2) && (zone < 24))
      epsg_code += 26900;

   else if ((hemisphere == 'N') && (datum_code == "PRP-M"))
      epsg_code += 24800;

   else if ((hemisphere == 'S') && (datum_code == "PRP-M"))
      epsg_code += 24800 + 60;
   
   else
      epsg_code = 0;

   return epsg_code;
}

