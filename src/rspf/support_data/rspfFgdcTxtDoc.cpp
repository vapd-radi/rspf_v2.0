//----------------------------------------------------------------------------
//
// File rspfFgdcTxtDoc.cpp
// 
// License:  LGPL
// 
// See LICENSE.txt file in the top level directory for more details.
//
// Author:  David Burken
//
// Description: rspfFgdcTxtDoc class definition.
//
// FGDC = "Federal Geographic Data Committee"
//
// See: http://www.fgdc.gov/
// 
//----------------------------------------------------------------------------

#include <rspf/support_data/rspfFgdcTxtDoc.h>
#include <rspf/base/rspfException.h>
#include <rspf/base/rspfFilename.h>
#include <rspf/base/rspfNotify.h>
#include <rspf/base/rspfString.h>
#include <rspf/projection/rspfMapProjectionFactory.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static const std::string ALTITUDE_DISTANCE_UNITS_KW = "altitude_distance_units";
static const std::string FGDC_FILE_KW               = "fgdc_file";
static const std::string FGDC_VERSION_KW            = "fgdc_version";
static const std::string FGDC_VERSION_001_1998      = "FGDC-STD-001-1998";

rspfFgdcTxtDoc::rspfFgdcTxtDoc()
   : m_kwl( new rspfKeywordlist() )
{
}

rspfFgdcTxtDoc::~rspfFgdcTxtDoc()
{
   m_kwl = 0; // Not a leak, m_kwl is a rspfRefPtr.
}

bool rspfFgdcTxtDoc::open(const rspfFilename& file)
{
   bool result = false;

   // Open the file:
   std::ifstream str( file.c_str(), std::ios_base::in );

   if ( str.good() )
   {
      std::string key = "Metadata_Standard_Version";
      std::string value;
      if ( findKey( str, true, key, value ) )
      {
         if ( value == FGDC_VERSION_001_1998 )
         {
            result = true;
            m_kwl->addPair( FGDC_VERSION_KW, value );
            m_kwl->addPair( FGDC_FILE_KW, file.string() );

            // Store for getAltitudeDistanceUnits() method if key found.
            key = "Altitude_Distance_Units";
            if ( findKey( str, false, key, value ) )
            {
               m_kwl->addPair( ALTITUDE_DISTANCE_UNITS_KW, value );
            }
         }
      }
   }

   return result;
}

void rspfFgdcTxtDoc::close()
{
   m_kwl->clear();
}

void rspfFgdcTxtDoc::getProjection(rspfRefPtr<rspfProjection>& proj)
{
   static const char M[] = "rspfFgdcTxtDoc::getProjection";
   
   try
   {
      // Get the file name:
      std::string file = m_kwl->findKey( FGDC_FILE_KW );
      if ( file.size() )
      {
         // Get the version:
         std::string version = m_kwl->findKey( FGDC_VERSION_KW );
         if ( version.size() )
         {
            // Note: Currently only coded against FGDC-STD-001-1998
            if ( version ==  FGDC_VERSION_001_1998)
            {
               // Open the file:
               std::ifstream str( file.c_str(), std::ios_base::in );
               if ( str.good() )
               {
                  // Find the Spatial_Reference_Information  section:
                  std::string key = "Spatial_Reference_Information:";
                  if ( findKey( str, key ) )
                  {
                     getProjectionV1( str, proj );
                  }
               }
            }
         }
      }
   }
   catch (const rspfException& e)
   {
      rspfNotify(rspfNotifyLevel_WARN) << M << " caught exception:\n" << e.what() << std::endl;
   }
}

void rspfFgdcTxtDoc::getAltitudeDistanceUnits(std::string& units) const
{
   units = m_kwl->findKey( ALTITUDE_DISTANCE_UNITS_KW );
}

