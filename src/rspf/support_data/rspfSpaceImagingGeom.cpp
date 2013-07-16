//*******************************************************************
// License:  See top level LICENSE.txt file.
//
// Author: Garrett Potts
// 
// Description: This class extends the stl's string class.
//
//********************************************************************
//  $Id: rspfSpaceImagingGeom.cpp 17206 2010-04-25 23:20:40Z dburken $
#include <rspf/support_data/rspfSpaceImagingGeom.h>
#include <rspf/base/rspfKeywordNames.h>
#include <rspf/base/rspfNotifyContext.h>
#include <sstream>
#include <iostream>
using namespace std;

const char* rspfSpaceImagingGeom::SIG_PRODUCER_KW = "Producer";
const char* rspfSpaceImagingGeom::SIG_PROJECT_NAME_KW = "Project Name";
const char* rspfSpaceImagingGeom::SIG_FILENAME_KW = "Filename";
const char* rspfSpaceImagingGeom::SIG_BAND_KW = "Band";
const char* rspfSpaceImagingGeom::SIG_BITS_PER_PIXEL_KW = "Bits/Pixel";
const char* rspfSpaceImagingGeom::SIG_NUMBER_OF_BANDS_KW = "Number of Bands";
const char* rspfSpaceImagingGeom::SIG_DATUM_KW = "Datum";
const char* rspfSpaceImagingGeom::SIG_PROJECTION_KW = "Projection";
const char* rspfSpaceImagingGeom::SIG_SELECTED_PROJECTION_KW = "Selected Projection";
const char* rspfSpaceImagingGeom::SIG_ZONE_KW = "Zone";
const char* rspfSpaceImagingGeom::SIG_UL_EASTING_KW = "UL Map X (Easting)";
const char* rspfSpaceImagingGeom::SIG_UL_NORTHING_KW = "UL Map Y (Northing)";
const char* rspfSpaceImagingGeom::SIG_PIXEL_SIZE_X_KW = "Pixel Size X";
const char* rspfSpaceImagingGeom::SIG_PIXEL_SIZE_Y_KW = "Pixel Size Y";
const char* rspfSpaceImagingGeom::SIG_COLUMNS_KW = "Columns";
const char* rspfSpaceImagingGeom::SIG_ROWS_KW = "Rows";

rspfSpaceImagingGeom::rspfSpaceImagingGeom()
{
}

rspfSpaceImagingGeom::rspfSpaceImagingGeom(const rspfFilename& file,
                                             const char* prefix)
{
   setGeometry(file);
   thePrefix = prefix;
}

void rspfSpaceImagingGeom::setGeometry(const rspfFilename& file)
{
   theSpaceImagingGeometry.clear();

   theSpaceImagingGeometry.addFile(file.c_str());
}

void rspfSpaceImagingGeom::setGeometry(const rspfKeywordlist& kwl)
{
   theSpaceImagingGeometry.clear();

   theSpaceImagingGeometry = kwl;
}

void rspfSpaceImagingGeom::exportToOssim(rspfKeywordlist& kwl,
                                          const char* prefix)const
{
   const rspfKeywordlist::KeywordMap& geomMap = theSpaceImagingGeometry.getMap();
   rspfKeywordlist::KeywordMap::const_iterator node = geomMap.begin();
   
   while(node != geomMap.end())
   {
      rspfString keyword = spaceImagingToOssimKeyword((*node).first);
      rspfString value   = spaceImagingToOssimValue((*node).first,
                                                     (*node).second);
      if(keyword != "")
      {
         kwl.add(prefix,
                 keyword.c_str(),
                 value.c_str(),
                 true);
      }
      ++node;
   }
}

rspfString rspfSpaceImagingGeom::spaceImagingToOssimKeyword(const rspfString& spaceImagingKeyword)const
{
   if(spaceImagingKeyword == SIG_NUMBER_OF_BANDS_KW)
   {
      return rspfString (rspfKeywordNames::NUMBER_BANDS_KW);
   }
   else if(spaceImagingKeyword == SIG_DATUM_KW)
   {
      return rspfString(rspfKeywordNames::DATUM_KW);
   }
   else if((spaceImagingKeyword == SIG_PROJECTION_KW)||
		   (spaceImagingKeyword == SIG_SELECTED_PROJECTION_KW))
   {
      return rspfString(rspfKeywordNames::TYPE_KW);
   }
   else if(spaceImagingKeyword == SIG_ZONE_KW)
   {
      return rspfString(rspfKeywordNames::ZONE_KW);
   }
   else if(spaceImagingKeyword == SIG_UL_EASTING_KW)
   {
      return rspfString(rspfKeywordNames::TIE_POINT_EASTING_KW);
   }
   else if(spaceImagingKeyword == SIG_UL_NORTHING_KW)
   {
      return rspfString(rspfKeywordNames::TIE_POINT_NORTHING_KW);
   }
   else if(spaceImagingKeyword == SIG_PIXEL_SIZE_X_KW)
   {
      return rspfString(rspfKeywordNames::METERS_PER_PIXEL_X_KW);
   }
   else if(spaceImagingKeyword == SIG_PIXEL_SIZE_Y_KW)
   {
      return rspfString(rspfKeywordNames::METERS_PER_PIXEL_Y_KW);
   }
   else if(spaceImagingKeyword == SIG_COLUMNS_KW)
   {
      return rspfString(rspfKeywordNames::NUMBER_SAMPLES_KW);
   }
   else if(spaceImagingKeyword == SIG_ROWS_KW)
   {
      return rspfString(rspfKeywordNames::NUMBER_LINES_KW);
   }
   
   return rspfString("");
}

