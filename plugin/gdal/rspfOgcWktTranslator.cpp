#include <cstdio>
#include <gdal.h>
#include <geovalues.h>
#include "rspfOgcWktTranslator.h"
#include <ogr_spatialref.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfKeyword.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/projection/rspfAlbersProjection.h>
#include <rspf/projection/rspfCylEquAreaProjection.h>
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfEpsgProjectionDatabase.h>
#include <rspf/projection/rspfEpsgProjectionFactory.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfUnitTypeLut.h>
static const double SEMI_MAJOR_AXIS_WGS84 = 6378137.0;
static const double SEMI_MINOR_AXIS_WGS84 = 6356752.3142;
static rspfTrace traceDebug("rspfOgcWktTranslator:debug");
rspfOgcWktTranslator::rspfOgcWktTranslator()
{
   initializeDatumTable();
   initializeProjectionTable();
}
#define EPSG_CODE_MAX 32767
#define USER_DEFINED  32767
rspfString rspfOgcWktTranslator::fromOssimKwl(const rspfKeywordlist &kwl,
                                                const char *prefix)const
{
   rspfString projType = kwl.find(rspfKeywordNames::TYPE_KW);
   rspfString datumType = kwl.find(rspfKeywordNames::DATUM_KW);
   
   rspfString wktString;
   OGRSpatialReference oSRS;
   
   if(projType == "")
   {
      return wktString;
   }
   
   rspfString zone = kwl.find(prefix, rspfKeywordNames::ZONE_KW);
   rspfString hemisphere = kwl.find(prefix, rspfKeywordNames::HEMISPHERE_KW);
   rspfString parallel1  = kwl.find(prefix, rspfKeywordNames::STD_PARALLEL_1_KW);
   rspfString parallel2  = kwl.find(prefix, rspfKeywordNames::STD_PARALLEL_2_KW);
   rspfString originLat  = kwl.find(prefix, rspfKeywordNames::ORIGIN_LATITUDE_KW);
   rspfString centralMeridian = kwl.find(prefix, rspfKeywordNames::CENTRAL_MERIDIAN_KW);
   rspfString scale = kwl.find(prefix, rspfKeywordNames::SCALE_FACTOR_KW);
   rspfString pcsCode = kwl.find(prefix, rspfKeywordNames::PCS_CODE_KW);
   rspfDpt falseEastingNorthing;
   falseEastingNorthing.x = 0.0;
   falseEastingNorthing.y = 0.0;
   const char *lookup =
      kwl.find(prefix, rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW);
   if (lookup)
   {
      rspfUnitType units =
         static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->
                                    getEntryNumber(lookup));
      
      lookup = kwl.find(prefix, rspfKeywordNames::FALSE_EASTING_NORTHING_KW);
      if (lookup)
      {
         rspfDpt eastingNorthing;
         eastingNorthing.toPoint(std::string(lookup));
         
         switch (units)
         {
            case RSPF_METERS:
            {
               falseEastingNorthing = eastingNorthing;
               break;
            }
            case RSPF_FEET:
            case RSPF_US_SURVEY_FEET:
            {
               rspfUnitConversionTool ut;
               ut.setValue(eastingNorthing.x, units);
               falseEastingNorthing.x = ut.getValue(RSPF_METERS);
               ut.setValue(eastingNorthing.y, units);
               falseEastingNorthing.y = ut.getValue(RSPF_METERS);
               break;
            }
            default:
            {
               rspfNotify(rspfNotifyLevel_WARN)
                  << "rspfOgcWktTranslator::fromOssimKwl WARNING!"
                  << "Unhandled unit type for "
                  << rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW
                  << ":  " 
                  << ( rspfUnitTypeLut::instance()->
                       getEntryString(units).c_str() )
                  << endl;
               break;
            }
         } // End of switch (units)
         
      }  // End of if (FALSE_EASTING_NORTHING_KW)
   } // End of if (FALSE_EASTING_NORTHING_UNITS_KW)
   else
   {
      lookup =  kwl.find(prefix, rspfKeywordNames::FALSE_EASTING_KW);
      if(lookup)
      {
         falseEastingNorthing.x = fabs(rspfString(lookup).toFloat64());
      }
      
      lookup =  kwl.find(prefix, rspfKeywordNames::FALSE_NORTHING_KW);
      if(lookup)
      {
         falseEastingNorthing.y = fabs(rspfString(lookup).toFloat64());
      }
   }
   oSRS.SetLinearUnits("Meter", 1.0);
   int pcsCodeVal = (pcsCode.empty() == false) ? pcsCode.toInt() : EPSG_CODE_MAX;
   if(pcsCodeVal < EPSG_CODE_MAX)
   {
      rspfEpsgProjectionDatabase* proj_db = rspfEpsgProjectionDatabase::instance();
      rspfString pcsCodeName = proj_db->findProjectionName(pcsCodeVal);
      if ( pcsCodeName.contains("HARN") && pcsCodeName.contains("_Feet") )
      {
         rspfString feetStr("_Feet");
         rspfString newPcsCodeName( pcsCodeName.before(feetStr).c_str() );
         rspfString epsg_spec = proj_db->findProjectionCode(newPcsCodeName);
         rspf_uint32 new_code = epsg_spec.after(":").toUInt32();
         if (new_code)
          pcsCodeVal = new_code;
      }
      oSRS.importFromEPSG( pcsCodeVal );
   }
   else if(projType == "rspfUtmProjection")
   {
#if 0
      hemisphere = hemisphere.trim().upcase();
      
      if(hemisphere != "")
      {
         oSRS.SetUTM(zone.toLong(), hemisphere != "S");
      }
      else
      {
         oSRS.SetUTM(zone.toLong(), true);
      }
#else
      short gcs = USER_DEFINED;
      if (datumType == "WGE") gcs = GCS_WGS_84;
      else if (datumType == "WGD") gcs = GCS_WGS_72;
      else if (datumType == "NAR-C") gcs = GCS_NAD83;
      else if (datumType == "NAR") gcs = GCS_NAD83;
      else if (datumType == "NAS-C") gcs = GCS_NAD27;
      else if (datumType == "NAS") gcs = GCS_NAD27;
      else if (datumType == "ADI-M") gcs = GCS_Adindan;
      else if (datumType == "ARF-M") gcs = GCS_Arc_1950;
      else if (datumType == "ARS-M") gcs = GCS_Arc_1960;
      else if (datumType == "EUR-7" || datumType == "EUR-M") gcs = GCS_ED50;
      else if ((datumType == "OGB-7") ||
               (datumType == "OGB-M") ||
               (datumType == "OGB-A") ||
               (datumType == "OGB-B") ||
               (datumType == "OGB-C") ||
               (datumType == "OGB-D")) gcs = GCS_OSGB_1936;
      else if  (datumType == "TOY-M")  gcs = GCS_Tokyo;
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)
               << "DATUM = " << datumType << " tag not written " << std::endl
               << "Please let us know so we can add it"          << std::endl;
         }
      }
      int mapZone = zone.toInt();
      hemisphere = hemisphere.trim().upcase();
      bool bDoImportFromEPSG = false;
      switch ( gcs )
      {
         case GCS_WGS_84:
         {
            if (hemisphere == "N") // Northern hemisphere.
            {
               pcsCodeVal = 32600 + mapZone;
            }
            else // Southern hemisphere.
            {
               pcsCodeVal = 32700 + mapZone;
            }
            bDoImportFromEPSG = true;
            break;
         }
         case GCS_WGS_72:
         {
            if (hemisphere == "N") // Northern hemisphere.
            {
               pcsCodeVal = 32200 + mapZone;
            }
            else // Southern hemisphere.
            {
               pcsCodeVal = 32300 + mapZone;
            }
            bDoImportFromEPSG = true;
            break;
         }
         case GCS_NAD27:
         {
            if (hemisphere == "N") // Northern hemisphere.
            {
               pcsCodeVal = 26700 + mapZone;
            }
            else // Southern hemisphere.
            {
               pcsCodeVal = 32000 + mapZone;
            }
            bDoImportFromEPSG = true;
            break;
         }
         case GCS_NAD83:
         {
            if (hemisphere == "N") // Northern hemisphere.
            {
               pcsCodeVal = 26900 + mapZone;
            }
            else // Southern hemisphere.
            {
               pcsCodeVal = 32100 + mapZone;
            }
            bDoImportFromEPSG = true;
            break;
         }
         default:
         {
            if (mapZone > 0) // Northern hemisphere.
            {
               pcsCodeVal = 16000 + mapZone;
            }
            else if (mapZone < 0) // Southern hemisphere.
            {
               hemisphere = "S";
               pcsCodeVal = 16100 + abs(mapZone);
            }
            break;
         }
      } // End of "switch ( gcs )"
      if ( bDoImportFromEPSG == true )
         oSRS.importFromEPSG( pcsCodeVal );
      else
      {
         if(hemisphere != "")
         {
            oSRS.SetUTM(zone.toLong(), hemisphere != "S");
         }
         else
         {
            oSRS.SetUTM(zone.toLong(), true);
         }
      }
