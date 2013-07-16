//*******************************************************************
//
// LGPL
// 
// Author:  Garrett Potts
//
//*******************************************************************
//  $Id: rspfApplanixEOFile.cpp 20483 2012-01-21 15:42:22Z dburken $

#include <rspf/support_data/rspfApplanixEOFile.h>
#include <rspf/base/rspfKeywordlist.h>
#include <rspf/base/rspfRegExp.h>
#include <rspf/base/rspfTrace.h>
#include <rspf/base/rspfCommon.h>
#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

static rspfTrace traceDebug("rspfApplanixEOFile:debug");

static std::istream& applanix_skipws(std::istream& in)
{
   int c = in.peek();
   while(((c == ' ') ||
          (c == '\r') ||
          (c == '\t') ||
          (c == '\n'))&&
         (in.good()))
   {
      in.ignore(1);
      c = in.peek();
   }
   
   return in;
}

std::ostream& operator <<(std::ostream& out, const rspfApplanixEORecord& src)
{
   if(src.theField.size())
   {
      std::copy(src.theField.begin(),
                src.theField.end(),
                std::ostream_iterator<rspfString>(out, " "));
   }

   return out;
}

std::ostream& operator <<(std::ostream& out, const rspfApplanixEOFile& src)
{
   rspf_uint32 idx = 0;
   out << src.theHeader << std::endl << std::endl;

   if(src.theRecordFormat.size())
   {
      std::copy(src.theRecordFormat.begin(),
                src.theRecordFormat.end()-1,
                std::ostream_iterator<rspfString>(out, ", "));
      out << *(src.theRecordFormat.end()-1);
      out << std::endl;
   }

   out << "Kappa Cardinal Rotation = " << src.theKappaCardinal << std::endl;
   out << "Bore Sight tx = " << src.theBoreSightTx << std::endl
       << "Bore Sight ty = " << src.theBoreSightTy << std::endl
       << "Bore Sight tz = " << src.theBoreSightTz << std::endl;
   
   out << "Lever arm lx = " << src.theLeverArmLx << std::endl
       << "Lever arm ly = " << src.theLeverArmLy << std::endl
       << "Lever arm lz = " << src.theLeverArmLz << std::endl;
   
   out << "Shift Value X = " << src.theShiftValuesX << std::endl
       << "Shift Value Y = " << src.theShiftValuesY << std::endl
       << "Shift Value Z = " << src.theShiftValuesZ << std::endl;
   
   out << "Mapping Frame Datum      = " << src.theMappingFrameDatum      << std::endl
       << "Mapping Frame Projection = " << src.theMappingFrameProjection << std::endl
       << "Central Meridian         = " << src.theCentralMeridian << std::endl
       << "Origin of Latitude       = " << src.theOriginLatitude  << std::endl
       << "Grid Scale Factor        = " << src.theGridScaleFactor << std::endl
       << "False easting            = " << src.theFalseEasting    << std::endl
       << "False northing           = " << src.theFalseNorthing   << std::endl;

   out << "theUtmZone               = " << src.theUtmZone << std::endl;
   out << "theUtmHemisphere         = " << src.theUtmHemisphere << std::endl;
   
   if(src.theApplanixRecordList.size())
   {
      out << std::endl;
      for(idx = 0; idx < src.theApplanixRecordList.size()-1; ++idx)
      {
         out << *(src.theApplanixRecordList[idx]) << std::endl;
      }
      out << *(src.theApplanixRecordList[idx]);
      
   }

   return out;
}

rspfApplanixEOFile::rspfApplanixEOFile()
{
   theMinLat = rspf::nan();
   theMinLon = rspf::nan();
   theMaxLat = rspf::nan();
   theMaxLon = rspf::nan();
}

bool rspfApplanixEOFile::parseFile(const rspfFilename& file)
{
   std::ifstream in(file.c_str());

   return parseStream(in);
}

bool rspfApplanixEOFile::isEOFile(const rspfFilename& file)const
{
   std::ifstream in(file.c_str());

   return isEOFile(in);
}

bool rspfApplanixEOFile::isEOFile(std::istream& in)const
{
   rspfString header;

   return parseHeader(in, header);
}

