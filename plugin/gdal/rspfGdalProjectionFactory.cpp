#include <sstream>
#include "rspfGdalProjectionFactory.h"
#include "rspfGdalTileSource.h"
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/projection/rspfProjectionFactoryRegistry.h>
#include <rspf/projection/rspfUtmProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfEquDistCylProjection.h>
#include <rspf/projection/rspfTransMercatorProjection.h>
#include <rspf/projection/rspfOrthoGraphicProjection.h>
#include "rspfOgcWktTranslator.h"
#include <rspf/base/rspfTrace.h>
#include "rspfOgcWktTranslator.h"
#include <rspf/projection/rspfPolynomProjection.h>
#include <rspf/projection/rspfBilinearProjection.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <rspf/base/rspfTieGptSet.h>
#include <rspf/base/rspfUnitConversionTool.h>
#include <rspf/base/rspfUnitTypeLut.h>
static rspfTrace traceDebug("rspfGdalProjectionFactory:debug");
rspfGdalProjectionFactory* rspfGdalProjectionFactory::theInstance = 0;
static rspfOgcWktTranslator wktTranslator;
rspfGdalProjectionFactory* rspfGdalProjectionFactory::instance()
{
   if(!theInstance)
   {
      theInstance = new rspfGdalProjectionFactory;
   }
   return (rspfGdalProjectionFactory*) theInstance;
}
rspfProjection* rspfGdalProjectionFactory::createProjection(const rspfFilename& filename,
                                                             rspf_uint32 entryIdx)const
{
   rspfKeywordlist kwl;
   if(rspfString(filename).trim().empty()) return 0;
   GDALDatasetH  h = GDALOpen(filename.c_str(), GA_ReadOnly);
   GDALDriverH   driverH = 0;
   rspfProjection* proj = 0;
   if(h)
   {
      driverH = GDALGetDatasetDriver( h );
      rspfString driverName( driverH ? GDALGetDriverShortName( driverH ) : "" );
      if(driverName == "NITF")
      {
         GDALClose(h);
         return 0;
      }
      if(entryIdx != 0)
      {
         char** papszMetadata = GDALGetMetadata( h, "SUBDATASETS" );
         if( papszMetadata&&(CSLCount(papszMetadata) < static_cast<rspf_int32>(entryIdx)) )
         {
            rspfNotify(rspfNotifyLevel_WARN) << "rspfGdalProjectionFactory::createProjection: We don't support multi entry handlers through the factory yet, only through the handler!";
            GDALClose(h);
            return 0;
         }
         else
         {
            GDALClose(h);
            return 0;
         }
      }
      
      rspfString wkt(GDALGetProjectionRef( h ));
      double geoTransform[6];
      bool transOk = GDALGetGeoTransform( h, geoTransform ) == CE_None;
      bool wktTranslatorOk = wkt.empty()?false:wktTranslator.toOssimKwl(wkt, kwl);
      if(!wktTranslatorOk)
      {
         rspf_uint32 gcpCount = GDALGetGCPCount(h);
         if(gcpCount > 3)
         {
            rspf_uint32 idx = 0;
            const GDAL_GCP* gcpList = GDALGetGCPs(h);
            rspfTieGptSet tieSet;
            if(gcpList)
            {
               for(idx = 0; idx < gcpCount; ++idx)
               {
                  rspfDpt dpt(gcpList[idx].dfGCPPixel,
                               gcpList[idx].dfGCPLine);
                  rspfGpt gpt(gcpList[idx].dfGCPY,
                               gcpList[idx].dfGCPX,
                               gcpList[idx].dfGCPZ);
                  tieSet.addTiePoint(new rspfTieGpt(gpt, dpt, .5));
               }
               
               rspfBilinearProjection* tempProj = new rspfBilinearProjection;
			   //tempProj->setupOptimizer("1 x y x2 xy y2 x3 y3 xy2 x2y z xz yz");
               tempProj->optimizeFit(tieSet);
               proj = tempProj;
            }
         }
      }
      if ( transOk && proj==0 )
      {
         rspfString proj_type(kwl.find(rspfKeywordNames::TYPE_KW));
         rspfString datum_type(kwl.find(rspfKeywordNames::DATUM_KW));
         rspfString units(kwl.find(rspfKeywordNames::UNITS_KW));
         if ( proj_type.trim().empty() &&
              (driverName == "MrSID" || driverName == "JP2MrSID") )
         {
            bool bClose = true;
            if( geoTransform[2] == 0.0 && geoTransform[4] == 0.0 )
            {
               rspfString projTag( GDALGetMetadataItem( h, "IMG__PROJECTION_NAME", "" ) );
               if ( projTag.contains("Geographic") )
               {
                  bClose = false;
                  
                  kwl.add(rspfKeywordNames::TYPE_KW,
                          "rspfEquDistCylProjection", true);
                  proj_type = kwl.find( rspfKeywordNames::TYPE_KW );
                  
                  rspfString unitTag( GDALGetMetadataItem( h, "IMG__HORIZONTAL_UNITS", "" ) );
                  if ( unitTag.contains("dd") ) // decimal degrees
                  {
                     units = "degrees";
                  }
                  else if ( unitTag.contains("dm") ) // decimal minutes
                  {
                     units = "minutes";
                  }
                  else if ( unitTag.contains("ds") ) // decimal seconds
                  {
                     units = "seconds";
                  }
               }
            }
            
            if ( bClose == true )
            {
               GDALClose(h);
               return 0;
            }
         }
         if(driverName == "MrSID" || driverName == "JP2MrSID")
         {
            const char* rasterTypeStr = GDALGetMetadataItem( h, "GEOTIFF_CHAR__GTRasterTypeGeoKey", "" );
            rspfString rasterTypeTag( rasterTypeStr );
            if ( rasterTypeTag.contains("RasterPixelIsArea") )
            {
               geoTransform[0] += fabs(geoTransform[1]) / 2.0;
               geoTransform[3] -= fabs(geoTransform[5]) / 2.0;
            }
         }
         kwl.remove(rspfKeywordNames::UNITS_KW);
         rspfDpt gsd(fabs(geoTransform[1]),
                      fabs(geoTransform[5]));
         rspfDpt tie(geoTransform[0], geoTransform[3]);
         rspfUnitType savedUnitType = static_cast<rspfUnitType>(rspfUnitTypeLut::instance()->getEntryNumber(units)); 
         rspfUnitType unitType = savedUnitType;
         if(unitType == RSPF_UNIT_UNKNOWN)
         {
            unitType = RSPF_METERS;
         }
         if(driverName == "AIG")
         {
            tie.x += fabs(gsd.x) / 2.0;
            tie.y -= fabs(gsd.y) / 2.0;
         }
         
         if(proj_type == "rspfLlxyProjection" ||
            proj_type == "rspfEquDistCylProjection")
         {
            int nPixelsLon = GDALGetRasterXSize(h)/2.0;
            int nPixelsLat = GDALGetRasterYSize(h)/2.0;
            double shiftLon =  nPixelsLon * fabs(gsd.x);
            double shiftLat = -nPixelsLat * fabs(gsd.y);
            double centerLon = tie.x + shiftLon;
            double centerLat = tie.y + shiftLat;
            kwl.add(rspfKeywordNames::ORIGIN_LATITUDE_KW,
                    centerLat,
                    true);
            kwl.add(rspfKeywordNames::CENTRAL_MERIDIAN_KW,
                    centerLon,
                    true);
            kwl.add(rspfKeywordNames::TIE_POINT_LAT_KW,
                    tie.y, 
                    true);
            kwl.add(rspfKeywordNames::TIE_POINT_LON_KW,
                    tie.x, 
                    true);
            kwl.add(rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
                    gsd.y, 
                    true);
            kwl.add(rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
                    gsd.x, 
                    true);
            if(savedUnitType == RSPF_UNIT_UNKNOWN)
            {
               unitType = RSPF_DEGREES;
            }
         }
         kwl.add(rspfKeywordNames::PIXEL_SCALE_XY_KW,
                 gsd.toString(),
                 true);
         kwl.add(rspfKeywordNames::PIXEL_SCALE_UNITS_KW,
                 units,
                 true);
         kwl.add(rspfKeywordNames::TIE_POINT_XY_KW,
                 tie.toString(),
                 true);
         kwl.add(rspfKeywordNames::TIE_POINT_UNITS_KW,
                 units,
                 true);
         
         std::stringstream mString;
         mString << rspfString::toString(geoTransform[1], 20)
                 << " " << rspfString::toString(geoTransform[2], 20)
                 << " " << 0 << " "
                 << rspfString::toString(geoTransform[0], 20)
                 << " " << rspfString::toString(geoTransform[4], 20)
                 << " " << rspfString::toString(geoTransform[5], 20)
                 << " " << 0 << " "
                 << rspfString::toString(geoTransform[3], 20)
                 << " " << 0 << " " << 0 << " " << 1 << " " << 0
                 << " " << 0 << " " << 0 << " " << 0 << " " << 1;
         
         kwl.add(rspfKeywordNames::IMAGE_MODEL_TRANSFORM_MATRIX_KW, mString.str().c_str(), true);
         if(driverName == "AIG" && datum_type == "OSGB_1936")
         {
            rspfFilename prj_file = filename.path() + "/prj.adf";
            
            if(prj_file.exists())
            {
               rspfKeywordlist prj_kwl(' ');
               prj_kwl.addFile(prj_file);
               
               rspfString proj = prj_kwl.find("Projection");
               
               if(proj.upcase().contains("GREATBRITAIN"))
               {
                  
                  kwl.add(rspfKeywordNames::TYPE_KW,
                          "rspfBngProjection", true);
                  
                  rspfString datum  = prj_kwl.find("Datum");
                  
                  if(datum != "")
                  {
                     if(datum == "OGB_A")
                        datum = "OGB-A";
                     else if(datum == "OGB_B")
                        datum = "OGB-B";
                     else if(datum == "OGB_C")
                        datum = "OGB-C";
                     else if(datum == "OGB_D")
                        datum = "OGB-D";
                     else if(datum == "OGB_M")
                        datum = "OGB-M";
                     else if(datum == "OGB_7")
                        datum = "OGB-7";
                     
                     kwl.add(rspfKeywordNames::DATUM_KW, 
                             datum, true);
                  }
               }
            }
         }
     }
	 if(traceDebug())
	 {
		 rspfNotify(rspfNotifyLevel_DEBUG) << "rspfGdalProjectionFactory: createProjection KWL = \n " << kwl << std::endl;
	 }
      GDALClose(h);
      proj = rspfProjectionFactoryRegistry::instance()->createProjection(kwl);
  }
   return proj;
}
rspfProjection* rspfGdalProjectionFactory::createProjection(const rspfKeywordlist &keywordList,
                                                             const char *prefix) const
{
   const char *lookup = keywordList.find(prefix, rspfKeywordNames::TYPE_KW);
   if(lookup&&(!rspfString(lookup).empty()))
   {
      rspfProjection* proj = createProjection(rspfString(lookup));
      if(proj)
      {
         rspfKeywordlist tempKwl;
         rspfKeywordlist tempKwl2;
         proj->saveState(tempKwl);
         tempKwl2.add(keywordList, prefix, true);
         tempKwl.add(prefix, tempKwl2,  true);
         tempKwl.add(prefix, rspfKeywordNames::TYPE_KW, proj->getClassName(), true);
	 proj->loadState(tempKwl); 	
         if(traceDebug())
         {
            tempKwl.clear();
            proj->saveState(tempKwl);
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG)<< "rspfGdalProjectionFactory::createProjection Debug: resulting projection \n " << tempKwl << std::endl;
            }
         }
         return proj;
      }
   }
   return 0;
}
rspfProjection* rspfGdalProjectionFactory::createProjection(const rspfString &name) const
{
   rspfString tempName = name;
   if(traceDebug())
   {
       rspfNotify(rspfNotifyLevel_WARN) << "rspfGdalProjectionFactory::createProjection: "<< name << "\n";
   }
   tempName = tempName.trim();
   rspfString testName(tempName.begin(),
                       tempName.begin()+6);
   testName = testName.upcase();
   if((testName == "PROJCS")||
      (testName == "GEOGCS"))
   {
      rspfKeywordlist kwl;
      if ( theWktTranslator.toOssimKwl(name.c_str(), kwl, "") )
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_DEBUG)<< "rspfGdalProjectionFactory::createProjection Debug: trying to create projection \n " << kwl << std::endl;
         }
         return rspfProjectionFactoryRegistry::instance()->createProjection(kwl,"");
      }
   }
   return 0;
}
rspfObject* rspfGdalProjectionFactory::createObject(const rspfString& typeName)const
{
   return createProjection(typeName);
}
rspfObject* rspfGdalProjectionFactory::createObject(const rspfKeywordlist& kwl,
                                                     const char* prefix)const
{
   return createProjection(kwl, prefix);
}
void rspfGdalProjectionFactory::getTypeNameList(std::vector<rspfString>& /* typeList */)const
{
}
std::list<rspfString> rspfGdalProjectionFactory::getList()const
{
   std::list<rspfString> result;
   
   return result;
}