void rspfFgdcTxtDoc::getProjectionV1(std::ifstream& str, rspfRefPtr<rspfProjection>& proj)
{
   static const char M[] = "rspfFgdcTxtDoc::getProjectionV1";
   
   std::string fgdcKey;
   std::string fgdcValue;
   std::string key;
   std::string value;
   rspfKeywordlist projectionKwl;
   
   fgdcKey = "Grid_Coordinate_System_Name";
   if ( findKey( str, true, fgdcKey, fgdcValue ) )
   {
      if ( fgdcValue == "Universal Transverse Mercator" )
      {
         key = "type";
         value = "rspfUtmProjection";
         projectionKwl.addPair(key, value);

         // Get the zone:
         fgdcKey = "UTM_Zone_Number";
         if ( findKey( str, true, fgdcKey, fgdcValue ) )
         {
            key = "zone";
            value = fgdcValue;
            projectionKwl.addPair(key, value);
         }
         else
         {
            std::string errMsg = M;
            errMsg += " ERROR: Could not determine utm zone!";
            throw rspfException(errMsg);
         }

         // Check for Southern hemisphere.
         fgdcKey = "False_Northing";
         if ( findKey( str, true, fgdcKey, fgdcValue ) )
         {
            key = "hemisphere";
            
            // Hemisphere( North false easting = 0.0, South = 10000000):
            rspf_float64 f = rspfString::toFloat64( fgdcKey.c_str() );
            if ( f == 0.0 )
            {
               value = "N";
            }
            else
            {
               value = "S";
            }
            projectionKwl.addPair(key, value);
         }
         
      } // UTM section:
      else
      {
         // Exception thrown so that we see that we are not handling a projection.
         std::string errMsg = M;
         errMsg += "ERROR: Unhandled projection: ";
         errMsg += fgdcValue;
         throw rspfException(errMsg);  
      }

      fgdcKey = "Horizontal_Datum_Name";
      if ( findKey( str, true, fgdcKey, fgdcValue ) )
      {
         getOssimDatum( fgdcValue, value );
         key = "datum";
         projectionKwl.addPair(key, value);
      }
      
   } // Matches: findKey( Grid_Coordinate_System_Name )

   if ( projectionKwl.getSize() )
   {
      proj = rspfMapProjectionFactory::instance()->createProjection(projectionKwl);
   }
   else
   {
      proj = 0;
   }
}

bool rspfFgdcTxtDoc::findKey( std::ifstream& str, const std::string& key)
{
   bool result = false;
   rspfString line;
   while ( !str.eof() )
   {
      // Read line:
      std::getline( str, line.string() );
      if ( line.size() )
      {
         // Eat white space.
         line.trim();
         if ( line.string() == key )
         {
            result = true;
            break;
         }
      }
   }
   return result;
}

bool rspfFgdcTxtDoc::findKey(
   std::ifstream& str, bool seekBack, const std::string& key, std::string& value)
{
   bool result = false;

   std::vector<rspfString> fgdcKeyValue;
   rspfString separator = ":";
   rspfString line;
   rspfKeywordlist projectionKwl;
   std::streampos currentPosition = str.tellg();
   
   while ( !str.eof() )
   {
      // Read line:
      std::getline( str, line.string() );
      
      if ( line.size() )
      {
         // Eat white space.
         line.trim();
         
         // Split between ':'
         fgdcKeyValue.clear();
         line.split(fgdcKeyValue, separator, false);
         if ( fgdcKeyValue.size() == 2 )
         {
            fgdcKeyValue[0].trim();

            if ( fgdcKeyValue[0].size() )
            {
               if ( fgdcKeyValue[0] == key )
               {
                  // Found it.  Initialize value and get out.
                  result = true;
                  fgdcKeyValue[1].trim();
                  value = fgdcKeyValue[1].string();
                  break; // from while loop.
               }
            }
         }
      }
   }

   if ( seekBack )
   {
      str.seekg(currentPosition);
   }
   
   return result;
}

// Could be moved to base class for xml and text doc.
void rspfFgdcTxtDoc::getOssimDatum( const std::string& fgdcDatumString,
                                     std::string& rspfDatumCode ) const
{
   rspfString horizdn = fgdcDatumString;
   horizdn.downcase();
   if ( horizdn == "north american datum of 1983" )
   {
      rspfDatumCode = "NAR-C";
   }
   else if ( horizdn == "north american datum of 1927" )
   {
      rspfDatumCode = "NAS-C";
   }
   else if ( horizdn == "wgs84")
   {
      rspfDatumCode = "WGE";
   }
   else
   {
      // Exception thrown so that we see that we are not handling a datum.
      std::string errMsg = "rspfFgdcTxtDoc::getOssimDatum ERROR: Unhandled datum: ";
      errMsg += horizdn.string();
      throw rspfException(errMsg);
   }
}