bool rspfApplanixEOFile::parseStream(std::istream& in)
{
   theRecordIdMap.clear();
   rspfString line;
   int c = '\0';
   if(!parseHeader(in, theHeader))
   {
      return false;
   }

   // now parse parameters
   in>>applanix_skipws;
   line = "";
   while(in.good()&&
         !line.contains("RECORD FORMAT"))
   {
      std::getline(in, line.string());
      line = line.upcase();
      line = line.substitute("\r","\n", true);
      if(line.contains("KAPPA CARDINAL"))
      {
         theKappaCardinal = line;
         theKappaCardinal = theKappaCardinal.substitute("KAPPA CARDINAL ROTATION","");
         theKappaCardinal = theKappaCardinal.substitute(":","");
         theKappaCardinal = theKappaCardinal.substitute("\n","");
      }
      else if(line.contains("LEVER ARM"))
      {
         rspfKeywordlist kwl('=');
         line = line.substitute("LEVER ARM VALUES:",
                                "");
         line = line.substitute(",",
                                "\n",
                                true);
         std::istringstream in(line);

         kwl.parseStream(in);

         theLeverArmLx = kwl.find("LX");
         theLeverArmLy = kwl.find("LY");
         theLeverArmLz = kwl.find("LZ");
      }
      else if(line.contains("BORESIGHT VALUES"))
      {
         rspfKeywordlist kwl('=');
         line = line.substitute("BORESIGHT VALUES:",
                                "");
         line = line.substitute(",",
                                "\n",
                                true);

         std::istringstream in(line);

         kwl.parseStream(in);

         
         theBoreSightTx = kwl.find("TX");
         theBoreSightTy = kwl.find("TY");
         theBoreSightTz = kwl.find("TZ");
      }
      else if(line.contains("SHIFT VALUES:"))
      {
         rspfKeywordlist kwl('=');
         line = line.substitute("SHIFT VALUES:","");
         line = line.substitute(",",
                                "\n",
                                true);

         std::istringstream in(line);
         kwl.parseStream(in);

         
         theShiftValuesX = kwl.find("X");
         theShiftValuesY = kwl.find("Y");
         theShiftValuesZ = kwl.find("Z");
         
      }
      else if(line.contains("GRID:"))
      { 
         rspfKeywordlist kwl(':');
         line = line.substitute(";",
                                "\n",
                                true);
         std::istringstream in(line);
         kwl.parseStream(in);
         theUtmZone = kwl.find("ZONE");

         if(theUtmZone.contains("NORTH"))
         {
            theUtmHemisphere = "North";
         }
         else
         {
            theUtmHemisphere = "South";
         }
          theUtmZone = theUtmZone.replaceAllThatMatch("UTM|\\(.*\\)|NORTH|SOUTH","");
          theUtmZone = theUtmZone.trim();
      }
      else if(line.contains("FRAME DATUM"))
      {
         rspfKeywordlist kwl(':');
         line = line.substitute(";",
                                "\n",
                                true);
         std::istringstream in(line);
         kwl.parseStream(in);

         theMappingFrameDatum = kwl.find("MAPPING FRAME DATUM");
         theMappingFrameProjection = kwl.find("MAPPING FRAME PROJECTION");
         theMappingFrameDatum = theMappingFrameDatum.trim();
         theMappingFrameProjection = theMappingFrameProjection.trim();
      }
      else if(line.contains("POSPROC SBET"))
      {
         theSbetField = line.after(":");
         theSbetField = theSbetField.trim();
      }
      else if(line.contains("CENTRAL MERIDIAN"))
      {
         theCentralMeridian = line;
         theCentralMeridian = theCentralMeridian.substitute("CENTRAL MERIDIAN","");
         theCentralMeridian = theCentralMeridian.substitute("=","");
         theCentralMeridian = theCentralMeridian.substitute("DEG","");
         theCentralMeridian = theCentralMeridian.substitute(";","");
      }
      else if(line.contains("LATITUDE OF THE GRID ORIGIN"))
      {
         rspfKeywordlist kwl('=');
         line = line.substitute(";",
                                "\n",
                                true);
         std::istringstream in(line);
         kwl.parseStream(in);

         theOriginLatitude  = kwl.find("LATITUDE OF THE GRID ORIGIN");
         theGridScaleFactor = kwl.find("GRID SCALE FACTOR");
      }
      else if(line.contains("FALSE EASTING"))
      {
         rspfKeywordlist kwl('=');
         line = line.substitute(";",
                                "\n",
                                true);
         std::istringstream in(line);
         kwl.parseStream(in);

         theFalseEasting  = kwl.find("FALSE EASTING");
         theFalseNorthing = kwl.find("FALSE NORTHING");
      }
   }

   in>>applanix_skipws;

   c = in.get();

   std::vector<rspfString> fieldArray;
   rspfString field;

   while(in.good()&&
         (c!='\n')&&
         (c!='\r'))
   {
      field = "";
      while((c != ',')&&
            (c != '\n')&&
            (c != '\r'))
      {
         field += (char)c;
         c = in.get();
      }
      if((c!='\n')&&
         (c!='\r'))
      {
         c = in.get();
      }
      field = field.trim();
      if(field != "")
      {
         theRecordFormat.push_back(field);
      }
   }
   in>>applanix_skipws;

   if(in.peek() == '(')
   {
      std::getline(in, line.string());
   }
   in>>applanix_skipws;
   rspfRefPtr<rspfApplanixEORecord> record = new rspfApplanixEORecord((rspf_uint32)theRecordFormat.size());
   rspf_int32 latIdx = getFieldIdx("LAT");
   rspf_int32 lonIdx = getFieldIdx("LONG");;
   bool hasLatLon = (latIdx >=0)&&(lonIdx >= 0);

   
   if(hasLatLon)
   {
      theMinLat = 90.0;
      theMaxLat = -90.0;
      theMinLon = 180.0;
      theMaxLon = -180.0;
   }
   else
   {
      theMinLat = rspf::nan();
      theMaxLat = rspf::nan();
      theMinLon = rspf::nan();
      theMaxLon = rspf::nan();
   }
   
   while(in.good()&&theRecordFormat.size())
   {
      std::getline(in, line.string());
      line = line.trim();
      if(line != "")
      {
         std::istringstream inStr(line);
         rspf_uint32 idx;
         rspfString value;

         for(idx = 0; idx < theRecordFormat.size(); ++idx)
         {
            inStr >> (*record)[idx];
         }
         if(hasLatLon)
         {
            double lat = (*record)[latIdx].toDouble();
            double lon = (*record)[lonIdx].toDouble();

            if(lat<theMinLat) theMinLat = lat;
            if(lat>theMaxLat) theMaxLat = lat;
            if(lon<theMinLon) theMinLon = lon;
            if(lon>theMaxLon) theMaxLon = lon;
            
         }
         theApplanixRecordList.push_back(new rspfApplanixEORecord(*record));
      }
   }
   indexRecordIds();

   if(traceDebug())
   {
      if(hasLatLon)
      {
         rspfNotify(rspfNotifyLevel_DEBUG) << "min lat: " << theMinLat << std::endl
                                             << "min lon: " << theMinLon << std::endl
                                             << "max lat: " << theMaxLat << std::endl
                                             << "max lon: " << theMaxLon << std::endl;
      }
   }
   return true;
}