rspfString rspfSpaceImagingGeom::spaceImagingToOssimValue(
   const rspfString& spaceImagingKeyword,
   const rspfString& /* spaceImagingValue */)const
{
   const char* value = theSpaceImagingGeometry.find(thePrefix.c_str(),
                                                    spaceImagingKeyword.c_str());

   if(!value) return rspfString("");
   
   if(spaceImagingKeyword == SIG_DATUM_KW)
   {
      // better to have a lut for this.  For now I'll put it
      // here.  As the list gets larger let's move it to a
      // lut then.
      rspfString datum = value;
      datum = datum.trim().upcase();
      
      if( datum == "WGS84")
      {
         return rspfString("WGE");
      }
   }
   else if((spaceImagingKeyword == SIG_PROJECTION_KW)||
		   (spaceImagingKeyword == SIG_SELECTED_PROJECTION_KW))
   {
      rspfString proj = value;
      proj = proj.trim().upcase();
      
      if(proj == "UNIVERSAL TRANSVERSE MERCATOR")
      {
         return rspfString("rspfUtmProjection");
      }
      else
      {
		  rspfNotify(rspfNotifyLevel_WARN)  << "WARNING rspfSpaceImagingGeom::spaceImagingToOssimValue: Projection " << value << " doesn't have a converter yet!" << endl
		  << "Please add it" << endl;
      }
   }
   else if(spaceImagingKeyword == SIG_UL_EASTING_KW)
   {
      rspfString easting = value;
      easting = easting.trim();

      ostringstream os;
      os << easting;

      rspfString eastingValue;
      rspfString eastingUnits;

      istringstream is(os.str());
      is >> eastingValue >> eastingUnits;

      eastingUnits = eastingUnits.trim().upcase();
      if(eastingUnits!= "METERS")
      {
		  rspfNotify(rspfNotifyLevel_WARN)  << "WARNING rspfSpaceImagingGeom::spaceImagingToOssimValue: units " << eastingUnits << " not supported yet!" << endl;
      }

      return eastingValue;
   }
   else if(spaceImagingKeyword == SIG_UL_NORTHING_KW)
   {
      rspfString northing = value;
      northing = northing.trim();

      ostringstream os;
      os << northing;

      rspfString northingValue;
      rspfString northingUnits;

      istringstream is(os.str());
      is >> northingValue >> northingUnits;

      northingUnits = northingUnits.trim().upcase();
      if(northingUnits!= "METERS")
      {
		  rspfNotify(rspfNotifyLevel_WARN)  << "WARNING rspfSpaceImagingGeom::spaceImagingToOssimValue: units " << northingUnits << " not supported yet!" << endl;
      }
      
      return northingValue;
   }
   else if(spaceImagingKeyword == SIG_PIXEL_SIZE_X_KW)
   {
      rspfString gsd = value;
      gsd = gsd.trim();

      ostringstream os;
      os << gsd;

      rspfString gsdValue;
      rspfString gsdUnits;

      istringstream is(os.str());
      is >> gsdValue >> gsdUnits;

      gsdUnits = gsdUnits.trim().upcase();
      if(gsdUnits!= "METERS")
      {
		  rspfNotify(rspfNotifyLevel_WARN)  << "WARNING rspfSpaceImagingGeom::spaceImagingToOssimValue: units " << gsdUnits << " not supported yet!" << endl;
      }
      
      return gsdValue;
   }
   else if(spaceImagingKeyword == SIG_PIXEL_SIZE_Y_KW)
   {
      rspfString gsd = value;
      gsd = gsd.trim();

      ostringstream os;
      os << gsd;

      rspfString gsdValue;
      rspfString gsdUnits;

      istringstream is(os.str());
      is >> gsdValue >> gsdUnits;

      gsdUnits = gsdUnits.trim().upcase();
      if(gsdUnits!= "METERS")
      {
		  rspfNotify(rspfNotifyLevel_WARN)  << "WARNING rspfSpaceImagingGeom::spaceImagingToOssimValue: units " << gsdUnits << " not supported yet!" << endl;
      }
      
      return gsdValue;
   }
   
   return rspfString(value);
}

rspfString rspfSpaceImagingGeom::getBandName()const
{
   return theSpaceImagingGeometry.find(SIG_BAND_KW);
}

rspfFilename rspfSpaceImagingGeom::getFilename()const
{
   return theSpaceImagingGeometry.find(SIG_FILENAME_KW);
}

rspfString rspfSpaceImagingGeom::getProducer()const
{
   return theSpaceImagingGeometry.find(SIG_PRODUCER_KW);
}