#endif
   }
   else if(projType == "rspfLlxyProjection")
   {
      OGRSpatialReference oGeogCS;
      
      oGeogCS.SetEquirectangular(0.0,
                                 0.0,
                                 0.0,
                                 0.0);
      oGeogCS.SetAngularUnits(SRS_UA_DEGREE, atof(SRS_UA_DEGREE_CONV));
      
      oSRS.CopyGeogCSFrom( &oGeogCS );  
   }
   else if(projType == "rspfEquDistCylProjection")
   {
      OGRSpatialReference oGeogCS;
      
      oGeogCS.SetEquirectangular(originLat.toDouble(),
                                 centralMeridian.toDouble(),
                                 falseEastingNorthing.x,
                                 falseEastingNorthing.y);
      oGeogCS.SetAngularUnits(SRS_UA_DEGREE, atof(SRS_UA_DEGREE_CONV));
      
      oSRS.CopyGeogCSFrom( &oGeogCS );  
   }
   else if(projType == "rspfSinusoidalProjection")
   {
      oSRS.SetSinusoidal(centralMeridian.toDouble(),
                         falseEastingNorthing.x,
                         falseEastingNorthing.y);
   }
   else if(projType == "rspfCylEquAreaProjection")
   {
      oSRS.SetCEA(originLat.toDouble(),
                  centralMeridian.toDouble(),
                  falseEastingNorthing.x,
                  falseEastingNorthing.y);
   }
   else if(projType == "rspfCassiniProjection")
   {
      oSRS.SetCS(originLat.toDouble(),
                 centralMeridian.toDouble(),
                 falseEastingNorthing.x,
                 falseEastingNorthing.y);
   }
   else if(projType == "rspfAlbersProjection")
   {
      oSRS.SetACEA(parallel1.toDouble(),
                   parallel2.toDouble(),
                   originLat.toDouble(),
                   centralMeridian.toDouble(),
                   falseEastingNorthing.x,
                   falseEastingNorthing.y);
   }
   else if(projType == "rspfAzimEquDistProjection")
   {
      oSRS.SetAE(originLat.toDouble(),
                 centralMeridian.toDouble(),
                 falseEastingNorthing.x,
                 falseEastingNorthing.y);
   }
   else if(projType == "rspfEckert4Projection")
   {
      oSRS.SetEckertIV(centralMeridian.toDouble(),
                       falseEastingNorthing.x,
                       falseEastingNorthing.y);
   }
   else if(projType == "rspfEckert6Projection")
   {
      oSRS.SetEckertVI(centralMeridian.toDouble(),
                       falseEastingNorthing.x,
                       falseEastingNorthing.y);
   }
   else if(projType == "rspfGnomonicProjection")
   {
      oSRS.SetGnomonic(originLat.toDouble(),
                       centralMeridian.toDouble(),
                       falseEastingNorthing.x,
                       falseEastingNorthing.y);
   }
   else if(projType == "rspfLambertConformalConicProjection")
   {
      oSRS.SetLCC(parallel1.toDouble(),
                  parallel2.toDouble(),
                  originLat.toDouble(),
                  centralMeridian.toDouble(),
                  falseEastingNorthing.x,
                  falseEastingNorthing.y);
   }
   else if(projType == "rspfVanDerGrintenProjection")
   {
      oSRS.SetVDG(centralMeridian.toDouble(),
                  falseEastingNorthing.x,
                  falseEastingNorthing.y);
   }
   else if(projType == "rspfMillerProjection")
   {
      oSRS.SetMC(originLat.toDouble(),
                 centralMeridian.toDouble(),
                 falseEastingNorthing.x,
                 falseEastingNorthing.y);
   }
   else if(projType == "rspfMercatorProjection")
   {
      oSRS.SetMercator(originLat.toDouble(),
                       centralMeridian.toDouble(),
                       scale.toDouble(),
                       falseEastingNorthing.x,
                       falseEastingNorthing.y);
   }
   else if(projType == "rspfMollweidProjection")
   {
      oSRS.SetMollweide(centralMeridian.toDouble(),
                        falseEastingNorthing.x,
                        falseEastingNorthing.y);
   }
   else if(projType == "rspfNewZealandMapGridProjection")
   {
      oSRS.SetNZMG(originLat.toDouble(),
                   centralMeridian.toDouble(),
                   falseEastingNorthing.x,
                   falseEastingNorthing.y);
   }
   else if(projType == "rspfOrthoGraphicProjection")
   {
      oSRS.SetOrthographic(originLat.toDouble(),
                           centralMeridian.toDouble(),
                           falseEastingNorthing.x,
                           falseEastingNorthing.y);
   }
   else if(projType == "rspfPolarStereoProjection")
   {
      oSRS.SetPS(originLat.toDouble(),
                 centralMeridian.toDouble(),
                 scale.toDouble(),
                 falseEastingNorthing.x,
                 falseEastingNorthing.y);
   }
   else if(projType == "rspfPolyconicProjectio")
   {
      oSRS.SetPolyconic(originLat.toDouble(),
                        centralMeridian.toDouble(),
                        falseEastingNorthing.x,
                        falseEastingNorthing.y);
   }
   else if(projType == "rspfStereographicProjection")
   {
      oSRS.SetStereographic(originLat.toDouble(),
                            centralMeridian.toDouble(),
                            scale.toDouble(),
                            falseEastingNorthing.x,
                            falseEastingNorthing.y);
   }
   else if(projType == "rspfTransMercatorProjection")
   {
      oSRS.SetTM(originLat.toDouble(),
                 centralMeridian.toDouble(),
                 scale.toDouble(),
                 falseEastingNorthing.x,
                 falseEastingNorthing.y);
   }
   else 
   {
      cerr << "rspfOgcWktTranslator::fromOssimKwl:\n"
           << "Projection translation for "
           << projType
           << " not supported "
           << endl;
   }
   
   if(pcsCodeVal >= EPSG_CODE_MAX)
   {
      datumType =  datumType.upcase();
      
      if(datumType == "WGE")
      {
         oSRS.SetWellKnownGeogCS("WGS84");
      }
      else if(datumType == "WGD")
      {
         oSRS.SetWellKnownGeogCS("WGS72");      
      }
      else if(datumType == "NAS-C") //1927
      {
         oSRS.SetWellKnownGeogCS("NAD27");      
      }
      else if(datumType == "NAS") //1927
      {
         oSRS.SetWellKnownGeogCS("NAD27");      
      }
      else if(datumType == "NAR-C") // 1983
      {
         oSRS.SetWellKnownGeogCS("NAD83");
      }
      else if(datumType == "NAR") // 1983
      {
         oSRS.SetWellKnownGeogCS("NAD83");
      }
      else if(datumType == "NTF")
      {
        oSRS.SetWellKnownGeogCS("EPSG:4275");
      }
      else
      {
         cerr << "rspfOgcWktTranslator::fromOssimKwl: Datum translation for "
              << datumType
              <<" not supported"
              << endl;
      }
   }
   char* exportString = NULL;
   oSRS.exportToWkt(&exportString);
   
   if(exportString)
   {
      wktString = exportString;
      OGRFree(exportString);
   }
   return wktString;
   
}
bool rspfOgcWktTranslator::toOssimKwl( const rspfString& wktString,
                                        rspfKeywordlist &kwl,
                                        const char *prefix)const
{
   static const char MODULE[] = "rspfOgcWktTranslator::toOssimKwl";
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << MODULE << " entered...\n";
   }
   
   const char* wkt = wktString.c_str();
   
   OGRSpatialReferenceH  hSRS = NULL;
   rspfDpt falseEastingNorthing;
   
   hSRS = OSRNewSpatialReference(NULL);
   if( OSRImportFromWkt( hSRS, (char **) &wkt ) != OGRERR_NONE )
   {
      OSRDestroySpatialReference( hSRS );
      return false;
   }
   
   rspfString rspfProj = "";
   const char* epsg_code = OSRGetAttrValue( hSRS, "AUTHORITY", 1 );
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "epsg_code: " << (epsg_code?epsg_code:"null") << "\n";
   }
   
   const char* units = NULL;
   OGR_SRSNode* node = ((OGRSpatialReference *)hSRS)->GetRoot();
   int nbChild  = node->GetChildCount();
   for (int i = 0; i < nbChild; i++)
   {
      OGR_SRSNode* curChild = node->GetChild(i);
      if (strcmp(curChild->GetValue(), "UNIT") == 0)
      {
         units = curChild->GetChild(0)->GetValue();
      }
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "units: " << (units?units:"null") << "\n";
   }
   
   rspfString rspf_units;
   bool bGeog = OSRIsGeographic(hSRS);
   if ( bGeog == false )
   {
      rspf_units = "meters";
      if ( units != NULL )
      {
         rspfString s = units;
         s.downcase();
         
         if( ( s == rspfString("us survey foot") ) ||
             ( s == rspfString("u.s. foot") ) ||
             ( s == rspfString("foot_us") ) )
         {
            rspf_units = "us_survey_feet";
         }
         else if( s == rspfString("degree") )
         {
            rspf_units = "degrees";
         }
         else if( ( s == rspfString("meter") ) ||
                  ( s == rspfString("metre") ) )
         {
            rspf_units = "meters";
         }
      }
   }
   else
   {
      rspf_units = "degrees";
   }
   if(traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << "rspf_units: " << rspf_units << "\n";
   }
   if (epsg_code)
   {
      rspfString epsg_spec ("EPSG:"); 
      epsg_spec += rspfString::toString(epsg_code);
      rspfProjection* proj = rspfEpsgProjectionFactory::instance()->createProjection(epsg_spec);
      if (proj)
         rspfProj = proj->getClassName();
      delete proj;
   }
   if(rspfProj == "") 
   {
      const char* pszProjection = OSRGetAttrValue( hSRS, "PROJECTION", 0 );
      if(pszProjection)
      {
         rspfProj = wktToOssimProjection(pszProjection);
      }
      else
      {
         rspfString localCs = OSRGetAttrValue( hSRS, "LOCAL_CS", 0 );
         localCs = localCs.upcase();
         if(localCs == "GREATBRITAIN_GRID")
         {
            rspfProj = "rspfBngProjection";
         }
         else if (rspf_units.contains("degree"))
         {
            rspfProj = "rspfEquDistCylProjection";
         }
      }
   }
   if(rspfProj == "rspfEquDistCylProjection" )
      rspf_units = "degrees";
   kwl.add(prefix, rspfKeywordNames::UNITS_KW, rspf_units, true);
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << "DEBUG:"
         << "\nrspfProj = " << rspfProj << endl;
   }
   
   kwl.add(prefix, rspfKeywordNames::TYPE_KW, rspfProj.c_str(), true);
   falseEastingNorthing.x = OSRGetProjParm(hSRS, SRS_PP_FALSE_EASTING, 0.0, NULL);
   falseEastingNorthing.y = OSRGetProjParm(hSRS, SRS_PP_FALSE_NORTHING, 0.0, NULL);
   if (epsg_code)
   {
      kwl.add(prefix, rspfKeywordNames::PCS_CODE_KW, epsg_code, true);
   }
   if(rspfProj == "rspfBngProjection")
   {
      kwl.add(prefix,
              rspfKeywordNames::TYPE_KW,
              "rspfBngProjection",
              true);
   }
   else if(rspfProj == "rspfCylEquAreaProjection")
   {
      kwl.add(prefix,
              rspfKeywordNames::STD_PARALLEL_1_KW,
              OSRGetProjParm(hSRS, SRS_PP_STANDARD_PARALLEL_1, 0.0, NULL),
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::ORIGIN_LATITUDE_KW,
              OSRGetProjParm(hSRS, SRS_PP_STANDARD_PARALLEL_1, 0.0, NULL),
              true);
      rspfUnitType units =
         static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->
                                    getEntryNumber(rspf_units.c_str()));
      if ( units == RSPF_METERS || 
           units == RSPF_FEET   || 
           units == RSPF_US_SURVEY_FEET )
      {
         kwl.add(prefix,
                 rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
                 falseEastingNorthing.toString(),
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
                 rspf_units,
                 true);
      }
   }
   else if(rspfProj == "rspfEquDistCylProjection")
   {
      kwl.add(prefix,
              rspfKeywordNames::TYPE_KW,
              "rspfEquDistCylProjection",
              true);
      
      rspfUnitType units =
         static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->
                                    getEntryNumber(rspf_units.c_str()));
      if ( units == RSPF_METERS || 
           units == RSPF_FEET   || 
           units == RSPF_US_SURVEY_FEET )
      {
         kwl.add(prefix,
                 rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
                 falseEastingNorthing.toString(),
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
                 rspf_units,
                 true);
      }
      kwl.add(prefix,
              rspfKeywordNames::ORIGIN_LATITUDE_KW,
              OSRGetProjParm(hSRS, SRS_PP_LATITUDE_OF_ORIGIN, 0.0, NULL),
              true);
      kwl.add(prefix,
              rspfKeywordNames::CENTRAL_MERIDIAN_KW,
              OSRGetProjParm(hSRS, SRS_PP_CENTRAL_MERIDIAN, 0.0, NULL),
              true);
   }
   else if( (rspfProj == "rspfLambertConformalConicProjection") ||
            (rspfProj == "rspfAlbersProjection") )
   {
      kwl.add(prefix,
              rspfKeywordNames::TYPE_KW,
              rspfProj.c_str(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
              falseEastingNorthing.toString(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
              rspf_units,
              true);
      kwl.add(prefix,
              rspfKeywordNames::ORIGIN_LATITUDE_KW,
              OSRGetProjParm(hSRS, SRS_PP_LATITUDE_OF_ORIGIN, 0.0, NULL),
              true);
      kwl.add(prefix,
              rspfKeywordNames::CENTRAL_MERIDIAN_KW,
              OSRGetProjParm(hSRS, SRS_PP_CENTRAL_MERIDIAN, 0.0, NULL),
              true);
      kwl.add(prefix,
              rspfKeywordNames::STD_PARALLEL_1_KW,
              OSRGetProjParm(hSRS, SRS_PP_STANDARD_PARALLEL_1, 0.0, NULL),
              true);
      kwl.add(prefix,
              rspfKeywordNames::STD_PARALLEL_2_KW,
              OSRGetProjParm(hSRS, SRS_PP_STANDARD_PARALLEL_2, 0.0, NULL),
              true);
   }
   else if(rspfProj == "rspfMercatorProjection")
   {
      kwl.add(prefix,
              rspfKeywordNames::TYPE_KW,
              "rspfMercatorProjection",
              true);
      kwl.add(prefix,
              rspfKeywordNames::ORIGIN_LATITUDE_KW,
              OSRGetProjParm(hSRS, SRS_PP_LATITUDE_OF_ORIGIN, 0.0, NULL),
              true);
      kwl.add(prefix,
              rspfKeywordNames::CENTRAL_MERIDIAN_KW,
              OSRGetProjParm(hSRS, SRS_PP_CENTRAL_MERIDIAN, 0.0, NULL),
              true);
      kwl.add(prefix,
              rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
              falseEastingNorthing.toString(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
              rspf_units,
              true);
   }
   else if(rspfProj == "rspfSinusoidalProjection")
   {
      kwl.add(prefix,
              rspfKeywordNames::TYPE_KW,
              "rspfSinusoidalProjection",
              true);
      kwl.add(prefix,
              rspfKeywordNames::CENTRAL_MERIDIAN_KW,
              OSRGetProjParm(hSRS, SRS_PP_CENTRAL_MERIDIAN, 0.0, NULL),
              true);
      kwl.add(prefix,
              rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
              falseEastingNorthing.toString(),
              true);
      kwl.add(prefix,
              rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
              rspf_units,
              true);
   }
   else if(rspfProj == "rspfTransMercatorProjection")
   {
      int bNorth;
      int nZone = OSRGetUTMZone( hSRS, &bNorth );
      if( nZone != 0 )
      {
         kwl.add(prefix,
                 rspfKeywordNames::TYPE_KW,
                 "rspfUtmProjection",
                 true);
         
         kwl.add(prefix,
                 rspfKeywordNames::ZONE_KW,
                 nZone,
                 true);
         if( bNorth )
         {
            kwl.add(prefix, rspfKeywordNames::HEMISPHERE_KW, "N", true);
         }
         else
         {
            kwl.add(prefix, rspfKeywordNames::HEMISPHERE_KW, "S", true);
         }
      }            
      else
      {
         kwl.add(prefix,
                 rspfKeywordNames::TYPE_KW,
                 "rspfTransMercatorProjection",
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::SCALE_FACTOR_KW,
                 OSRGetProjParm(hSRS, SRS_PP_SCALE_FACTOR, 1.0, NULL),
                 true);
         
         kwl.add(prefix,
                 rspfKeywordNames::ORIGIN_LATITUDE_KW,
                 OSRGetProjParm(hSRS, SRS_PP_LATITUDE_OF_ORIGIN, 0.0, NULL),
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::CENTRAL_MERIDIAN_KW,
                 OSRGetProjParm(hSRS, SRS_PP_CENTRAL_MERIDIAN, 0.0, NULL),
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::FALSE_EASTING_NORTHING_KW,
                 falseEastingNorthing.toString(),
                 true);
         kwl.add(prefix,
                 rspfKeywordNames::FALSE_EASTING_NORTHING_UNITS_KW,
                 rspf_units,
                 true);
      }
   }
   else
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG)
            << "rspfOgcWktTranslator::toOssimKwl DEBUG:\n"
            << "Projection conversion to RSPF not supported !!!!!!!!!\n"
            << "Please send the following string to the development staff\n" 
            << "to be added to the transaltion to RSPF\n"
            << wkt << endl;
      }
      return false;
   }
   const char *datum = OSRGetAttrValue( hSRS, "DATUM", 0 );
   rspfString oDatum = "WGE";
    
   if( datum )
   {
      oDatum = wktToOssimDatum(datum);
      if(oDatum == "")
      {
         oDatum = "WGE";
      }
   }
       
   kwl.add(prefix, rspfKeywordNames::DATUM_KW, oDatum, true);
     
   OSRDestroySpatialReference( hSRS );
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG)
         << MODULE << " exit status = true"
         << std::endl;
   }    
      
   return true;
}
void rspfOgcWktTranslator::initializeDatumTable()
{
  
  
   theOssimToWktDatumTranslation.insert(make_pair(std::string("NAS-C"),
                                                  std::string("North_American_Datum_1927")));
   theOssimToWktDatumTranslation.insert(make_pair(std::string("NAR-C"),
                                                  std::string("North_American_Datum_1983")));
   theOssimToWktDatumTranslation.insert(make_pair(std::string("NAS"),
                                                  std::string("North_American_Datum_1927")));
   theOssimToWktDatumTranslation.insert(make_pair(std::string("NAR"),
                                                  std::string("North_American_Datum_1983")));
   theOssimToWktDatumTranslation.insert(make_pair(std::string("WGE"),
                                                  std::string("WGS_1984")));
   theWktToOssimDatumTranslation.insert(make_pair(std::string("OSGB_1936"),
                                                  std::string("OGB-B"))); 
   theWktToOssimDatumTranslation.insert(make_pair(std::string("Nouvelle_Triangulation_Francaise"),
                                                  std::string("NTF")));
}
void rspfOgcWktTranslator::initializeProjectionTable()
{
   theWktToOssimProjectionTranslation.insert(make_pair(std::string(SRS_PT_TRANSVERSE_MERCATOR),
                                                       std::string("rspfTransMercatorProjection")));
   theWktToOssimProjectionTranslation.insert(make_pair(std::string(SRS_PT_EQUIRECTANGULAR),
                                                       std::string("rspfEquDistCylProjection")));
   theWktToOssimProjectionTranslation.insert(make_pair(std::string(SRS_PT_ALBERS_CONIC_EQUAL_AREA),
                                                       std::string("rspfAlbersProjection")));
   theWktToOssimProjectionTranslation.insert(make_pair(std::string(SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP),
                                                       std::string("rspfLambertConformalConicProjection")));
   theWktToOssimProjectionTranslation.insert(make_pair(std::string(SRS_PT_SINUSOIDAL),
                                                       std::string("rspfSinusoidalProjection")));
   theWktToOssimProjectionTranslation.insert(make_pair(std::string(SRS_PT_MERCATOR_1SP),
                                                       std::string("rspfMercatorProjection")));
   theOssimToWktProjectionTranslation.insert(make_pair(std::string("rspfTransMercatorProjection"),
                                                       std::string(SRS_PT_TRANSVERSE_MERCATOR)));
   theOssimToWktProjectionTranslation.insert(make_pair(std::string("rspfEquDistCylProjection"),
                                                       std::string(SRS_PT_EQUIRECTANGULAR)));
   theOssimToWktProjectionTranslation.insert(make_pair(std::string("rspfAlbersProjection"),
                                                       std::string(SRS_PT_ALBERS_CONIC_EQUAL_AREA)));
   theOssimToWktProjectionTranslation.insert(make_pair(std::string("rspfLambertConformalConicProjection"),
                                                       std::string(SRS_PT_LAMBERT_CONFORMAL_CONIC_2SP)));
   theOssimToWktProjectionTranslation.insert(make_pair(std::string("rspfSinusoidalProjection"),
                                                       std::string(SRS_PT_SINUSOIDAL)));
   theOssimToWktProjectionTranslation.insert(make_pair(std::string("rspfMercatorProjection"),
                                                       std::string(SRS_PT_MERCATOR_1SP)));
   
}
rspfString rspfOgcWktTranslator::wktToOssimDatum(const rspfString& datum)const
{
   if(datum.contains("North_American_Datum_1927"))
   {
      return "NAS-C";
   }
   if(datum.contains("North_American_Datum_1983"))
   {
      return "NAR-C";
   }
   if(datum.contains("WGS_1984"))
   {
      return "WGE";
   }
   if(datum.contains("OSGA"))
   {
      return "OGB-A";
   }
   if(datum.contains("OSGB"))
   {
      return "OGB-B";
   }
   if(datum.contains("OSGC"))
   {
      return "OGB-C";
   }
   if(datum.contains("OSGD"))
   {
      return "OGB-D";
   }
   if(datum.contains("Nouvelle_Triangulation_Francaise"))
   {
      return "NTF";
   }
   
   return "";
}
rspfString rspfOgcWktTranslator::rspfToWktDatum(const rspfString& datum)const
{
   rspfString result;
   map<std::string, std::string>::const_iterator i = theOssimToWktDatumTranslation.find(datum);
   if(i != theOssimToWktDatumTranslation.end())
   {
      result = (*i).second;
   }
   return result;
}
rspfString rspfOgcWktTranslator::wktToOssimProjection(const rspfString& datum)const
{
   std::string result;
   map<std::string, std::string>::const_iterator i =
      theWktToOssimProjectionTranslation.find(datum);
   if(i != theWktToOssimProjectionTranslation.end())
   {
      result = (*i).second;
   }
   return result;
}
rspfString rspfOgcWktTranslator::rspfToWktProjection(const rspfString& datum)const
{
   rspfString result;
   map<std::string, std::string>::const_iterator i =
      theOssimToWktProjectionTranslation.find(datum);
   if(i != theOssimToWktProjectionTranslation.end())
   {
      result = (*i).second;
   }
   return result;
}