bool rspfApplanixEOFile::parseHeader(const rspfFilename& file,
                                      rspfString& header)const
{
   std::ifstream in(file.c_str());

   return parseHeader(in, header);
}

bool rspfApplanixEOFile::parseHeader(std::istream& in,
                                      rspfString& header)const
{
   header = "";
   in >> applanix_skipws;

   int c = in.peek();
   
   if((c!='*')||
      (!in.good()))
   {
      return false;
   }
   rspfString line;

   std::getline(in, line.string());
   line = line .substitute("\r",
                           "\n");
   if(!line.contains("************"))
   {
      return false;
   }

   header = line;
   header += "\n";

   in>>applanix_skipws;
   std::getline(in, line.string());
   line = line .substitute("\r",
                           "\n");
   
   while((in.good())&&
         (!line.contains("***********")))
   {
      header += line;
      header += "\n";
      std::getline(in, line.string());
      line = line .substitute("\r",
                              "\n");
   }
   header += line;
   header += "\n";

   if(!header.contains("Applanix"))
   {
      return false;
   }
   
   return true;
}

void rspfApplanixEOFile::indexRecordIds()
{
   rspfString id;
   rspf_uint32 idx;
   rspf_int32 idIdx = getFieldIdxLike("ID");
   rspf_uint32 size = (rspf_uint32)theApplanixRecordList.size();
   theRecordIdMap.clear();

   if(idIdx < 0) return;

   for(idx = 0; idx < size; ++idx)
   {
      id = (*theApplanixRecordList[idx])[idIdx];
      theRecordIdMap.insert(std::make_pair(id, theApplanixRecordList[idx]));
   }
}

rspf_int32 rspfApplanixEOFile::getFieldIdxLike(const rspfString& searchKey)const
{
   rspfString key = searchKey;
   rspf_uint32 idx = 0;
   key = key.upcase();

   for(idx = 0; idx < theRecordFormat.size(); ++idx)
   {
      if(rspfString::upcase(theRecordFormat[idx]).contains(key))
      {
         return (int)idx;
      }
   }

   return -1;
}

rspf_int32 rspfApplanixEOFile::getFieldIdx(const rspfString& searchKey)const
{
   rspfString key = searchKey;
   rspf_uint32 idx = 0;
   
   key = key.upcase();

   for(idx = 0; idx < theRecordFormat.size(); ++idx)
   {
      if(rspfString::upcase(theRecordFormat[idx]) == key)
      {
         return (int)idx;
      }
   }
   
   return -1;
}


