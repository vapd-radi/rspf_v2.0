#include <cstdio>
#include <cstdlib>
#include <rspf/projection/rspfNitfMapModel.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfTrace.h>
RTTI_DEF1(rspfNitfMapModel, "rspfNitfMapModel", rspfSensorModel);
static rspfTrace traceExec  ("rspfNitfMapModel:exec");
static rspfTrace traceDebug ("rspfNitfMapModel:debug");
static const char* PIAIMC_TAG  = "PIAIMC";
static const char* USE26A_TAG  = "USE26A";
static const char* USE00A_TAG  = "USE00A";
static const char* IMAGE_ID_KW = "image_id";
rspfNitfMapModel::rspfNitfMapModel()
   : rspfSensorModel()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::rspfNitfMapModel: entering..." << std::endl;
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::rspfNitfMapModel: returning..." << std::endl;
}
rspfNitfMapModel::rspfNitfMapModel(const rspfFilename& init_file)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::rspfNitfMapModel(init_file): entering..." << std::endl;
   FILE* fptr = fopen (init_file.chars(), "r");
   if (!fptr)
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfNitfMapModel::rspfNitfMapModel(init_file): Could not open NITF file at: <" << init_file << ">."
                                             << " Aborting with error..." << std::endl;
      }
      theErrorStatus = 1;
      return;
   }
   char* header = new char [6000];
   fread(header, 6000, 1, fptr);
   fclose(fptr);
   fptr = 0;
   if (strncmp(header, "NITF02.00", 9))
   {
      if (traceDebug())
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::rspfNitfMapModel(init_file):"
                                             << "\n\tFailed attempt to open <" << init_file
                                             << "> as NITF2.00."<<endl;
      }
      theErrorStatus = 1;
      delete [] header;
      return;
   }
   char charbuf[80];
   strncpy(charbuf, &header[447], 28);
   charbuf[28] = '\0';
   theImageID = charbuf;
   strncpy(charbuf, &header[737], 8);
   charbuf[8] = '\0';
   theImageSize.line = atoi(charbuf);
   strncpy(charbuf, &header[745], 8);
   charbuf[8] = '\0';
   theImageSize.samp = atoi(charbuf);
   
   rspfString coord;
   strncpy(charbuf, &header[775], 1);
   charbuf[1] = '\0';
   coord = charbuf;
   if (coord == "G")
   {
      double degreeLat;
      double degreeLon;
      double minute;
      double second;
      rspfString hemis;
      
      strncpy(charbuf, &header[780], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[778], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[776], 2);
      charbuf[2] = '\0';
      degreeLat = atof(charbuf);
      degreeLat = degreeLat + minute;
      strncpy(charbuf, &header[782], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "S")
         degreeLat = 0 - degreeLat;
      strncpy(charbuf, &header[788], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[786], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[783], 3);
      charbuf[3] = '\0';
      degreeLon = atof(charbuf);
      degreeLon = degreeLon + minute;
      strncpy(charbuf, &header[790], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "W")
         degreeLon = 0 - degreeLon;
      rspfDpt ip0 (degreeLon, degreeLat);
      strncpy(charbuf, &header[795], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[793], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[791], 2);
      charbuf[2] = '\0';
      degreeLat = atof(charbuf);
      degreeLat = degreeLat + minute;
      strncpy(charbuf, &header[797], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "S")
         degreeLat = 0 - degreeLat;
      strncpy(charbuf, &header[803], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[801], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[798], 3);
      charbuf[3] = '\0';
      degreeLon = atof(charbuf);
      degreeLon = degreeLon + minute;
      strncpy(charbuf, &header[805], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "W")
         degreeLon = 0 - degreeLon;
      rspfDpt ip1 (degreeLon, degreeLat);
      strncpy(charbuf, &header[810], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[808], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[806], 2);
      charbuf[2] = '\0';
      degreeLat = atof(charbuf);
      degreeLat = degreeLat + minute;
      strncpy(charbuf, &header[812], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "S")
         degreeLat = 0 - degreeLat;
      strncpy(charbuf, &header[818], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[816], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[813], 3);
      charbuf[3] = '\0';
      degreeLon = atof(charbuf);
      degreeLon = degreeLon + minute;
      strncpy(charbuf, &header[820], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "W")
         degreeLon = 0 - degreeLon;
      rspfDpt ip2 (degreeLon, degreeLat);
      strncpy(charbuf, &header[825], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[823], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[821], 2);
      charbuf[2] = '\0';
      degreeLat = atof(charbuf);
      degreeLat = degreeLat + minute;
      strncpy(charbuf, &header[827], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "S")
         degreeLat = 0 - degreeLat;
      strncpy(charbuf, &header[833], 2);
      charbuf[2] = '\0';
      second = atof(charbuf);
      second = second / 60;
      strncpy(charbuf, &header[831], 2);
      charbuf[2] = '\0';
      minute = atof(charbuf);
      minute = (minute + second) / 60;
      strncpy(charbuf, &header[828], 3);
      charbuf[3] = '\0';
      degreeLon = atof(charbuf);
      degreeLon = degreeLon + minute;
      strncpy(charbuf, &header[835], 1);
      charbuf[1] = '\0';
      hemis = charbuf;
      if (hemis == "W")
         degreeLon = 0 - degreeLon;
      rspfDpt ip3 (degreeLon, degreeLat);
      
      theBoundGndPolygon
         = rspfPolygon (ip0, ip1, ip2, ip3);
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfNitfMapModel::rspfNitfMapModel(init_file): Could not find the corner coordinates!! "
                                            << " Aborting with error..." << std::endl;
      }
      theErrorStatus = 1;
      delete [] header;
      return;
   }
   
   
   const char* tag_ptr = strstr(header, PIAIMC_TAG);
   if (tag_ptr)
   {
      strncpy(charbuf, &tag_ptr[348], 7);
      charbuf[7] = '\0';
      theGSD.line = atof(charbuf)/12.0 * MTRS_PER_FT;
      theGSD.samp = theGSD.line;
   }
   else
   {
      if(traceDebug())
      {
         rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfNitfMapModel::rspfNitfMapModel(init_file): Could not find the <" << PIAIMC_TAG
                                            << "> tag in the NITF file at: <" << init_file << "> to extract GSD."
                                            << " Searching for alternate <" << USE26A_TAG << ">... "<< std::endl;
      }
      
      const char* tag_ptr = strstr(header, USE26A_TAG);
      if (tag_ptr)
      {
         strncpy(charbuf, &tag_ptr[15], 5);
         charbuf[6] = '\0';
         theGSD.line = atof(charbuf)/12.0 * MTRS_PER_FT;
         theGSD.samp = theGSD.line;
      }
      else
      {
         if(traceDebug())
         {
            rspfNotify(rspfNotifyLevel_WARN) << "WARNING rspfNitfMapModel::rspfNitfMapModel(init_file): Could not find the <" << USE26A_TAG
                                               << "> tag in the NITF file at: <" << init_file << "> to extract GSD."
                                               << " Searching for alternate <" << USE00A_TAG << ">... "<< std::endl;
         }
         const char* tag_ptr = strstr(header, USE00A_TAG);
         if (!tag_ptr)
         {
            if(traceDebug())
            {
               rspfNotify(rspfNotifyLevel_DEBUG) << "WARNING rspfNitfMapModel::rspfNitfMapModel(init_file): Could not find the <" << USE00A_TAG
                                                   << "> tag in the NITF file at: <"
                                                   << init_file << ">." << " Aborting with error..." << std::endl;
            }
            theErrorStatus = 1;
            delete [] header;
            return;
         }
         strncpy(charbuf, &tag_ptr[15], 5);
         charbuf[6] = '\0';
         theGSD.line = atof(charbuf)/12.0 * MTRS_PER_FT;
         theGSD.samp = theGSD.line;
      }
   }
   rspfString drivePart;
   rspfString pathPart;
   rspfString filePart;
   rspfString extPart;
   init_file.split(drivePart,
                  pathPart,
                  filePart,
                  extPart);
   rspfFilename geom_filename;
   geom_filename.merge(drivePart,
                       pathPart,
                       filePart,
                       "geom");
   rspfKeywordlist geom_kwl;
   saveState(geom_kwl);
   geom_kwl.write(geom_filename.chars());
   delete [] header;
   if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::rspfNitfMapModel: Exited..." << std::endl;
}
rspfNitfMapModel::rspfNitfMapModel(const rspfKeywordlist& geom_kwl)
   : rspfSensorModel()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::rspfNitfMapModel(geom_file): entering..." << std::endl;
   loadState(geom_kwl);
   if (traceExec()) rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::rspfNitfMapModel(geom_file): Exited..." << std::endl;
}
rspfNitfMapModel::~rspfNitfMapModel()
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::~rspfNitfMapModel: entering..." << std::endl;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::~rspfNitfMapModel: returning..." << std::endl;
}
void rspfNitfMapModel::lineSampleHeightToWorld(const rspfDpt& /* image_point */,
                                                const double&   /* height */,
                                                rspfGpt&       /* gpt */) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::lineSampleHeightToWorld: entering..." << std::endl;
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::lineSampleHeightToWorld: exiting..." << std::endl;
}
std::ostream& rspfNitfMapModel::print(std::ostream& out) const
{
   out << "\nDump of rspfNitfMapModel object at " << hex << this << ":"
       << endl;
   return rspfSensorModel::print(out);
}
bool rspfNitfMapModel::saveState(rspfKeywordlist& kwl,
                              const char* prefix) const
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::saveState: entering..." << std::endl;
   rspfSensorModel::saveState(kwl, prefix);
      kwl.add(prefix, 
              IMAGE_ID_KW,
              theImageID.chars());
      kwl.add(prefix,
              rspfKeywordNames::NUMBER_LINES_KW,
              theImageSize.line,
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::NUMBER_SAMPLES_KW,
              theImageSize.samp,
              true);
      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_Y_KW,
              theGSD.line,
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::METERS_PER_PIXEL_X_KW,
              theGSD.samp,
              true);
      kwl.add(prefix,
              rspfKeywordNames::UL_Y_KW,
              0,
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::UL_X_KW,
              0,
              true);
      kwl.add(prefix,
              rspfKeywordNames::LR_Y_KW,
              theImageSize.line - 1,
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::LR_X_KW,
              theImageSize.samp - 1,
              true);
      rspfDpt ulcorner;
      if(!theBoundGndPolygon.vertex(0, ulcorner))
      {
         ulcorner = rspfDpt(0,0);
      }
      
      kwl.add(prefix,
              rspfKeywordNames::UL_LAT_KW,
              ulcorner.lat,
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::UL_LON_KW,
              ulcorner.lon,
              true);
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_LAT_KW,
              ulcorner.lat,
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::TIE_POINT_LON_KW,
              ulcorner.lon,
              true);
      rspfDpt corner;
      if(!theBoundGndPolygon.nextVertex(corner))
      {
         corner = rspfDpt(0,0);
      }
      kwl.add(prefix,
              rspfKeywordNames::UR_LAT_KW,
              corner.lat,
              true);
      
      kwl.add(prefix,
              rspfKeywordNames::UR_LON_KW,
              corner.lon,
              true);
      rspfDpt lrcorner;
      if(!theBoundGndPolygon.nextVertex(lrcorner))
      {
         lrcorner = rspfDpt(0,0);
      }
      kwl.add(prefix,
              rspfKeywordNames::LR_LAT_KW,
              lrcorner.lat,
              true);
      kwl.add(prefix,
              rspfKeywordNames::LR_LON_KW,
              lrcorner.lon,
              true);
      
      if(!theBoundGndPolygon.nextVertex(corner))
      {
         corner = rspfDpt(0,0);
      }
      kwl.add(prefix,
              rspfKeywordNames::LL_LAT_KW,
              corner.lat,
              true);
      kwl.add(prefix,
              rspfKeywordNames::LL_LON_KW,
              corner.lon,
              true);
      kwl.add(prefix,
              rspfKeywordNames::ORIGIN_LATITUDE_KW,
              (ulcorner.lat + lrcorner.lat)/2,
              true);
      kwl.add(prefix,
              rspfKeywordNames::CENTRAL_MERIDIAN_KW,
              (lrcorner.lon + ulcorner.lon)/2,
              true);
      kwl.add(prefix,
              rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LAT,
              fabs((ulcorner.lat - lrcorner.lat))/(theImageSize.line - 1),
              true);
      kwl.add(prefix,
              rspfKeywordNames::DECIMAL_DEGREES_PER_PIXEL_LON,
              fabs((lrcorner.lon - ulcorner.lon))/(theImageSize.samp - 1),
              true);
      kwl.add(prefix, rspfKeywordNames::TYPE_KW, "rspfEquDistCylProjection");
      if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::saveState: returning..." << std::endl;
      return true;
}
bool rspfNitfMapModel::loadState(const rspfKeywordlist& kwl,
                                  const char* prefix) 
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::loadState: entering..." << std::endl;
   if (traceDebug())
   {
      rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::loadState:"
                                          << "\nInput kwl:  " << kwl
                                          << std::endl;
   }
   const char* value = 0;
   const char* keyword = 0;
   bool success = false;
   value = kwl.find(prefix, rspfKeywordNames::TYPE_KW);
   if (!value || (strcmp(value, TYPE_NAME(this))))
      goto loadStateError;
       
   success = rspfSensorModel::loadState(kwl, prefix);
   if (!success) goto loadStateError;
 
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::loadState: returning..." << std::endl;
   return true;
 
 loadStateError:
   theErrorStatus++;
   rspfNotify(rspfNotifyLevel_FATAL) << "FATAL rspfNitfMapModel::loadState: encountered parsing the following required keyword: "
                                       << "<" << keyword << ">. Check the keywordlist for proper syntax."
                                       << std::endl;
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::loadState: returning with error..." << std::endl;
   return false;   
}
void rspfNitfMapModel::writeGeomTemplate(ostream& os)
{
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::writeGeomTemplate: entering..." << std::endl;
   os <<
      "//**************************************************************\n"
      "// Template for LandSat model keywordlist\n"
      "//**************************************************************\n"
      << rspfKeywordNames::TYPE_KW << ": " << "rspfNitfMapModel" << endl;
   rspfSensorModel::writeGeomTemplate(os);
   
   if (traceExec())  rspfNotify(rspfNotifyLevel_DEBUG) << "DEBUG rspfNitfMapModel::writeGeomTemplate: returning..." << std::endl;
   return;
}