const rspfRefPtr<rspfApplanixEORecord> rspfApplanixEOFile::getRecordGivenId(const rspfString& id)const
{

   if(theRecordIdMap.size())
   {
      std::map<rspfString, rspfRefPtr<rspfApplanixEORecord>,
         rspfStringLtstr >::const_iterator iter = theRecordIdMap.find(id);
      
      if(iter!=theRecordIdMap.end())
      {
         return iter->second;
      }
   }
   else
   {
      rspf_uint32 idx;
      rspf_int32 idxId = getFieldIdx("ID");
      
      if(idxId >= 0)
      {
         for(idx = 0; idx < theApplanixRecordList.size(); ++idx)
         {
            if((*theApplanixRecordList[idx])[idxId] == id)
            {
               return theApplanixRecordList[idx];
            }
         }
      }
   }
   
   return 0;
}

rspf_uint32 rspfApplanixEOFile::getNumberOfRecords()const
{
   return (rspf_uint32)theApplanixRecordList.size();
}

const rspfRefPtr<rspfApplanixEORecord> rspfApplanixEOFile::getRecord(rspf_uint32 idx)const
{
   if(idx < theApplanixRecordList.size())
   {
      return theApplanixRecordList[idx];
   }
   
   return 0;
}

bool rspfApplanixEOFile::isHeightAboveMSL()const
{
   return (getFieldIdx("ORTHOMETRIC HEIGHT") >= 0);
}

bool rspfApplanixEOFile::isEcefModel()const
{
   rspf_int32 xIdx = getFieldIdx("X");
   rspf_int32 yIdx = getFieldIdx("Y");
   rspf_int32 zIdx = getFieldIdx("Z");
   rspf_int32 rollIdx = getFieldIdx("ROLL");
   rspf_int32 pitchIdx = getFieldIdx("PITCH");
   rspf_int32 headingIdx = getFieldIdx("HEADING");

   return ((xIdx >=0)&&
           (yIdx >=0)&&
           (zIdx >=0)&&
           (rollIdx>=0)&&
           (pitchIdx>=0)&&
           (headingIdx>=0));
}


bool rspfApplanixEOFile::isUtmFrame()const
{
   return ((getFieldIdxLike("EASTING")>=0)&&
            (getFieldIdxLike("NORTHING") >=0)&&
            (getFieldIdxLike("HEIGHT") >= 0)&&
            (getFieldIdxLike("OMEGA") >=0)&&
            (getFieldIdxLike("PHI") >=0)&&
            (getFieldIdxLike("KAPPA") >=0));
            
   return (getUtmZone() >= 0.0);
}

rspf_int32 rspfApplanixEOFile::getUtmZone()const
{
   return theUtmZone.toInt32();
}

rspfString rspfApplanixEOFile::getUtmHemisphere()const
{
   return theUtmHemisphere;
}

rspfString rspfApplanixEOFile::getSbetField()const
{
   return theSbetField;
}

double rspfApplanixEOFile::getBoreSightTx()const
{
   return theBoreSightTx.toDouble();
}

double rspfApplanixEOFile::getBoreSightTy()const
{
   return theBoreSightTy.toDouble();
}

double rspfApplanixEOFile::getBoreSightTz()const
{
   return theBoreSightTz.toDouble();
}

double rspfApplanixEOFile::getLeverArmLx()const
{
   return theLeverArmLx.toDouble();
}

double rspfApplanixEOFile::getLeverArmLy()const
{
   return theLeverArmLy.toDouble();
}

double rspfApplanixEOFile::getLeverArmLz()const
{
   return theLeverArmLz.toDouble();
}

double rspfApplanixEOFile::getKardinalKappa()const
{
   return theKappaCardinal.toDouble();
}

double rspfApplanixEOFile::getShiftValuesX()const
{
   return theShiftValuesX.toDouble();
}

double rspfApplanixEOFile::getShiftValuesY()const
{
   return theShiftValuesY.toDouble();
}

double rspfApplanixEOFile::getShiftValuesZ()const
{
   return theShiftValuesZ.toDouble();
}

rspfString rspfApplanixEOFile::getMappingFrameDatum()const
{
   return theMappingFrameDatum;
}

rspfString rspfApplanixEOFile::getMappingFrameProjection()const
{
   return theMappingFrameProjection;
}

double rspfApplanixEOFile::getMappingFrameCentralMeridian()const
{
   return theCentralMeridian.toDouble();
}

double rspfApplanixEOFile::getMappingFrameOriginLatitude()const
{
   return theOriginLatitude.toDouble();
}

double rspfApplanixEOFile::getMappingFrameGridScaleFactor()const
{
   return theGridScaleFactor.toDouble();
}

double rspfApplanixEOFile::getMappingFrameFalseEasting()const
{
   return theFalseEasting.toDouble();
}

double rspfApplanixEOFile::getMappingFrameFalseNorthing()const
{
   return theFalseNorthing.toDouble();
}

rspfString rspfApplanixEOFile::convertToOssimDatumCode(const rspfString& datum)const
{
   rspfString result = "WGE";
   rspfString temp = datum;
   temp = temp.upcase();

   if(temp == "NAD83")
   {
      result = "NAR-C";
   }

   return result;
}
